cmake_minimum_required(VERSION 3.23)

project(ScummVM)

add_library(common)

target_sources(
  common
  PRIVATE archive.cpp
          base64.cpp
          btea.cpp
          concatstream.cpp
          config-manager.cpp
          coroutines.cpp
          dbcs-str.cpp
          debug.cpp
          engine_data.cpp
          error.cpp
          events.cpp
          file.cpp
          fs.cpp
          gui_options.cpp
          hashmap.cpp
          language.cpp
          localization.cpp
          macresman.cpp
          memory.cpp
          memorypool.cpp
          md5.cpp
          mutex.cpp
          osd_message_queue.cpp
          path.cpp
          platform.cpp
          punycode.cpp
          random.cpp
          rational.cpp
          rendermode.cpp
          rotationmode.cpp
          scummsys.h
          str.cpp
          stream.cpp
          streamdebug.cpp
          str-base.cpp
          str-enc.cpp
          encodings/singlebyte.cpp
          system.cpp
          textconsole.cpp
          text-to-speech.cpp
          tokenizer.cpp
          translation.cpp
          unicode-bidi.cpp
          ustr.cpp
          util.cpp
          xpfloat.cpp
          zip-set.cpp
          std/std.cpp)

add_subdirectory(formats)
add_subdirectory(compression)

if(ENABLE_EVENTRECORDER)
  target_sources(common PRIVATE recorderfile.cpp)
endif()

if(ENABLE_UPDATES)
  target_sources(common PRIVATE updates.cpp)
endif()

target_include_directories(
  common
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/.."
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
