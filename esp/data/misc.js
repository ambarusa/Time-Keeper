const gateway = `ws://${window.location.hostname}/ws`;

function onClose() {
    console.log('WS: Connection closed, reconnecting in 2s\u2026');
    setTimeout(() => socket.init(socket._onMessage, socket._pageId), 2000);
}

function onOpen() {
    console.log('WS: Connection opened');
    socket.websocket.send('PAGE ' + socket._pageId + ' ');
}

const socket = {
    init(onMessage, pageId) {
        socket._onMessage = onMessage;
        socket._pageId = pageId;
        socket.websocket = new WebSocket(gateway);
        socket.websocket.onopen = onOpen;
        socket.websocket.onclose = onClose;
        socket.websocket.onmessage = onMessage;
    }
};

function setActiveNavLink() {
    const path = window.location.pathname;
    document.querySelectorAll('nav a:not(.navbar-brand)').forEach(link => {
        if (!link.hash && (
            (link.pathname === '/' && path === '/') ||
            (link.pathname !== '/' && path.startsWith(link.pathname))
        )) link.classList.add('active');
    });
}

function submitValidation(event) {
    if (!event.currentTarget.checkValidity()) {
        event.preventDefault();
        event.stopPropagation();
    }
    event.currentTarget.classList.add('was-validated');
}

function onPageUnload(event) {
    event.preventDefault();
    event.returnValue = null;
}