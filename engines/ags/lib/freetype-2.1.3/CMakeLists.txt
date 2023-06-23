add_library(freetype)

set_target_properties(freetype PROPERTIES
    C_STANDARD 11
    C_EXTENSIONS NO
)

target_include_directories(freetype PUBLIC include )

target_sources(freetype 
    PRIVATE
    src/base/ftbase.c
    src/base/ftglyph.c
    src/base/ftinit.c
    src/base/ftmm.c

    src/autohint/autohint.c
    src/bdf/bdf.c
    src/cache/ftcache.c
    src/cff/cff.c
    src/cid/type1cid.c
    src/gzip/ftgzip.c
    src/pcf/pcf.c
    src/pfr/pfr.c
    src/psaux/psaux.c
    src/pshinter/pshinter.c
    src/psnames/psmodule.c
    src/raster/raster.c
    src/sfnt/sfnt.c
    src/smooth/smooth.c
    src/truetype/truetype.c
    src/type1/type1.c
    src/type42/type42.c
    src/winfonts/winfnt.c

    # the following files may be unix only:
    src/base/ftbbox.c
    src/base/ftbdf.c
    src/base/ftpfr.c
    src/base/fttype1.c
    src/base/ftxf86.c
)

if (WIN32)
    target_sources(freetype PRIVATE 
        src/base/ftsystem.c 
        builds/win32/ftdebug.c
    )
else()
    # Currently assume all non-windows platforms are unix-y
    target_sources(freetype PRIVATE 
        builds/unix/ftsystem.c 
        src/base/ftdebug.c
    )
endif()

#target_link_libraries(freetype PUBLIC ZLIB::ZLIB BZip2::BZip2)

add_library(FreeType::FreeType ALIAS freetype)
