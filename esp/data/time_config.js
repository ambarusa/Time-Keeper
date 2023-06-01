let timer, timestamp;
let manual_mode, ntp_fields;

function onMessage(event) {
    console.log("WS Message received: " + event.data);

    try {
        // parse the JSON data from the message
        const data = JSON.parse(event.data);
        // set the time zone
        document.getElementById('ntp_tz').value = data.ntp_tz;
        document.getElementById('manual_mode').checked = data.manual_mode == true;
        processTimeFields();
        document.getElementById('ntp_server').value = data.ntp_server;
    } catch { }
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
    let inputs = ntp_fields.querySelectorAll("input");
    for (let i = 0; i < inputs.length; i++)
        inputs[i].disabled = (manual_mode.checked) ? true : false;
}

window.addEventListener('unload', onPageUnload);

function onLoad() {
    manual_mode = document.getElementById('manual_mode');
    ntp_fields = document.getElementById('ntp_fields');

    setActiveNavLink();
    socket.init(onMessage)

    /* Handling Manual Mode Checkbox */
    manual_mode.addEventListener('change', processTimeFields);

    /* Validating Time Form*/
    const time_form = document.getElementById('time_form');
    time_form.addEventListener('submit', submitValidation);
}

window.addEventListener('load', onLoad);