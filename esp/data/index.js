var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

var brightness;

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

function processSlider() {
    if (brightness === null)
        return;
    if (document.getElementById("light_manual").checked)
        brightness.disabled = !document.getElementById("light_manual").checked;
    else
        brightness.disabled = true;
}

function onMessage(event) {
    console.log("WS Message received: " + event.data);
    var first = event.data.split(" ")[0];
    var second = event.data.split(" ")[1];
    switch (first) {
        case "LIGHTMODE":
            id = "light_" + second;
            document.getElementById(id.toLowerCase()).checked = true;
            processSlider();
            break;
        case "BRIGHTNESS":
            brightness.value = second;
            break;
    }
}

function onLoad(event) {
    initWebSocket();

    brightness = document.getElementById('brightness');
    document.getElementById("light_form").addEventListener('change', processSlider);
    document.getElementById("light_submit").addEventListener('click', function () {
        mode = document.querySelector('input[name="light_mode"]:checked').id;

        websocket.send("LIGHTMODE " + mode.split("_")[1] + " ");
        if (brightness !== null && mode.split("_")[1] === "manual")
            websocket.send("BRIGHTNESS " + brightness.value + " ");

    });
}

window.addEventListener('load', onLoad);