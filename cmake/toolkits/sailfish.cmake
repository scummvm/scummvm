# Sailfish OS toolkit configuration

set(HOST_OS "linux" CACHE INTERNAL "")

add_compile_definitions(SAILFISH)
set(HOST_EXE_PREFIX "org.scummvm." PARENT_SCOPE)
set(BACKEND "sailfish" CACHE STRING "Backend to build" FORCE)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_VKEYBD ON)

set(DETECT_HOST TRUE CACHE INTERNAL "")
