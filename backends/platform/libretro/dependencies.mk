######################################################################
# Common settings and functions
######################################################################

# Submodules functions
DEPS_SUBMODULES             := libretro-deps libretro-common

DEPS_FOLDER_libretro-deps   := libretro-deps
DEPS_URL_libretro-deps      := https://github.com/libretro/libretro-deps
DEPS_COMMIT_libretro-deps   := e362764d603eb4fa84560f2d009fef091f6d1447

DEPS_FOLDER_libretro-common := libretro-common
DEPS_URL_libretro-common    := https://github.com/libretro/libretro-common
DEPS_COMMIT_libretro-common := 20a43ba79fe6b4ec094b3b20b7bc88f4cfe916fa

submodule_test  = $(if $(shell result=$$($(SCRIPTS_PATH)/configure_submodules.sh $(DEPS_URL_$(1)) $(DEPS_COMMIT_$(1)) $(DEPS_PATH) $(DEBUG_ALLOW_DIRTY_SUBMODULES) $(DEPS_FOLDER_$(1))) ; { [ -z $$result ] || [ ! $$result = 0 ] ; } && printf error),$(1))
$(info Configuring submodules...)
SUBMODULE_FAILED = $(strip $(findstring $(foreach SUBMODULE,$(DEPS_SUBMODULES),$(call submodule_test,$(SUBMODULE))),$(DEPS_SUBMODULES)))

ifneq ($(SUBMODULE_FAILED),)
   $(error Configuration of following submodules failed: $(SUBMODULE_FAILED))
else
   $(info - Submodules configured)
endif

# Shared libs functions
this_lib_available := no
sharedlibs_test_cc            = '\#include <$(this_lib_subpath)$(this_lib_header)>\nint main(){return 0;}'
sharedlibs_get_include_path   = $(shell printf $(sharedlibs_test_cc) | $(CC) -E -Wp,-v - 2>/dev/null | grep "$(this_lib_subpath)$(this_lib_header)" | cut -d \" -f 2 | sed "s|/$(this_lib_header)||")
sharedlibs_this_lib_includes  = $(if $(this_lib_subpath),-I$(call sharedlibs_get_include_path))
sharedlibs_is_lib_available   = $(if $(shell result=$$(printf $(sharedlibs_test_cc) | $(CC) -xc -Wall -O -o /dev/null $(this_lib_flags) $(sharedlibs_this_lib_includes) - > /dev/null 2>&1 ; printf $$?) ;  { [ -z $$result ] || [ ! $$result = 0 ] ; } && printf error),no,yes)
sharedlibs_system_lib_message = $(info - Use system shared $(shell printf ' $(this_lib_flags)' | sed -e "s|.*-l||" -e "s| .*||"): $(this_lib_available))

######################################################################
#  libretro-common settings
######################################################################

INCLUDES  += -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/include \
	-I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/include/compat
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/file/file_path_io.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/file/file_path.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/file/retro_dirent.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/vfs/vfs_implementation.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/string/stdstring.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/time/rtime.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/streams/file_stream.o

ifeq ($(USE_LIBCO), 1)
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/libco/libco.o
ifeq ($(platform), genode)
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/libco/genode.o
endif
else
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/rthreads/rthreads.o
endif

ifneq ($(STATIC_LINKING), 1)
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/encodings/encoding_utf.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/compat/fopen_utf8.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-common)/compat/compat_strl.o
endif

######################################################################
# fluidsynth settings
######################################################################

ifeq ($(USE_FLUIDSYNTH), 1)
DEFINES += -DUSE_FLUIDSYNTH
this_lib_subpath :=
this_lib_header := fluidsynth.h
this_lib_flags := -lfluidsynth
include $(ROOT_PATH)/sharedlib_test.mk
ifneq ($(this_lib_available), yes)
DEFINES += -DUSE_FLUIDLITE
INCLUDES += -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/include \
	-I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src \
	-I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/include \
	-I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libogg/include
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_chan.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_chorus.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_conv.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_defsfont.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_dsp_float.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_gen.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_hash.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_list.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_mod.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_ramsfont.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_rev.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_settings.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_synth.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_sys.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_tuning.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/fluidsynth/src/fluid_voice.o
endif
endif

######################################################################
# libFLAC settings
######################################################################

ifeq ($(USE_FLAC), 1)
DEFINES += -DUSE_FLAC
this_lib_subpath :=
this_lib_header := FLAC/format.h
this_lib_flags := -lFLAC
include $(ROOT_PATH)/sharedlib_test.mk
ifneq ($(this_lib_available), yes)
INCLUDES += -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/include
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/bitreader.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/cpu.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/crc.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/fixed.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/format.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/lpc.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/md5.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/memory.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/metadata_object.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/stream_decoder.o

ifeq ($(platform), win)
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libFLAC/share/win_utf8_io/win_utf8_io.o \
	 $(SCUMMVM_PATH)/backends/platform/sdl/win32/win32_wrapper.o
endif
endif
endif

######################################################################
# libvorbis settings
######################################################################

