
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
	$(srcdir)/devtools/credits.pl --xml-website > $(srcdir)/../../residualvm-web/data/credits.xml
#	$(srcdir)/devtools/credits.pl --xml-docbook > $(srcdir)/../../docs/trunk/docbook/credits.xml


#
# Rules which automatically and implicitly rebuild the credits and
# MD5 tables when needed.
# These are currently disabled, because if the input data changes, then
# the generated files should be checked in, too. Otherwise, we'd reduce
# portability to system on which our devtools can't (automatically) be
# run for some reason.
#

#scumm/scumm-md5.h: $(srcdir)/devtools/scumm-md5.txt devtools/md5table$(EXEEXT)
#	devtools/md5table$(EXEEXT) --c++ < $< > $@

#AUTHORS: $(srcdir)/devtools/credits.pl
#	$(srcdir)/devtools/credits.pl --text > $@

#gui/credits.h: $(srcdir)/devtools/credits.pl
#	$(srcdir)/devtools/credits.pl --cpp > $@
.PHONY: clean-devtools devtools credits
