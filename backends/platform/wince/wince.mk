backends/platform/wince/PocketSCUMM.o: $(srcdir)/backends/platform/wince/PocketSCUMM.rc
    $(QUIET)$(MKDIR) $(*D)
    $(WINDRES) $(WINDRESFLAGS) -I$(srcdir)/backends/platform/wince $< $@
