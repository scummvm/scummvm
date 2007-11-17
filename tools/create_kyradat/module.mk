MODULE := tools/create_kyradat

MODULE_DIRS += \
	tools/create_kyradat/

TOOLS += \
	$(MODULE)/create_kyradat$(EXEEXT)

MODULE_OBJS := \
	$(MODULE)/create_kyradat.o \
	$(MODULE)/pak.o \
	$(MODULE)/md5.o \
	$(MODULE)/util.o

$(MODULE)/create_kyradat$(EXEEXT): $(MODULE_OBJS)
	$(CXX) $(LDFLAGS) $+ -o $@
