MODULE := engines/glk

MODULE_OBJS := \
	blorb.o \
	conf.o \
	debugger.o \
	detection.o \
	events.o \
	fonts.o \
	glk.o \
	glk_api.o \
	glk_dispa.o \
	pc_speaker.o \
	picture.o \
	quetzal.o \
	raw_decoder.o \
	screen.o \
	selection.o \
	sound.o \
	streams.o \
	time.o \
	unicode.o \
	unicode_gen.o \
	utils.o \
	windows.o \
	window_graphics.o \
	window_pair.o \
	window_text_buffer.o \
	window_text_grid.o

ifdef ENABLE_GLK_ADRIFT
MODULE_OBJS += \
	adrift/adrift.o \
	adrift/detection.o \
	adrift/os_glk.o \
	adrift/scdebug.o \
	adrift/scevents.o \
	adrift/scexpr.o \
	adrift/scgamest.o \
	adrift/scinterf.o \
	adrift/sclibrar.o \
	adrift/sclocale.o \
	adrift/scmemos.o \
	adrift/scnpcs.o \
	adrift/scobjcts.o \
	adrift/scparser.o \
	adrift/scprintf.o \
	adrift/scprops.o \
	adrift/scresour.o \
	adrift/screstrs.o \
	adrift/scrunner.o \
	adrift/sctaffil.o \
	adrift/sctafpar.o \
	adrift/sctasks.o \
	adrift/scutils.o \
	adrift/scvars.o \
	adrift/serialization.o \
	adrift/sxfile.o \
	adrift/sxglob.o \
	adrift/sxutils.o
endif

ifdef ENABLE_GLK_ADVSYS
MODULE_OBJS += \
	advsys/advsys.o \
	advsys/detection.o \
	advsys/game.o \
	advsys/glk_interface.o \
	advsys/vm.o
endif

ifdef ENABLE_GLK_ALAN2
MODULE_OBJS += \
	alan2/alan2.o \
	alan2/detection.o \
	alan2/alan_version.o \
	alan2/args.o \
	alan2/debug.o \
	alan2/decode.o \
	alan2/exe.o \
	alan2/glkio.o \
	alan2/inter.o \
	alan2/main.o \
	alan2/params.o \
	alan2/parse.o \
	alan2/reverse.o \
	alan2/rules.o \
	alan2/stack.o \
	alan2/sysdep.o \
	alan2/term.o \
	alan2/types.o
endif

ifdef ENABLE_GLK_ALAN3
MODULE_OBJS += \
	alan3/acode.o \
	alan3/act.o \
	alan3/actor.o \
	alan3/alan3.o \
	alan3/alan_version.o \
	alan3/alt_info.o \
	alan3/attribute.o \
	alan3/checkentry.o \
	alan3/class.o \
	alan3/compatibility.o \
	alan3/container.o \
	alan3/current.o \
	alan3/debug.o \
	alan3/decode.o \
	alan3/detection.o \
	alan3/dictionary.o \
	alan3/event.o \
	alan3/exe.o \
	alan3/fnmatch.o \
	alan3/glkio.o \
	alan3/instance.o \
	alan3/inter.o \
	alan3/lists.o \
	alan3/literal.o \
	alan3/location.o \
	alan3/main.o \
	alan3/memory.o \
	alan3/msg.o \
	alan3/options.o \
	alan3/output.o \
	alan3/parameter_position.o \
	alan3/params.o \
	alan3/parse.o \
	alan3/reverse.o \
	alan3/rules.o \
	alan3/save.o \
	alan3/scan.o \
	alan3/score.o \
	alan3/set.o \
	alan3/stack.o \
	alan3/state.o \
	alan3/state_stack.o \
	alan3/syntax.o \
	alan3/sysdep.o \
	alan3/syserr.o \
	alan3/types.o \
	alan3/utils.o \
	alan3/word.o
endif

ifdef ENABLE_GLK_FROTZ
MODULE_OBJS += \
	frotz/bitmap_font.o \
	frotz/config.o \
	frotz/detection.o \
	frotz/frotz.o \
	frotz/glk_interface.o \
	frotz/mem.o \
	frotz/pics.o \
	frotz/pics_decoder.o \
	frotz/processor.o \
	frotz/processor_buffer.o \
	frotz/processor_input.o \
	frotz/processor_maths.o \
	frotz/processor_mem.o \
	frotz/processor_objects.o \
	frotz/processor_screen.o \
	frotz/processor_streams.o \
	frotz/processor_table.o \
	frotz/processor_text.o \
	frotz/processor_variables.o \
	frotz/processor_windows.o \
	frotz/quetzal.o \
	frotz/screen.o \
	frotz/sound_folder.o \
	frotz/windows.o
