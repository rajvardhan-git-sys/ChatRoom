#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include "chatRoom.hpp"
#include "html_content.hpp"
#include <cstdlib>

// =========================================================================
// Room
// =========================================================================

void Room::join(ParticipantPtr participant) {
    participants_.insert(participant);
    std::cout << "Client joined. Total: " << participants_.size() << std::endl;
}

void Room::leave(ParticipantPtr participant) {
    participants_.erase(participant);
    std::cout << "Client left. Total: " << participants_.size() << std::endl;
}

void Room::broadcast(const std::string& message, ParticipantPtr sender) {
    for (auto& p : participants_) {
        if (p != sender) {
            p->send(message);
        }
    }
}

// =========================================================================
// WebSocketSession
// =========================================================================

WebSocketSession::WebSocketSession(tcp::socket socket, Room& room)
    : ws_(std::move(socket)), room_(room) {}

void WebSocketSession::run(http::request<http::string_body> req) {
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res) {
            res.set(http::field::server, "ChatRoom");
        }));

    ws_.async_accept(req,
        [self = shared_from_this()](beast::error_code ec) {
            self->onAccept(ec);
        });
}

void WebSocketSession::send(const std::string& msg) {
    writeQueue_.push_back(msg);
    if (!writing_) {
        doWrite();
    }
}

void WebSocketSession::onAccept(beast::error_code ec) {
    if (ec) {
        std::cerr << "WebSocket accept error: " << ec.message() << std::endl;
        return;
    }
    room_.join(shared_from_this());
    doRead();
}

void WebSocketSession::doRead() {
    ws_.async_read(buffer_,
        [self = shared_from_this()](beast::error_code ec, std::size_t bytes) {
            self->onRead(ec, bytes);
        });
}

void WebSocketSession::onRead(beast::error_code ec, std::size_t) {
    if (ec) {
        if (ec != websocket::error::closed) {
            std::cerr << "WebSocket read error: " << ec.message() << std::endl;
        }
        room_.leave(shared_from_this());
        return;
    }

    std::string message = beast::buffers_to_string(buffer_.data());
    buffer_.consume(buffer_.size());

    std::cout << "Message: " << message << std::endl;
    room_.broadcast(message, shared_from_this());
    doRead();
}

void WebSocketSession::doWrite() {
    if (writeQueue_.empty()) {
        writing_ = false;
        return;
    }
    writing_ = true;
    ws_.text(true);
    ws_.async_write(net::buffer(writeQueue_.front()),
        [self = shared_from_this()](beast::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "WebSocket write error: " << ec.message() << std::endl;
                self->room_.leave(self);
                return;
            }
            self->writeQueue_.pop_front();
            self->doWrite();
        });
}

// =========================================================================
// HttpSession
// =========================================================================

HttpSession::HttpSession(tcp::socket socket, Room& room)
    : stream_(std::move(socket)), room_(room) {}

void HttpSession::run() {
    doRead();
}

void HttpSession::doRead() {
    req_ = {};
    stream_.expires_after(std::chrono::seconds(30));
    http::async_read(stream_, buffer_, req_,
        [self = shared_from_this()](beast::error_code ec, std::size_t) {
            if (ec) return;
            self->handleRequest();
        });
}

void HttpSession::handleRequest() {
    // WebSocket upgrade request
    if (websocket::is_upgrade(req_)) {
        auto ws = std::make_shared<WebSocketSession>(stream_.release_socket(), room_);
        ws->run(std::move(req_));
        return;
    }

    // Serve the HTML chat page
    http::response<http::string_body> res;

    if (req_.method() == http::verb::get &&
        (req_.target() == "/" || req_.target() == "/index.html")) {
        res.result(http::status::ok);
        res.set(http::field::content_type, "text/html; charset=utf-8");
        res.body() = getHtmlPage();
    } else {
        res.result(http::status::not_found);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Not Found";
    }

    res.version(req_.version());
    res.set(http::field::server, "ChatRoom");
    res.keep_alive(req_.keep_alive());
    res.prepare_payload();

    auto sp = std::make_shared<http::response<http::string_body>>(std::move(res));
    auto self = shared_from_this();
    http::async_write(stream_, *sp,
        [self, sp](beast::error_code ec, std::size_t) {
            if (!ec && sp->keep_alive()) {
                self->doRead();
            } else {
                beast::error_code shutdown_ec;
                self->stream_.socket().shutdown(tcp::socket::shutdown_send, shutdown_ec);
            }
        });
}

// =========================================================================
// Listener
// =========================================================================

Listener::Listener(net::io_context& ioc, tcp::endpoint endpoint, Room& room)
    : ioc_(ioc), acceptor_(ioc), room_(room) {
    beast::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) throw std::runtime_error("Open: " + ec.message());

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) throw std::runtime_error("Set option: " + ec.message());

    acceptor_.bind(endpoint, ec);
    if (ec) throw std::runtime_error("Bind: " + ec.message());

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) throw std::runtime_error("Listen: " + ec.message());
}

void Listener::run() {
    doAccept();
}

void Listener::doAccept() {
    acceptor_.async_accept(ioc_,
        [self = shared_from_this()](beast::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<HttpSession>(std::move(socket), self->room_)->run();
            } else {
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }
            self->doAccept();
        });
}

// =========================================================================
// Main
// =========================================================================

int main(int argc, char* argv[]) {
    try {
        const char* port_str = nullptr;
        if (argc >= 2) {
            port_str = argv[1];
        } else {
            port_str = std::getenv("PORT");
            if (!port_str) {
                std::cerr << "Usage: chatRoom <port> OR set PORT environment variable\n";
                return 1;
            }
        }

        auto port = static_cast<unsigned short>(std::atoi(port_str));

        net::io_context ioc;
        Room room;

        auto listener = std::make_shared<Listener>(
            ioc, tcp::endpoint(tcp::v4(), port), room);
        listener->run();

        std::cout << "ChatRoom server running on port " << port << std::endl;
        ioc.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}