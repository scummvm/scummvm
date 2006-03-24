MODULE := tools

MODULE_DIRS += \
	tools


#######################################################################
# Tools directory
#######################################################################

TOOLS := tools/convbdf$(EXEEXT) tools/md5table$(EXEEXT)


# Make sure the 'all' / 'clean' targets build/clean the tools, too
all: tools
clean: clean-tools

# Main target
tools: $(TOOLS)

clean-tools:
	-$(RM) $(TOOLS)

#
# Build rules for the tools
#

tools/convbdf$(EXEEXT): $(srcdir)/tools/convbdf.c
	$(MKDIR) tools/$(DEPDIR)
	$(CC) -Wall -o $@ $<

tools/md5table$(EXEEXT): $(srcdir)/tools/md5table.c
	$(MKDIR) tools/$(DEPDIR)
	$(CC) -Wall -o $@ $<

#
# Rules to explicitly rebuild the credits / MD5 tables
#

credits:
	$(srcdir)/tools/credits.pl --text > AUTHORS
	$(srcdir)/tools/credits.pl --tex > doc/10.tex
	$(srcdir)/tools/credits.pl --rtf > Credits.rtf
	$(srcdir)/tools/credits.pl --cpp > gui/credits.h
	$(srcdir)/tools/credits.pl --html > ../../web/trunk/credits.inc
	$(srcdir)/tools/credits.pl --xml > ../../docs/trunk/docbook/credits.xml

md5scumm: tools/md5table$(EXEEXT)
	tools/md5table$(EXEEXT) --c++ < $(srcdir)/tools/scumm-md5.txt > engines/scumm/scumm-md5.h
	tools/md5table$(EXEEXT) --php < $(srcdir)/tools/scumm-md5.txt > ../web/docs/md5.inc


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



.PHONY: clean-tools tools credits md5scumm md5simon
