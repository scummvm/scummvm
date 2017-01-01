# ============================================================================
#  Name	 : help.mk
#  Part of  : ScummVM
#
#  Description: This is file for creating .hlp file
# 
# ============================================================================


makmake :
	cshlpcmp ScummVM.xml

ifeq (WINS,$(findstring WINS, $(PLATFORM)))
	copy ScummVM.hlp $(EPOCROOT)epoc32\$(PLATFORM)\c\resource\help
endif

clean :
	del ScummVM.hlp
	del ScummVM.hlp.hrh

ifeq (WINS,$(findstring WINS, $(PLATFORM)))
	copy ScummVM.hlp $(EPOCROOT)epoc32\$(PLATFORM)\c\resource\help
endif

bld freeze lib cleanlib final resource savespace releasables :

