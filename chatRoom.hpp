#ifndef CHATROOM_HPP
#define CHATROOM_HPP

#include <iostream>
#include <string>
#include <deque>
#include <set>
#include <memory>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Abstract base for any connected client
class Participant {
public:
    virtual void send(const std::string& msg) = 0;
    virtual ~Participant() = default;
};

using ParticipantPtr = std::shared_ptr<Participant>;

// Manages all connected clients and message broadcasting
class Room {
public:
    void join(ParticipantPtr participant);
    void leave(ParticipantPtr participant);
    void broadcast(const std::string& message, ParticipantPtr sender);
private:
    std::set<ParticipantPtr> participants_;
};

// Handles a WebSocket connection
class WebSocketSession : public Participant,
                          public std::enable_shared_from_this<WebSocketSession> {
public:
    WebSocketSession(tcp::socket socket, Room& room);
    void run(http::request<http::string_body> req);
    void send(const std::string& msg) override;
private:
    void onAccept(beast::error_code ec);
    void doRead();
    void onRead(beast::error_code ec, std::size_t bytes_transferred);
    void doWrite();

    websocket::stream<beast::tcp_stream> ws_;
    Room& room_;
    beast::flat_buffer buffer_;
    std::deque<std::string> writeQueue_;
    bool writing_ = false;
};

// Handles initial HTTP connection — serves HTML or upgrades to WebSocket
class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(tcp::socket socket, Room& room);
    void run();
private:
    void doRead();
    void handleRequest();

    beast::tcp_stream stream_;
    Room& room_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
};

// Accepts incoming TCP connections
class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(net::io_context& ioc, tcp::endpoint endpoint, Room& room);
    void run();
private:
    void doAccept();

    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    Room& room_;
};

#endif // CHATROOM_HPP