ifeq ($(USE_VORBIS), 1)
DEFINES += -DUSE_VORBIS
this_lib_subpath :=
this_lib_header := vorbis/codec.h
this_lib_flags := -lvorbis
include $(ROOT_PATH)/sharedlib_test.mk
ifneq ($(this_lib_available), yes)
INCLUDES += -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libogg/include \
	-I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/include \
	-I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libogg/src/bitwise.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libogg/src/framing.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/analysis.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/bitrate.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/block.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/codebook.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/envelope.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/floor0.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/floor1.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/info.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/lookup.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/lpc.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/lsp.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/mapping0.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/mdct.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/psy.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/registry.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/res0.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/sharedbook.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/smallft.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/synthesis.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/vorbisenc.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/vorbisfile.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libvorbis/lib/window.o
endif
endif

######################################################################
# tremor settings
######################################################################

ifeq ($(USE_TREMOR), 1)
DEFINES += -DUSE_TREMOR -DUSE_VORBIS
this_lib_subpath :=
this_lib_header := tremor/ivorbiscodec.h
this_lib_flags := -ltremor
include $(ROOT_PATH)/sharedlib_test.mk
ifneq ($(this_lib_available), yes)
INCLUDES  += -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/bitwise.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/block.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/codebook.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/floor0.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/floor1.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/framing.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/info.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/mapping0.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/mdct.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/registry.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/res012.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/sharedbook.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/synthesis.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/vorbisfile.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/tremor/window.o
endif
endif

######################################################################
# libz settings
######################################################################

ifeq ($(USE_ZLIB), 1)
DEFINES += -DUSE_ZLIB  -DWANT_ZLIB
this_lib_subpath :=
this_lib_header := zlib.h
this_lib_flags := -lz
include $(ROOT_PATH)/sharedlib_test.mk
ifneq ($(this_lib_available), yes)
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/deflate.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/gzlib.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/uncompr.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/zutil.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/inffast.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/gzread.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/crc32.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/gzwrite.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/inflate.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/infback.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/inftrees.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/trees.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/gzclose.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/compress.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libz/adler32.o
endif
endif

######################################################################
# libmad settings
######################################################################

ifeq ($(USE_MAD), 1)
DEFINES += -DUSE_MAD -DFPM_DEFAULT
this_lib_subpath :=
this_lib_header := mad.h
this_lib_flags := -lmad
include $(ROOT_PATH)/sharedlib_test.mk
ifneq ($(this_lib_available), yes)
INCLUDES += -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libmad
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libmad/bit.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libmad/decoder.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libmad/frame.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libmad/huffman.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libmad/layer12.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libmad/layer3.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libmad/stream.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libmad/synth.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libmad/timer.o
endif
endif

######################################################################
# libfaad settings
######################################################################

ifeq ($(USE_FAAD), 1)
DEFINES += -DUSE_FAAD
this_lib_subpath :=
this_lib_header := faad.h
this_lib_flags := -lfaad
include $(ROOT_PATH)/sharedlib_test.mk
ifneq ($(this_lib_available), yes)
INCLUDES += -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/include -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/bits.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/cfft.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/common.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/decoder.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/drc.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/error.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/filtbank.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/hcr.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/huffman.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/ic_predict.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/is.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/lt_predict.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/mdct.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/mp4.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/ms.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/output.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/pns.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/pulse.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/ps_dec.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/ps_syntax.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/rvlc.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/sbr_dct.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/sbr_dec.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/sbr_e_nf.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/sbr_fbt.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/sbr_hfadj.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/sbr_hfgen.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/sbr_huff.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/sbr_qmf.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/sbr_syntax.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/sbr_tf_grid.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/specrec.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/syntax.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libfaad/libfaad/tns.o
endif
endif

######################################################################
# libpng settings
######################################################################

ifeq ($(USE_PNG), 1)
DEFINES += -DUSE_PNG
this_lib_subpath :=
this_lib_header := png.h
this_lib_flags := -lpng
include $(ROOT_PATH)/sharedlib_test.mk
ifneq ($(this_lib_available), yes)
INCLUDES += -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/png.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngerror.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngget.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngmem.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngpread.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngread.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngrio.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngrtran.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngrutil.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngset.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngtrans.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngwrite.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngwutil.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngwtran.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libpng/pngwio.o
endif
endif

######################################################################
# libjpeg settings
######################################################################

ifeq ($(USE_JPEG), 1)
DEFINES += -DUSE_JPEG -DJDCT_DEFAULT=JDCT_IFAST
this_lib_subpath :=
this_lib_header := jerror.h
this_lib_flags := -ljpeg
include $(ROOT_PATH)/sharedlib_test.mk
ifneq ($(this_lib_available), yes)
INCLUDES += -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jaricom.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcapimin.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcapistd.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcarith.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jccoefct.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jccolor.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcdctmgr.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcinit.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jchuff.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcmarker.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcmainct.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcmaster.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcomapi.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcphuff.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcprepct.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jcsample.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdapimin.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdapistd.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdarith.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdcoefct.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdcolor.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jddctmgr.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdhuff.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdinput.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdmarker.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdmainct.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdmaster.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdmerge.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdphuff.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdpostct.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdsample.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jdtrans.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jerror.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jidctflt.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jidctfst.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jidctint.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jidctred.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jfdctflt.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jfdctfst.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jfdctint.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jmemmgr.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jmemnobs.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jquant1.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jquant2.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jutils.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/libjpeg/jsimd_none.o
endif
endif

