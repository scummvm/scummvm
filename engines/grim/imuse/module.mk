MODULE := engine/imuse

MODULE_OBJS := \
	imuse.o \
	imuse_mcmp_mgr.o \
	imuse_music.o \
	imuse_script.o \
	imuse_sndmgr.o \
	imuse_tables.o \
	imuse_track.o

# Include common rules
include $(srcdir)/rules.mk
