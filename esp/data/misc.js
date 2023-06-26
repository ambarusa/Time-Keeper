const gateway = `ws://${window.location.hostname}/ws`;

function onClose(e) {
    console.log('Connection closed');
    socket.init((onMessage), 2000);
}

function onOpen(e, uniqueID) {
    console.log('Connection opened');
    socket.websocket.send('PAGE ' + uniqueID + ' ');
}

const socket = {
    init(onMessage, uniqueID) {
        socket.websocket = new WebSocket(gateway);
        socket.websocket.onopen = (event) => onOpen(event, uniqueID);
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

// this will disable the leave page warning pop-up of the forms
function onPageUnload(event) {
    event.preventDefault();
    event.returnValue = null;
}