MODULE := engines/glk/detection

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# Sub-engine detection objects
DETECT_OBJS += $(MODULE)/../adrift/detection.o
DETECT_OBJS += $(MODULE)/../advsys/detection.o
DETECT_OBJS += $(MODULE)/../agt/detection.o
DETECT_OBJS += $(MODULE)/../alan2/detection.o
DETECT_OBJS += $(MODULE)/../alan3/detection.o
DETECT_OBJS += $(MODULE)/../archetype/detection.o
DETECT_OBJS += $(MODULE)/../comprehend/detection.o
DETECT_OBJS += $(MODULE)/../glulx/detection.o
DETECT_OBJS += $(MODULE)/../hugo/detection.o
DETECT_OBJS += $(MODULE)/../jacl/detection.o
DETECT_OBJS += $(MODULE)/../level9/detection.o
DETECT_OBJS += $(MODULE)/../magnetic/detection.o
DETECT_OBJS += $(MODULE)/../quest/detection.o
DETECT_OBJS += $(MODULE)/../scott/detection.o
DETECT_OBJS += $(MODULE)/../tads/detection.o
DETECT_OBJS += $(MODULE)/../zcode/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_GLK), STATIC_PLUGIN)
# Dependencies of detection objects
DETECT_OBJS += $(MODULE)/../blorb.o
DETECT_OBJS += $(MODULE)/../advsys/game.o
endif
