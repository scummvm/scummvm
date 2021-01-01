MODULE := engines/glk

MODULE_OBJS := \
	blorb.o \
	conf.o \
	debugger.o \
	events.o \
	fonts.o \
	glk.o \
	glk_api.o \
	glk_dispa.o \
	metaengine.o \
	pc_speaker.o \
	picture.o \
	quetzal.o \
	raw_decoder.o \
	screen.o \
	selection.o \
	sound.o \
	speech.o \
	streams.o \
	time.o \
	unicode.o \
	unicode_gen.o \
	utils.o \
	windows.o \
	window_graphics.o \
	window_pair.o \
	window_text_buffer.o \
	window_text_grid.o \
	adrift/adrift.o \
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
	adrift/sxutils.o \
	advsys/advsys.o \
	advsys/game.o \
	advsys/glk_interface.o \
	advsys/vm.o \
	agt/agil.o \
	agt/agt.o \
	agt/agtread.o \
	agt/agxfile.o \
	agt/auxfile.o \
	agt/debugcmd.o \
	agt/disassemble.o \
	agt/exec.o \
	agt/filename.o \
	agt/gamedata.o \
	agt/interface.o \
	agt/metacommand.o \
	agt/object.o \
	agt/os_glk.o \
	agt/parser.o \
	agt/runverb.o \
	agt/savegame.o \
	agt/token.o \
	agt/util.o \
	agt/vars.o \
	alan2/alan2.o \
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
	alan2/types.o \
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
	alan3/word.o \
	archetype/archetype.o \
	archetype/array.o \
	archetype/crypt.o \
	archetype/error.o \
	archetype/expression.o \
	archetype/game_stat.o \
	archetype/heap_sort.o \
	archetype/id_table.o \
	archetype/interpreter.o \
	archetype/keywords.o \
	archetype/linked_list.o \
	archetype/misc.o \
	archetype/parser.o \
	archetype/saveload.o \
	archetype/semantic.o \
	archetype/string.o \
	archetype/sys_object.o \
	archetype/timestamp.o \
	archetype/token.o \
	comprehend/charset.o \
	comprehend/comprehend.o \
	comprehend/debugger.o \
	comprehend/debugger_dumper.o \
	comprehend/dictionary.o \
	comprehend/draw_surface.o \
	comprehend/file_buf.o \
	comprehend/game.o \
	comprehend/game_cc.o \
	comprehend/game_data.o \
	comprehend/game_oo.o \
	comprehend/game_opcodes.o \
	comprehend/game_tm.o \
	comprehend/game_tr1.o \
	comprehend/game_tr2.o \
	comprehend/pics.o \
	glulx/accel.o \
	glulx/exec.o \
	glulx/float.o \
	glulx/funcs.o \
	glulx/gestalt.o \
	glulx/glkop.o \
	glulx/glulx.o \
	glulx/heap.o \
	glulx/operand.o \
	glulx/search.o \
	glulx/serial.o \
	glulx/string.o \
	glulx/vm.o \
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
	hugo/resource_archive.o \
	hugo/stringfn.o \
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
	jacl/utils.o \
	level9/bitmap.o \
	level9/level9.o \
	level9/level9_main.o \
	level9/os_glk.o \
	magnetic/emu.o \
	magnetic/glk.o \
	magnetic/magnetic.o \
	quest/geas_file.o \
	quest/geas_glk.o \
	quest/geas_runner.o \
	quest/geas_state.o \
	quest/geas_util.o \
	quest/quest.o \
	quest/read_file.o \
	quest/string.o \
	quest/streams.o \
	scott/scott.o \
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
	tads/tads3/tads3.o \
	zcode/bitmap_font.o \
	zcode/config.o \
	zcode/zcode.o \
	zcode/glk_interface.o \
	zcode/mem.o \
	zcode/pics.o \
	zcode/pics_decoder.o \
	zcode/processor.o \
	zcode/processor_buffer.o \
	zcode/processor_input.o \
	zcode/processor_maths.o \
	zcode/processor_mem.o \
	zcode/processor_objects.o \
	zcode/processor_screen.o \
	zcode/processor_streams.o \
	zcode/processor_table.o \
	zcode/processor_text.o \
	zcode/processor_variables.o \
	zcode/processor_windows.o \
	zcode/quetzal.o \
	zcode/screen.o \
	zcode/sound_folder.o \
	zcode/windows.o \
	detection.o \
	adrift/detection.o \
	advsys/detection.o \
	agt/detection.o \
	alan2/detection.o \
	alan3/detection.o \
	archetype/detection.o \
	comprehend/detection.o \
	glulx/detection.o \
	hugo/detection.o \
	jacl/detection.o \
	level9/detection.o \
	magnetic/detection.o \
	quest/detection.o \
	scott/detection.o \
	tads/detection.o \
	zcode/detection.o


# This module can be built as a plugin
ifeq ($(ENABLE_GLK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_GLK), STATIC_PLUGIN)
# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# Sub-engine detection objects
DETECT_OBJS += $(MODULE)/adrift/detection.o
DETECT_OBJS += $(MODULE)/advsys/detection.o
DETECT_OBJS += $(MODULE)/agt/detection.o
DETECT_OBJS += $(MODULE)/alan2/detection.o
DETECT_OBJS += $(MODULE)/alan3/detection.o
DETECT_OBJS += $(MODULE)/archetype/detection.o
DETECT_OBJS += $(MODULE)/comprehend/detection.o
DETECT_OBJS += $(MODULE)/glulx/detection.o
DETECT_OBJS += $(MODULE)/hugo/detection.o
DETECT_OBJS += $(MODULE)/jacl/detection.o
DETECT_OBJS += $(MODULE)/level9/detection.o
DETECT_OBJS += $(MODULE)/magnetic/detection.o
DETECT_OBJS += $(MODULE)/quest/detection.o
DETECT_OBJS += $(MODULE)/scott/detection.o
DETECT_OBJS += $(MODULE)/tads/detection.o
DETECT_OBJS += $(MODULE)/zcode/detection.o

# Dependencies of detection objects
DETECT_OBJS += $(MODULE)/blorb.o
DETECT_OBJS += $(MODULE)/advsys/game.o
endif
