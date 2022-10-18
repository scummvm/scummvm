/*global Module*/
Module["arguments"] = [];
Module["arguments"].push("--config=/local/scummvm.ini");

// https://developer.mozilla.org/en-US/docs/Web/API/Gamepad_API only works in secure contexts and supported browsers. 
// This disables joystick support to avoid a crash when initializing the sdl subsystem without the gamepad API being available.
if (!navigator.getGamepads && !navigator.webkitGetGamepads) {
    Module["arguments"].push("--joystick=-1")
}

// Add all parameters passed via the fragment identifier
if (window.location.hash.length > 0) {
    params = decodeURI(window.location.hash.substring(1)).split(" ")
    params.forEach((param) => {
        Module["arguments"].push(param);
    })
}
