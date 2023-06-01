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

window.addEventListener('unload', onPageUnload);

function onLoad() {
    setActiveNavLink();
    initSwitch();
}

window.addEventListener('load', onLoad);