endif

ifdef ENABLE_GLK_GLULXE
MODULE_OBJS += \
	glulxe/accel.o \
	glulxe/detection.o \
	glulxe/exec.o \
	glulxe/float.o \
	glulxe/funcs.o \
	glulxe/gestalt.o \
	glulxe/glkop.o \
	glulxe/glulxe.o \
	glulxe/heap.o \
	glulxe/operand.o \
	glulxe/search.o \
	glulxe/serial.o \
	glulxe/string.o \
	glulxe/vm.o
endif

ifdef ENABLE_GLK_HUGO
MODULE_OBJS += \
	hugo/detection.o \
	hugo/heexpr.o \
	hugo/heglk.o \
	hugo/hemedia.o \
	hugo/hemisc.o \
	hugo/heobject.o \
	hugo/heparse.o \
	hugo/heres.o \
	hugo/herun.o \
	hugo/heset.o \
	hugo/htokens.o \
	hugo/hugo.o \
	hugo/stringfn.o
endif

ifdef ENABLE_GLK_JACL
MODULE_OBJS += \
	jacl/detection.o \
	jacl/display.o \
	jacl/encapsulate.o \
	jacl/errors.o \
	jacl/findroute.o \
	jacl/glk_saver.o \
	jacl/interpreter.o \
	jacl/jacl.o \
	jacl/jacl_main.o \
	jacl/jpp.o \
	jacl/libcsv.o \
	jacl/loader.o \
	jacl/logging.o \
	jacl/parser.o \
	jacl/resolvers.o \
	jacl/utils.o
endif

ifdef ENABLE_GLK_MAGNETIC
MODULE_OBJS += \
	magnetic/detection.o \
	magnetic/emu.o \
	magnetic/graphics.o \
	magnetic/magnetic.o \
	magnetic/sound.o
endif

ifdef ENABLE_GLK_QUEST
MODULE_OBJS += \
	quest/detection.o \
	quest/geas_file.o \
	quest/geas_glk.o \
	quest/geas_runner.o \
	quest/geas_state.o \
	quest/geas_util.o \
	quest/quest.o \
	quest/read_file.o \
	quest/string.o \
	quest/streams.o
endif

ifdef ENABLE_GLK_SCOTT
MODULE_OBJS += \
	scott/detection.o \
	scott/scott.o
endif

ifdef ENABLE_GLK_TADS
MODULE_OBJS += \
	tads/detection.o \
	tads/os_banners.o \
	tads/os_buffer.o \
	tads/os_glk.o \
	tads/os_frob_tads.o \
	tads/os_parse.o \
	tads/tads.o \
	tads/tads2/built_in.o \
	tads/tads2/character_map.o \
	tads/tads2/command_line.o \
	tads/tads2/data.o \
	tads/tads2/debug.o \
	tads/tads2/error.o \
	tads/tads2/error_handling.o \
	tads/tads2/error_message.o \
	tads/tads2/execute_command.o \
	tads/tads2/file_io.o \
	tads/tads2/get_string.o \
	tads/tads2/line_source_file.o \
	tads/tads2/list.o \
	tads/tads2/ltk.o \
	tads/tads2/memory_cache.o \
	tads/tads2/memory_cache_heap.o \
	tads/tads2/memory_cache_swap.o \
	tads/tads2/object.o \
	tads/tads2/os.o \
	tads/tads2/output.o \
	tads/tads2/play.o \
	tads/tads2/post_compilation.o \
	tads/tads2/qa_scriptor.o \
	tads/tads2/regex.o \
	tads/tads2/run.o \
	tads/tads2/runstat.o \
	tads/tads2/runtime_app.o \
	tads/tads2/runtime_driver.o \
	tads/tads2/tads2.o \
	tads/tads2/tokenizer.o \
	tads/tads2/tokenizer_hash.o \
	tads/tads2/vocabulary.o \
	tads/tads2/vocabulary_parser.o \
	tads/tads3/tads3.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_GLK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
