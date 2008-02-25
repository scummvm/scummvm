# $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/tools/trunk/Makefile $
# $Id: Makefile 30664 2008-01-27 19:47:41Z jvprat $

MODULE := tools/create_lure

MODULE_OBJS := \
	create_lure_dat.o \
	process_actions.o

# Set the name of the executable
TOOL_EXECUTABLE := create_lure

# Include common rules
include $(srcdir)/rules.mk
