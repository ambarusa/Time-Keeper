let mqtt_enabled, mqtt_fields;

function initSwitch() {
    document.getElementById('reset_sw').addEventListener('click', function () {
        if (confirm('This will reset everything. Continue?'))
            location.replace("/reset_config")
    });
    document.getElementById('restart_sw').addEventListener('click', function () {
        if (confirm('The device will restart. Continue?'))
            location.replace("/restart")
    });
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
    } catch {
        // it is not a JSON message
        const first = event.data.split(" ")[0];
        let second = event.data.split(" ")[1];
        switch (first) {
            case "MQTT_STATUS":
                second = event.data.split(" ").slice(1).join(" ");
                document.getElementById('mqtt_status').innerHTML = second;
                break;
            case "MQTT_EN":
                document.getElementById('mqtt_en').checked = second == true;
                processMQTTFields();
                break;
        }
    }
}

function processMQTTFields() {
    mqtt_fields.style.display = (mqtt_enabled.checked) ? 'block' : 'none';
    document.getElementById("mqtt_en_init").disabled = (mqtt_enabled.checked) ? true : false;
    let inputs = mqtt_fields.querySelectorAll("input");
    for (let i = 0; i < inputs.length; i++)
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
    socket.init(onMessage);
    initSwitch();

    /* Handling MQTT Enabled Checkbox */
    mqtt_enabled.addEventListener('change', processMQTTFields);

    /* Validating MQTT Form */
    const mqtt_form = document.getElementById('mqtt_form');
    mqtt_form.addEventListener('submit', submitValidation);

    /* Validating Wi-Fi Form */
    const wifi_form = document.getElementById('wifi_form');
    wifi_form.addEventListener('submit', submitValidation);
}

window.addEventListener('load', onLoad);