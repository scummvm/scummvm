# $URL$
# $Id$

MODULE := devtools

MODULE_DIRS += \
	devtools/

#######################################################################
# Tools directory
#######################################################################

DEVTOOLS := 

include $(srcdir)/devtools/*/module.mk

.PHONY: $(srcdir)/devtools/*/module.mk

# Make sure the 'all' / 'clean' targets build/clean the devtools, too
#all:
clean: clean-devtools

# Main target
devtools: $(DEVTOOLS)

clean-devtools:
	-$(RM) $(DEVTOOLS)

.PHONY: clean-devtools devtools
