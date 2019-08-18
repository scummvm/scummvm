ifdef WRAP_MALLOC
	LDFLAGS += -Wl,--wrap,malloc -Wl,--wrap,free
endif

ifdef DYNAMIC_MODULES
plugins: $(EXECUTABLE) backends/platform/wince/stub.o backends/platform/wince/PocketSCUMM.o
	$(CXX) -nostdlib backends/platform/wince/stub.o backends/platform/wince/PocketSCUMM.o -L. -lscummvm -o scummvm.exe
	
backends/platform/wince/stub.o: $(srcdir)/backends/platform/wince/stub.cpp
	$(CXX) -c $(srcdir)/backends/platform/wince/stub.cpp -o backends/platform/wince/stub.o
endif
