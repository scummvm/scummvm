cmake_minimum_required(VERSION 3.23)

project(devtools)

include_directories("${CMAKE_SOURCE_DIR}/..")

# Create Access Tool
add_executable(
  create_access create_access/create_access_dat.cpp create_access/amazon_resources.cpp
                create_access/martian_resources.cpp
)

# TODO: Create Achivement Tool

# TODO: Create Bagel Tool

# TODO: Create Blade Runner Tool

# Create Cryo Tool
add_executable(create_cryo_dat create_cryo/create_cryo_dat.cpp)

# Create Cryomni3d Tool
add_executable(
  create_cryomni3d_dat create_cryomni3d/create_cryomni3d_dat.cpp create_cryomni3d/util.cpp
                       create_cryomni3d/versailles.cpp
)

# Create Drascula Tool
add_executable(create_drascula create_drascula/create_drascula.cpp)

# TODO: Create Encoding Tool

# TODO: Create Engine Tool

# Create Freescape Tool
file(
  GLOB
  ARCHIVE_FILES
  "create_freescape/*.bmp"
  "create_freescape/*.wav"
  "create_freescape/version"
)

# TODO: Fix the archive folders...
file(
  ARCHIVE_CREATE
  OUTPUT
  "${CMAKE_BINARY_DIR}/freescape.dat"
  PATHS
  ${ARCHIVE_FILES}
  FORMAT
  "zip"
)

# Create Glk Tool
add_executable(create_glk create_glk/create_glk.cpp)

# Create Hugo Tool
add_executable(create_hugo create_hugo/create_hugo.cpp)

# Create Kyra Tool
add_executable(
  create_kyradat
  create_kyradat/create_kyradat.cpp
  create_kyradat/games.cpp
  create_kyradat/md5.cpp
  create_kyradat/pak.cpp
  create_kyradat/resources.cpp
  create_kyradat/types.cpp
  create_kyradat/util.cpp
)

# Create Lure Tool
add_executable(create_lure create_lure/create_lure_dat.cpp create_lure/process_actions.cpp)

# TODO Create MacGUI Tool

# TOOO: Create Macventure Tool

# Create mm/xeen Tool
add_executable(
  create_xeen
  create_mm/create_xeen/create_xeen.cpp
  create_mm/create_xeen/cc.cpp
  create_mm/create_xeen/clouds.cpp
  create_mm/create_xeen/constants.cpp
  create_mm/create_xeen/hashmap.cpp
  create_mm/create_xeen/map.cpp
  create_mm/create_xeen/memorypool.cpp
  create_mm/create_xeen/str.cpp
  create_mm/create_xeen/swords.cpp
)

# Create Mort Dat Tool
add_executable(create_mortdat create_mortdat/create_mortdat.cpp)

# Create Myst3 Tool
add_executable(create_myst3 create_myst3/create_myst3.cpp)

# Create Nancy Tool
add_executable(
  create_nancy create_nancy/cif.cpp create_nancy/create_nancy.cpp create_nancy/file.cpp
)

# Create Neverhood Tool
add_executable(
  create_neverhood create_neverhood/create_neverhood.cpp create_neverhood/md5.cpp
                   create_neverhood/util.cpp
)

# TODO: Create Pink Tool

# TODO: Create Project Tool

# Create Supernova Tools
add_executable(
  create_supernova create_supernova/file.cpp create_supernova/po_parser.cpp
                   create_supernova/create_supernova.cpp
)
add_executable(create_image create_supernova/create_image/create_image.cpp)

# Create Teen Agent Tool
add_executable(create_teenagent create_teenagent/create_teenagent.cpp create_teenagent/util.cpp)

# Create Titanic Tool
find_package(ZLIB)
if(ZLIB_FOUND)
  add_executable(create_titanic)
  target_sources(
    create_titanic
    PRIVATE create_titanic/create_titanic_dat.cpp
            create_titanic/hashmap.cpp
            create_titanic/memorypool.cpp
            create_titanic/script_preresponses.cpp
            create_titanic/script_quotes.cpp
            create_titanic/script_ranges.cpp
            create_titanic/script_responses.cpp
            create_titanic/script_states.cpp
            create_titanic/base-str.cpp
            create_titanic/str.cpp
            create_titanic/tag_maps.cpp
            create_titanic/winexe.cpp
            create_titanic/winexe_pe.cpp
            create_titanic/zlib.cpp
  )
  target_link_libraries(create_titanic PRIVATE ZLIB::ZLIB)
endif()

# Create Tony Tool
add_executable(create_tony create_tony/create_tony.cpp)

# Create Translations Tool
add_executable(
  create_translations create_translations/po_parser.cpp create_translations/create_translations.cpp
)

# Create Ultima Tool
add_executable(
  create_ultima create_ultima/create_ultima.cpp create_ultima/file.cpp
                create_ultima/ultima4_resources.cpp
)

# TODO: Create Ultima 8 Tool

# Extract Mort Tool
add_executable(extract_mort extract_mort/extract_mort.cpp)

# TODO: Packaging Tool

# QTable Tool
add_executable(qtable qtable/qtable.cpp)

# TODO: SCI Tool

# Skycpt Tool
add_executable(
  skycpt
  skycpt/AsciiCptCompile.cpp
  skycpt/KmpSearch.cpp
  skycpt/TextFile.cpp
  skycpt/cptcompiler.cpp
  skycpt/cpthelp.cpp
  skycpt/idFinder.cpp
)

# TODO: Targets from module.mk - md5table, convbdf, make-scumm-fontdata, wwwroot, fonts.dat,
# credits, md5scummm
