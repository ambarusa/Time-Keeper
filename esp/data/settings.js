var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

var timer;
var mqtt_enabled, mqtt_fields;

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

function initSwitch() {
    document.getElementById('reset_sw').addEventListener('click', function () {
        if (confirm('This will reset everything. Continue?')) {

            location.replace("/reset_config")
        } else {
            // Do nothing!
        }
    });
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

        document.getElementById('wifi_status').innerHTML = data.wifi_status;
        document.getElementById('wifi_ssid').value = data.wifi_ssid;
        document.getElementById('mqtt_status').innerHTML = data.mqtt_status;
        document.getElementById('mqtt_en').checked = data.mqtt_en == true;
        processMQTTFields();
        document.getElementById('mqtt_host').value = data.mqtt_host;
        document.getElementById('mqtt_port').value = data.mqtt_port;
        document.getElementById('mqtt_qossub').value = data.mqtt_qossub;
        document.getElementById('mqtt_qospub').value = data.mqtt_qospub;
        document.getElementById('mqtt_cli').value = data.mqtt_cli;
        document.getElementById('mqtt_user').value = data.mqtt_user;
        document.getElementById('mqtt_autodisc').value = data.mqtt_autodisc;
    } catch (error) {
        console.error(error);
    }
}

function processMQTTFields() {
    mqtt_fields.style.display = (mqtt_enabled.checked) ? 'block' : 'none';
    document.getElementById("mqtt_en_init").disabled = (mqtt_enabled.checked) ? true : false;
    var inputs = mqtt_fields.querySelectorAll("input");
    for (var i = 0; i < inputs.length; i++)
        inputs[i].disabled = (mqtt_enabled.checked) ? false : true;
}

// this will disable the leave page warning pop-up of the forms
function onUnload(event) {
    event.preventDefault();
    event.returnValue = null;
}

window.addEventListener('unload', onUnload);

function onLoad() {
    mqtt_enabled = document.getElementById('mqtt_en');
    mqtt_fields = document.getElementById('mqtt_fields');

    setActiveNavLink();
    initWebSocket();
    initSwitch();

    /* Handling MQTT Enabled Checkbox */
    mqtt_enabled.addEventListener('change', processMQTTFields);

    /* Handling MQTT Form */
    const mqtt_form = document.getElementById('mqtt_form');
    mqtt_form.addEventListener('submit', (event) => {

        // stop form submission
        event.preventDefault();

        mqtt_form.submit();
    });

    /* Handling Wi-Fi Form */
    const wifi_form = document.getElementById('wifi_form');
    wifi_form.addEventListener('submit', (event) => {

        // stop form submission
        event.preventDefault();

        wifi_form.submit();
    });
}

window.addEventListener('load', onLoad);