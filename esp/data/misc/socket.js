const gateway = require('./gateway')

function onClose(e) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onOpen(e) {
    console.log('Connection opened');
}

const socket = {
    init(onMessage) {
        socket.websocket = new WebSocket(gateway);
        socket.websocket.onopen = onOpen;
        socket.websocket.onclose = onClose;
        socket.websocket.onmessage = onMessage;
    }
};

export default socket