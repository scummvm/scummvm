
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

#
# Build rules for the devtools
#

#
# Rules to explicitly rebuild the credits / MD5 tables.
# The rules for the files in the "web" resp. "docs" modules
# assume that you are invoking "make" from within a complete
# checkout of the ScummVM repository. Of course if that is not the
# case, then you have to modify those paths...
#

credits:
	$(srcdir)/devtools/credits.pl --text > $(srcdir)/AUTHORS
#	$(srcdir)/devtools/credits.pl --rtf > $(srcdir)/Credits.rtf
	$(srcdir)/devtools/credits.pl --cpp > $(srcdir)/gui/credits.h
	$(srcdir)/devtools/credits.pl --xml-website > $(srcdir)/../../web/trunk/data/credits.xml
#	$(srcdir)/devtools/credits.pl --xml-docbook > $(srcdir)/../../docs/trunk/docbook/credits.xml


.PHONY: clean-devtools devtools credits
