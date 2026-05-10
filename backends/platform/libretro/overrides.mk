# Specific platform overrides, to be dropped asap...

# -DEMSCRIPTEN flag should not be needed for emscripten build of libretro core, as it is a specific Project flag
# needed for emscripten backend. Anyways in several places, both in ScummVM and libretro-common,
# it is used as platform flag instead of __EMSCRIPTEN__, hence we need to keep EMSCRIPTEN flag for now, and apply
# the following workaround to solve relevant conflicts of the libretro core with emscripten backend at build time.
ifeq ($(platform), emscripten)
backends/fs/emscripten/%.o:
	@:
backends/libbackends.a:
	@:
MODULE_OBJS-backends := $(filter-out backends/fs/emscripten/%,$(MODULE_OBJS-backends))
backends/libbackends.a: $(MODULE_OBJS-backends)
	$(AR) $@ $(MODULE_OBJS-backends)
	$(RANLIB) $@

$(SCUMMVM_PATH)/gui/textviewer.o: CXXFLAGS += -UEMSCRIPTEN
$(SCUMMVM_PATH)/gui/textviewer.o: CFLAGS   += -UEMSCRIPTEN
$(SCUMMVM_PATH)/gui/textviewer.o: CPPFLAGS += -UEMSCRIPTEN
endif
