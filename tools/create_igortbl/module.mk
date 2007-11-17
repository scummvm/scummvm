MODULE := tools/create_igortbl

MODULE_DIRS += \
	tools/create_igortbl/

TOOLS += \
	$(MODULE)/create_igortbl$(EXEEXT)

MODULE_OBJS := \
	$(MODULE)/create_igortbl.o

$(MODULE)/create_igortbl$(EXEEXT): $(MODULE_OBJS)
	$(CXX) $(LDFLAGS) $+ -o $@
