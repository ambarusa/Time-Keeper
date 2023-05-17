const gateway = `ws://${window.location.hostname}/ws`;

function onClose(e) {
    console.log('Connection closed');
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

function setActiveNavLink() {
    const currentUrl = window.location.pathname;
    let links = document.querySelectorAll('nav a');
    for (let i = 0; i < links.length; i++) {
        if (links[i].classList.contains('navbar-brand'))
            continue;
        const linkUrl = links[i].pathname;
        if (linkUrl === '/' && currentUrl === '/') {
            links[i].classList.add('active');
            break;
        } else if (linkUrl !== '/' && currentUrl.startsWith(linkUrl)) {
            links[i].classList.add('active');
            break;
        }
    }
}

function submitValidation(event) {
    if (!event.currentTarget.checkValidity()) {
        event.preventDefault()
        event.stopPropagation()
    }
    event.currentTarget.classList.add('was-validated')
}
