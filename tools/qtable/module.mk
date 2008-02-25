# $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/tools/trunk/Makefile $
# $Id: Makefile 30664 2008-01-27 19:47:41Z jvprat $

MODULE := tools/qtable

MODULE_OBJS := \
	qtable.o

# Set the name of the executable
TOOL_EXECUTABLE := qtable

# Include common rules
include $(srcdir)/rules.mk
