var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

var timer, timestamp;
var manual_mode, ntp_fields;

function setActiveNavLink() {
    var currentUrl = window.location.pathname;
    var links = document.querySelectorAll('nav a');
    for (var i = 0; i < links.length; i++) {
        if (links[i].classList.contains('navbar-brand'))
            continue;
        var linkUrl = links[i].pathname;
        if (linkUrl === '/' && currentUrl === '/') {
            links[i].classList.add('active');
            break;
        } else if (linkUrl !== '/' && currentUrl.startsWith(linkUrl)) {
            links[i].classList.add('active');
            break;
        }
    }
}

function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    console.log("WS Message received: " + event.data);

    try {
        // parse the JSON data from the message
        const data = JSON.parse(event.data);
        // set the time zone
        document.getElementById('ntp_tz').value = data.ntp_tz;
        document.getElementById('manual_mode').checked = data.manual_mode === 'true';
        document.getElementById('ntp_server').value = data.ntp_server;
    } catch (error) {
        console.error(error);
    }
}

function getTimestamp() {
    document.getElementById("manual_mode").value = Math.floor(Date.now() / 1000);
}

function processTimeFields() {
    if (manual_mode.checked) {
        ntp_fields.style.display = 'none';
        getTimestamp();
        timer = setInterval(getTimestamp, 1000);
    }
    else {
        ntp_fields.style.display = 'block';
        clearInterval(timer);
    }
    document.getElementById("manual_mode_init").disabled = (manual_mode.checked) ? true : false;
    var inputs = ntp_fields.querySelectorAll("input");
    for (var i = 0; i < inputs.length; i++)
        inputs[i].disabled = (manual_mode.checked) ? true : false;
}

// this will disable the leave page warning pop-up of the forms
function onUnload(event) {
    event.preventDefault();
    event.returnValue = null;
}

window.addEventListener('unload', onUnload);

function onLoad() {
    manual_mode = document.getElementById('manual_mode');
    ntp_fields = document.getElementById('ntp_fields');

    setActiveNavLink();
    initWebSocket();

    /* Handling Manual Mode Checkbox */
    manual_mode.addEventListener('change', processTimeFields);
}

window.addEventListener('load', onLoad);
