# $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/tools/trunk/Makefile $
# $Id: Makefile 30664 2008-01-27 19:47:41Z jvprat $

MODULE := tools/skycpt

MODULE_OBJS := \
	AsciiCptCompile.o \
	KmpSearch.o \
	TextFile.o \
	cptcompiler.o \
	cpthelp.o \
	idFinder.o

# Set the name of the executable
TOOL_EXECUTABLE := skycpt

# Include common rules
include $(srcdir)/rules.mk
