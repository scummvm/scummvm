MODULE := common/formats

MODULE_OBJS := \
	cue.o \
	iso9660.o \
	iso9660archive.o \
	disk_image.o \
	formatinfo.o \
	iff_container.o \
	ini-file.o \
	json.o \
	markdown.o \
	po_parser.o \
	prodos.o \
	quicktime.o \
	winexe.o \
	winexe_ne.o \
	winexe_pe.o \
	xmlparser.o

# Include common rules
include $(srcdir)/rules.mk
