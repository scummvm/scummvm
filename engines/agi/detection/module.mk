MODULE := engines/agi/detection

MODULE_OBJS :=

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# External dependencies of detection.
# This is unneeded by the engine module itself,
# so seperate it completely.
DETECT_OBJS += $(MODULE)/wagparser.o
