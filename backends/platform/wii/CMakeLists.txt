cmake_minimum_required(VERSION 3.23)

add_executable(platform)

target_compile_definitions(platform PRIVATE __wii__ HW_RVL USE_RGB_COLOR)

target_link_directories(platform PRIVATE ${DEVKITPRO}/libogc/lib/wii)
target_link_libraries(platform PRIVATE gxflux fat)

target_sources(
  platform PRIVATE main.cpp options.cpp osystem.cpp osystem_gfx.cpp
                   osystem_sfx.cpp osystem_events.cpp)
