MODULE := common/formats

MODULE_OBJS := \
	iff_container.o \
	ini-file.o \
	json.o \
	prodos.o \
	quicktime.o \
	winexe.o \
	winexe_ne.o \
	winexe_pe.o \
	xmlparser.o

# Include common rules
include $(srcdir)/rules.mk
