// Workaround for https://github.com/emscripten-core/emscripten/pull/9803
// which results in mouse events not working anymore after context switches 
// (i.e. when launching a game)
/*global JSEvents*/
JSEvents.removeAllHandlersOnTarget = function(){};