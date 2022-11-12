var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

var timer, timestamp;
var manual_mode, mqtt_enabled, ntp_fields, mqtt_fields;

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
    var first = event.data.split(" ")[0];
    var second = event.data.split(" ")[1];
    switch (first) {
        case "MQTT":
            var text = event.data.slice(event.data.indexOf(" ") + 1);
            document.getElementById('mqtt_status').innerHTML = text;
            break;
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

function processMQTTFields() {
    mqtt_fields.style.display = (mqtt_enabled.checked) ? 'block' : 'none';
    document.getElementById("mqtt_en_init").disabled = (mqtt_enabled.checked) ? true : false;
    var inputs = mqtt_fields.querySelectorAll("input");
    for (var i = 0; i < inputs.length; i++)
        inputs[i].disabled = (mqtt_enabled.checked) ? false : true;
}

function onLoad(event) {
    manual_mode = document.getElementById('manual_mode');
    mqtt_enabled = document.getElementById('mqtt_en');
    ntp_fields = document.getElementById('ntp_fields');
    mqtt_fields = document.getElementById('mqtt_fields');

    initWebSocket();
    initSwitch();

    processTimeFields();
    processMQTTFields();

    /* Handling Manual Mode Checkbox */
    manual_mode.addEventListener('change', processTimeFields);

    /* Handling MQTT Enabled Checkbox */
    mqtt_enabled.addEventListener('change', processMQTTFields);

    /* Handling TIME Form */
    const time_form = document.getElementById('time_form');
    time_form.addEventListener('submit', (event) => {

        // stop form submission
        event.preventDefault();

        time_form.submit();
    });

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