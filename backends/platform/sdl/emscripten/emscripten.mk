
# Special generic target for emscripten static file hosting bundle
dist-emscripten: $(EXECUTABLE) $(PLUGINS)
	mkdir -p ./build-emscripten/data
	mkdir -p ./build-emscripten/data/games
	mkdir -p ./build-emscripten/doc
	cp $(EXECUTABLE) ./build-emscripten/
	cp $(EXECUTABLE:html=wasm) ./build-emscripten/
	cp $(EXECUTABLE:html=js) ./build-emscripten/
	cp $(DIST_FILES_DOCS) ./build-emscripten/doc
	cp $(DIST_FILES_THEMES) ./build-emscripten/data
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ./build-emscripten/data
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) ./build-emscripten/data
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) ./build-emscripten/data
endif
ifdef DIST_FILES_SHADERS
	mkdir -p ./build-emscripten/data/shaders
	cp $(DIST_FILES_SHADERS) ./build-emscripten/data/shaders
endif
ifeq ($(DYNAMIC_MODULES),1)
	mkdir -p ./build-emscripten/data/plugins
	@for i in $(PLUGINS); do cp $$i ./build-emscripten/data/plugins; done
endif
	$(EMSDK_NODE) "$(srcdir)/dists/emscripten/build-make_http_index.js" ./build-emscripten/data
	cp "$(srcdir)/dists/emscripten/assets/"* ./build-emscripten/
	cp "$(srcdir)/gui/themes/common-svg/logo.svg" ./build-emscripten/
	cp "$(srcdir)/icons/scummvm.ico" ./build-emscripten/favicon.ico
