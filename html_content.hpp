#ifndef HTML_CONTENT_HPP
#define HTML_CONTENT_HPP

#include <string>

inline std::string getHtmlPage() {
    return R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ChatRoom</title>
    <meta name="description" content="Real-time browser chat powered by C++ and Boost.Beast WebSockets">
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }

        body {
            font-family: 'Inter', sans-serif;
            background: #0a0a0f;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            color: #e0e0e0;
            overflow: hidden;
        }

        body::before {
            content: '';
            position: fixed;
            top: -50%; left: -50%;
            width: 200%; height: 200%;
            background:
                radial-gradient(ellipse at 20% 50%, rgba(102, 126, 234, 0.08) 0%, transparent 50%),
                radial-gradient(ellipse at 80% 20%, rgba(118, 75, 162, 0.08) 0%, transparent 50%),
                radial-gradient(ellipse at 50% 80%, rgba(236, 72, 153, 0.05) 0%, transparent 50%);
            animation: bgShift 20s ease-in-out infinite alternate;
            z-index: 0;
        }

        @keyframes bgShift {
            0%   { transform: translate(0, 0); }
            100% { transform: translate(-5%, -3%); }
        }

        /* ── Modal ── */
        .modal-overlay {
            position: fixed; inset: 0;
            background: rgba(0, 0, 0, 0.75);
            backdrop-filter: blur(12px);
            display: flex; align-items: center; justify-content: center;
            z-index: 1000;
            animation: fadeIn 0.3s ease;
        }
        .modal-overlay.hidden { display: none; }

        .modal {
            background: rgba(255, 255, 255, 0.04);
            backdrop-filter: blur(24px);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 24px;
            padding: 48px 40px;
            width: 400px;
            text-align: center;
            animation: slideUp 0.5s cubic-bezier(.16,1,.3,1);
            box-shadow: 0 24px 80px rgba(0,0,0,0.4);
        }

        .modal-logo {
            font-size: 48px;
            margin-bottom: 8px;
        }

        .modal h2 {
            font-size: 26px; font-weight: 700;
            margin-bottom: 6px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 50%, #ec4899 100%);
            -webkit-background-clip: text; -webkit-text-fill-color: transparent;
            background-clip: text;
        }

        .modal p {
            color: #666; margin-bottom: 28px; font-size: 14px; line-height: 1.5;
        }

        .modal input {
            width: 100%; padding: 14px 18px;
            background: rgba(255,255,255,0.06);
            border: 1px solid rgba(255,255,255,0.12);
            border-radius: 14px;
            color: #fff; font-size: 15px;
            font-family: 'Inter', sans-serif;
            outline: none;
            transition: border-color 0.3s, box-shadow 0.3s;
            margin-bottom: 16px;
        }
        .modal input:focus {
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102,126,234,0.15);
        }
        .modal input::placeholder { color: #555; }

        .modal button {
            width: 100%; padding: 14px;
            background: linear-gradient(135deg, #667eea, #764ba2);
            border: none; border-radius: 14px;
            color: #fff; font-size: 15px; font-weight: 600;
            cursor: pointer;
            transition: transform 0.2s, box-shadow 0.2s;
            font-family: 'Inter', sans-serif;
        }
        .modal button:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 30px rgba(102,126,234,0.3);
        }
        .modal button:active { transform: translateY(0); }

        /* ── Chat Container ── */
        .chat-container {
            position: relative; z-index: 1;
            width: 100%; max-width: 720px;
            height: 100vh;
            display: flex; flex-direction: column;
            padding: 16px;
        }

        /* ── Header ── */
        .chat-header {
            display: flex; align-items: center; justify-content: space-between;
            padding: 16px 24px;
            background: rgba(255,255,255,0.03);
            backdrop-filter: blur(20px);
            border: 1px solid rgba(255,255,255,0.06);
            border-radius: 18px;
            margin-bottom: 12px;
        }

        .chat-header-left {
            display: flex; align-items: center; gap: 12px;
        }

        .chat-header-left .logo {
            font-size: 24px;
        }

        .chat-header h1 {
            font-size: 18px; font-weight: 700;
            background: linear-gradient(135deg, #667eea, #ec4899);
            -webkit-background-clip: text; -webkit-text-fill-color: transparent;
            background-clip: text;
        }

        .status {
            display: flex; align-items: center; gap: 8px;
            font-size: 12px; color: #666;
        }

        .status-dot {
            width: 8px; height: 8px; border-radius: 50%;
            background: #ef4444;
            transition: background 0.3s, box-shadow 0.3s;
        }
        .status-dot.connected {
            background: #22c55e;
            box-shadow: 0 0 8px rgba(34,197,94,0.5);
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0%, 100% { box-shadow: 0 0 8px rgba(34,197,94,0.5); }
            50% { box-shadow: 0 0 16px rgba(34,197,94,0.3); }
        }

        /* ── Messages ── */
        .messages {
            flex: 1;
            overflow-y: auto;
            padding: 8px 0;
            display: flex; flex-direction: column;
            gap: 8px;
            scrollbar-width: thin;
            scrollbar-color: rgba(255,255,255,0.08) transparent;
        }
        .messages::-webkit-scrollbar { width: 6px; }
        .messages::-webkit-scrollbar-track { background: transparent; }
        .messages::-webkit-scrollbar-thumb {
            background: rgba(255,255,255,0.08); border-radius: 3px;
        }

        .message {
            max-width: 72%;
            padding: 10px 16px 8px;
            border-radius: 18px;
            animation: msgIn 0.35s cubic-bezier(.16,1,.3,1);
            word-wrap: break-word;
        }

        .message.own {
            align-self: flex-end;
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: #fff;
            border-bottom-right-radius: 6px;
        }

        .message.other {
            align-self: flex-start;
            background: rgba(255,255,255,0.06);
            border: 1px solid rgba(255,255,255,0.08);
            border-bottom-left-radius: 6px;
        }

        .message .sender {
            font-size: 11px; font-weight: 600;
            margin-bottom: 3px;
            letter-spacing: 0.02em;
        }
        .message.own .sender { color: rgba(255,255,255,0.7); }
        .message.other .sender { color: #667eea; }

        .message .text {
            font-size: 14px; line-height: 1.5;
        }

        .message .time {
            font-size: 10px; opacity: 0.45;
            margin-top: 4px; text-align: right;
        }

        .message.system {
            align-self: center;
            max-width: 85%;
            background: transparent;
            color: #555;
            font-size: 12px;
            padding: 6px 14px;
            border-radius: 20px;
            text-align: center;
            border: 1px solid rgba(255,255,255,0.04);
        }

        /* ── Input ── */
        .input-area {
            display: flex; gap: 10px;
            padding-top: 12px;
        }

        .input-area input {
            flex: 1;
            padding: 14px 20px;
            background: rgba(255,255,255,0.05);
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 18px;
            color: #fff; font-size: 14px;
            font-family: 'Inter', sans-serif;
            outline: none;
            transition: border-color 0.3s, box-shadow 0.3s;
        }
        .input-area input:focus {
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102,126,234,0.1);
        }
        .input-area input::placeholder { color: #444; }
        .input-area input:disabled {
            opacity: 0.4; cursor: not-allowed;
        }

        .input-area button {
            padding: 14px 28px;
            background: linear-gradient(135deg, #667eea, #764ba2);
            border: none; border-radius: 18px;
            color: #fff; font-size: 14px; font-weight: 600;
            cursor: pointer;
            transition: transform 0.15s, box-shadow 0.2s, opacity 0.2s;
            font-family: 'Inter', sans-serif;
            white-space: nowrap;
        }
        .input-area button:hover:not(:disabled) {
            transform: translateY(-1px);
            box-shadow: 0 6px 24px rgba(102,126,234,0.25);
        }
        .input-area button:active:not(:disabled) { transform: translateY(0); }
        .input-area button:disabled { opacity: 0.4; cursor: not-allowed; }

        /* ── Animations ── */
        @keyframes fadeIn {
            from { opacity: 0; } to { opacity: 1; }
        }
        @keyframes slideUp {
            from { opacity: 0; transform: translateY(24px) scale(0.96); }
            to   { opacity: 1; transform: translateY(0)   scale(1); }
        }
        @keyframes msgIn {
            from { opacity: 0; transform: translateY(12px); }
            to   { opacity: 1; transform: translateY(0); }
        }

        /* ── Responsive ── */
        @media (max-width: 600px) {
            .chat-container { padding: 8px; }
            .modal { padding: 32px 24px; margin: 16px; width: calc(100% - 32px); }
            .message { max-width: 85%; }
            .input-area button { padding: 14px 18px; }
        }
    </style>
</head>
<body>
    <!-- Username Modal -->
    <div class="modal-overlay" id="usernameModal">
        <div class="modal">
            <div class="modal-logo">&#128172;</div>
            <h2>ChatRoom</h2>
            <p>Pick a username and jump into the conversation</p>
            <input type="text" id="usernameInput" placeholder="Your username..." maxlength="20" autocomplete="off" autofocus>
            <button onclick="joinChat()">Join Chat</button>
        </div>
    </div>

    <!-- Chat Interface -->
    <div class="chat-container">
        <div class="chat-header">
            <div class="chat-header-left">
                <span class="logo">&#128172;</span>
                <h1>ChatRoom</h1>
            </div>
            <div class="status">
                <span class="status-dot" id="statusDot"></span>
                <span id="statusText">Disconnected</span>
            </div>
        </div>

        <div class="messages" id="messages"></div>

        <div class="input-area">
            <input type="text" id="messageInput" placeholder="Type a message..." autocomplete="off" disabled>
            <button id="sendBtn" onclick="sendMessage()" disabled>Send</button>
        </div>
    </div>

    <script>
        let ws = null;
        let username = '';

        const messagesEl   = document.getElementById('messages');
        const messageInput = document.getElementById('messageInput');
        const sendBtn      = document.getElementById('sendBtn');
        const statusDot    = document.getElementById('statusDot');
        const statusText   = document.getElementById('statusText');
        const usernameIn   = document.getElementById('usernameInput');
        const modal        = document.getElementById('usernameModal');

        usernameIn.addEventListener('keypress', e => { if (e.key === 'Enter') joinChat(); });
        messageInput.addEventListener('keypress', e => { if (e.key === 'Enter') sendMessage(); });

        function joinChat() {
            const name = usernameIn.value.trim();
            if (!name) return;
            username = name;
            modal.classList.add('hidden');
            connect();
        }

        function connect() {
            const proto = location.protocol === 'https:' ? 'wss:' : 'ws:';
            ws = new WebSocket(proto + '//' + location.host + '/ws');

            ws.onopen = () => {
                statusDot.classList.add('connected');
                statusText.textContent = 'Connected';
                messageInput.disabled = false;
                sendBtn.disabled = false;
                messageInput.focus();
                addSystem('You joined as ' + username);
            };

            ws.onmessage = evt => {
                try {
                    const d = JSON.parse(evt.data);
                    addMsg(d.username || '?', d.text || '', false);
                } catch (_) {
                    addSystem(evt.data);
                }
            };

            ws.onclose = () => {
                statusDot.classList.remove('connected');
                statusText.textContent = 'Reconnecting...';
                messageInput.disabled = true;
                sendBtn.disabled = true;
                addSystem('Connection lost. Reconnecting...');
                setTimeout(connect, 3000);
            };

            ws.onerror = () => { addSystem('Connection error'); };
        }

        function sendMessage() {
            const text = messageInput.value.trim();
            if (!text || !ws || ws.readyState !== WebSocket.OPEN) return;
            ws.send(JSON.stringify({ username, text }));
            addMsg(username, text, true);
            messageInput.value = '';
            messageInput.focus();
        }

        function addMsg(sender, text, own) {
            const el = document.createElement('div');
            el.className = 'message ' + (own ? 'own' : 'other');
            const t = new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
            el.innerHTML =
                '<div class="sender">' + esc(sender) + '</div>' +
                '<div class="text">'   + esc(text)   + '</div>' +
                '<div class="time">'   + t           + '</div>';
            messagesEl.appendChild(el);
            messagesEl.scrollTop = messagesEl.scrollHeight;
        }

        function addSystem(text) {
            const el = document.createElement('div');
            el.className = 'message system';
            el.textContent = text;
            messagesEl.appendChild(el);
            messagesEl.scrollTop = messagesEl.scrollHeight;
        }

        function esc(s) {
            const d = document.createElement('div');
            d.textContent = s;
            return d.innerHTML;
        }
    </script>
</body>
</html>
)HTML";
}

#endif // HTML_CONTENT_HPP
