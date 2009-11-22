# $URL$
# $Id$

MODULE := tools

MODULE_DIRS += \
	tools/


#######################################################################
# Tools directory
#######################################################################

TOOLS := \
	tools/convbdf$(EXEEXT) \
	tools/md5table$(EXEEXT) \
	tools/make-scumm-fontdata$(EXEEXT)

include $(srcdir)/tools/*/module.mk

.PHONY: $(srcdir)/tools/*/module.mk

# Make sure the 'all' / 'clean' targets build/clean the tools, too
#all:
clean: clean-tools

# Main target
tools: $(TOOLS)

clean-tools:
	-$(RM) $(TOOLS)

#
# Build rules for the tools
#

tools/convbdf$(EXEEXT): $(srcdir)/tools/convbdf.c
	$(QUIET)$(MKDIR) tools/$(DEPDIR)
	$(QUIET_LINK)$(CC) $(CFLAGS) -Wall -o $@ $<

tools/md5table$(EXEEXT): $(srcdir)/tools/md5table.c
	$(QUIET)$(MKDIR) tools/$(DEPDIR)
	$(QUIET_LINK)$(CC) $(CFLAGS) -Wall -o $@ $<

tools/make-scumm-fontdata$(EXEEXT): $(srcdir)/tools/make-scumm-fontdata.c
	$(QUIET)$(MKDIR) tools/$(DEPDIR)
	$(QUIET_LINK)$(CC) $(CFLAGS) -Wall -o $@ $<

#
# Rules to explicitly rebuild the credits / MD5 tables.
# The rules for the files in the "web" resp. "docs" modules
# assume that you are invoking "make" from within a complete
# checkout of the ScummVM repository. Of course if that is not the
# case, then you have to modify those paths...
#

credits:
	$(srcdir)/tools/credits.pl --text > AUTHORS
#	$(srcdir)/tools/credits.pl --rtf > Credits.rtf
	$(srcdir)/tools/credits.pl --cpp > gui/credits.h
	$(srcdir)/tools/credits.pl --xml-website > ../../web/trunk/data/credits.xml
#	$(srcdir)/tools/credits.pl --xml-docbook > ../../docs/trunk/docbook/credits.xml

md5scumm: tools/md5table$(EXEEXT)
	tools/md5table$(EXEEXT) --c++ < $(srcdir)/tools/scumm-md5.txt > engines/scumm/scumm-md5.h
	cp $(srcdir)/tools/scumm-md5.txt ../../web/trunk/data/scumm-md5.txt


#
# Rules which automatically and implicitly rebuild the credits and
# MD5 tables when needed.
# These are currently disabled, because if the input data changes, then
# the generated files should be checked in, too. Otherwise, we'd reduce
# portability to system on which our tools can't (automatically) be
# run for some reason.
#

#scumm/scumm-md5.h: $(srcdir)/tools/scumm-md5.txt tools/md5table$(EXEEXT)
#	tools/md5table$(EXEEXT) --c++ < $< > $@

#AUTHORS: $(srcdir)/tools/credits.pl
#	$(srcdir)/tools/credits.pl --text > $@

#gui/credits.h: $(srcdir)/tools/credits.pl
#	$(srcdir)/tools/credits.pl --cpp > $@


.PHONY: clean-tools tools credits md5scumm
