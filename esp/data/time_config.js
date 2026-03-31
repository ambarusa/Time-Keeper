let timer;
let manual_mode, ntp_fields;

function onMessage(event) {
    console.log('WS Message received: ' + event.data);
    try {
        const data = JSON.parse(event.data);
        document.getElementById('ntp_tz').value = data.ntp_tz;
        document.getElementById('manual_mode').checked = !!+data.manual_mode;
        processTimeFields();
        document.getElementById('ntp_server').value = data.ntp_server;
    } catch { }
}

function getTimestamp() {
    document.getElementById('manual_mode').value = Math.floor(Date.now() / 1000);
}

function processTimeFields() {
    clearInterval(timer);
    const checked = manual_mode.checked;
    ntp_fields.style.display = checked ? 'none' : 'block';
    document.getElementById('manual_mode_init').disabled = checked;
    ntp_fields.querySelectorAll('input').forEach(el => el.disabled = checked);
    if (checked) {
        getTimestamp();
        timer = setInterval(getTimestamp, 1000);
    }
}

window.addEventListener('unload', onPageUnload);

function onLoad() {
    manual_mode = document.getElementById('manual_mode');
    ntp_fields = document.getElementById('ntp_fields');

    setActiveNavLink();
    socket.init(onMessage, 'TIME');

    manual_mode.addEventListener('change', processTimeFields);
    document.getElementById('time_form').addEventListener('submit', submitValidation);
}

window.addEventListener('load', onLoad);