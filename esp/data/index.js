let brightness;


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

    try {
        // parse the JSON data from the message
        const data = JSON.parse(event.data);
        // set the light mode
        id = "light_" + data.light_mode;
        document.getElementById(id.toLowerCase()).checked = true;
        processSlider();
        // set the brightness
        if (brightness !== null) brightness.value = data.brightness;
    } catch (error) {
        // it is not a JSON message
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