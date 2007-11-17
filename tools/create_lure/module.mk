MODULE := tools/create_lure

MODULE_DIRS += \
	tools/create_lure/

TOOLS += \
	$(MODULE)/create_lure$(EXEEXT)

MODULE_OBJS := \
	$(MODULE)/create_lure_dat.o \
	$(MODULE)/process_actions.o

$(MODULE)/create_lure$(EXEEXT): $(MODULE_OBJS)
	$(CXX) $(LDFLAGS) $+ -o $@
