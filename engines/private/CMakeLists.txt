add_library(private STATIC)

target_sources(private PRIVATE code.cpp
    cursors.cpp
    decompiler.cpp
    funcs.cpp
    grammar.cpp
    lexer.cpp
    metaengine.cpp
    private.cpp
    savegame.cpp
    symbol.cpp


PRIVATE
FILE_SET HEADERS
FILES
    decompiler.h
    grammar.h
    private.h
    savegame.h
    symbol.h

)

# TODO: Add missing logic

add_library(private_detection)

target_sources(private_detection PRIVATE
 detection.cpp
)