all: PocketSCUMM.o

PocketSCUMM.o: $(srcdir)/backends/platform/wince/PocketSCUMM.rc
	mkdir -p backends/platform/wince
	$(WINDRES) -I$(srcdir)/backends/platform/wince $(srcdir)/backends/platform/wince/PocketSCUMM.rc backends/platform/wince/PocketSCUMM.o
