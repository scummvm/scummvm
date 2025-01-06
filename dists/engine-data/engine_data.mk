ifdef ENABLE_ACCESS
DIST_FILES_LIST += dists/engine-data/access.dat
endif
ifdef ENABLE_BAGEL
DIST_FILES_LIST += dists/engine-data/bagel.dat
endif
ifdef ENABLE_CRYO
DIST_FILES_LIST += dists/engine-data/cryo.dat
endif
ifdef ENABLE_CRYOMNI3D
DIST_FILES_LIST += dists/engine-data/cryomni3d.dat
endif
ifdef ENABLE_DARKSEED
DIST_FILES_LIST += dists/engine-data/darkseed.dat
endif
ifdef ENABLE_DRASCULA
DIST_FILES_LIST += dists/engine-data/drascula.dat
endif
ifdef ENABLE_HADESCH
DIST_FILES_LIST += dists/engine-data/hadesch_translations.dat
endif
ifdef ENABLE_HUGO
DIST_FILES_LIST += dists/engine-data/hugo.dat
endif
ifdef ENABLE_KYRA
DIST_FILES_LIST += dists/engine-data/kyra.dat
endif
ifdef ENABLE_LURE
DIST_FILES_LIST += dists/engine-data/lure.dat
endif
ifdef ENABLE_MACVENTURE
DIST_FILES_LIST += dists/engine-data/macventure.dat
endif
ifdef ENABLE_MORTEVIELLE
DIST_FILES_LIST += dists/engine-data/mort.dat
endif
ifdef ENABLE_NANCY
DIST_FILES_LIST += dists/engine-data/nancy.dat
endif
ifdef ENABLE_NEVERHOOD
DIST_FILES_LIST += dists/engine-data/neverhood.dat
endif
ifdef ENABLE_PRINCE
DIST_FILES_LIST += dists/engine-data/prince_translation.dat
endif
ifdef ENABLE_QUEEN
DIST_FILES_LIST += dists/engine-data/queen.tbl
endif
ifdef ENABLE_SKY
DIST_FILES_LIST += dists/engine-data/sky.cpt
endif
ifdef ENABLE_SUPERNOVA
DIST_FILES_LIST += dists/engine-data/supernova.dat
endif
ifdef ENABLE_TEENAGENT
DIST_FILES_LIST += dists/engine-data/teenagent.dat
endif
ifdef ENABLE_TITANIC
DIST_FILES_LIST += dists/engine-data/titanic.dat
endif
ifdef ENABLE_TONY
DIST_FILES_LIST += dists/engine-data/tony.dat
endif
ifdef ENABLE_TOON
DIST_FILES_LIST += dists/engine-data/toon.dat
endif
ifdef ENABLE_ULTIMA
DIST_FILES_LIST += dists/engine-data/ultima.dat
DIST_FILES_LIST += dists/engine-data/ultima8.dat
endif
ifdef ENABLE_WINTERMUTE
DIST_FILES_LIST += dists/engine-data/wintermute.zip
endif
ifdef ENABLE_MM
DIST_FILES_LIST += dists/engine-data/mm.dat
endif
ifdef ENABLE_FREESCAPE
DIST_FILES_LIST += dists/engine-data/freescape.dat
endif
ifdef USE_FREETYPE2
DIST_FILES_LIST += dists/engine-data/fonts.dat
endif
ifdef ENABLE_GRIM
DIST_FILES_LIST += dists/engine-data/grim-patch.lab
endif
ifdef ENABLE_MONKEY4
DIST_FILES_LIST += dists/engine-data/monkey4-patch.m4b
endif
ifdef ENABLE_MYST3
DIST_FILES_LIST += dists/engine-data/myst3.dat
endif


# pred.dic is currently only used for the AGI engine
ifdef ENABLE_AGI
DIST_FILES_LIST += dists/pred.dic
endif
