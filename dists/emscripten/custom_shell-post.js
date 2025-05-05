// Workaround for https://github.com/emscripten-core/emscripten/pull/9803
// which results in mouse events not working anymore after context switches 
// (i.e. when launching a game)
/*global JSEvents*/
JSEvents.removeAllHandlersOnTarget = function(){};

// Make sure to release any resources (e.g. RetroWave or Midi Devices) when leaving the page
window.addEventListener("beforeunload", function (e) {
	Module["_raise"](2); // SIGINT
});
