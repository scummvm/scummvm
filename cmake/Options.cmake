# Command line options that we would want to pass

# Define all of out default options. These can be overwritten via the command line, preset or platform file
set(BACKEND "sdl" CACHE STRING "Backend to build")

# Backend logic
set(BACKEND_VALUES
    "3ds;atari;android;dc;dingux;ds;gcw0;gph;ios7;maemo;n64;null;opendingux;openpandora;psp;psp2;samsungtv;sdl;switch;wii"
)
# Only allow our list of backends to be set
set_property(CACHE BACKEND PROPERTY STRINGS ${BACKEND_VALUES})