######################################################################
# theora settings
######################################################################

ifeq ($(USE_THEORADEC), 1)
DEFINES += -DUSE_THEORADEC
this_lib_subpath :=
this_lib_header := theora/theoradec.h
this_lib_flags := -ltheora
include $(ROOT_PATH)/sharedlib_test.mk
ifneq ($(this_lib_available), yes)
INCLUDES += -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/include
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/lib/bitpack.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/lib/decinfo.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/lib/decode.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/lib/dequant.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/lib/fragment.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/lib/huffdec.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/lib/idct.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/lib/info.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/lib/internal.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/lib/quant.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/theora/lib/state.o
endif
else
#undefine as in ScummVM macro definition is tested to enable theora
USE_THEORADEC =
endif

######################################################################
# freetype settings
######################################################################

ifeq ($(USE_FREETYPE2), 1)
DEFINES += -DUSE_FREETYPE2
# ft2build.h is included in scummvm sources, while freetype2/ft2build.h is available in includes path
this_lib_subpath := freetype2/
this_lib_header := ft2build.h
this_lib_flags := -lfreetype
include $(ROOT_PATH)/sharedlib_test.mk
ifneq ($(this_lib_available), yes)
DEFINES += -DFT2_BUILD_LIBRARY
INCLUDES += -I$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/include
OBJS_DEPS += $(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afangles.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afblue.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afcjk.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afdummy.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afglobal.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afhints.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afindic.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/aflatin.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afloader.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afmodule.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afpic.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afranges.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afshaper.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/autofit/afwarp.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/basepic.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftadvanc.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftapi.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftbitmap.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftcalc.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftgloadr.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftglyph.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/fthash.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftinit.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftsnames.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftobjs.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftsystem.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftoutln.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftrfork.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftstream.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftstroke.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/fttrigon.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/base/ftutil.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/bdf/bdfdrivr.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/bdf/bdflib.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cid/cidriver.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cid/cidgload.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cid/cidload.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cid/cidobjs.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cid/cidparse.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cf2arrst.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cf2blues.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cf2error.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cf2ft.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cf2font.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cf2hints.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cf2intrp.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cf2read.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cf2stack.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cffcmap.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cffgload.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cffload.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cffobjs.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cffparse.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cffpic.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/cff/cffdrivr.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/gzip/ftgzip.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/lzw/ftlzw.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pcf/pcfdrivr.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pcf/pcfread.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pcf/pcfutil.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pfr/pfrcmap.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pfr/pfrdrivr.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pfr/pfrgload.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pfr/pfrload.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pfr/pfrobjs.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pfr/pfrsbit.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/psaux/afmparse.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/psaux/psconv.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/psaux/psobjs.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/psaux/t1cmap.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/psaux/t1decode.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/psaux/psauxmod.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pshinter/pshalgo.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pshinter/pshglob.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pshinter/pshmod.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pshinter/pshpic.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/pshinter/pshrec.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/psnames/psmodule.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/raster/ftrend1.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/raster/ftraster.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/raster/rastpic.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/sfnt/sfdriver.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/sfnt/sfntpic.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/sfnt/sfobjs.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/sfnt/ttbdf.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/sfnt/ttcmap.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/sfnt/ttkern.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/sfnt/ttload.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/sfnt/ttmtx.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/sfnt/ttpost.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/sfnt/ttsbit.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/smooth/ftgrays.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/smooth/ftsmooth.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/type1/t1afm.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/type1/t1driver.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/type1/t1gload.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/type1/t1load.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/type1/t1objs.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/type1/t1parse.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/type42/t42drivr.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/type42/t42objs.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/type42/t42parse.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/truetype/ttdriver.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/truetype/ttgload.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/truetype/ttgxvar.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/truetype/ttinterp.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/truetype/ttobjs.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/truetype/ttpload.o \
	$(DEPS_PATH)/$(DEPS_FOLDER_libretro-deps)/freetype/src/winfonts/winfnt.o
endif
endif

######################################################################
# libcurl settings
######################################################################

ifeq ($(USE_CLOUD), 1)
this_lib_available := no
this_lib_subpath :=
this_lib_header := curl/curl.h
this_lib_flags := -lcurl
# No baked-in solution in libretro-deps, shared lib is the only option at this time
# ifeq ($(USE_SYSTEM_LIBS), 1)
this_lib_available := $(call sharedlibs_is_lib_available)
# endif
$(call sharedlibs_system_lib_message)
ifeq ($(this_lib_available), yes)
	LDFLAGS += $(this_lib_flags)
	INCLUDES += $(sharedlibs_this_lib_includes)
	USE_LIBCURL := 1
	DEFINES += -DUSE_CLOUD -DUSE_LIBCURL
else
$(info System libcurl not available, dropping cloud feature.)
endif
endif
