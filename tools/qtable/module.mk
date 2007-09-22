MODULE := tools/qtable

MODULE_DIRS += \
	tools/qtable/

TOOLS += \
	$(MODULE)/qtable$(EXEEXT)

MODULE_OBJS := \
	$(MODULE)/qtable.o

$(MODULE)/qtable$(EXEEXT): $(MODULE_OBJS)
	$(CXX) $(LDFLAGS) $+ -o $@
