var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

var timer, timestamp;

function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function initSwitch() {
    // document.getElementById('sf_pw_sw').addEventListener('click', function () {
    //     websocket.send((this.checked) ? "SF_POWER ON " : "SF_POWER OFF ");
    // });

    // document.getElementById('sf_st_sw').addEventListener('click', function () {
    //     websocket.send((this.checked) ? "SF_STEAM ON " : "SF_STEAM OFF ");
    // });

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
        case "TIME":
            document.getElementById("time").innerHTML = second;
            break;
        case "MQTT":
            var text = event.data.slice(event.data.indexOf(" ") + 1);
            document.getElementById('mqtt_status').innerHTML = text;
            break;
    }
}

function showMessage(input, message, type) {
    // get the small element and set the message
    const msg = input.parentNode.querySelector("small");
    msg.innerText = message;
    // update the class for the input
    input.className = type ? "success" : "error";
    return type;
}

function showError(input, message) {
    return showMessage(input, message, false);
}

function showSuccess(input) {
    return showMessage(input, "", true);
}

function hasValue(input, message) {
    if (input.value.trim() === "") {
        return showError(input, message);
    }
    return showSuccess(input);
}

function validatePort(input, requiredMsg, invalidMsg) {
    // check if the value is not empty
    if (!hasValue(input, requiredMsg)) {
        return false;
    }

    const port = input.value.trim();
    if (isNaN(port) || port < 0) {
        return showError(input, invalidMsg);
    }
    return true;
}

const VALUE_REQUIRED = "Please fill in the entry!"
const PORT_INVALID = "Please provide a valid port!"
var manual_mode, mqtt_enabled, ntp_fields, mqtt_fields;

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
        document.getElementById("manual_mode").value = "";
    }
}

function processMQTTFields() {
    mqtt_fields.style.display = (mqtt_enabled.checked) ? 'block' : 'none';
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

        // validate the form
        let serverValid = hasValue(time_form.elements['ntp_server'], VALUE_REQUIRED);

        // if valid, submit the form.
        if (serverValid)
            time_form.submit();
    });

    /* Handling MQTT Form */
    const mqtt_form = document.getElementById('mqtt_form');
    mqtt_form.addEventListener('submit', (event) => {

        // stop form submission
        event.preventDefault();

        // validate the form
        let hostValid = hasValue(mqtt_form.elements['mqtt_host'], VALUE_REQUIRED);
        let portValid = validatePort(mqtt_form.elements['mqtt_port'], VALUE_REQUIRED, PORT_INVALID);
        let clientValid = hasValue(mqtt_form.elements['mqtt_cli'], VALUE_REQUIRED);
        let userValid = hasValue(mqtt_form.elements['mqtt_user'], VALUE_REQUIRED);

        // if valid, submit the form.
        if (hostValid && portValid && clientValid && userValid) {
            mqtt_form.submit();
        }
    });
}

window.addEventListener('load', onLoad);