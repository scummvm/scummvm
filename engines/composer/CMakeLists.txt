add_library(composer STATIC)

target_sources(
  composer
  PRIVATE console.cpp
          composer.cpp
          graphics.cpp
          metaengine.cpp
          resource.cpp
          saveload.cpp
          scripting.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          console.h
          composer.h
          graphics.h
          resource.h
)

add_library(composer_detection)

target_sources(composer_detection PRIVATE detection.cpp)
