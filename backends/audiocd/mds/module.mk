MODULE := backends/audiocd/mds

MODULE_OBJS := \
	mds-disc.o

# Include common rules
include $(srcdir)/rules.mk
