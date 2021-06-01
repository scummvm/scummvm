// Workaround for https://github.com/emscripten-core/emscripten/pull/9803  which results in mouse events not working anymore after context switches
JSEvents.removeAllHandlersOnTarget = function(){}