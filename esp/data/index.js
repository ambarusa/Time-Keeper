let brightness;

function updateBrightnessLabel(value) {
    const lbl = document.getElementById('brightness_val');
    if (lbl) lbl.textContent = value + '%';
}

function processSlider() {
    if (!brightness) return;
    const lightManual = document.getElementById('light_manual');
    brightness.disabled = !lightManual.checked;
    updateBrightnessLabel(brightness.disabled ? '--' : brightness.value);
}

function setLightMode(mode) {
    const el = document.getElementById(('light_' + mode).toLowerCase());
    if (el) { el.checked = true; processSlider(); }
}

function onMessage(event) {
    console.log('WS Message received: ' + event.data);
    try {
        const data = JSON.parse(event.data);
        setLightMode(data.light_mode);
        if (brightness) { brightness.value = data.brightness; updateBrightnessLabel(data.brightness); }
    } catch {
        const parts = event.data.split(' ');
        switch (parts[0]) {
            case 'LIGHTMODE': setLightMode(parts[1]); break;
            case 'BRIGHTNESS':
                if (brightness) { brightness.value = parts[1]; updateBrightnessLabel(parts[1]); }
                break;
        }
    }
}

window.addEventListener('unload', onPageUnload);

function onLoad() {
    setActiveNavLink();
    socket.init(onMessage, 'INDEX');

    brightness = document.getElementById('brightness');
    if (brightness) {
        brightness.addEventListener('input', () => updateBrightnessLabel(brightness.value));
    }

    document.getElementById('light_form').addEventListener('change', processSlider);
    document.getElementById('light_submit').addEventListener('click', () => {
        const checked = document.querySelector('input[name="light_mode"]:checked');
        if (!checked) return;
        const mode = checked.id.split('_')[1];
        socket.websocket.send('LIGHTMODE ' + mode + ' ');
        if (brightness && !brightness.disabled)
            socket.websocket.send('BRIGHTNESS ' + brightness.value + ' ');
    });
}

window.addEventListener('load', onLoad);