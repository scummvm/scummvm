ifdef DISABLE_SCUMM
DEFINES += -DDISABLE_SCUMM
else
MODULES += engines/scumm

ifdef DISABLE_SCUMM_7_8
DEFINES += -DDISABLE_SCUMM_7_8
endif

ifdef DISABLE_HE
DEFINES += -DDISABLE_HE
endif

endif

ifdef DISABLE_SIMON
DEFINES += -DDISABLE_SIMON
else
MODULES += engines/simon
endif

ifdef DISABLE_SKY
DEFINES += -DDISABLE_SKY
else
MODULES += engines/sky
endif

ifdef DISABLE_SWORD1
DEFINES += -DDISABLE_SWORD1
else
MODULES += engines/sword1
endif

ifdef DISABLE_SWORD2
DEFINES += -DDISABLE_SWORD2
else
MODULES += engines/sword2
endif

ifdef DISABLE_QUEEN
DEFINES += -DDISABLE_QUEEN
else
MODULES += engines/queen
endif

ifdef DISABLE_SAGA
DEFINES += -DDISABLE_SAGA
else 
MODULES += engines/saga
endif

ifdef DISABLE_KYRA
DEFINES += -DDISABLE_KYRA
else 
MODULES += engines/kyra
endif

ifdef DISABLE_GOB
DEFINES += -DDISABLE_GOB
else 
MODULES += engines/gob
endif

ifdef DISABLE_LURE
DEFINES += -DDISABLE_LURE
else
MODULES += engines/lure
endif

ifdef DISABLE_CINE
DEFINES += -DDISABLE_CINE
else 
MODULES += engines/cine
endif

ifdef DISABLE_AGI
DEFINES += -DDISABLE_AGI
else 
MODULES += engines/agi
endif

