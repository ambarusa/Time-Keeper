let mqtt_enabled, mqtt_fields;

function setStatus(id, text) {
    document.getElementById(id).textContent = text;
}

function onMessage(event) {
    console.log('WS Message received: ' + event.data);
    try {
        const data = JSON.parse(event.data);
        setStatus('wifi_status', data.wifi_status);
        document.getElementById('wifi_ssid').value = data.wifi_ssid;
        setStatus('mqtt_status', data.mqtt_status);
        document.getElementById('mqtt_en').checked = !!+data.mqtt_en;
        processMQTTFields();
        document.getElementById('mqtt_host').value = data.mqtt_host;
        document.getElementById('mqtt_port').value = data.mqtt_port;
        document.getElementById('mqtt_qossub').value = data.mqtt_qossub;
        document.getElementById('mqtt_qospub').value = data.mqtt_qospub;
        document.getElementById('mqtt_cli').value = data.mqtt_cli;
        document.getElementById('mqtt_user').value = data.mqtt_user;
        document.getElementById('mqtt_autodisc').value = data.mqtt_autodisc;
    } catch {
        const parts = event.data.split(' ');
        switch (parts[0]) {
            case 'MQTT_STATUS':
                setStatus('mqtt_status', event.data.split(' ').slice(1).join(' '));
                break;
            case 'MQTT_EN':
                document.getElementById('mqtt_en').checked = !!+parts[1];
                processMQTTFields();
                break;
        }
    }
}

function processMQTTFields() {
    const enabled = mqtt_enabled.checked;
    mqtt_fields.style.display = enabled ? 'block' : 'none';
    document.getElementById('mqtt_en_init').disabled = enabled;
    mqtt_fields.querySelectorAll('input').forEach(el => el.disabled = !enabled);
}

window.addEventListener('unload', onPageUnload);

function onLoad() {
    mqtt_enabled = document.getElementById('mqtt_en');
    mqtt_fields = document.getElementById('mqtt_fields');

    setActiveNavLink();
    socket.init(onMessage, 'NET');

    mqtt_enabled.addEventListener('change', processMQTTFields);
    document.getElementById('mqtt_form').addEventListener('submit', submitValidation);
    document.getElementById('wifi_form').addEventListener('submit', submitValidation);
}

window.addEventListener('load', onLoad);