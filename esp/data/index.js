let brightness;


function processSlider() {
    if (brightness === null)
        return;

    const lightManual = document.getElementById("light_manual").checked;
    if (lightManual.checked)
        brightness.disabled = !lightManual.checked;
    else
        brightness.disabled = true;
}

function onMessage(event) {
    console.log("WS Message received: " + event.data);

    try {
        // parse the JSON data from the message
        const data = JSON.parse(event.data);
        // set the light mode
        const id = document.getElementById(("light_" + data.light_mode).toLowerCase());
        id.checked = true;
        processSlider();
        // set the brightness
        if (brightness !== null) brightness.value = data.brightness;
    } catch (error) {
        // it is not a JSON message
        const first = event.data.split(" ")[0];
        const second = event.data.split(" ")[1];
        switch (first) {
            case "LIGHTMODE":
                id = "light_" + second;
                id.checked = true;
                processSlider();
                break;
            case "BRIGHTNESS":
                brightness.value = second;
                break;
        }
    }
}

// this will disable the leave page warning pop-up of the forms
function onUnload(event) {
    event.preventDefault();
    event.returnValue = null;
}

window.addEventListener('unload', onUnload);

function onLoad() {

    setActiveNavLink();
    socket.init(onMessage);

    brightness = document.getElementById('brightness');
    document.getElementById("light_form").addEventListener('change', processSlider);
    document.getElementById("light_submit").addEventListener('click', function () {

        mode = document.querySelector('input[name="light_mode"]:checked').id;

        socket.websocket.send("LIGHTMODE " + mode.split("_")[1] + " ");
        if (brightness !== null && mode.split("_")[1] === "manual")
            socket.websocket.send("BRIGHTNESS " + brightness.value + " ");

    });
}

window.addEventListener('load', onLoad);