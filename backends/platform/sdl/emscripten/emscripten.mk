# use system python as fallback
# if EMSDK_PYTHON is not set
EMSDK_PYTHON ?= python3 


# Special generic target for emscripten static file hosting bundle
dist-emscripten: $(EXECUTABLE) $(PLUGINS)
	mkdir -p ./build-emscripten/
	mkdir -p ./build-emscripten/data
	mkdir -p ./build-emscripten/data/games
	mkdir -p ./build-emscripten/data/gui-icons
	mkdir -p ./build-emscripten/doc
	cp $(EXECUTABLE) ./build-emscripten/
	cp $(EXECUTABLE:html=wasm) ./build-emscripten/
	cp $(EXECUTABLE:html=js) ./build-emscripten/
	cp $(DIST_FILES_DOCS) ./build-emscripten/doc
	cp $(DIST_FILES_THEMES) ./build-emscripten/data
	zip -d ./build-emscripten/data/shaders.dat anti-aliasing/aa-shader-4.0.glslp anti-aliasing/reverse-aa.glslp cel/MMJ_Cel_Shader_MP.glslp \
		crt/crt-guest-dr-venom-fast.glslp crt/crt-guest-dr-venom.glslp crt/crt-hyllian-glow.glslp crt/crt-hyllian.glslp \
		crt/crtsim.glslp crt/gtuv50.glslp crt/yee64.glslp crt/yeetron.glslp cubic/cubic-gamma-correct.glslp \
		cubic/cubic.glslp denoisers/crt-fast-bilateral-super-xbr.glslp denoisers/fast-bilateral-super-xbr-4p.glslp \
		denoisers/fast-bilateral-super-xbr-6p.glslp denoisers/fast-bilateral-super-xbr.glslp dithering/bayer-matrix-dithering.glslp \
		dithering/gendither.glslp hqx/hq2x-halphon.glslp interpolation/bandlimit-pixel.glslp interpolation/controlled_sharpness.glslp \
		sabr/sabr-hybrid-deposterize.glslp scalefx/scalefx+rAA.glslp scalefx/scalefx-hybrid.glslp scalefx/scalefx.glslp \
		scalenx/scale2xSFX.glslp scalenx/scale3x.glslp sharpen/adaptive-sharpen-multipass.glslp sharpen/adaptive-sharpen.glslp \
		sharpen/super-xbr-super-res.glslp xbr/2xBR-lv1-multipass.glslp xbr/super-xbr-2p.glslp  xbr/super-xbr-3p-smoother.glslp \
		xbr/super-xbr-6p-adaptive.glslp xbr/super-xbr-6p-small-details.glslp xbr/super-xbr-6p.glslp xbr/super-xbr-deposterize.glslp \
		xbr/xbr-hybrid.glslp xbr/xbr-lv2-3d.glslp xbr/xbr-lv2-noblend.glslp xbr/xbr-lv2.glslp xbr/xbr-lv3-multipass.glslp \
		xbr/xbr-lv3.glslp xbr/xbr-mlv4-multipass.glslp 
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ./build-emscripten/data
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) ./build-emscripten/data
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) ./build-emscripten/data
endif
ifdef DIST_FILES_SOUNDFONTS
	cp $(DIST_FILES_SOUNDFONTS) ./build-emscripten/data
endif
ifdef DIST_FILES_SHADERS
	mkdir -p ./build-emscripten/data/shaders
	cp $(DIST_FILES_SHADERS) ./build-emscripten/data/shaders
endif
ifeq ($(DYNAMIC_MODULES),1)
	mkdir -p ./build-emscripten/data/plugins
	@for i in $(PLUGINS); do cp $$i ./build-emscripten/data/plugins; done
endif
	$(EMSDK_PYTHON) "$(srcdir)/dists/emscripten/build-make_http_index.py" ./build-emscripten/data
	cp "$(srcdir)/dists/emscripten/assets/"* ./build-emscripten/
	cp "$(srcdir)/gui/themes/common-svg/logo.svg" ./build-emscripten/
	cp "$(srcdir)/icons/scummvm.ico" ./build-emscripten/favicon.ico
