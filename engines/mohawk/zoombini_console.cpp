/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mohawk/zoombini_console.h"

#ifdef ENABLE_ZOOMBINI

#include "backends/keymapper/action.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "mohawk/cursors.h"
#include "mohawk/resource.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/paletteman.h"
#include "image/bmp.h"
#ifdef USE_PNG
#include "image/png.h"
#endif

#include <errno.h>

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_debug.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_page.h"
#include "mohawk/zoombini_pages/interactive_base.h"
#include "mohawk/zoombini_pages/puzzle_base.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

static constexpr const char *kXferRouteArrow = " -> ";

static Common::U32String formatManualShortcutMapping(const Common::HardwareInput &input) {
	Common::U32String description = input.description;
	if (input.type != Common::kHardwareInputTypeKeyboard)
		return description;

	const size_t separator = description.rfind('+');
	if (separator == Common::U32String::npos || separator + 2 != description.size())
		return description;

	Common::U32String keyName = description.substr(separator + 1);
	keyName.toUppercase();
	return description.substr(0, separator + 1) + keyName;
}

ZoombiniConsole::ZoombiniConsole(MohawkEngine_Zoombini *vm) : GUI::Debugger(), _vm(vm) {
	// [*] Sound (tSND) commands
	registerCmd(kCmdSound, WRAP_METHOD(ZoombiniConsole, Cmd_Sound));

	// [*] MIDI commands
	registerCmd(kCmdMidi, WRAP_METHOD(ZoombiniConsole, Cmd_Midi));

	// [*] Draw commands
	registerCmd(kCmdDraw, WRAP_METHOD(ZoombiniConsole, Cmd_Draw));

	// [*] Dump commands
	registerCmd(kCmdDump, WRAP_METHOD(ZoombiniConsole, Cmd_DumpResources));

	// [*] Feature (SCRB, SCRS) commands
	registerCmd(kCmdPrint, WRAP_METHOD(ZoombiniConsole, Cmd_Print));

	// [*] Plot commands
	registerCmd(kCmdPlot, WRAP_METHOD(ZoombiniConsole, Cmd_Plot));

	// [*] Game state commands
	registerCmd(kCmdGo, WRAP_METHOD(ZoombiniConsole, Cmd_Go));
	registerCmd(kCmdState, WRAP_METHOD(ZoombiniConsole, Cmd_State));
	registerCmd(kCmdBuiltinDebug, WRAP_METHOD(ZoombiniConsole, Cmd_BuiltinDebug));

	// [*] Page commands
	registerCmd(kCmdPage, WRAP_METHOD(ZoombiniConsole, Cmd_PagePuzzle));
	registerCmd(kCmdPuzzle, WRAP_METHOD(ZoombiniConsole, Cmd_PagePuzzle));

	// [*] Miscellaneous commands
	registerCmd(kCmdManShortcuts, WRAP_METHOD(ZoombiniConsole, Cmd_ManShortcuts));
	registerCmd(kCmdManBuiltinDebug, WRAP_METHOD(ZoombiniConsole, Cmd_ManBuiltinDebug));
}

ZoombiniConsole::~ZoombiniConsole() {
}

// [*] Sound (tSND) commands

bool ZoombiniConsole::Cmd_Sound(int argc, const char **argv) {
	if (argc == 2 && isHelpOption(argv[1])) {
		debugPrintf("Control Zoombini sound-effect playback.\n");
		debugPrintf("Usage: %s <subcommand> [arguments]\n\n", kCmdSound);
		debugPrintf("Subcommands:\n");
		debugPrintf("  %s <resource>\n", kSubCmdSoundPlay);
		debugPrintf("      Play an SND resource from the page or system archive.\n");
		debugPrintf("  %s\n", kSubCmdSoundStop);
		debugPrintf("      Stop the SFX channel.\n");
		return true;
	}

	if (argc < 2) {
		debugPrintf("Usage: %s <%s|%s> [arguments]\n", kCmdSound, kSubCmdSoundPlay, kSubCmdSoundStop);
		debugPrintf("\n");
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdSoundPlay) == 0)
		return CmdSub_SoundPlay(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdSoundStop) == 0)
		return CmdSub_SoundStop(argc, argv);

	debugPrintf("Unknown %s subcommand '%s'.\n", kCmdSound, argv[1]);
	debugPrintf("Usage: %s <%s|%s> [arguments]\n", kCmdSound, kSubCmdSoundPlay, kSubCmdSoundStop);
	debugPrintf("\n");
	return true;
}

bool ZoombiniConsole::CmdSub_SoundPlay(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Play a Zoombini sound effect from an SND resource.\n");
		debugPrintf("Usage: %s %s <resource>\n\n", kCmdSound, kSubCmdSoundPlay);
		debugPrintf("<resource> is a resource ID accepted by the console, such as\n");
		debugPrintf("4100, p:4100, s:4100, or 0x1004. The resource must be an SND\n");
		debugPrintf("resource in the page or system archive. Any currently playing\n");
		debugPrintf("sound effect is stopped before the requested sound is played.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 3) {
		debugPrintf("Usage: %s %s <value>\n", kCmdSound, kSubCmdSoundPlay);
		debugPrintf("\n");
		return true;
	}

	ZmbResource resource;
	if (!parseResourceId(argv[2], resource))
		return true;

	if (!_vm->hasResource(ID_SND, resource)) {
		debugPrintf("Cannot find resource SND(%s)\n", argv[2]);
		debugPrintf("\n");
		return true;
	}

	_vm->_sound->releaseAllLoadedSounds();
	_vm->_sound->playSound(resource, Audio::Mixer::kSFXSoundType);
	return false;
}

bool ZoombiniConsole::CmdSub_SoundStop(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Stop the currently playing Zoombini sound effect.\n");
		debugPrintf("Usage: %s %s\n\n", kCmdSound, kSubCmdSoundStop);
		debugPrintf("The command stops the SFX channel. It does not change any\n");
		debugPrintf("saved game or global sound option.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	debugPrintf("Stopping Sound\n");
	debugPrintf("\n");

	_vm->_sound->releaseAllLoadedSounds();
	return true;
}

// [*] MIDI commands

bool ZoombiniConsole::Cmd_Midi(int argc, const char **argv) {
	if (argc == 2 && isHelpOption(argv[1])) {
		if (_vm->isVersionFamilyTlcV2())
			debugPrintf("Control the built-in music test\n");
		else
			debugPrintf("Control MIDI playback and the built-in MIDI test\n");
		debugPrintf("Usage: %s <subcommand> [arguments]\n\n", kCmdMidi);
		debugPrintf("Subcommands:\n");
		if (!_vm->isVersionFamilyTlcV2()) {
			debugPrintf("  %s <midi-id>\n", kSubCmdMidiPlay);
			debugPrintf("      Play a tMID resource from the selected shared v1 MIDI archive.\n");
			debugPrintf("  %s\n", kSubCmdMidiStop);
			debugPrintf("      Stop MIDI playback.\n");
		}
		debugPrintf("  %s <enable|disable|play|next>\n", kSubCmdMidiBuiltinDebug);
		debugPrintf("      Enable or disable MIDI test mode, or invoke its built-in selector.\n");
		debugPrintf("\n");
		printBuiltinDebugStatus();
		return true;
	}

	if (argc < 2) {
		if (_vm->isVersionFamilyTlcV2())
			debugPrintf("Usage: %s %s <enable|disable|play|next>\n", kCmdMidi, kSubCmdMidiBuiltinDebug);
		else
			debugPrintf("Usage: %s <%s|%s|%s> [arguments]\n", kCmdMidi, kSubCmdMidiPlay, kSubCmdMidiStop, kSubCmdMidiBuiltinDebug);
		debugPrintf("\n");
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdMidiBuiltinDebug) == 0)
		return CmdSub_MidiBuiltinDebug(argc, argv);

	if (_vm->isVersionFamilyTlcV2()) {
		debugPrintf("TLC v2 supports only the '%s %s' command.\n", kCmdMidi, kSubCmdMidiBuiltinDebug);
		debugPrintf("Usage: %s %s <enable|disable|play|next>\n\n", kCmdMidi, kSubCmdMidiBuiltinDebug);
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdMidiPlay) == 0)
		return CmdSub_MidiPlay(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdMidiStop) == 0)
		return CmdSub_MidiStop(argc, argv);

	debugPrintf("Unknown %s subcommand '%s'.\n", kCmdMidi, argv[1]);
	debugPrintf("Usage: %s <%s|%s|%s> [arguments]\n", kCmdMidi, kSubCmdMidiPlay, kSubCmdMidiStop, kSubCmdMidiBuiltinDebug);
	debugPrintf("\n");
	return true;
}

bool ZoombiniConsole::CmdSub_MidiPlay(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Play a MIDI resource from the selected shared v1 MIDI archive.\n");
		debugPrintf("Usage: %s %s <midi-id>\n\n", kCmdMidi, kSubCmdMidiPlay);
		debugPrintf("<midi-id> is a decimal tMID resource ID in the selected shared v1 MIDI archive.\n");
		debugPrintf("Any currently playing MIDI sequence is stopped before playback starts.\n");
		debugPrintf("The command does not change the saved game state.\n");
		debugPrintf("\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 3) {
		debugPrintf("Usage: %s %s <value>\n", kCmdMidi, kSubCmdMidiPlay);
		debugPrintf("\n");
		return true;
	}

	// tMID uses the page resource kind but is owned by the selected shared MIDI archive.
	ZmbResource resource;
	if (!parseResourceId(argv[2], resource) || resource._archiveKind != ZmbResource::kPage)
		return true;

	if (!_vm->hasResource(ID_TMID, resource)) {
		debugPrintf("Cannot find resource MIDI(%s)\n", resource.toString().c_str());
		debugPrintf("\n");
		return true;
	}

	_vm->_midi->stop();
	_vm->_midi->playMidi(resource);
	return false;
}

bool ZoombiniConsole::CmdSub_MidiStop(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Stop the currently playing MIDI sequence.\n");
		debugPrintf("Usage: %s %s\n\n", kCmdMidi, kSubCmdMidiStop);
		debugPrintf("The command stops MIDI playback without changing any saved game or global music option.\n");
		debugPrintf("\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	debugPrintf("Stopping Midi\n");
	debugPrintf("\n");

	_vm->_midi->stop();
	return true;
}

bool ZoombiniConsole::CmdSub_MidiBuiltinDebug(int argc, const char **argv) {
	if (argc == 2 || hasHelpOption(argc, argv)) {
		debugPrintf("Original engine's built-in MIDI-test selector.\n");
		debugPrintf("Usage: %s %s <enable|disable|play|next>\n\n", kCmdMidi, kSubCmdMidiBuiltinDebug);
		debugPrintf("Original in-game toggle: press Ctrl+A, type lowercase 'midi',\n");
		debugPrintf("then press Space. The long NotiBox says 'midi test on' or\n");
		debugPrintf("'midi test off'. This mode is independent of the debug flag.\n\n");
		debugPrintf("  enable\n");
		debugPrintf("      Enable MIDI test mode and show the original notification.\n");
		debugPrintf("  disable\n");
		debugPrintf("      Disable MIDI test mode and show the original notification.\n");
		debugPrintf("After enabling the mode and closing the debugger:\n");
		debugPrintf("  Space\n");
		debugPrintf("      Run the current selector entry and show 'midi test:<id>'.\n");
		debugPrintf("      Leave the current MIDI playing; Shift+Space explicitly replaces it.\n");
		debugPrintf("  Shift+Space\n");
		debugPrintf("      Advance to the next of 18 selector entries, wrap, and run it.\n");
		debugPrintf("      The table is 30000, 30001, 30020-30023, 30025-30028,\n");
		debugPrintf("      30030-30033, and 30035-30038. v1 plays the selected tMID;\n");
		debugPrintf("      TLC v2 queues only Picker's MUSIC.MHK SND 30001 while Picker is active.\n\n");
		debugPrintf("Debugger counterparts:\n");
		debugPrintf("  play\n");
		debugPrintf("      Enable the mode if needed, then perform the same operation as Space.\n");
		debugPrintf("  next\n");
		debugPrintf("      Enable the mode if needed, then perform the same operation as Shift+Space.\n\n");
		debugPrintf("Original debug-flag activation for page/global keys: press Ctrl+A,\n");
		debugPrintf("type lowercase 'go snoids', then press Space.\n\n");
		printBuiltinDebugStatus();
		debugPrintf("\n");
		debugPrintf("Open the ScummVM debugger during play with the default Ctrl+Alt+D shortcut.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc == 3 && scumm_stricmp(argv[2], "enable") == 0) {
		_vm->setBuiltinMidiTestMode(true);
	} else if (argc == 3 && scumm_stricmp(argv[2], "disable") == 0) {
		_vm->setBuiltinMidiTestMode(false);
	} else if (argc == 3 && (scumm_stricmp(argv[2], "play") == 0 || scumm_stricmp(argv[2], "next") == 0)) {
		if (!_vm->isBuiltinMidiTestMode())
			_vm->setBuiltinMidiTestMode(true);
		const uint16 resourceId = _vm->playBuiltinMidiTestResource(scumm_stricmp(argv[2], "next") == 0);
		debugPrintf("Ran built-in MIDI-test selector %u.\n", resourceId);
	} else {
		debugPrintf("Usage: %s %s <enable|disable|play|next>\n\n", kCmdMidi, kSubCmdMidiBuiltinDebug);
		return true;
	}

	printBuiltinDebugStatus();
	debugPrintf("\n");
	return false;
}

// [*] Draw commands

bool ZoombiniConsole::Cmd_Draw(int argc, const char **argv) {
	if (argc == 2 && isHelpOption(argv[1])) {
		debugPrintf("Inspect or render image, shape, cursor, feature, message-box, and terrain displays.\n");
		debugPrintf("Usage: %s <subcommand> [arguments]\n\n", kCmdDraw);
		debugPrintf("Subcommands:\n");
		debugPrintf("  %s <cursor-id>\n", kSubCmdDrawCursor);
		debugPrintf("      Render a system CURS resource.\n");
		debugPrintf("  %s <image-id>\n", kSubCmdDrawImage);
		debugPrintf("      Render a page tBMP image.\n");
		debugPrintf("  %s <image-resource> <shape-index>\n", kSubCmdDrawShape);
		debugPrintf("      Render one 1-based shape from a tBMP resource.\n");
		debugPrintf("  %s <image-resource> [start-shape-index]\n", kSubCmdDrawShapes);
		debugPrintf("      Render all shapes from a 1-based starting index.\n");
		debugPrintf("  %s <image-resource> <scrb-id>\n", kSubCmdDrawFeature);
		debugPrintf("      Render an SCRB feature over a tBMP image.\n\n");
		debugPrintf("  %s <text-key>\n", kSubCmdDrawMsgBox);
		debugPrintf("      Render a localized text key in a Yes/No message box.\n\n");
		debugPrintf("  %s\n", kSubCmdDrawTerrain);
		debugPrintf("      Show the active page only where its terrain bitmap is\n");
		debugPrintf("      walkable, masking the rest with black.\n\n");
		debugPrintf("Resource IDs accept decimal or 0x-prefixed values; use p: or\n");
		debugPrintf("s: to select the page or system archive explicitly where supported.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 2) {
		debugPrintf("Usage: %s <%s|%s|%s|%s|%s|%s|%s> [arguments]\n", kCmdDraw,
					kSubCmdDrawCursor, kSubCmdDrawImage, kSubCmdDrawShape, kSubCmdDrawShapes, kSubCmdDrawFeature, kSubCmdDrawMsgBox, kSubCmdDrawTerrain);
		debugPrintf("\n");
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdDrawCursor) == 0)
		return CmdSub_DrawCursor(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDrawImage) == 0)
		return CmdSub_DrawImage(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDrawShape) == 0)
		return CmdSub_DrawShape(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDrawShapes) == 0)
		return CmdSub_DrawShapes(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDrawFeature) == 0)
		return CmdSub_DrawFeature(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDrawMsgBox) == 0)
		return CmdSub_DrawMsgBox(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDrawTerrain) == 0)
		return CmdSub_DrawTerrain(argc, argv);

	debugPrintf("Unknown %s subcommand '%s'.\n", kCmdDraw, argv[1]);
	debugPrintf("Usage: %s <%s|%s|%s|%s|%s|%s|%s> [arguments]\n", kCmdDraw,
				kSubCmdDrawCursor, kSubCmdDrawImage, kSubCmdDrawShape, kSubCmdDrawShapes, kSubCmdDrawFeature, kSubCmdDrawMsgBox, kSubCmdDrawTerrain);
	debugPrintf("\n");
	return true;
}

// [*] Dump commands

bool ZoombiniConsole::Cmd_DumpResources(int argc, const char **argv) {
	if (argc == 2 && isHelpOption(argv[1])) {
		debugPrintf("Export Zoombini resources or runtime text data.\n");
		debugPrintf("Usage: %s <subcommand> [arguments]\n\n", kCmdDump);
		debugPrintf("Subcommands:\n");
		debugPrintf("  %s <midi-id>\n", kSubCmdDumpMidi);
		debugPrintf("      Export one page MIDI resource as a .mid file.\n");
		debugPrintf("  %s <image-id> [bmp|png]\n", kSubCmdDumpImage);
		debugPrintf("      Export one page tBMP image. The default format is BMP.\n");
		debugPrintf("  %s <image-resource> [shpl-id] [bmp|png]\n", kSubCmdDumpShapes);
		debugPrintf("      Export every shape in a tBMP resource.\n");
		debugPrintf("  %s [<scrb-resource>]\n", kSubCmdDumpFeature);
		debugPrintf("      Export one SCRB feature description, or all of them\n");
		debugPrintf("      from the active page when no ID is given.\n");
		debugPrintf("  %s [<scrs-resource>]\n", kSubCmdDumpScrs);
		debugPrintf("      Export one SCRS snoid script description, or all of them\n");
		debugPrintf("      from the active page when no ID is given.\n");
		debugPrintf("  %s [filename]\n", kSubCmdDumpTexts);
		debugPrintf("      Export localized strings and credit lines. The default file is\n");
		debugPrintf("      dumps/ZOOMBINI_texts.txt; .csv and .tsv select tabular output.\n");
		debugPrintf("  %s\n", kSubCmdDumpSave);
		debugPrintf("      Export the raw state file backing the current game.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 2) {
		debugPrintf("Usage: %s <%s|%s|%s|%s|%s|%s|%s> [arguments]\n", kCmdDump,
					kSubCmdDumpMidi, kSubCmdDumpImage, kSubCmdDumpShapes, kSubCmdDumpFeature,
					kSubCmdDumpScrs, kSubCmdDumpTexts, kSubCmdDumpSave);
		debugPrintf("\n");
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdDumpMidi) == 0)
		return CmdSub_DumpMidi(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDumpImage) == 0)
		return CmdSub_DumpImage(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDumpShapes) == 0)
		return CmdSub_DumpShapes(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDumpFeature) == 0)
		return CmdSub_DumpFeature(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDumpScrs) == 0)
		return CmdSub_DumpScrs(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDumpTexts) == 0)
		return CmdSub_DumpTexts(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDumpSave) == 0)
		return CmdSub_DumpSave(argc, argv);

	debugPrintf("Unknown %s subcommand '%s'.\n", kCmdDump, argv[1]);
	debugPrintf("Usage: %s <%s|%s|%s|%s|%s|%s|%s> [arguments]\n", kCmdDump,
				kSubCmdDumpMidi, kSubCmdDumpImage, kSubCmdDumpShapes, kSubCmdDumpFeature,
				kSubCmdDumpScrs, kSubCmdDumpTexts, kSubCmdDumpSave);
	debugPrintf("\n");
	return true;
}

// [*] Dump subcommands (MIDI)

bool ZoombiniConsole::CmdSub_DumpMidi(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Export one tMID resource from the selected shared v1 MIDI archive.\n");
		debugPrintf("Usage: %s %s <midi-id>\n\n", kCmdDump, kSubCmdDumpMidi);
		debugPrintf("<midi-id> is a decimal tMID resource ID in the shared MIDI archive.\n");
		debugPrintf("The output file is written below dumps/ as\n");
		debugPrintf("ZOOMBINI_MIDI_pNNNN.mid.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 3) {
		debugPrintf("Usage: %s %s <value>\n", kCmdDump, kSubCmdDumpMidi);
		debugPrintf("\n");
		return true;
	}

	// tMID uses the page resource kind but is owned by the selected shared MIDI archive.
	ZmbResource resource;
	if (!parseResourceId(argv[2], resource) || resource._archiveKind != ZmbResource::kPage)
		return true;
	const int16 resid = resource._id;

	if (!_vm->hasResource(ID_TMID, ZmbResource(ZmbResource::kPage, resid))) {
		debugPrintf("Cannot find resource MIDI(%s)\n", argv[2]);
		debugPrintf("\n");
		return true;
	}

	Common::SeekableReadStream *midiStream = _vm->getResource(ID_TMID, ZmbResource(ZmbResource::kPage, resid));
	if (!midiStream) {
		debugPrintf("Failed to read MIDI resource(%s)\n", argv[2]);
		debugPrintf("\n");
		return true;
	}

	Common::String filename = Common::String::format("ZOOMBINI_MIDI_p%04d.mid", resid);
	Common::String filepath = "dumps/" + filename;

	Common::DumpFile out;
	if (!out.open(Common::Path(filepath, '/'), true)) {
		debugPrintf("Failed to open file for writing: %s\n", filepath.c_str());
		debugPrintf("\n");
		delete midiStream;
		return true;
	}

	// Copy the entire stream to the file
	uint32 size = midiStream->size();
	byte *buffer = new byte[size];
	midiStream->read(buffer, size);
	out.write(buffer, size);
	delete[] buffer;
	delete midiStream;
	out.close();

	debugPrintf("Successfully exported MIDI to %s\n", filename.c_str());
	debugPrintf("\n");
	return true;
}

// [*] Draw subcommands (image resources)

bool ZoombiniConsole::CmdSub_DrawCursor(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Render a system cursor resource in the debug dialog.\n");
		debugPrintf("Usage: %s %s <cursor-id>\n\n", kCmdDraw, kSubCmdDrawCursor);
		debugPrintf("<cursor-id> is a decimal CURS resource ID from the system\n");
		debugPrintf("archive. The command opens a renderer debug dialog.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 3) {
		debugPrintf("Usage: %s %s <cursorId>\n", kCmdDraw, kSubCmdDrawCursor);
		debugPrintf("\n");
		return true;
	}

	// Cursor is always in system ZOOMBINI.MHK
	int32 parsedCursorId = 0;
	if (!parseInt(argv[2], parsedCursorId) || parsedCursorId < 0 || 0x7FFF < parsedCursorId) {
		debugPrintf("Cannot parse argument %s\n", argv[2]);
		debugPrintf("\n");
		return true;
	}
	const int16 cursorId = static_cast<int16>(parsedCursorId);

	if (!_vm->hasResource(ID_CURS, ZmbResource(ZmbResource::kSystem, cursorId))) {
		debugPrintf("Cannot find resource CURS(%s)\n", argv[2]);
		debugPrintf("\n");
		return true;
	}

	ZoombiniDebugCommand cmd;
	cmd.setDrawCursor(ZmbResource(ZmbResource::kSystem, cursorId));
	_vm->openDebugDialog(cmd);
	return false;
}

bool ZoombiniConsole::CmdSub_DrawImage(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Render a page bitmap image in the debug dialog.\n");
		debugPrintf("Usage: %s %s <image-id>\n\n", kCmdDraw, kSubCmdDrawImage);
		debugPrintf("<image-id> is a decimal page tBMP resource ID. A matching\n");
		debugPrintf("SHPL palette is preferred; compound-shape images may not have\n");
		debugPrintf("a standalone palette resource.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 3) {
		debugPrintf("Usage: %s %s <imageId>\n", kCmdDraw, kSubCmdDrawImage);
		debugPrintf("\n");
		return true;
	}

	// There is no palette resource in system ZOOMBINI.MHK
	int32 parsedImageId = 0;
	if (!parseInt(argv[2], parsedImageId) || parsedImageId < 0 || 0x7FFF < parsedImageId) {
		debugPrintf("Cannot parse argument %s\n", argv[2]);
		debugPrintf("\n");
		return true;
	}
	const int16 imageId = static_cast<int16>(parsedImageId);

	bool missingPalette = false;
	if (!_vm->hasResource(ID_SHPL, ZmbResource(ZmbResource::kPage, imageId))) { // palette
		debugPrintf("Cannot find resource SHPL(%s), maybe the bitmap is a compound shape?\n", argv[2]);
		missingPalette = true;
	}

	if (!_vm->hasResource(ID_TBMP, ZmbResource(ZmbResource::kPage, imageId))) { // background bitmap
		debugPrintf("Cannot find resource tBMP(%s)\n", argv[2]);
		debugPrintf("\n");
		return true;
	}

	ZoombiniDebugCommand cmd;
	cmd.setDrawImage(ZmbResource(ZmbResource::kPage, imageId));
	_vm->openDebugDialog(cmd);
	if (missingPalette)
		debugPrintf("\n");
	return false;
}

bool ZoombiniConsole::CmdSub_DrawShape(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Render one shape from a tBMP resource.\n");
		debugPrintf("Usage: %s %s <image-resource> <shape-index>\n\n", kCmdDraw, kSubCmdDrawShape);
		debugPrintf("<image-resource> accepts a decimal or 0x-prefixed ID and an\n");
		debugPrintf("optional p: or s: archive prefix. <shape-index> identifies the\n");
		debugPrintf("shape within the image and follows the resource's 1-based shape\n");
		debugPrintf("numbering.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 4) {
		debugPrintf("Usage: %s %s <imageId> <shapeIdx>\n", kCmdDraw, kSubCmdDrawShape);
		debugPrintf("\n");
		return true;
	}

	ZmbResource resource;
	if (!parseResourceId(argv[2], resource))
		return true;

	if (!_vm->hasResource(ID_TBMP, resource)) {
		debugPrintf("Cannot find resource tBMP(%s)\n", argv[2]);
		debugPrintf("\n");
		return true;
	}

	errno = 0;
	uint16 shapeIdx = static_cast<uint16>(strtoul(argv[3], nullptr, 10));
	if (errno != 0) {
		debugPrintf("Cannot parse argument %s\n", argv[3]);
		debugPrintf("\n");
		return true;
	}

	ZoombiniDebugCommand cmd;
	cmd.setDrawShape(resource, shapeIdx);
	_vm->openDebugDialog(cmd);
	return false;
}

bool ZoombiniConsole::CmdSub_DrawShapes(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Render all shapes from a tBMP resource.\n");
		debugPrintf("Usage: %s %s <image-resource> [start-shape-index]\n\n", kCmdDraw, kSubCmdDrawShapes);
		debugPrintf("The optional start index is 1-based and defaults to 1. The\n");
		debugPrintf("command opens a renderer debug dialog for the shape sequence.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (!(3 <= argc && argc <= 4)) {
		debugPrintf("Usage: %s %s <imageId> [startShapeIdx]\n", kCmdDraw, kSubCmdDrawShapes);
		debugPrintf("\n");
		return true;
	}

	ZmbResource resource;
	if (!parseResourceId(argv[2], resource))
		return true;
	uint16 startShapeIdx = 1;
	if (argc == 4) {
		errno = 0;
		startShapeIdx = static_cast<uint16>(strtoul(argv[3], nullptr, 10));
		if (errno != 0) {
			debugPrintf("Cannot parse argument %s\n", argv[3]);
			debugPrintf("\n");
			return true;
		}
		if (startShapeIdx < 1) {
			debugPrintf("[startShapeId] is 1-based idx!\n");
			debugPrintf("\n");
			return true;
		}
	}

	if (!_vm->hasResource(ID_TBMP, resource)) {
		debugPrintf("Cannot find resource tBMP(%s)\n", argv[2]);
		debugPrintf("\n");
		return true;
	}

	uint32 shapeCount = _vm->_gfx->getShapeCount(resource);
	if (shapeCount < startShapeIdx) {
		debugPrintf("startShapeIdx exceeded shape count %u\n", shapeCount);
		debugPrintf("\n");
		return true;
	}

	ZoombiniDebugCommand cmd;
	cmd.setDrawShapes(resource, startShapeIdx);
	_vm->openDebugDialog(cmd);
	return false;
}

bool ZoombiniConsole::CmdSub_DrawFeature(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Render an SCRB feature over a tBMP image.\n");
		debugPrintf("Usage: %s %s <image-resource> <scrb-id>\n\n", kCmdDraw, kSubCmdDrawFeature);
		debugPrintf("<image-resource> accepts a decimal or 0x-prefixed ID with an\n");
		debugPrintf("optional p: or s: archive prefix. <scrb-id> is the SCRB ID in\n");
		debugPrintf("the same archive. The command opens a renderer debug dialog.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 4) {
		debugPrintf("Usage: %s %s <imageId> <scrbId>\n", kCmdDraw, kSubCmdDrawFeature);
		debugPrintf("\n");
		return true;
	}

	ZmbResource resource;
	if (!parseResourceId(argv[2], resource))
		return true;

	if (!_vm->hasResource(ID_TBMP, resource)) {
		debugPrintf("Cannot find resource tBMP(%s)\n", argv[2]);
		debugPrintf("\n");
		return true;
	}

	int32 parsedScrbId = 0;
	if (!parseInt(argv[3], parsedScrbId) || parsedScrbId < 0 || 0x7FFF < parsedScrbId) {
		debugPrintf("Cannot parse argument %s!\n", argv[3]);
		debugPrintf("\n");
		return true;
	}
	const int16 scrbId = static_cast<int16>(parsedScrbId);

	if (!_vm->hasResource(ID_SCRB, ZmbResource(resource._archiveKind, scrbId))) {
		debugPrintf("Cannot find resource SCRB(%s)\n", argv[3]);
		debugPrintf("\n");
		return true;
	}

	ZoombiniDebugCommand cmd;
	cmd.setDrawFeature(resource, scrbId);
	_vm->openDebugDialog(cmd);
	return false;
}

bool ZoombiniConsole::CmdSub_DrawMsgBox(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Render one localized text key in a Yes/No message box.\n");
		debugPrintf("Usage: %s %s <text-key>\n\n", kCmdDraw, kSubCmdDrawMsgBox);
		debugPrintf("The text key is a decimal or 0x-prefixed ZoombiniText key.\n");
		debugPrintf("For example, %s %s 800 displays the debug-save warning.\n", kCmdDraw, kSubCmdDrawMsgBox);
		debugPrintf("Use '%s texts' to export the available localized keys.\n\n", kCmdDump);
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 3) {
		debugPrintf("Usage: %s %s <text-key>\n", kCmdDraw, kSubCmdDrawMsgBox);
		debugPrintf("\n");
		return true;
	}

	int32 textKey = 0;
	if (!parseInt(argv[2], textKey)) {
		debugPrintf("\n");
		return true;
	}

	// Reject unknown keys while the debugger remains open to show the diagnostic.
	const Common::U32String &message = _vm->_text->getLocalizedString(static_cast<uint32>(textKey));
	if (message.empty()) {
		debugPrintf("No localized text for key %d.\n", textKey);
		debugPrintf("\n");
		return true;
	}

	// Returning false closes the debugger before the engine opens the modal.
	_vm->requestMsgBoxDialog(static_cast<uint32>(textKey));
	return false;
}

bool ZoombiniConsole::CmdSub_DrawTerrain(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Show the active page through its terrain bitmap in the debug dialog.\n");
		debugPrintf("Usage: %s %s\n\n", kCmdDraw, kSubCmdDrawTerrain);
		debugPrintf("Only terrain pixels with value 1 remain visible.\n");
		debugPrintf("The other pixels are masked with black.\n");
		debugPrintf("If the active page has no terrain bitmap, the debug display is\n");
		debugPrintf("fully black and shows a diagnostic message below the top line.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 2) {
		debugPrintf("Usage: %s %s\n", kCmdDraw, kSubCmdDrawTerrain);
		debugPrintf("\n");
		return true;
	}

	ZoombiniPage *activePage = _vm->getActivePage();
	if (!activePage) {
		debugPrintf("No active Zoombini page\n");
		debugPrintf("\n");
		return true;
	}
	ZoombiniDebugCommand cmd;
	cmd.setDrawTerrain();
	_vm->openDebugDialog(cmd);
	return false;
}

// [*] Dump subcommands (image resources)

bool ZoombiniConsole::CmdSub_DumpImage(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Export one page tBMP image with its SHPL palette.\n");
		debugPrintf("Usage: %s %s <image-id> [bmp|png]\n\n", kCmdDump, kSubCmdDumpImage);
		debugPrintf("<image-id> is a decimal page resource ID. The optional output\n");
		debugPrintf("format defaults to BMP; PNG requires libpng support in the build.\n");
		debugPrintf("Files are written below dumps/ as ZOOMBINI_tBMP_pNNNN.ext.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 3 || 4 < argc) {
		debugPrintf("Usage: %s %s <imageId> [bmp|png]\n", kCmdDump, kSubCmdDumpImage);
		debugPrintf("\n");
		return true;
	}
	bool exportAsPng = false;
	if (argc == 4 && !parseDumpImageFormat(argv[3], exportAsPng))
		return true;

	// There is no palette resource in system ZOOMBINI.MHK
	int32 parsedImageId = 0;
	if (!parseInt(argv[2], parsedImageId) || parsedImageId < 0 || 0x7FFF < parsedImageId) {
		debugPrintf("Cannot parse argument %s\n", argv[2]);
		debugPrintf("\n");
		return true;
	}
	const int16 imageId = static_cast<int16>(parsedImageId);

	// In DumpImage, SHPL (palette) resource must exist in the page archive
	if (!_vm->hasResource(ID_SHPL, ZmbResource(ZmbResource::kPage, imageId))) {
		debugPrintf("Cannot find resource SHPL(%s)\n", argv[2]);
		debugPrintf("\n");
		return true;
	}

	if (!_vm->hasResource(ID_TBMP, ZmbResource(ZmbResource::kPage, imageId))) {
		debugPrintf("Cannot find resource tBMP(%s)\n", argv[2]);
		debugPrintf("\n");
		return true;
	}

	// Read palette
	byte palette[3 * 256];
	memset(palette, 0, ARRAYSIZE(palette));
	{
		if (!_vm->_gfx->readPalette(imageId, palette, ARRAYSIZE(palette))) {
			debugPrintf("Failed to load palette from SHPL %04u\n", imageId);
			debugPrintf("\n");
			return true;
		}
	}

	// Read image surface
	MohawkSurface *imgSurface = _vm->_gfx->findImage(ZmbResource(ZmbResource::kPage, imageId));
	if (!imgSurface) {
		debugPrintf("Failed to load image %u\n", imageId);
		debugPrintf("\n");
		return true;
	}
	Graphics::Surface *surface = imgSurface->getSurface();
	if (!surface || surface->h == 0 || surface->w == 0) {
		debugPrintf("Invalid surface for image %u\n", imageId);
		debugPrintf("\n");
		return true;
	}

	const char *extension = exportAsPng ? "PNG" : "BMP";
	Common::String filename = Common::String::format("ZOOMBINI_tBMP_p%04u.%s", imageId, extension);
	if (exportSurfaceToImage(filename, surface, palette, exportAsPng)) {
		debugPrintf("Successfully exported image %u to %s\n", imageId, filename.c_str());
	} else {
		debugPrintf("Failed to export image %u to %s\n", imageId, extension);
	}

	debugPrintf("\n");
	return true;
}

bool ZoombiniConsole::CmdSub_DumpShapes(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Export every shape from a tBMP resource.\n");
		debugPrintf("Usage: %s %s <image-resource> [shpl-id] [bmp|png]\n\n", kCmdDump, kSubCmdDumpShapes);
		debugPrintf("The optional SHPL ID selects a palette; if omitted, the current\n");
		debugPrintf("screen palette is used. The format defaults to BMP; PNG requires\n");
		debugPrintf("libpng support. Resource IDs accept p: and s: archive prefixes.\n");
		debugPrintf("Output files are written below dumps/.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (!(3 <= argc && argc <= 5)) {
		debugPrintf("Usage: %s %s <imageId> [shplId] [bmp|png]\n", kCmdDump, kSubCmdDumpShapes);
		debugPrintf("\n");
		return true;
	}

	ZmbResource resource;
	if (!parseResourceId(argv[2], resource))
		return true;
	bool exportAsPng = false;
	int16 shplId = 0;
	if (argc == 4 && isDumpImageFormat(argv[3])) {
		if (!parseDumpImageFormat(argv[3], exportAsPng))
			return true;
	} else if (4 <= argc) {
		int32 parsedShplId = 0;
		if (!parseInt(argv[3], parsedShplId) || parsedShplId < 0 || 0x7FFF < parsedShplId) {
			debugPrintf("Cannot parse argument %s!\n", argv[3]);
			debugPrintf("\n");
			return true;
		}
		shplId = static_cast<int16>(parsedShplId);
		if (shplId < 1) {
			debugPrintf("[shplId] must be larger then 0!\n");
			debugPrintf("\n");
			return true;
		}
	}
	if (argc == 5 && !parseDumpImageFormat(argv[4], exportAsPng))
		return true;

	// Collect palette
	Common::String palLogStr;
	byte palette[3 * 256];
	memset(palette, 0, ARRAYSIZE(palette));
	if (shplId == 0) { // Read current palette
		_vm->_system->getPaletteManager()->grabPalette(palette, 0, 256);
		palLogStr = "with current palettes";
	} else { // Read palette from a SHPL resource
		// There is no SHPL resources in system ZOOMBINI.MHK
		if (!_vm->hasResource(ID_SHPL, ZmbResource(ZmbResource::kPage, shplId))) {
			debugPrintf("Cannot find resource SHPL %04d\n", shplId);
			debugPrintf("\n");
			return true;
		}

		if (!_vm->_gfx->readPalette(shplId, palette, ARRAYSIZE(palette))) {
			debugPrintf("Failed to load palette from SHPL %04d\n", shplId);
			debugPrintf("\n");
			return true;
		}

		palLogStr = Common::String::format("with SHPL %04d palettes", shplId);
	}

	// Read the shape data
	if (!_vm->hasResource(ID_TBMP, resource)) {
		debugPrintf("Cannot find resource tBMP(%s)\n", argv[2]);
		debugPrintf("\n");
		return true;
	}

	uint32 shapeCount = _vm->_gfx->getShapeCount(resource);

	const char *extension = exportAsPng ? "PNG" : "BMP";

	// Export shape bitmaps
	uint16 exportedCount = 0;
	for (uint16 shapeIdx = 1; shapeIdx <= shapeCount; shapeIdx++) {
		MohawkSurface *shapeSurface = _vm->_gfx->findShape(resource, shapeIdx);
		if (!shapeSurface) {
			debugPrintf("Warning: Failed to load shape %u\n", shapeIdx);
			continue;
		}

		Graphics::Surface *surface = shapeSurface->getSurface();
		if (!surface || surface->h == 0 || surface->w == 0) {
			debugPrintf("Warning: Invalid surface for shape %u\n", shapeIdx);
			continue;
		}

		Common::String filename = Common::String::format("ZOOMBINI_tBMP_%s_shape_%03u.%s", resource.toString().c_str(), shapeIdx, extension);
		if (exportSurfaceToImage(filename, surface, palette, exportAsPng)) {
			exportedCount += 1;
			debugPrintf("Successfully exported image %s to %s\n", resource.toString().c_str(), filename.c_str());
		} else {
			debugPrintf("Failed to export image %s to %s\n", resource.toString().c_str(), extension);
		}
	}

	debugPrintf("Successfully exported %03u of %03u shapes from image %s %s\n", exportedCount, shapeCount, resource.toString().c_str(), palLogStr.c_str());
	debugPrintf("\n");
	return true;
}

// [*] Feature (SCRB, SCRS) commands

bool ZoombiniConsole::Cmd_Print(int argc, const char **argv) {
	if (argc == 2 && isHelpOption(argv[1])) {
		debugPrintf("Print SCRB feature and SCRS snoid script diagnostics.\n");
		debugPrintf("Usage: %s <subcommand> [arguments]\n\n", kCmdPrint);
		debugPrintf("Subcommands:\n");
		debugPrintf("  %s [<scrb-resource>]\n", kSubCmdPrintFeature);
		debugPrintf("      Print one SCRB feature's frame and hotspot metadata, or\n");
		debugPrintf("      every SCRB feature on the active page when no ID is given.\n");
		debugPrintf("  %s\n", kSubCmdPrintRunners);
		debugPrintf("      List feature runners registered on the active page.\n");
		debugPrintf("  %s [<scrs-resource>]\n", kSubCmdPrintScrs);
		debugPrintf("      Print one SCRS snoid script's metadata and hotspots, or\n");
		debugPrintf("      every SCRS resource on the active page when no ID is given.\n");
		return true;
	}

	if (argc < 2) {
		debugPrintf("Usage: %s <%s|%s|%s> [arguments]\n", kCmdPrint,
					kSubCmdPrintFeature, kSubCmdPrintRunners, kSubCmdPrintScrs);
		debugPrintf("\n");
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdPrintFeature) == 0)
		return CmdSub_PrintFeature(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdPrintRunners) == 0)
		return CmdSub_PrintRunners(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdPrintScrs) == 0)
		return CmdSub_PrintScrs(argc, argv);

	debugPrintf("Unknown %s subcommand '%s'.\n", kCmdPrint, argv[1]);
	debugPrintf("Usage: %s <%s|%s|%s> [arguments]\n", kCmdPrint,
				kSubCmdPrintFeature, kSubCmdPrintRunners, kSubCmdPrintScrs);
	debugPrintf("\n");
	return true;
}

bool ZoombiniConsole::CmdSub_PrintFeature(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Print SCRB feature descriptions.\n");
		debugPrintf("Usage: %s %s [<scrb-resource>]\n\n", kCmdPrint, kSubCmdPrintFeature);
		debugPrintf("With an ID, print one parsed SCRB resource; without an ID,\n");
		debugPrintf("print every SCRB resource on the active page. The resource ID\n");
		debugPrintf("accepts decimal or 0x-prefixed values with an optional p: or\n");
		debugPrintf("s: archive prefix.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (3 < argc) {
		debugPrintf("Print SCRB feature descriptions\n");
		debugPrintf("Usage: %s %s [<scrbId>]\n", kCmdPrint, kSubCmdPrintFeature);
		debugPrintf("\n");
		return true;
	}

	if (argc == 3) {
		ZmbResource resource;
		if (!parseResourceId(argv[2], resource))
			return true;

		if (!_vm->hasResource(ID_SCRB, resource)) {
			debugPrintf("Cannot find resource SCRB(%s)\n", argv[2]);
			debugPrintf("\n");
			return true;
		}

		debugPrintf("%s", buildScrbDescription(resource).c_str());
		debugPrintf("\n");
		return true;
	}

	// Get all SCRB resource IDs from the active page
	Common::Array<int16> resIds = _vm->getResourceIDList(ZmbResource::kPage, ID_SCRB);
	if (resIds.empty()) {
		debugPrintf("No SCRB resources found in current page\n");
		debugPrintf("\n");
		return true;
	}

	for (uint resourceIdx = 0; resourceIdx < resIds.size(); resourceIdx++)
		debugPrintf("%s", buildScrbDescription(ZmbResource(ZmbResource::kPage, resIds[resourceIdx])).c_str());

	debugPrintf("\n");
	return false;
}

bool ZoombiniConsole::CmdSub_PrintRunners(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("List feature runners currently registered on the active page.\n");
		debugPrintf("Usage: %s %s\n\n", kCmdPrint, kSubCmdPrintRunners);
		debugPrintf("The listing includes runner kind, registration index, SCRB and\n");
		debugPrintf("tBMP resources, frame state, flags, and render/animation state.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 2) {
		debugPrintf("Usage: %s %s\n", kCmdPrint, kSubCmdPrintRunners);
		debugPrintf("\n");
		return true;
	}

	ZoombiniPage *page = _vm->getActivePage();
	if (!page) {
		debugPrintf("No active Zoombini page\n");
		debugPrintf("\n");
		return true;
	}

	Common::Array<ZmbLoadedFeatureInfo> features;
	page->collectDebugLoadedFeatures(features);
	debugPrintf("Loaded feature runners on page %u (%u total):\n", static_cast<uint32>(page->getPageType()), features.size());
	for (const ZmbLoadedFeatureInfo &info : features) {
		const ZmbFeature *feature = info._feature;
		uint32 renderActivated;
		uint32 animateActivated;
		if (feature->isRenderActivated())
			renderActivated = 1u;
		else
			renderActivated = 0u;
		if (feature->isAnimateActivated())
			animateActivated = 1u;
		else
			animateActivated = 0u;
		debugPrintf("  [%s] runner=%u registration=%u SCRB=%u tBMP=%s frame=%d flags=%08x render=%u animate=%u\n",
					getLoadedFeatureListKindName(info._listKind), feature->getId(), feature->getRegistrationIndex(), feature->getScrbId(),
					feature->getResource().toString().c_str(), static_cast<int>(feature->getLastFrameIdx()), feature->getFlags(),
					renderActivated, animateActivated);
	}

	debugPrintf("\n");
	return true;
}

Common::String ZoombiniConsole::buildScrbDescription(const ZmbResource &resource) {
	const ZmbScriptDecoder::DecodedScrb *decodedScrb = nullptr;
	ZoombiniPage *page = _vm->getCurrentPage();
	if (resource._archiveKind == ZmbResource::kSystem)
		decodedScrb = _vm->getSystemDecodedScrb(resource._id);
	else if (page)
		decodedScrb = page->getDecodedScrb(resource);
	if (!decodedScrb)
		return Common::String::format("SCRB_%s: malformed resource\n", resource.toString().c_str());

	Common::String text = Common::String::format("SCRB_%s: FrameCount(%u) MaxFrameIdx(%u)\n",
												 resource.toString().c_str(), decodedScrb->frames.size(), decodedScrb->frames.size() - 1);

	for (uint32 frameIdx = 0; frameIdx < decodedScrb->frames.size(); frameIdx++) {
		const ZmbDecodedScriptFrame &frame = decodedScrb->frames[frameIdx];
		for (uint32 hotspotIdx = 0; hotspotIdx < frame.hotspots.size(); hotspotIdx++) {
			const ZmbHotspot &hs = frame.hotspots[hotspotIdx];
			text += Common::String::format("  Frame(%u): Hotspot ID(%u) at (%u, %u)\n", hs._frame, hs._shapeIdx, hs._x, hs._y);
		}
	}

	return text;
}

// [*] Dump subcommands (SCRB features)

bool ZoombiniConsole::CmdSub_DumpFeature(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Export SCRB feature descriptions.\n");
		debugPrintf("Usage: %s %s [<scrb-resource>]\n\n", kCmdDump, kSubCmdDumpFeature);
		debugPrintf("With an ID, export one parsed SCRB resource; without an ID,\n");
		debugPrintf("export every SCRB resource on the active page to one combined\n");
		debugPrintf("file. The resource ID accepts decimal or 0x-prefixed values\n");
		debugPrintf("with an optional p: or s: archive prefix. Files are written\n");
		debugPrintf("below dumps/ and include the active page number in their\n");
		debugPrintf("names.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (3 < argc) {
		debugPrintf("Export SCRB feature descriptions\n");
		debugPrintf("Usage: %s %s [<scrbId>]\n", kCmdDump, kSubCmdDumpFeature);
		debugPrintf("\n");
		return true;
	}

	ZoombiniPage *activePage = _vm->getActivePage();

	if (argc == 3) {
		ZmbResource resource;
		if (!parseResourceId(argv[2], resource))
			return true;

		if (!_vm->hasResource(ID_SCRB, resource)) {
			debugPrintf("Cannot find resource SCRB(%s)\n", argv[2]);
			debugPrintf("\n");
			return true;
		}

		const Common::String filename = Common::String::format("ZOOMBINI_page%02u_SCRB_%s.txt",
															   static_cast<uint32>(activePage->getPageType()), resource.toString().c_str());

		Common::DumpFile out;
		if (!out.open(Common::Path("dumps/" + filename, '/'), true)) {
			debugPrintf("Failed to open file for writing: dumps/%s\n", filename.c_str());
			debugPrintf("\n");
			return true;
		}

		out.writeString(buildScrbDescription(resource));
		out.close();
		debugPrintf("Successfully exported SCRB feature to dumps/%s\n", filename.c_str());
		debugPrintf("\n");
		return false;
	}

	// Get all SCRB resource IDs from the active page
	Common::Array<int16> resIds = _vm->getResourceIDList(ZmbResource::kPage, ID_SCRB);
	if (resIds.empty()) {
		debugPrintf("No SCRB resources found in current page\n");
		debugPrintf("\n");
		return true;
	}

	const Common::String filename = Common::String::format("ZOOMBINI_page%02u_SCRB_all.txt",
														   static_cast<uint32>(activePage->getPageType()));

	Common::DumpFile out;
	if (!out.open(Common::Path("dumps/" + filename, '/'), true)) {
		debugPrintf("Failed to open file for writing: dumps/%s\n", filename.c_str());
		debugPrintf("\n");
		return true;
	}

	for (uint resourceIdx = 0; resourceIdx < resIds.size(); resourceIdx++)
		out.writeString(buildScrbDescription(ZmbResource(ZmbResource::kPage, resIds[resourceIdx])));

	out.close();
	debugPrintf("Successfully exported SCRB features to dumps/%s\n", filename.c_str());
	debugPrintf("\n");
	return false;
}

// [*] Print subcommand (SCRS snoid scripts)

bool ZoombiniConsole::CmdSub_PrintScrs(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Print SCRS snoid script descriptions.\n");
		debugPrintf("Usage: %s %s [<scrs-resource>]\n\n", kCmdPrint, kSubCmdPrintScrs);
		debugPrintf("With an ID, print one parsed SCRS resource; without an ID,\n");
		debugPrintf("print every SCRS resource on the active page. The resource ID\n");
		debugPrintf("accepts decimal or 0x-prefixed values with an optional p: or\n");
		debugPrintf("s: archive prefix.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (3 < argc) {
		debugPrintf("Print SCRS snoid script descriptions\n");
		debugPrintf("Usage: %s %s [<scrsId>]\n", kCmdPrint, kSubCmdPrintScrs);
		debugPrintf("\n");
		return true;
	}

	if (argc == 3) {
		ZmbResource resource;
		if (!parseResourceId(argv[2], resource))
			return true;

		if (!_vm->hasResource(ID_SCRS, resource)) {
			debugPrintf("Cannot find resource SCRS(%s)\n", argv[2]);
			debugPrintf("\n");
			return true;
		}

		debugPrintf("%s", buildScrsDescription(resource).c_str());
		debugPrintf("\n");
		return true;
	}

	Common::Array<int16> resIds = _vm->getResourceIDList(ZmbResource::kPage, ID_SCRS);
	if (resIds.empty()) {
		debugPrintf("No SCRS resources found in current page\n");
		debugPrintf("\n");
		return true;
	}

	for (uint resourceIdx = 0; resourceIdx < resIds.size(); resourceIdx++)
		debugPrintf("%s", buildScrsDescription(ZmbResource(ZmbResource::kPage, resIds[resourceIdx])).c_str());

	debugPrintf("\n");
	return false;
}

Common::String ZoombiniConsole::buildScrsDescription(const ZmbResource &resource) {
	const ZmbScriptDecoder::DecodedScrs *decodedScrs = nullptr;
	ZoombiniPage *page = _vm->getCurrentPage();
	if (resource._archiveKind == ZmbResource::kSystem)
		decodedScrs = _vm->getSystemDecodedScrs(resource._id);
	else if (page)
		decodedScrs = page->getDecodedScrs(resource);
	if (!decodedScrs)
		return Common::String::format("SCRS_%s: malformed resource\n", resource.toString().c_str());

	Common::String text = Common::String::format("SCRS_%s: TraitLayout(%d) FrameCount(%u) MaxFrameIdx(%u)\n",
												 resource.toString().c_str(), static_cast<int16>(decodedScrs->traitLayout), decodedScrs->frames.size(),
												 decodedScrs->frames.size() - 1);

	for (uint32 frameIdx = 0; frameIdx < decodedScrs->frames.size(); frameIdx++) {
		for (uint32 hotspotIdx = 0; hotspotIdx < decodedScrs->frames[frameIdx].hotspots.size(); hotspotIdx++) {
			const ZmbHotspot &hs = decodedScrs->frames[frameIdx].hotspots[hotspotIdx];
			text += Common::String::format("  Frame(%u): Hotspot ID(%u) at (%d, %d)\n", hs._frame, hs._shapeIdx, hs._x, hs._y);
		}
	}

	return text;
}

// [*] Dump subcommands (SCRS snoid scripts)

bool ZoombiniConsole::CmdSub_DumpScrs(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Export SCRS snoid script descriptions.\n");
		debugPrintf("Usage: %s %s [<scrs-resource>]\n\n", kCmdDump, kSubCmdDumpScrs);
		debugPrintf("With an ID, export one parsed SCRS resource; without an\n");
		debugPrintf("ID, export every SCRS resource on the active page to one\n");
		debugPrintf("combined file. The resource ID accepts decimal or 0x-prefixed\n");
		debugPrintf("values with an optional p: or s: archive prefix. Files are\n");
		debugPrintf("written below dumps/ and include the active page number in\n");
		debugPrintf("their names.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (3 < argc) {
		debugPrintf("Export SCRS snoid script descriptions\n");
		debugPrintf("Usage: %s %s [<scrsId>]\n", kCmdDump, kSubCmdDumpScrs);
		debugPrintf("\n");
		return true;
	}

	ZoombiniPage *activePage = _vm->getActivePage();

	if (argc == 3) {
		ZmbResource resource;
		if (!parseResourceId(argv[2], resource))
			return true;

		if (!_vm->hasResource(ID_SCRS, resource)) {
			debugPrintf("Cannot find resource SCRS(%s)\n", argv[2]);
			debugPrintf("\n");
			return true;
		}

		const Common::String filename = Common::String::format("ZOOMBINI_page%02u_SCRS_%s.txt",
															   static_cast<uint32>(activePage->getPageType()), resource.toString().c_str());

		Common::DumpFile out;
		if (!out.open(Common::Path("dumps/" + filename, '/'), true)) {
			debugPrintf("Failed to open file for writing: dumps/%s\n", filename.c_str());
			debugPrintf("\n");
			return true;
		}

		out.writeString(buildScrsDescription(resource));
		out.close();
		debugPrintf("Successfully exported SCRS snoid script to dumps/%s\n", filename.c_str());
		debugPrintf("\n");
		return false;
	}

	Common::Array<int16> resIds = _vm->getResourceIDList(ZmbResource::kPage, ID_SCRS);
	if (resIds.empty()) {
		debugPrintf("No SCRS resources found in current page\n");
		debugPrintf("\n");
		return true;
	}

	const Common::String filename = Common::String::format("ZOOMBINI_page%02u_SCRS_all.txt",
														   static_cast<uint32>(activePage->getPageType()));

	Common::DumpFile out;
	if (!out.open(Common::Path("dumps/" + filename, '/'), true)) {
		debugPrintf("Failed to open file for writing: dumps/%s\n", filename.c_str());
		debugPrintf("\n");
		return true;
	}

	for (uint resourceIdx = 0; resourceIdx < resIds.size(); resourceIdx++)
		out.writeString(buildScrsDescription(ZmbResource(ZmbResource::kPage, resIds[resourceIdx])));

	out.close();
	debugPrintf("Successfully exported SCRS snoid scripts to dumps/%s\n", filename.c_str());
	debugPrintf("\n");
	return false;
}

// [*] Dump subcommands (text resources)

bool ZoombiniConsole::CmdSub_DumpTexts(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Export localized strings and credit lines at runtime.\n");
		debugPrintf("Usage: %s %s [filename]\n\n", kCmdDump, kSubCmdDumpTexts);
		debugPrintf("Without a filename, write dumps/ZOOMBINI_texts.txt. A .csv or\n");
		debugPrintf(".tsv suffix selects tabular output with a header row; any other\n");
		debugPrintf("suffix uses the default key/value text format. Newlines, tabs,\n");
		debugPrintf("and backslashes in text are escaped for the selected format.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (3 < argc) {
		debugPrintf("Dump runtime Zoombini localized strings and credit lines\n");
		debugPrintf("Usage: %s %s [filename]\n", kCmdDump, kSubCmdDumpTexts);
		debugPrintf("\n");
		return true;
	}

	Common::String filepath = (argc == 3) ? argv[2] : "dumps/ZOOMBINI_texts.txt";
	bool csv = filepath.hasSuffixIgnoreCase(".csv");
	bool tsv = filepath.hasSuffixIgnoreCase(".tsv");
	Common::DumpFile out;
	if (!out.open(Common::Path(filepath, '/'), true)) {
		debugPrintf("Failed to open %s for writing\n", filepath.c_str());
		debugPrintf("\n");
		return true;
	}

	if (csv)
		out.writeString("type,key,paragraph,line,blankLines,text\n");
	else if (tsv)
		out.writeString("type\tkey\tparagraph\tline\tblankLines\ttext\n");

	Common::Array<ZoombiniText::LocalizedString> strings;
	_vm->_text->getLocalizedStrings(strings);
	for (const ZoombiniText::LocalizedString &entry : strings) {
		Common::String text = escapeTextDumpField(entry._text);
		if (csv) {
			out.writeString(Common::String::format("string,%u,,,,%s\n",
												   entry._key, quoteCsvDumpField(text).c_str()));
		} else if (tsv) {
			out.writeString(Common::String::format("string\t%u\t\t\t\t%s\n",
												   entry._key, text.c_str()));
		} else {
			out.writeString(Common::String::format("[\"%u\"] = %s\n",
												   entry._key, quoteSimpleDumpField(text).c_str()));
		}
	}

	Common::Array<ZoombiniText::CreditParagraph> paragraphs;
	_vm->_text->getLocalizedCredits(paragraphs);
	for (uint paragraphIndex = 0; paragraphIndex < paragraphs.size(); paragraphIndex++) {
		const ZoombiniText::CreditParagraph &paragraph = paragraphs[paragraphIndex];
		for (uint lineIndex = 0; lineIndex < paragraph._lines.size(); lineIndex++) {
			Common::String text = escapeTextDumpField(paragraph._lines[lineIndex]);
			Common::String creditKey = ZoombiniText::formatCreditLineKey(paragraphIndex, lineIndex);
			if (csv) {
				out.writeString(Common::String::format("credit,%s,%u,%u,%u,%s\n",
													   creditKey.c_str(), paragraphIndex, lineIndex, paragraph._blankLineCount, quoteCsvDumpField(text).c_str()));
			} else if (tsv) {
				out.writeString(Common::String::format("credit\t%s\t%u\t%u\t%u\t%s\n",
													   creditKey.c_str(), paragraphIndex, lineIndex, paragraph._blankLineCount, text.c_str()));
			} else {
				out.writeString(Common::String::format("[\"%s\"] = %s\n",
													   creditKey.c_str(),
													   quoteSimpleDumpField(text).c_str()));
			}
		}
	}

	out.close();
	debugPrintf("Dumped %u strings and %u credit paragraphs to %s\n", strings.size(), paragraphs.size(), filepath.c_str());
	debugPrintf("\n");
	return true;
}

// [*] Dump subcommands (save file)
bool ZoombiniConsole::CmdSub_DumpSave(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Export the live game state in the raw save layout.\n");
		debugPrintf("Usage: %s %s\n\n", kCmdDump, kSubCmdDumpSave);
		debugPrintf("The current game must be bound to a saved roster slot. The command\n");
		debugPrintf("serializes the in-memory game state with the release's save writer\n");
		debugPrintf("and writes the result directly to dumps/ZOOM####.TXT. Unsaved\n");
		debugPrintf("in-memory changes are part of the export; the stored save file is\n");
		debugPrintf("not read and the roster file is not exported.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 2) {
		debugPrintf("Usage: %s %s\n", kCmdDump, kSubCmdDumpSave);
		debugPrintf("\n");
		return true;
	}

	if (!requireGameStateReady(Common::String::format("%s %s", kCmdDump, kSubCmdDumpSave)))
		return true;

	if (_vm->_state->getActiveSaveSlot() == ZoombiniGameState::kUnsavedNewGame) {
		debugPrintf("The current game has not been saved yet; there is no ZOOM#### name for the export.\n");
		debugPrintf("\n");
		return true;
	}

	ZmbRosterEntry *entry = _vm->_state->getActiveSaveRosterEntry();
	if (!entry) {
		debugPrintf("Cannot resolve the roster entry of the current game.\n");
		debugPrintf("\n");
		return true;
	}
	// Rebuild the legacy ZOOM#### stem from its validated ASCII digits
	// instead of decoding and re-encoding the raw bytes.
	const Common::String saveFileNumStr = entry->getSaveFileNumStr();
	if (saveFileNumStr.empty()) {
		debugPrintf("The roster entry of the current game has an invalid state-file stem.\n");
		debugPrintf("\n");
		return true;
	}

	const Common::String filepath = Common::String::format("dumps/ZOOM%s.TXT", saveFileNumStr.c_str());

	Common::DumpFile out;
	if (!out.open(Common::Path(filepath, '/'), true)) {
		debugPrintf("Failed to open file for writing: %s\n", filepath.c_str());
		debugPrintf("\n");
		return true;
	}

	_vm->_state->dumpCurrentState(out);
	const bool writeFailed = out.err();
	const uint32 size = static_cast<uint32>(out.pos());
	out.close();

	if (writeFailed) {
		debugPrintf("Failed while writing %s\n", filepath.c_str());
		debugPrintf("\n");
		return true;
	}

	debugPrintf("Successfully exported live game state to %s (%u bytes)\n", filepath.c_str(), size);
	debugPrintf("\n");
	return true;
}

// [*] Plot commands

bool ZoombiniConsole::Cmd_Plot(int argc, const char **argv) {
	if (argc == 2 && isHelpOption(argv[1])) {
		debugPrintf("Open a renderer debug dialog that plots primitive geometry.\n");
		debugPrintf("Usage: %s <subcommand> [arguments]\n\n", kCmdPlot);
		debugPrintf("Subcommands:\n");
		debugPrintf("  %s <x> <y> [color]\n", kSubCmdPlotPoint);
		debugPrintf("      Plot one pixel.\n");
		debugPrintf("  %s <x0> <y0> <x1> <y1> [color]\n", kSubCmdPlotLine);
		debugPrintf("      Plot a line between two points.\n");
		debugPrintf("  %s <x1> <y1> <x2> <y2> [color]\n", kSubCmdPlotRect);
		debugPrintf("      Plot a rectangle from top-left to bottom-right.\n\n");
		debugPrintf("Coordinates are 0-based; integer arguments accept a 0x prefix.\n");
		debugPrintf("Color is an 8-bit palette index and defaults to white.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 2) {
		debugPrintf("Usage: %s <%s|%s|%s> ...\n", kCmdPlot, kSubCmdPlotPoint, kSubCmdPlotLine, kSubCmdPlotRect);
		debugPrintf("  %s: %s %s <x> <y> [color]\n", kSubCmdPlotPoint, kCmdPlot, kSubCmdPlotPoint);
		debugPrintf("  %s:  %s %s <x0> <y0> <x1> <y1> [color]\n", kSubCmdPlotLine, kCmdPlot, kSubCmdPlotLine);
		debugPrintf("  %s:  %s %s <x1> <y1> <x2> <y2> [color]\n", kSubCmdPlotRect, kCmdPlot, kSubCmdPlotRect);
		debugPrintf("\n");
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdPlotPoint) == 0)
		return CmdSub_PlotPoint(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdPlotLine) == 0)
		return CmdSub_PlotLine(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdPlotRect) == 0)
		return CmdSub_PlotRect(argc, argv);

	debugPrintf("Unknown %s type '%s'. Use %s, %s, or %s.\n", kCmdPlot, argv[1],
				kSubCmdPlotPoint, kSubCmdPlotLine, kSubCmdPlotRect);
	debugPrintf("\n");
	return true;
}

bool ZoombiniConsole::CmdSub_PlotPoint(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Plot one pixel in the shape screen.\n");
		debugPrintf("Usage: %s %s <x> <y> [color]\n\n", kCmdPlot, kSubCmdPlotPoint);
		debugPrintf("x and y are 0-based screen coordinates. color is an 8-bit\n");
		debugPrintf("palette index and defaults to %u (white).\n\n", ZoombiniGraphics::kColor0A_White);
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 4 || 5 < argc) {
		debugPrintf("Usage: %s %s <x> <y> [color]\n", kCmdPlot, kSubCmdPlotPoint);
		debugPrintf("  x, y: coordinates (0-based, hex supported with 0x prefix)\n");
		debugPrintf("  color: color value (8-bit palette index, default: %u)\n", ZoombiniGraphics::kColor0A_White);
		debugPrintf("\n");
		return true;
	}

	int32 xVal = 0, yVal = 0, colorVal = 0;
	if (!parseInt(argv[2], xVal) || !parseInt(argv[3], yVal)) {
		debugPrintf("\n");
		return true;
	}

	uint32 color = static_cast<uint32>(ZoombiniGraphics::kColor0A_White);
	if (argc == 5) {
		if (!parseInt(argv[4], colorVal)) {
			debugPrintf("\n");
			return true;
		}
		if (colorVal < 0 || 0xFF < colorVal) {
			debugPrintf("Error: Color must be 0-255\n");
			debugPrintf("\n");
			return true;
		}
		color = static_cast<uint32>(colorVal);
	}

	int16 x = static_cast<int16>(xVal);
	int16 y = static_cast<int16>(yVal);

	Graphics::Surface *screen = _vm->_gfx->getScreen(ZoombiniGraphics::kShapeScreen);
	if (x < 0 || screen->w <= x || y < 0 || screen->h <= y) {
		debugPrintf("Coordinates out of bounds (screen size: %d x %d)\n", screen->w, screen->h);
		debugPrintf("\n");
		return true;
	}

	ZoombiniDebugCommand cmd;
	cmd.setPlotPoint(x, y, color);
	_vm->openDebugDialog(cmd);
	return false;
}

bool ZoombiniConsole::CmdSub_PlotLine(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Plot a line in the shape screen.\n");
		debugPrintf("Usage: %s %s <x0> <y0> <x1> <y1> [color]\n\n", kCmdPlot, kSubCmdPlotLine);
		debugPrintf("(x0, y0) is the 0-based start coordinate and (x1, y1) is the\n");
		debugPrintf("0-based end coordinate. color is an 8-bit palette index and\n");
		debugPrintf("defaults to %u (white).\n\n", ZoombiniGraphics::kColor0A_White);
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 6 || 7 < argc) {
		debugPrintf("Usage: %s %s <x0> <y0> <x1> <y1> [color]\n", kCmdPlot, kSubCmdPlotLine);
		debugPrintf("  x0, y0: start coordinates (0-based, hex supported with 0x prefix)\n");
		debugPrintf("  x1, y1: end coordinates (0-based, hex supported with 0x prefix)\n");
		debugPrintf("  color: color value (8-bit palette index, default: %u)\n", ZoombiniGraphics::kColor0A_White);
		debugPrintf("\n");
		return true;
	}

	int32 x0Val = 0, y0Val = 0, x1Val = 0, y1Val = 0, colorVal = 0;
	if (!parseInt(argv[2], x0Val) || !parseInt(argv[3], y0Val) ||
		!parseInt(argv[4], x1Val) || !parseInt(argv[5], y1Val)) {
		debugPrintf("\n");
		return true;
	}

	uint32 color = static_cast<uint32>(ZoombiniGraphics::kColor0A_White);
	if (argc == 7) {
		if (!parseInt(argv[6], colorVal)) {
			debugPrintf("\n");
			return true;
		}
		if (colorVal < 0 || 0xFF < colorVal) {
			debugPrintf("Error: Color must be 0-255\n");
			debugPrintf("\n");
			return true;
		}
		color = static_cast<uint32>(colorVal);
	}

	int16 x0 = static_cast<int16>(x0Val);
	int16 y0 = static_cast<int16>(y0Val);
	int16 x1 = static_cast<int16>(x1Val);
	int16 y1 = static_cast<int16>(y1Val);

	ZoombiniDebugCommand cmd;
	cmd.setPlotLine(x0, y0, x1, y1, color);
	_vm->openDebugDialog(cmd);
	return false;
}

bool ZoombiniConsole::CmdSub_PlotRect(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Plot a rectangle in the shape screen.\n");
		debugPrintf("Usage: %s %s <x1> <y1> <x2> <y2> [color]\n\n", kCmdPlot, kSubCmdPlotRect);
		debugPrintf("(x1, y1) is the 0-based top-left corner and (x2, y2) is the\n");
		debugPrintf("bottom-right corner. x2 must be greater than x1 and y2 must be\n");
		debugPrintf("greater than y1. color is an 8-bit palette index and defaults to\n");
		debugPrintf("%u (white).\n\n", ZoombiniGraphics::kColor0A_White);
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 6 || 7 < argc) {
		debugPrintf("Usage: %s %s <x1> <y1> <x2> <y2> [color]\n", kCmdPlot, kSubCmdPlotRect);
		debugPrintf("  x1, y1: top-left corner coordinates (0-based, hex supported with 0x prefix)\n");
		debugPrintf("  x2, y2: bottom-right corner coordinates (0-based, hex supported with 0x prefix)\n");
		debugPrintf("  color: color value (8-bit palette index, default: %u)\n", ZoombiniGraphics::kColor0A_White);
		debugPrintf("\n");
		return true;
	}

	int32 x1Val = 0, y1Val = 0, x2Val = 0, y2Val = 0, colorVal = 0;
	if (!parseInt(argv[2], x1Val) || !parseInt(argv[3], y1Val) ||
		!parseInt(argv[4], x2Val) || !parseInt(argv[5], y2Val)) {
		debugPrintf("\n");
		return true;
	}

	uint32 color = static_cast<uint32>(ZoombiniGraphics::kColor0A_White);
	if (argc == 7) {
		if (!parseInt(argv[6], colorVal)) {
			debugPrintf("\n");
			return true;
		}
		if (colorVal < 0 || 0xFF < colorVal) {
			debugPrintf("Error: Color must be 0-255\n");
			debugPrintf("\n");
			return true;
		}
		color = static_cast<uint32>(colorVal);
	}

	int16 x1 = static_cast<int16>(x1Val);
	int16 y1 = static_cast<int16>(y1Val);
	int16 x2 = static_cast<int16>(x2Val);
	int16 y2 = static_cast<int16>(y2Val);

	if (x2 <= x1 || y2 <= y1) {
		debugPrintf("Invalid rectangle coordinates\n");
		debugPrintf("\n");
		return true;
	}

	ZoombiniDebugCommand cmd;
	cmd.setPlotRect(x1, y1, x2, y2, color);
	_vm->openDebugDialog(cmd);
	return false;
}

// [*] Game state commands

bool ZoombiniConsole::Cmd_Go(int argc, const char **argv) {
	if (argc == 2 && isHelpOption(argv[1])) {
		debugPrintf("Navigate directly to a transition or puzzle page.\n");
		debugPrintf("Usage: %s <subcommand> [arguments]\n\n", kCmdGo);
		debugPrintf("Subcommands:\n");
		debugPrintf("  %s <destination> [level]\n", kSubCmdGoXfer);
		debugPrintf("      Jump to a selected transition page.\n");
		debugPrintf("  %s <puzzle> <level> [count]\n", kSubCmdGoPractice);
		debugPrintf("      Start a selected puzzle directly in practice mode.\n\n");
		debugPrintf("Use '%s %s --help' or '%s %s --help' for detailed help.\n\n",
					kCmdGo, kSubCmdGoXfer, kCmdGo, kSubCmdGoPractice);
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 2) {
		debugPrintf("Usage: %s <%s|%s> ...\n", kCmdGo, kSubCmdGoXfer, kSubCmdGoPractice);
		debugPrintf("\n");
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdGoXfer) == 0)
		return CmdSub_GoXfer(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdGoPractice) == 0)
		return CmdSub_GoPractice(argc, argv);

	debugPrintf("Unknown %s subcommand '%s'.\n", kCmdGo, argv[1]);
	debugPrintf("Usage: %s <%s|%s> ...\n", kCmdGo, kSubCmdGoXfer, kSubCmdGoPractice);
	debugPrintf("\n");
	return true;
}

bool ZoombiniConsole::CmdSub_GoXfer(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Jump directly to a selected transition page.\n");
		debugPrintf("Usage: %s %s <destination> [level]\n\n", kCmdGo, kSubCmdGoXfer);
		debugPrintf("<destination> is a puzzle destination name or DI page number.\n");
		debugPrintf("[level] is an optional difficulty from 1 (easiest) to 4 (hardest).\n");
		debugPrintf("The command preserves or creates an active debug pack, simulates\n");
		debugPrintf("the source puzzle completion flags, and closes the debugger after\n");
		debugPrintf("queuing the transition.\n\n");
		debugPrintf("Destinations:\n");
		debugPrintf("  bridge, tunnels, pizza, bc1, ferry, lilly, slides, bc2north,\n");
		debugPrintf("  fleens, hotel, net, bc2south, caves, smoke, maze, town\n");
		debugPrintf("Numeric DI page values are also accepted. This command requires\n");
		debugPrintf("a created game or a loaded save.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (!requireGameStateReady(Common::String::format("%s %s", kCmdGo, kSubCmdGoXfer)))
		return true;

	uint32 xferRouteCount = 0;
	const ZmbXferRouteInfo *xferRoutes = ZmbXferRouteInfo::getZmbXferRouteInfos(xferRouteCount);

	if (argc < 3 || 4 < argc) {
		debugPrintf("Jump to the xfer (transition) page to a chosen destination.\n");
		debugPrintf("Usage: %s %s <destination> [level]\n", kCmdGo, kSubCmdGoXfer);
		debugPrintf("  destination: name or DI page number of the target puzzle\n");
		debugPrintf("  level: optional difficulty level (1-4)\n");
		debugPrintf("Available destinations:\n");
		for (uint i = 0; i < xferRouteCount; i++) {
			debugPrintf("  %-10s  (DI %2d)  ", xferRoutes[i].name,
						static_cast<int>(xferRoutes[i].destPage));
			if (xferRoutes[i].srcPageName) {
				debugPrintf("[%s] %s%s%s\n", xferRoutes[i].routeName, xferRoutes[i].srcPageName, kXferRouteArrow, xferRoutes[i].destName);
			} else {
				debugPrintf("%s%s%s\n", xferRoutes[i].routeName, kXferRouteArrow, xferRoutes[i].destName);
			}
		}
		debugPrintf("\n");
		return true;
	}

	// Parse optional level parameter
	uint16 level = 0;
	if (argc == 4) {
		int32 levelVal;
		if (!ZmbResource::parseInt(argv[3], levelVal) || levelVal < 1 || 4 < levelVal) {
			debugPrintf("Invalid level '%s'. Must be 1-4.\n", argv[3]);
			debugPrintf("\n");
			return true;
		}
		level = static_cast<uint16>(levelVal);
	}

	// Match by name (case-insensitive) or DI page number
	ZmbSrcPageKind srcSiPage = ZmbSrcPageKind::kMinus1;
	for (uint i = 0; i < xferRouteCount; i++) {
		if (scumm_stricmp(argv[2], xferRoutes[i].name) == 0) {
			srcSiPage = xferRoutes[i].srcPage;
			break;
		}
	}

	const char *destArg = argv[2];
	const bool destLooksNumeric = ('0' <= destArg[0] && destArg[0] <= '9') ||
								  destArg[0] == '-' || destArg[0] == '+';
	if (srcSiPage == ZmbSrcPageKind::kMinus1 && destLooksNumeric) {
		int32 numVal;
		if (ZmbResource::parseInt(destArg, numVal)) {
			for (uint i = 0; i < xferRouteCount; i++) {
				if (static_cast<int16>(numVal) == static_cast<int16>(xferRoutes[i].destPage)) {
					srcSiPage = xferRoutes[i].srcPage;
					break;
				}
			}
		}
	}

	if (srcSiPage == ZmbSrcPageKind::kMinus1) {
		debugPrintf("Unknown destination '%s'. Use %s %s without arguments to see available destinations.\n",
					argv[2], kCmdGo, kSubCmdGoXfer);
		debugPrintf("\n");
		return true;
	}

	// Materialize the live party before using the active pack for the transition.
	// Puzzle setup clears every pack Snoid's occupancy, and only passing Snoids become occupied again.
	// @ref ZoombiniConsole::CmdSub_GoXfer() bypasses that result and must carry every Snoid still owned by the puzzle.
	// Rest pages already distinguish the traveling party from residents, so preserve their stored occupancy.
	ZoombiniPage *activePage = _vm->getActivePage();
	if (activePage && 0 < activePage->getPackSnoidCount()) {
		const ZoombiniPageType pageType = activePage->getPageType();
		if (ZoombiniPageType::kBridge <= pageType && pageType <= ZoombiniPageType::kMaze)
			activePage->schedulePackSnoids(true, true);
		activePage->saveSnoidsToPack();
	}

	ZmbStateActivePack &activePack = _vm->_state->_f._zmbPackActive;
	int16 generatedSnoidCount = 0;
	int16 debitedIsleCount = 0;
	if (activePack.getPackZmbCount() == 0) {
		// An empty page and empty active pack need a usable debug fallback.
		// Apply the same trait availability and 625-entry limit as Picker.
		generatedSnoidCount = _vm->_state->generatePickerRandomPack();
		debitedIsleCount = _vm->_state->subtractDebugGeneratedSnoidsFromIsle(generatedSnoidCount);
	}
	_vm->_state->markDebugStateMutation();

	// Set the difficulty when specified, as in practice mode.
	if (0 < level)
		_vm->_state->_practiceLevel = level;

	// Simulate source-puzzle completion so its per-puzzle level flag is up to date.
	// @ref ZoombiniInteractive::executeDeparture() normally updates routing before starting the transition.
	// It calls @ref ZoombiniInteractive::routeNonOccupiedToRestingPack() for that update.
	// That records the route-level bit and perfect-clear bit in @ref ZmbStateFile::_pageLevelFlags.
	// @ref ZoombiniConsole::CmdSub_GoXfer() bypasses the puzzle, so update those flags explicitly.
	// Otherwise, the preceding segment would use stale completion colors.
	const ZmbXferRouteInfo *xferRoute = ZmbXferRouteInfo::getZmbXferRouteInfo(srcSiPage);
	const ZmbDestPageKind srcDi = xferRoute ? xferRoute->srcPuzzlePage : ZmbDestPageKind::kUnk_00;
	const int16 xferRouteId = xferRoute ? static_cast<int16>(xferRoute->routeId) : -1;
	if (0 <= xferRouteId && !_vm->_state->inPracticeMode()) {
		ZmbStateFile &f = _vm->_state->_f;
		// Container icons use route-completion slots rather than the per-puzzle slots.
		// A debug jump can skip the container departure that normally records them.
		for (int16 routeId = 0; routeId < xferRouteId; routeId += 1)
			f.setRouteCompletionFlag(static_cast<ZmbRouteId>(routeId), f._routeLevels[routeId]);

		if (srcDi != ZmbDestPageKind::kUnk_00) {
			const uint16 srcRouteLevel = f._routeLevels[xferRouteId];
			const uint8 srcBitmask = static_cast<uint8>(1 << (srcRouteLevel & 3));
			// Set both the played and perfect nibbles because the simulated jump assumes a perfect run.
			// A natural play-through completes every earlier puzzle on the route at the current level.
			// The route map draws each traveled leg from its puzzle flag, so backfill every preceding puzzle.
			// For example, "go xfer pizza" at level 4 needs Bridge bit 3 as well as the Tunnels bit.
			const int16 firstDi = static_cast<int16>(ZmbDestPageKind::kBridge_07) + xferRouteId * 3;
			for (int16 di = firstDi; di <= static_cast<int16>(srcDi); di++) {
				f._pageLevelFlags[di - 4] |= srcBitmask;
				f._pageLevelFlags[di - 4] |= static_cast<uint8>(srcBitmask << 4);
			}
		}
	}

	// Close the current page and queue the transition.
	_vm->_debugPreserveActivePackOnXferClose = true;
	_vm->_xferSrcPage = srcSiPage;
	_vm->setNextPage(ZoombiniPageType::kXfer);
	if (_vm->getActivePage())
		_vm->getActivePage()->close();

	if (0 < generatedSnoidCount)
		debugPrintf("Active pack was empty; generated %d random snoids (%d debited from Isle)\n", generatedSnoidCount, debitedIsleCount);
	else
		debugPrintf("Preserving %d snoids in the current active pack\n", activePack.getPackZmbCount());
	debugPrintf("Jumping to xfer with destination (level %u)\n", level);
	debugPrintf("\n");
	return false; // Close the debugger console
}

bool ZoombiniConsole::CmdSub_GoPractice(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Start a selected puzzle directly in practice mode.\n");
		debugPrintf("Usage: %s %s <puzzle> <level> [count]\n\n", kCmdGo, kSubCmdGoPractice);
		debugPrintf("<puzzle> is a puzzle name or DI page number. <level> is 1-4,\n");
		debugPrintf("from easiest to hardest. [count] is an optional 1-16 Zoombini\n");
		debugPrintf("pack size that defaults to 16. The command creates the practice\n");
		debugPrintf("pack, skips the transition page, and closes the debugger.\n\n");
		debugPrintf("Puzzles:\n");
		debugPrintf("  bridge, tunnels, pizza, ferry, lilly, slides, fleens, hotel,\n");
		debugPrintf("  net, caves, smoke, maze\n");
		debugPrintf("Numeric DI page values are also accepted. This command requires\n");
		debugPrintf("a created game or a loaded save.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (!requireGameStateReady(Common::String::format("%s %s", kCmdGo, kSubCmdGoPractice)))
		return true;

	uint32 xferRouteCount = 0;
	const ZmbXferRouteInfo *xferRoutes = ZmbXferRouteInfo::getZmbXferRouteInfos(xferRouteCount);
	const auto isPuzzleDestination = [](const ZmbXferRouteInfo &route) {
		return ZmbDestPageKind::kBridge_07 <= route.destPage && route.destPage <= ZmbDestPageKind::kMaze_18;
	};

	if (argc < 4 || 5 < argc) {
		debugPrintf("Jump directly to a puzzle page in practice mode at a specified difficulty level.\n");
		debugPrintf("Usage: %s %s <puzzle> <level> [count]\n", kCmdGo, kSubCmdGoPractice);
		debugPrintf("  <level> is 1-4 (1=easiest, 4=hardest)\n");
		debugPrintf("  [count] is the optional active-pack size, 1-16 (default 16)\n");
		debugPrintf("Available puzzles:\n");
		for (uint i = 0; i < xferRouteCount; i++) {
			if (!isPuzzleDestination(xferRoutes[i]))
				continue;
			debugPrintf("  %-10s  %s\n", xferRoutes[i].name, xferRoutes[i].destName);
		}
		debugPrintf("\n");
		return true;
	}

	// Parse puzzle name or page type number
	ZoombiniPageType targetPage = ZoombiniPageType::kNone;
	int32 numVal;
	if (ZmbResource::parseInt(argv[2], numVal)) {
		for (uint i = 0; i < xferRouteCount; i++) {
			if (!isPuzzleDestination(xferRoutes[i]))
				continue;
			if (static_cast<int16>(numVal) == static_cast<int16>(xferRoutes[i].destPage)) {
				targetPage = static_cast<ZoombiniPageType>(static_cast<int16>(xferRoutes[i].destPage));
				break;
			}
		}
	} else {
		for (uint i = 0; i < xferRouteCount; i++) {
			if (!isPuzzleDestination(xferRoutes[i]))
				continue;
			if (scumm_stricmp(argv[2], xferRoutes[i].name) == 0) {
				targetPage = static_cast<ZoombiniPageType>(static_cast<int16>(xferRoutes[i].destPage));
				break;
			}
		}
	}

	if (targetPage == ZoombiniPageType::kNone) {
		debugPrintf("Unknown puzzle '%s'. Use %s %s without arguments to see available puzzles.\n",
					argv[2], kCmdGo, kSubCmdGoPractice);
		debugPrintf("\n");
		return true;
	}

	// Parse level
	int32 level;
	if (!ZmbResource::parseInt(argv[3], level) || level < 1 || 4 < level) {
		debugPrintf("Invalid level '%s'. Must be 1-4.\n", argv[3]);
		debugPrintf("\n");
		return true;
	}

	// Parse the optional active-pack size
	int16 packCount = 16;
	if (argc == 5) {
		int32 countVal;
		if (!ZmbResource::parseInt(argv[4], countVal) || countVal < 1 || 16 < countVal) {
			debugPrintf("Invalid count '%s'. Must be 1-16.\n", argv[4]);
			debugPrintf("\n");
			return true;
		}
		packCount = static_cast<int16>(countVal);
	}

	// Set practice mode at the specified difficulty level
	_vm->_state->_practiceLevel = static_cast<uint16>(level);

	// Generate random snoids as the active pack
	_vm->_state->generateRandomPack(packCount);
	_vm->_state->markDebugStateMutation();

	// Navigate directly to the puzzle page (skip xfer transition)
	_vm->setNextPage(targetPage);
	if (_vm->getActivePage())
		_vm->getActivePage()->close();

	debugPrintf("Practice mode: level %d\n", static_cast<int>(level));
	debugPrintf("Generated %d random snoids in active pack\n", static_cast<int>(packCount));
	debugPrintf("Jumping directly to puzzle page %d\n", static_cast<int>(targetPage));
	debugPrintf("\n");
	return false; // Close the debugger console
}

// [*] State subcommands

void ZoombiniConsole::printAvailableStateKeys() {
	debugPrintf("Gameplay toggles:\n");
	debugPrintf("  %s\n", kStateKeyOptionToggleSfx);
	debugPrintf("  %s\n", kStateKeyOptionToggleBgm);
	debugPrintf("  %s\n", kStateKeyOptionToggleStickyMouse);
	debugPrintf("  %s\n", kStateKeyOptionToggleCursor);
	debugPrintf("  %s\n", kStateKeyOptionToggleAutoStickyMouse);
	debugPrintf("  %s\n", kStateKeyOptionToggleLessAction);
	debugPrintf("  %s\n", kStateKeyOptionToggleTransitions);

	if (_vm->isVersionFamilyTlcV2()) {
		debugPrintf("  %s\n", kStateKeyOptionToggleTouchSense);
		debugPrintf("  %s\n", kStateKeyOptionToggleHelpAudio);
	}

	debugPrintf("Route difficulty levels (settable range 1-4):\n");
	debugPrintf("  %s\n", kStateKeyProgressRoute1Level);
	debugPrintf("  %s\n", kStateKeyProgressRoute2Level);
	debugPrintf("  %s\n", kStateKeyProgressRoute3Level);
	debugPrintf("  %s\n", kStateKeyProgressRoute4Level);

	if (_vm->hasRoutePerfectCounterState()) {
		debugPrintf("Route perfect counters (settable range 0-2; normal gameplay advances the route level at 3):\n");
		debugPrintf("  %s\n", kStateKeyProgressRoute1PerfectCounter);
		debugPrintf("  %s\n", kStateKeyProgressRoute2PerfectCounter);
		debugPrintf("  %s\n", kStateKeyProgressRoute3PerfectCounter);
		debugPrintf("  %s\n", kStateKeyProgressRoute4PerfectCounter);
	}

	debugPrintf("Town memorial records (settable from RodMap or a shelter page):\n");
	debugPrintf("  %s (0/1, on/off, true/false, or yes/no)\n", kStateKeyMemorialActivePattern);
	debugPrintf("  %s (yyyyMMdd)\n", kStateKeyMemorialDatePattern);

	debugPrintf("Gameplay state:\n");
	debugPrintf("  %s\n", kStateKeyGameplayDebug);
	debugPrintf("MIDI runtime state (not saved; retained across page changes):\n");
	debugPrintf("  %s\n", kStateKeyMidiDebug);
	debugPrintf("Fleens trait transforms (settable range noted; next setup applies its normal level rules):\n");
	debugPrintf("  %s (0-5)\n", kStateKeyGameplayFleensHairValueRotation);
	debugPrintf("  %s (0-5)\n", kStateKeyGameplayFleensEyesValueRotation);
	debugPrintf("  %s (0-5)\n", kStateKeyGameplayFleensNoseValueRotation);
	debugPrintf("  %s (0-5)\n", kStateKeyGameplayFleensFeetValueRotation);
	debugPrintf("  %s (0=direct, 1-4=one-based Fleen body slot)\n", kStateKeyGameplayFleensHairDestSlot);
	debugPrintf("  %s (0=direct, 1-4=one-based Fleen body slot)\n", kStateKeyGameplayFleensEyesDestSlot);
	debugPrintf("  %s (0=direct, 1-4=one-based Fleen body slot)\n", kStateKeyGameplayFleensNoseDestSlot);
	debugPrintf("  %s (0=direct, 1-4=one-based Fleen body slot)\n", kStateKeyGameplayFleensFeetDestSlot);

	debugPrintf("Maze next-layout selector (base|alt; restored only at maze difficulty 4):\n");
	debugPrintf("  %s\n", kStateKeyGameplayMazeNextLayout);
}

void ZoombiniConsole::printStateSetWarning() {
	debugPrintf("Experimental feature: [state set] may corrupt the save.\n");
	debugPrintf("Use [state set] at your own risk; you are responsible for any side effects.\n");
}

ZoombiniConsole::StateKeyKind ZoombiniConsole::parseStateKey(const char *key) {
	if (scumm_stricmp(key, kStateKeyOptionToggleSfx) == 0)
		return kStateKindSfx;
	if (scumm_stricmp(key, kStateKeyOptionToggleBgm) == 0)
		return kStateKindBgm;
	if (scumm_stricmp(key, kStateKeyOptionToggleStickyMouse) == 0)
		return kStateKindStickyMouse;
	if (scumm_stricmp(key, kStateKeyOptionToggleCursor) == 0)
		return kStateKindCursor;
	if (scumm_stricmp(key, kStateKeyOptionToggleAutoStickyMouse) == 0)
		return kStateKindAutoStickyMouse;
	if (scumm_stricmp(key, kStateKeyOptionToggleLessAction) == 0)
		return kStateKindLessAction;
	if (scumm_stricmp(key, kStateKeyOptionToggleTransitions) == 0)
		return kStateKindTransitions;
	if (scumm_stricmp(key, kStateKeyOptionToggleTouchSense) == 0)
		return kStateKindTouchSense;
	if (scumm_stricmp(key, kStateKeyOptionToggleHelpAudio) == 0)
		return kStateKindHelpAudio;
	if (scumm_stricmp(key, kStateKeyProgressRoute1Level) == 0)
		return kStateKindRoute1Level;
	if (scumm_stricmp(key, kStateKeyProgressRoute2Level) == 0)
		return kStateKindRoute2Level;
	if (scumm_stricmp(key, kStateKeyProgressRoute3Level) == 0)
		return kStateKindRoute3Level;
	if (scumm_stricmp(key, kStateKeyProgressRoute4Level) == 0)
		return kStateKindRoute4Level;
	if (scumm_stricmp(key, kStateKeyProgressRoute1PerfectCounter) == 0)
		return kStateKindRoute1PerfectCounter;
	if (scumm_stricmp(key, kStateKeyProgressRoute2PerfectCounter) == 0)
		return kStateKindRoute2PerfectCounter;
	if (scumm_stricmp(key, kStateKeyProgressRoute3PerfectCounter) == 0)
		return kStateKindRoute3PerfectCounter;
	if (scumm_stricmp(key, kStateKeyProgressRoute4PerfectCounter) == 0)
		return kStateKindRoute4PerfectCounter;
	if (scumm_stricmp(key, kStateKeyGameplayDebug) == 0)
		return kStateKindDebug;
	if (scumm_stricmp(key, kStateKeyMidiDebug) == 0)
		return kStateKindMidiDebug;
	if (scumm_stricmp(key, kStateKeyGameplayMazeNextLayout) == 0)
		return kStateKindMazeNextLayout;

	return kStateKindUnknown;
}

ZoombiniConsole::StateKeyKind ZoombiniConsole::parseMemorialStateKey(const char *key, uint &routeIndex, uint &difficultyLevel) {
	routeIndex = 0;
	difficultyLevel = 0;

	for (uint routeNumber = 1; routeNumber <= 4; routeNumber += 1) {
		for (uint levelNumber = 1; levelNumber <= 4; levelNumber += 1) {
			const Common::String activeKey = Common::String::format("memorial.route%u.level%u.active", routeNumber, levelNumber);
			if (scumm_stricmp(key, activeKey.c_str()) == 0) {
				routeIndex = routeNumber - 1;
				difficultyLevel = levelNumber;
				return kStateKindMemorialActive;
			}

			const Common::String dateKey = Common::String::format("memorial.route%u.level%u.date", routeNumber, levelNumber);
			if (scumm_stricmp(key, dateKey.c_str()) == 0) {
				routeIndex = routeNumber - 1;
				difficultyLevel = levelNumber;
				return kStateKindMemorialDate;
			}
		}
	}

	return kStateKindUnknown;
}

ZoombiniConsole::StateKeyKind ZoombiniConsole::parseFleensTraitStateKey(const char *key, uint &traitIndex) {
	traitIndex = 0;

	if (scumm_stricmp(key, kStateKeyGameplayFleensHairValueRotation) == 0) {
		traitIndex = static_cast<uint>(ZmbTrait::kTraitHair);
		return kStateKindFleensTraitValueRotation;
	}
	if (scumm_stricmp(key, kStateKeyGameplayFleensEyesValueRotation) == 0) {
		traitIndex = static_cast<uint>(ZmbTrait::kTraitEyes);
		return kStateKindFleensTraitValueRotation;
	}
	if (scumm_stricmp(key, kStateKeyGameplayFleensNoseValueRotation) == 0) {
		traitIndex = static_cast<uint>(ZmbTrait::kTraitNose);
		return kStateKindFleensTraitValueRotation;
	}
	if (scumm_stricmp(key, kStateKeyGameplayFleensFeetValueRotation) == 0) {
		traitIndex = static_cast<uint>(ZmbTrait::kTraitFeet);
		return kStateKindFleensTraitValueRotation;
	}
	if (scumm_stricmp(key, kStateKeyGameplayFleensHairDestSlot) == 0) {
		traitIndex = static_cast<uint>(ZmbTrait::kTraitHair);
		return kStateKindFleensTraitDestSlot;
	}
	if (scumm_stricmp(key, kStateKeyGameplayFleensEyesDestSlot) == 0) {
		traitIndex = static_cast<uint>(ZmbTrait::kTraitEyes);
		return kStateKindFleensTraitDestSlot;
	}
	if (scumm_stricmp(key, kStateKeyGameplayFleensNoseDestSlot) == 0) {
		traitIndex = static_cast<uint>(ZmbTrait::kTraitNose);
		return kStateKindFleensTraitDestSlot;
	}
	if (scumm_stricmp(key, kStateKeyGameplayFleensFeetDestSlot) == 0) {
		traitIndex = static_cast<uint>(ZmbTrait::kTraitFeet);
		return kStateKindFleensTraitDestSlot;
	}

	return kStateKindUnknown;
}

bool ZoombiniConsole::isStateKeyAvailable(StateKeyKind keyKind) const {
	if (keyKind == kStateKindUnknown)
		return false;

	const bool isGameV2 = _vm->isVersionFamilyTlcV2();
	const bool routePerfectCounters = _vm->hasRoutePerfectCounterState();
	if (keyKind == kStateKindTouchSense || keyKind == kStateKindHelpAudio)
		return isGameV2;
	if (kStateKindRoute1Level <= keyKind && keyKind <= kStateKindRoute4Level)
		return true;
	if (kStateKindRoute1PerfectCounter <= keyKind && keyKind <= kStateKindRoute4PerfectCounter)
		return routePerfectCounters;
	if (keyKind == kStateKindMemorialActive || keyKind == kStateKindMemorialDate)
		return true;

	return true;
}

bool ZoombiniConsole::isRouteStateMutationAllowed() const {
	const ZoombiniPage *activePage = _vm->getActivePage();
	if (!activePage)
		return false;

	switch (activePage->getPageType()) {
	case ZoombiniPageType::kRodMap:
	case ZoombiniPageType::kPicker:
	case ZoombiniPageType::kBasecamp1:
	case ZoombiniPageType::kBasecamp2:
	case ZoombiniPageType::kTown:
		return true;
	default:
		return false;
	}
}

static const char *mazeNextLayoutName(int16 difficultyLevel, int16 counter) {
	if (counter < 0 || difficultyLevel < 1 || 4 < difficultyLevel)
		return "unknown";
	const int16 modulus = (difficultyLevel == 4) ? 3 : 2;
	const int16 variantIdx = static_cast<int16>(counter % modulus);
	if (variantIdx == 0)
		return "base";
	if (difficultyLevel == 4)
		return (variantIdx == 1) ? "restored" : "alt";
	return "alt";
}

void ZoombiniConsole::printStateKeyValue(const char *key, StateKeyKind keyKind, uint memorialRouteIndex, uint memorialLevel,
										 uint fleensTraitIndex) {
	const ZmbStateFile &state = _vm->_state->_f;
	const char *value = nullptr;
	switch (keyKind) {
	case kStateKindSfx:
		value = state.getSfxEnabled() ? "on" : "off";
		break;
	case kStateKindBgm:
		value = state.getBgmEnabled() ? "on" : "off";
		break;
	case kStateKindStickyMouse:
		value = state.getStickyMouseEnabled() ? "on" : "off";
		break;
	case kStateKindCursor:
		value = state.getCursorVisible() ? "on" : "off";
		break;
	case kStateKindDebug:
		value = state.getDebugEnabled() ? "on" : "off";
		break;
	case kStateKindMidiDebug:
		value = _vm->isBuiltinMidiTestMode() ? "on" : "off";
		break;
	case kStateKindAutoStickyMouse:
		value = state.getAutoStickyMouseEnabled() ? "on" : "off";
		break;
	case kStateKindLessAction:
		value = state.getLessActionEnabled() ? "on" : "off";
		break;
	case kStateKindTransitions:
		value = _vm->_state->getEnableTransitions() ? "on" : "off";
		break;
	case kStateKindTouchSense:
		value = state.getTouchSenseEnabled() ? "on" : "off";
		break;
	case kStateKindHelpAudio:
		value = state.getHelpAudioEnabled() ? "on" : "off";
		break;
	default:
		break;
	}

	if (kStateKindRoute1Level <= keyKind && keyKind <= kStateKindRoute4Level) {
		const uint routeIndex = static_cast<uint>(keyKind - kStateKindRoute1Level);
		debugPrintf("State key '%s': %d\n", key, state._routeLevels[routeIndex] + 1);
		return;
	}

	if (kStateKindRoute1PerfectCounter <= keyKind && keyKind <= kStateKindRoute4PerfectCounter) {
		const uint routeIndex = static_cast<uint>(keyKind - kStateKindRoute1PerfectCounter);
		debugPrintf("State key '%s': %d\n", key, state._routePerfectCounters[routeIndex]);
		return;
	}

	if (keyKind == kStateKindMemorialActive) {
		const bool active = _vm->_state->readMemorialActive(static_cast<ZmbRouteId>(memorialRouteIndex), static_cast<int16>(memorialLevel));
		debugPrintf("State key '%s': %s\n", key, active ? "on" : "off");
		return;
	}

	if (keyKind == kStateKindMemorialDate) {
		uint16 year = 0;
		byte month = 0;
		byte day = 0;
		if (!_vm->_state->readMemorialDate(static_cast<ZmbRouteId>(memorialRouteIndex), static_cast<int16>(memorialLevel), year, month, day)) {
			debugPrintf("State key '%s': inactive\n", key);
			return;
		}

		debugPrintf("State key '%s': %04u%02u%02u\n", key,
					static_cast<uint32>(year), static_cast<uint32>(month), static_cast<uint32>(day));
		return;
	}

	if (keyKind == kStateKindFleensTraitValueRotation || keyKind == kStateKindFleensTraitDestSlot) {
		const byte *traitValues = nullptr;
		if (keyKind == kStateKindFleensTraitValueRotation) {
			if (_vm->isVersionFamilyTlcV2())
				traitValues = state._v2FleensTraitValueRotations;
			else
				traitValues = state._v1FleensTraitValueRotations;
		} else {
			if (_vm->isVersionFamilyTlcV2())
				traitValues = state._v2FleensTraitDestSlots;
			else
				traitValues = state._v1FleensTraitDestSlots;
		}
		debugPrintf("State key '%s': %u\n", key, static_cast<uint32>(traitValues[fleensTraitIndex]));
		return;
	}

	if (keyKind == kStateKindMazeNextLayout) {
		const int16 difficultyLevel = static_cast<int16>(_vm->_state->readPageRouteLevel(ZoombiniPageType::kMaze) + 1);
		if (difficultyLevel < 1 || 4 < difficultyLevel) {
			debugPrintf("State key '%s': maze route difficulty is not in range 1-4.\n", key);
			return;
		}
		const ZoombiniGameState::MazeLayoutVariantState &variants = _vm->_state->getMazeLayoutVariantState();
		int16 counter = 0;
		switch (difficultyLevel) {
		case 1:
			counter = variants._level1;
			break;
		case 2:
			counter = variants._level2;
			break;
		case 3:
			counter = variants._level3;
			break;
		default:
			counter = variants._level4;
			break;
		}
		debugPrintf("State key '%s': %s\n", key, mazeNextLayoutName(difficultyLevel, counter));
		return;
	}

	if (value) {
		debugPrintf("State key '%s': %s\n", key, value);
		return;
	}

	debugPrintf("State key '%s' has no printable value.\n", key);
}

bool ZoombiniConsole::CmdSub_StateSet(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Set one supported value in live game or runtime state.\n");
		debugPrintf("Usage: %s %s <key> <value>\n\n", kCmdState, kSubCmdSet);
		debugPrintf("Boolean keys accept 0/1, on/off, true/false, or yes/no:\n");
		printAvailableStateKeys();
		debugPrintf("\n");
		debugPrintf("The following warning applies to serialized state keys:\n");
		printStateSetWarning();
		debugPrintf("The %s key is runtime-only and is not written to the save.\n", kStateKeyMidiDebug);
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (!requireGameStateReady(Common::String::format("%s %s", kCmdState, kSubCmdSet)))
		return true;

	if (argc != 4) {
		debugPrintf("Usage: %s %s <key> <value>\n", kCmdState, kSubCmdSet);
		debugPrintf("Use '%s %s' to list supported keys.\n", kCmdState, kSubCmdHelp);
		debugPrintf("\n");
		return true;
	}

	const char *key = argv[2];
	const char *valueArg = argv[3];

	uint memorialRouteIndex = 0;
	uint memorialLevel = 0;
	StateKeyKind keyKind = parseStateKey(key);
	if (keyKind == kStateKindUnknown)
		keyKind = parseMemorialStateKey(key, memorialRouteIndex, memorialLevel);
	uint fleensTraitIndex = 0;
	if (keyKind == kStateKindUnknown)
		keyKind = parseFleensTraitStateKey(key, fleensTraitIndex);

	if (keyKind == kStateKindUnknown) {
		debugPrintf("Unknown state key '%s'.\n", key);
		debugPrintf("Use '%s %s' to list supported keys.\n", kCmdState, kSubCmdHelp);
		debugPrintf("\n");
		return true;
	}

	if (!isStateKeyAvailable(keyKind)) {
		debugPrintf("State key '%s' is not available in the %s release.\n", key, _vm->getVersionFamilyName());
		debugPrintf("\n");
		return true;
	}

	if (keyKind == kStateKindMazeNextLayout) {
		const int16 difficultyLevel = static_cast<int16>(_vm->_state->readPageRouteLevel(ZoombiniPageType::kMaze) + 1);
		if (difficultyLevel < 1 || 4 < difficultyLevel) {
			debugPrintf("State key '%s': maze route difficulty is not in range 1-4.\n", key);
			debugPrintf("\n");
			return true;
		}

		int16 nextCounter = -1;
		if (scumm_stricmp(valueArg, "base") == 0) {
			nextCounter = 0;
		} else if (scumm_stricmp(valueArg, "alt") == 0) {
			nextCounter = (difficultyLevel == 4) ? 2 : 1;
		} else if (scumm_stricmp(valueArg, "restored") == 0) {
			if (difficultyLevel != 4) {
				debugPrintf("Value 'restored' requires maze difficulty 4; the maze route is at difficulty %d.\n",
							static_cast<int>(difficultyLevel));
				debugPrintf("\n");
				return true;
			}
			nextCounter = 1;
		} else {
			debugPrintf("Invalid maze next-layout value '%s'. Use base, alt, or restored.\n", valueArg);
			debugPrintf("\n");
			return true;
		}

		ZoombiniGameState::MazeLayoutVariantState &variants = _vm->_state->getMazeLayoutVariantState();
		int16 *counterPtr = &variants._level1;
		switch (difficultyLevel) {
		case 2:
			counterPtr = &variants._level2;
			break;
		case 3:
			counterPtr = &variants._level3;
			break;
		case 4:
			counterPtr = &variants._level4;
			break;
		default:
			break;
		}
		const int16 previousCounter = *counterPtr;
		if (*counterPtr == nextCounter) {
			debugPrintf("State key '%s': no-op (already %s)\n", key, mazeNextLayoutName(difficultyLevel, previousCounter));
			debugPrintf("\n");
			return true;
		}

		printStateSetWarning();
		*counterPtr = nextCounter;
		_vm->_state->markDebugStateMutation();
		debugPrintf("State key '%s':\n", key);
		debugPrintf("  before: %s\n", mazeNextLayoutName(difficultyLevel, previousCounter));
		debugPrintf("  after:  %s\n", mazeNextLayoutName(difficultyLevel, nextCounter));
		debugPrintf("  rollback: state set %s %s\n", key, mazeNextLayoutName(difficultyLevel, previousCounter));
		debugPrintf("\n");
		return true;
	}

	if (keyKind == kStateKindFleensTraitValueRotation || keyKind == kStateKindFleensTraitDestSlot) {
		const int32 maximumValue = (keyKind == kStateKindFleensTraitValueRotation) ? 5 : 4;
		int32 newValue = 0;
		if (!ZmbResource::parseInt(valueArg, newValue) || newValue < 0 || maximumValue < newValue) {
			debugPrintf("Invalid Fleens trait value '%s'. Use a value from 0 to %d (hex supported).\n",
						valueArg, static_cast<int>(maximumValue));
			debugPrintf("\n");
			return true;
		}

		ZmbStateFile &state = _vm->_state->_f;
		byte *traitValues = nullptr;
		if (keyKind == kStateKindFleensTraitValueRotation) {
			if (_vm->isVersionFamilyTlcV2())
				traitValues = state._v2FleensTraitValueRotations;
			else
				traitValues = state._v1FleensTraitValueRotations;
		} else {
			if (_vm->isVersionFamilyTlcV2())
				traitValues = state._v2FleensTraitDestSlots;
			else
				traitValues = state._v1FleensTraitDestSlots;
		}

		const byte previousValue = traitValues[fleensTraitIndex];
		if (previousValue == newValue) {
			debugPrintf("State key '%s': no-op (already %d)\n", key, static_cast<int>(previousValue));
			debugPrintf("\n");
			return true;
		}

		printStateSetWarning();
		traitValues[fleensTraitIndex] = static_cast<byte>(newValue);
		_vm->_state->markDebugStateMutation();
		debugPrintf("State key '%s':\n", key);
		debugPrintf("  before: %d\n", static_cast<int>(previousValue));
		debugPrintf("  after:  %d\n", static_cast<int>(newValue));
		debugPrintf("  rollback: state set %s %d\n", key, static_cast<int>(previousValue));
		debugPrintf("\n");
		return true;
	}

	if (kStateKindRoute1Level <= keyKind && keyKind <= kStateKindMemorialDate &&
		!isRouteStateMutationAllowed()) {
		debugPrintf("Route or memorial state key '%s' can only be set from RodMap or a shelter page.\n", key);
		debugPrintf("\n");
		return true;
	}

	ZmbStateFile &state = _vm->_state->_f;
	const bool isBooleanKey =
		(kStateKindSfx <= keyKind && keyKind <= kStateKindHelpAudio) ||
		keyKind == kStateKindMemorialActive || keyKind == kStateKindDebug || keyKind == kStateKindMidiDebug;
	if (isBooleanKey) {
		bool enabled = false;
		bool valid = true;
		if (scumm_stricmp(valueArg, "on") == 0 || scumm_stricmp(valueArg, "true") == 0 ||
			scumm_stricmp(valueArg, "yes") == 0) {
			enabled = true;
		} else if (scumm_stricmp(valueArg, "off") == 0 || scumm_stricmp(valueArg, "false") == 0 ||
				   scumm_stricmp(valueArg, "no") == 0) {
			enabled = false;
		} else {
			int32 numericValue = 0;
			valid = ZmbResource::parseInt(valueArg, numericValue) &&
					(numericValue == 0 || numericValue == 1);
			enabled = numericValue == 1;
		}

		if (!valid) {
			debugPrintf("Invalid boolean value '%s'. Use 0/1, on/off, true/false, or yes/no.\n", valueArg);
			debugPrintf("\n");
			return true;
		}

		bool previousValue = false;
		switch (keyKind) {
		case kStateKindSfx:
			previousValue = state.getSfxEnabled();
			break;
		case kStateKindBgm:
			previousValue = state.getBgmEnabled();
			break;
		case kStateKindStickyMouse:
			previousValue = state.getStickyMouseEnabled();
			break;
		case kStateKindCursor:
			previousValue = state.getCursorVisible();
			break;
		case kStateKindDebug:
			previousValue = state.getDebugEnabled();
			break;
		case kStateKindMidiDebug:
			previousValue = _vm->isBuiltinMidiTestMode();
			break;
		case kStateKindAutoStickyMouse:
			previousValue = state.getAutoStickyMouseEnabled();
			break;
		case kStateKindLessAction:
			previousValue = state.getLessActionEnabled();
			break;
		case kStateKindTransitions:
			previousValue = _vm->_state->getEnableTransitions();
			break;
		case kStateKindTouchSense:
			previousValue = state.getTouchSenseEnabled();
			break;
		case kStateKindHelpAudio:
			previousValue = state.getHelpAudioEnabled();
			break;
		case kStateKindMemorialActive:
			previousValue = _vm->_state->readMemorialActive(static_cast<ZmbRouteId>(memorialRouteIndex), static_cast<int16>(memorialLevel));
			break;
		default:
			break;
		}

		if (keyKind == kStateKindMemorialActive && previousValue == enabled) {
			debugPrintf("State key '%s': no-op (already %s)\n", key, enabled ? "on" : "off");
			debugPrintf("\n");
			return true;
		}

		if (keyKind != kStateKindMidiDebug)
			printStateSetWarning();

		switch (keyKind) {
		case kStateKindSfx:
			state.setSfxEnabled(enabled);
			break;
		case kStateKindBgm:
			state.setBgmEnabled(enabled);
			break;
		case kStateKindStickyMouse:
			state.setStickyMouseEnabled(enabled);
			break;
		case kStateKindCursor:
			state.setCursorVisible(enabled);
			break;
		case kStateKindDebug:
			_vm->setBuiltinDebugMode(enabled);
			break;
		case kStateKindMidiDebug:
			_vm->setBuiltinMidiTestMode(enabled);
			break;
		case kStateKindAutoStickyMouse:
			state.setAutoStickyMouseEnabled(enabled);
			break;
		case kStateKindLessAction:
			state.setLessActionEnabled(enabled);
			break;
		case kStateKindTransitions:
			_vm->_state->setEnableTransitions(enabled, false);
			break;
		case kStateKindTouchSense:
			state.setTouchSenseEnabled(enabled);
			break;
		case kStateKindHelpAudio:
			state.setHelpAudioEnabled(enabled);
			break;
		case kStateKindMemorialActive:
			if (!_vm->_state->setMemorialActive(static_cast<ZmbRouteId>(memorialRouteIndex), static_cast<int16>(memorialLevel), enabled)) {
				debugPrintf("Could not set memorial active state for '%s'.\n", key);
				debugPrintf("\n");
				return true;
			}
			break;
		default:
			break;
		}

		if (keyKind != kStateKindDebug && keyKind != kStateKindMidiDebug)
			_vm->_state->markDebugStateMutation();
		debugPrintf("State key '%s':\n", key);
		debugPrintf("  before: %s\n", previousValue ? "on" : "off");
		debugPrintf("  after:  %s\n", enabled ? "on" : "off");
		debugPrintf("  rollback: state set %s %s\n", key, previousValue ? "on" : "off");
		debugPrintf("\n");
		return true;
	}

	if (kStateKindRoute1Level <= keyKind && keyKind <= kStateKindRoute4Level) {
		int32 difficultyLevel = 0;
		if (!ZmbResource::parseInt(valueArg, difficultyLevel) || difficultyLevel < 1 || 4 < difficultyLevel) {
			debugPrintf("Invalid route difficulty level '%s'. Use a value from 1 to 4 (hex supported).\n", valueArg);
			debugPrintf("\n");
			return true;
		}

		const uint routeIndex = static_cast<uint>(keyKind - kStateKindRoute1Level);
		const int16 previousLevel = state._routeLevels[routeIndex] + 1;
		if (previousLevel == difficultyLevel) {
			debugPrintf("State key '%s': no-op (already level=%d)\n", key, previousLevel);
			debugPrintf("\n");
			return true;
		}

		printStateSetWarning();
		if (!_vm->_state->setRouteDifficultyLevel(static_cast<ZmbRouteId>(routeIndex), static_cast<int16>(difficultyLevel))) {
			debugPrintf("Could not set route difficulty level for '%s'.\n", key);
			debugPrintf("\n");
			return true;
		}

		_vm->_state->markDebugStateMutation();
		debugPrintf("State key '%s':\n", key);
		debugPrintf("  before: level=%d\n", previousLevel);
		debugPrintf("  after:  level=%d\n", difficultyLevel);
		debugPrintf("  rollback: state set %s %d\n", key, previousLevel);
		debugPrintf("\n");
		return true;
	}

	if (keyKind == kStateKindMemorialDate) {
		uint16 year = 0;
		byte month = 0;
		byte day = 0;
		if (!parseMemorialDate(valueArg, year, month, day)) {
			debugPrintf("Invalid memorial date '%s'. Use yyyyMMdd with a valid month and day.\n", valueArg);
			debugPrintf("\n");
			return true;
		}

		uint16 previousYear = 0;
		byte previousMonth = 0;
		byte previousDay = 0;
		const bool previousActive = _vm->_state->readMemorialDate(static_cast<ZmbRouteId>(memorialRouteIndex), static_cast<int16>(memorialLevel),
																  previousYear, previousMonth, previousDay);
		if (previousActive && previousYear == year && previousMonth == month && previousDay == day) {
			debugPrintf("State key '%s': no-op (already %04u%02u%02u)\n", key,
						static_cast<uint32>(year), static_cast<uint32>(month), static_cast<uint32>(day));
			debugPrintf("\n");
			return true;
		}

		printStateSetWarning();
		if (!_vm->_state->setMemorialDate(static_cast<ZmbRouteId>(memorialRouteIndex), static_cast<int16>(memorialLevel), year, month, day)) {
			debugPrintf("Could not set memorial date for '%s'.\n", key);
			debugPrintf("\n");
			return true;
		}

		_vm->_state->markDebugStateMutation();
		debugPrintf("State key '%s':\n", key);
		if (previousActive) {
			debugPrintf("  before: %04u%02u%02u\n", static_cast<uint32>(previousYear),
						static_cast<uint32>(previousMonth), static_cast<uint32>(previousDay));
		} else {
			debugPrintf("  before: inactive\n");
		}
		debugPrintf("  after:  %04u%02u%02u\n", static_cast<uint32>(year),
					static_cast<uint32>(month), static_cast<uint32>(day));
		if (previousActive) {
			debugPrintf("  rollback: state set %s %04u%02u%02u\n", key,
						static_cast<uint32>(previousYear), static_cast<uint32>(previousMonth), static_cast<uint32>(previousDay));
		} else {
			const Common::String activeKey = Common::String::format("memorial.route%u.level%u.active",
																	memorialRouteIndex + 1, memorialLevel);
			debugPrintf("  rollback: state set %s off\n", activeKey.c_str());
		}
		debugPrintf("\n");
		return true;
	}

	int32 perfectCounter = 0;
	if (!ZmbResource::parseInt(valueArg, perfectCounter) || perfectCounter < 0 || 2 < perfectCounter) {
		debugPrintf("Invalid perfect counter '%s'. Use a value from 0 to 2 (hex supported).\n", valueArg);
		debugPrintf("\n");
		return true;
	}

	const uint routeIndex = static_cast<uint>(keyKind - kStateKindRoute1PerfectCounter);
	const int16 previousCounter = state._routePerfectCounters[routeIndex];
	printStateSetWarning();

	switch (keyKind) {
	case kStateKindRoute1PerfectCounter:
	case kStateKindRoute2PerfectCounter:
	case kStateKindRoute3PerfectCounter:
	case kStateKindRoute4PerfectCounter:
		state._routePerfectCounters[routeIndex] = static_cast<int16>(perfectCounter);
		break;
	default:
		break;
	}

	_vm->_state->markDebugStateMutation();
	debugPrintf("State key '%s':\n", key);
	debugPrintf("  before: perfectCounter=%d\n", previousCounter);
	debugPrintf("  after:  perfectCounter=%d\n", perfectCounter);
	debugPrintf("  rollback: state set %s %d\n", key, previousCounter);
	debugPrintf("\n");
	return true;
}

bool ZoombiniConsole::Cmd_State(int argc, const char **argv) {
	enum SubCmdKind {
		kSubGet,
		kSubSet,
		kSubHelp,
	};

	SubCmdKind subCmdKind = kSubGet;
	bool explicitGet = false;
	if (2 <= argc && scumm_stricmp(argv[1], kSubCmdHelp) == 0)
		subCmdKind = kSubHelp;
	else if (2 <= argc && scumm_stricmp(argv[1], kSubCmdGet) == 0)
		explicitGet = true;
	else if (2 <= argc && scumm_stricmp(argv[1], kSubCmdSet) == 0)
		subCmdKind = kSubSet;

	if (subCmdKind == kSubSet)
		return CmdSub_StateSet(argc, argv);

	if (subCmdKind == kSubHelp || hasHelpOption(argc, argv)) {
		debugPrintf("Show or adjust debugger-exposed game and runtime state.\n");
		debugPrintf("Usage: %s [...]\n", kCmdState);
		debugPrintf("\n");
		debugPrintf("Built-in subcommands:\n");
		debugPrintf("  (default) or %s [<key>]\n", kSubCmdGet);
		debugPrintf("      Print the current global state, or one key when supplied.\n");
		debugPrintf("  %s <key> <value>\n", kSubCmdSet);
		debugPrintf("      Set one supported value in the live state.\n");
		debugPrintf("\n");
		debugPrintf("Boolean values: 0/1, on/off, true/false, or yes/no.\n");
		debugPrintf("Available state-set keys for this release:\n");
		printAvailableStateKeys();
		debugPrintf("\n");
		debugPrintf("The default '%s' and '%s %s' commands only read state and do not modify it.\n",
					kCmdState, kCmdState, kSubCmdGet);
		debugPrintf("The following warning applies to serialized keys used with '%s %s':\n", kCmdState, kSubCmdSet);
		printStateSetWarning();
		debugPrintf("The %s key is runtime-only and is not written to the save.\n", kStateKeyMidiDebug);
		debugPrintf("Each change prints its previous value, new value, and a rollback command.\n");
		debugPrintf("\nOptions:\n");
		printHelpOption();
		return true;
	}

	if ((explicitGet && argc != 2 && argc != 3) || (!explicitGet && argc != 1)) {
		debugPrintf("Usage: %s [%s [<key>]|%s|%s <key> <value>]\n", kCmdState, kSubCmdGet, kSubCmdHelp, kSubCmdSet);
		debugPrintf("\n");
		return true;
	}

	if (!requireGameStateReady(kCmdState))
		return true;

	if (explicitGet && argc == 3) {
		const char *key = argv[2];
		uint memorialRouteIndex = 0;
		uint memorialLevel = 0;
		StateKeyKind keyKind = parseStateKey(key);
		if (keyKind == kStateKindUnknown)
			keyKind = parseMemorialStateKey(key, memorialRouteIndex, memorialLevel);
		uint fleensTraitIndex = 0;
		if (keyKind == kStateKindUnknown)
			keyKind = parseFleensTraitStateKey(key, fleensTraitIndex);
		if (keyKind == kStateKindUnknown) {
			debugPrintf("Unknown state key '%s'.\n", key);
			debugPrintf("Use '%s %s' to list supported keys.\n", kCmdState, kSubCmdHelp);
			debugPrintf("\n");
			return true;
		}
		if (!isStateKeyAvailable(keyKind)) {
			debugPrintf("State key '%s' is not available in the %s release.\n", key, _vm->getVersionFamilyName());
			debugPrintf("\n");
			return true;
		}

		printStateKeyValue(key, keyKind, memorialRouteIndex, memorialLevel, fleensTraitIndex);
		debugPrintf("\n");
		return true;
	}

	const ZmbStateFile &state = _vm->_state->_f;
	const char *pageName = "unknown";
	switch (state._currentPage) {
	case ZmbDestPageKind::kIsle_03:
		pageName = "isle";
		break;
	case ZmbDestPageKind::kBC1_04:
		pageName = "bc1";
		break;
	case ZmbDestPageKind::kBC2_05:
		pageName = "bc2";
		break;
	case ZmbDestPageKind::kTown_06:
		pageName = "town";
		break;
	case ZmbDestPageKind::kBridge_07:
		pageName = "bridge";
		break;
	case ZmbDestPageKind::kTunnels_08:
		pageName = "tunnels";
		break;
	case ZmbDestPageKind::kPizza_09:
		pageName = "pizza";
		break;
	case ZmbDestPageKind::kFerry_10:
		pageName = "ferry";
		break;
	case ZmbDestPageKind::kLilly_11:
		pageName = "lilly";
		break;
	case ZmbDestPageKind::kSlides_12:
		pageName = "slides";
		break;
	case ZmbDestPageKind::kFleens_13:
		pageName = "fleens";
		break;
	case ZmbDestPageKind::kHotel_14:
		pageName = "hotel";
		break;
	case ZmbDestPageKind::kNet_15:
		pageName = "net";
		break;
	case ZmbDestPageKind::kCaves_16:
		pageName = "caves";
		break;
	case ZmbDestPageKind::kSmoke_17:
		pageName = "smoke";
		break;
	case ZmbDestPageKind::kMaze_18:
		pageName = "maze";
		break;
	default:
		break;
	}

	debugPrintf("ZmbStateFile:\n");
	const bool tlcState = _vm->isVersionFamilyTlcV2();
	const bool routePerfectCounters = _vm->hasRoutePerfectCounterState();
	debugPrintf("  release=%s\n", _vm->getVersionFamilyName());
	debugPrintf("  magic=0x%04X dirty=%c autoStickyDelay=%u\n",
				static_cast<uint32>(state._magic006B), state._isDirty ? 'T' : 'F',
				static_cast<uint32>(state._autoStickyDelay));
	if (tlcState) {
		debugPrintf("  currentRoute=%u currentPage=%u (%s) previousPage=%u\n",
					static_cast<uint32>(state._currentRoute), static_cast<uint32>(state._currentPage), pageName,
					static_cast<uint32>(state._v2PreviousPage));
	} else {
		debugPrintf("  currentRoute=%u currentPage=%u (%s)\n",
					static_cast<uint32>(state._currentRoute), static_cast<uint32>(state._currentPage), pageName);
	}
	char sfxState;
	if (state.getSfxEnabled())
		sfxState = 'T';
	else
		sfxState = 'F';
	char bgmState;
	if (state.getBgmEnabled())
		bgmState = 'T';
	else
		bgmState = 'F';
	char stickyMouseState;
	if (state.getStickyMouseEnabled())
		stickyMouseState = 'T';
	else
		stickyMouseState = 'F';
	char cursorState;
	if (state.getCursorVisible())
		cursorState = 'T';
	else
		cursorState = 'F';
	char autoStickyState;
	if (state.getAutoStickyMouseEnabled())
		autoStickyState = 'T';
	else
		autoStickyState = 'F';
	char lessActionState;
	if (state.getLessActionEnabled())
		lessActionState = 'T';
	else
		lessActionState = 'F';
	char transitionsState;
	if (_vm->_state->getEnableTransitions())
		transitionsState = 'T';
	else
		transitionsState = 'F';
	debugPrintf("  options: sfx=%c bgm=%c stickyMouse=%c cursor=%c autoSticky=%c lessAction=%c transitions=%c\n",
				sfxState, bgmState, stickyMouseState, cursorState, autoStickyState, lessActionState, transitionsState);
	if (tlcState) {
		char touchSenseState;
		if (state.getTouchSenseEnabled())
			touchSenseState = 'T';
		else
			touchSenseState = 'F';
		char helpAudioState;
		if (state.getHelpAudioEnabled())
			helpAudioState = 'T';
		else
			helpAudioState = 'F';
		debugPrintf("  TLC options: touchSense=%c helpAudio=%c\n",
					touchSenseState, helpAudioState);
	}
	debugPrintf("Gameplay:\n");
	debugPrintf("  debug=%c\n", state.getDebugEnabled() ? 'T' : 'F');
	debugPrintf("MIDI runtime:\n");
	debugPrintf("  debug=%c testResource=%u\n",
				_vm->isBuiltinMidiTestMode() ? 'T' : 'F',
				static_cast<uint32>(_vm->getBuiltinMidiTestResource()));
	const byte *fleensTraitValueRotations =
		tlcState ? state._v2FleensTraitValueRotations : state._v1FleensTraitValueRotations;
	const byte *fleensTraitDestSlots =
		tlcState ? state._v2FleensTraitDestSlots : state._v1FleensTraitDestSlots;
	debugPrintf("  Fleens traits: valueRotations=[hair=%u eyes=%u nose=%u feet=%u]\n",
				static_cast<uint32>(fleensTraitValueRotations[ZmbTrait::kTraitHair]),
				static_cast<uint32>(fleensTraitValueRotations[ZmbTrait::kTraitEyes]),
				static_cast<uint32>(fleensTraitValueRotations[ZmbTrait::kTraitNose]),
				static_cast<uint32>(fleensTraitValueRotations[ZmbTrait::kTraitFeet]));
	debugPrintf("                 destSlots=[hair=%u eyes=%u nose=%u feet=%u]\n",
				static_cast<uint32>(fleensTraitDestSlots[ZmbTrait::kTraitHair]),
				static_cast<uint32>(fleensTraitDestSlots[ZmbTrait::kTraitEyes]),
				static_cast<uint32>(fleensTraitDestSlots[ZmbTrait::kTraitNose]),
				static_cast<uint32>(fleensTraitDestSlots[ZmbTrait::kTraitFeet]));

	static constexpr const char *const kRouteNames[4] = {
		"big-bad-hungry:", "whos-bayou:", "deep-dark-forest:", "mountains-of-despair:"};
	debugPrintf("Routes:\n");
	for (int16 i = 0; i < 4; i++) {
		if (routePerfectCounters) {
			debugPrintf("  %d %-21s level=%u perfectCounter=%d\n", i + 1, kRouteNames[i],
						static_cast<uint32>(state._routeLevels[i]), state._routePerfectCounters[i]);
		} else {
			debugPrintf("  %d %-21s level=%u\n", i + 1, kRouteNames[i],
						static_cast<uint32>(state._routeLevels[i]));
		}
	}

	const uint32 route1 = state._levelFlagRouteBigBadHungry;
	const uint32 route2 = state._levelFlagLoWhosBayouHiDeepDarkForest & 0x0F;
	const uint32 route3 = (state._levelFlagLoWhosBayouHiDeepDarkForest >> 4) & 0x0F;
	const uint32 route4 = state._levelFlagRouteMontDespair;

	debugPrintf("  completionFlags (read-only; use progress.route[1-4].perfectCounter to adjust progress when available):\n");
	debugPrintf("    %-21s %s (0x%02X)\n", kRouteNames[0], formatCompletionFlags(route1).c_str(), route1);
	debugPrintf("    %-21s %s (0x%02X)\n", kRouteNames[1], formatCompletionFlags(route2).c_str(), route2);
	debugPrintf("    %-21s %s (0x%02X)\n", kRouteNames[2], formatCompletionFlags(route3).c_str(), route3);
	debugPrintf("    %-21s %s (0x%02X)\n", kRouteNames[3], formatCompletionFlags(route4).c_str(), route4);

	const struct {
		const char *name;
		const ZmbStateFile::PageFlag *value;
	} kPageFlags[] = {
		{"isle", &state._pageFlagIsle},
		{"bridge", &state._pageFlagBridge},
		{"tunnels", &state._pageFlagTunnels},
		{"pizza", &state._pageFlagPizza},
		{"bc1", &state._pageFlagBasecamp1},
		{"ferry", &state._pageFlagFerry},
		{"lilly", &state._pageFlagLilly},
		{"slides", &state._pageFlagSlides},
		{"fleens", &state._pageFlagFleens},
		{"hotel", &state._pageFlagHotel},
		{"net", &state._pageFlagNet},
		{"bc2", &state._pageFlagBasecamp2},
		{"caves", &state._pageFlagCaves},
		{"smoke", &state._pageFlagSmoke},
		{"maze", &state._pageFlagMaze},
		{"town", &state._pageFlagTown}};

	debugPrintf("Page flags (low 12 bits=saturating visit count, 0x1000=first Hard trigger, 0x2000=second Hard trigger):\n");
	for (uint i = 0; i < ARRAYSIZE(kPageFlags); i++) {
		const ZmbStateFile::PageFlag &pageFlag = *kPageFlags[i].value;
		const uint16 rawValue = pageFlag.getRaw();
		const uint16 visitCount = pageFlag.getVisitCount();
		const bool firstHardTrigger = pageFlag.hasFirstHardGroupTrigger();
		const bool secondHardTrigger = pageFlag.hasSecondHardGroupTrigger();
		char firstHardTriggerState;
		if (firstHardTrigger)
			firstHardTriggerState = 'T';
		else
			firstHardTriggerState = 'F';
		char secondHardTriggerState;
		if (secondHardTrigger)
			secondHardTriggerState = 'T';
		else
			secondHardTriggerState = 'F';
		debugPrintf("  %-8s: raw=0x%04X visitCount=%4u firstHardTrigger=%c secondHardTrigger=%c\n",
					kPageFlags[i].name, static_cast<unsigned>(rawValue), static_cast<unsigned>(visitCount),
					firstHardTriggerState, secondHardTriggerState);
	}

	const struct {
		const char *name;
		const ZmbStateActivePack *pack;
	} kPacks[] = {
		{"isle", &state._zmbPackIsle},
		{"bc1", &state._zmbPackBC1},
		{"bc2", &state._zmbPackBC2},
		{"active", &state._zmbPackActive}};
	debugPrintf("Packs:\n");
	debugPrintf("  columns: name=pack, count=serialized entries,\n"
				"           occupied       = entries assigned to pedestal positions\n"
				"           unoccupied     = entries assigned to saved storage-belt positions\n"
				"           skipOccupied   = If 'T', skips creating field/pedestal entries during this load pass\n"
				"           skipUnoccupied = If 'T', skips creating storage-belt entries during this load pass;\n");
	int32 packTotal = 0;
	for (uint packEntryIdx = 0; packEntryIdx < ARRAYSIZE(kPacks); packEntryIdx++) {
		const ZmbStateActivePack &pack = *kPacks[packEntryIdx].pack;
		int16 occupied = 0;
		for (int16 entryIdx = 0; entryIdx < pack.getPackZmbCount(); entryIdx++) {
			if (pack.getEntry(entryIdx).getIsOccupied())
				occupied += 1;
		}
		const int16 count = pack.getPackZmbCount();
		char skipOccupiedState;
		if (pack.getSkipOccupiedEntries())
			skipOccupiedState = 'T';
		else
			skipOccupiedState = 'F';
		char skipUnoccupiedState;
		if (pack.getSkipUnoccupiedEntries())
			skipUnoccupiedState = 'T';
		else
			skipUnoccupiedState = 'F';
		debugPrintf("  %-8s: count=%d occupied=%d unoccupied=%d skipOccupied=%c skipUnoccupied=%c\n",
					kPacks[packEntryIdx].name, count, occupied, count - occupied,
					skipOccupiedState, skipUnoccupiedState);
		packTotal += count;
	}
	debugPrintf("  skip pair filters runner materialization: FF=all, TF=unoccupied only, FT=occupied only, TT=none\n");
	debugPrintf("Stored Zoombinis:\n");
	debugPrintf("  basecamp1: logical=%d header=%d populated=%d\n", state._zmbStoredBC1Count,
				state._storedChunkBC1.getStoredCount(), state._storedChunkBC1.getPopulatedEntryCount());
	debugPrintf("  basecamp2: logical=%d header=%d populated=%d\n", state._zmbStoredBC2Count,
				state._storedChunkBC2.getStoredCount(), state._storedChunkBC2.getPopulatedEntryCount());
	debugPrintf("  town:      logical=%d header=%d populated=%d\n", state._zmbStoredTownCount,
				state._storedChunkTown.getStoredCount(), state._storedChunkTown.getPopulatedEntryCount());
	debugPrintf("  packEntries=%d logicalStored=%d totalTracked=%d\n", packTotal,
				state._zmbStoredBC1Count + state._zmbStoredBC2Count + state._zmbStoredTownCount,
				packTotal + state._zmbStoredBC1Count + state._zmbStoredBC2Count + state._zmbStoredTownCount);
	debugPrintf("\n");
	return true;
}

Common::String ZoombiniConsole::formatCompletionFlags(uint8 flags) {
	Common::String result;
	for (int i = 0; i < 4; i++) {
		if ((flags >> i) & 1)
			result += Common::String::format("L%d ", i + 1);
	}

	if (result.empty())
		return "none";

	result.trim();
	return result;
}

// [*] Page/Puzzle commands

bool ZoombiniConsole::Cmd_BuiltinDebug(int argc, const char **argv) {
	ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(_vm->getActivePage());
	if (!interactive) {
		debugPrintf("The active page is not an interactive page.\n\n");
		return true;
	}

	if (argc < 2 || hasHelpOption(argc, argv)) {
		debugPrintf("Builtin debug functions from the original Zoombini engine.\n");
		debugPrintf("In-game activation: press Ctrl+A, type 'go snoids', press Space.\n");
		debugPrintf("\n");
		debugPrintf("Direct debugger commands run regardless of the debug flag state.\n");
		debugPrintf("That debug flag gates only the original in-game debug shortcuts.\n");
		debugPrintf("\n");
		debugPrintf("Usage: %s <action> [arguments]\n\n", kCmdBuiltinDebug);
		printBuiltinDebugStatus();
		debugPrintf("\n");
		debugPrintf("Actions:\n%s\n", interactive->debugGetGlobalDebugCommandHelp().c_str());
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	Common::String output;
	const bool commandFailed = interactive->debugDoGlobalDebugCommand(argc, argv, output);
	if (!output.empty()) {
		debugPrintf("%s", output.c_str());
		if (output[output.size() - 1] != '\n')
			debugPrintf("\n");
		if (output.size() < 2 || output[output.size() - 2] != '\n')
			debugPrintf("\n");
	}
	printBuiltinDebugStatus();
	debugPrintf("\n");
	return commandFailed;
}

bool ZoombiniConsole::Cmd_PagePuzzle(int argc, const char **argv) {
	if (argc < 2 || isHelpOption(argv[1])) {
		ZoombiniPage *page = _vm->getActivePage();
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		const bool canSetChances = interactive && interactive->debugCanSetChances();

		debugPrintf("Inspect or control the active Zoombini page.\n");
		debugPrintf("Usage: %s|%s <subcommand> [arguments]\n", kCmdPage, kCmdPuzzle);
		debugPrintf("\n");
		debugPrintf("Built-in subcommands:\n");
		debugPrintf("  %s <action> [arguments]\n", kSubCmdPageBuiltinDebug);
		debugPrintf("      Run a page-local builtin debug function.\n");
		if (interactive && interactive->debugCanReplayActiveHelpSound())
			debugPrintf("      The active page also accepts: replay-help\n");
		debugPrintf("  %s\n", kSubCmdPagePuzzleFinish);
		debugPrintf("      Trigger the active puzzle's departure path.\n");
		debugPrintf("  %s\n", kSubCmdPagePuzzleAnswer);
		debugPrintf("      Print the active puzzle's generated answer.\n");
		debugPrintf("  %s|%s\n", kSubCmdPagePuzzleChance, kSubCmdPagePuzzleChances);
		if (canSetChances)
			debugPrintf("      Show chances, or set remaining chances for the active puzzle.\n");
		else
			debugPrintf("      Show chances; chance setting is unsupported on the active page.\n");
		debugPrintf("\n");

		Common::String pageHelp;
		if (page)
			pageHelp = page->debugGetPageCommandHelp();
		if (!pageHelp.empty()) {
			debugPrintf("Active-page subcommands:\n");
			debugPrintf("%s", pageHelp.c_str());
			debugPrintf("\n");
		} else {
			debugPrintf("The active page provides no additional subcommands.\n\n");
		}

		Common::String builtinDebugHelp;
		if (page)
			builtinDebugHelp = page->debugGetBuiltinDebugCommandHelp();
		if (!builtinDebugHelp.empty()) {
			debugPrintf("Active-page built-in debug actions:\n");
			debugPrintf("%s", builtinDebugHelp.c_str());
			debugPrintf("\n");
		}

		debugPrintf("Use %s <subcommand> --help for subcommand-specific details.\n\n", kCmdPage);
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdPageBuiltinDebug) == 0)
		return CmdSub_PageBuiltinDebug(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdPagePuzzleFinish) == 0)
		return CmdSub_Puzzle_Finish(argc, argv);
	else if (scumm_stricmp(argv[1], kSubCmdPagePuzzleAnswer) == 0)
		return CmdSub_Puzzle_Answer(argc, argv);
	else if (scumm_stricmp(argv[1], kSubCmdPagePuzzleChance) == 0 || scumm_stricmp(argv[1], kSubCmdPagePuzzleChances) == 0)
		return CmdSub_Puzzle_Chances(argc, argv);

	ZoombiniPage *page = _vm->getActivePage();
	if (!page) {
		debugPrintf("No active page.\n");
		debugPrintf("\n");
		return true;
	}

	Common::String output;
	const bool commandFailed = page->debugDoPageCommand(argc, argv, output);
	if (!output.empty()) {
		debugPrintf("%s", output.c_str());
		if (output[output.size() - 1] != '\n')
			debugPrintf("\n");
		if (output.size() < 2 || output[output.size() - 2] != '\n')
			debugPrintf("\n");
	}
	return commandFailed;
}

bool ZoombiniConsole::CmdSub_PageBuiltinDebug(int argc, const char **argv) {
	ZoombiniPage *page = _vm->getActivePage();
	if (!page) {
		debugPrintf("No active page.\n\n");
		return true;
	}

	if (argc < 3 || hasHelpOption(argc, argv)) {
		debugPrintf("Run a page-local builtin debug function.\n");
		debugPrintf("\n");
		debugPrintf("This direct debugger command runs regardless of the debug flag state.\n");
		debugPrintf("\n");
		debugPrintf("Open the ScummVM debugger during play with the default Ctrl+Alt+D shortcut.\n");
		debugPrintf("Usage: %s %s <action> [arguments]\n\n", kCmdPage, kSubCmdPageBuiltinDebug);
		printBuiltinDebugStatus();
		debugPrintf("\n");
		const Common::String help = page->debugGetBuiltinDebugCommandHelp();
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		if (interactive && interactive->debugCanReplayActiveHelpSound())
			debugPrintf("Actions:\n  replay-help (in-game: F1)\n      Replay the active page's F1 help sound.\n");
		else
			debugPrintf("Actions:\n");
		if (!help.empty())
			debugPrintf("%s\n", help.c_str());
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (scumm_stricmp(argv[2], "replay-help") == 0) {
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		if (!interactive || !interactive->debugReplayActiveHelpSound()) {
			debugPrintf("The active page has no F1 help sound.\n\n");
			return true;
		}
		debugPrintf("Replayed the active page's F1 help sound.\n\n");
		printBuiltinDebugStatus();
		debugPrintf("\n");
		return false;
	}

	Common::String output;
	const bool commandFailed = page->debugDoBuiltinDebugCommand(argc, argv, output);
	if (!output.empty()) {
		debugPrintf("%s", output.c_str());
		if (output[output.size() - 1] != '\n')
			debugPrintf("\n");
		if (output.size() < 2 || output[output.size() - 2] != '\n')
			debugPrintf("\n");
	}
	printBuiltinDebugStatus();
	debugPrintf("\n");
	return commandFailed;
}

// [*] Page/Puzzle subcommands

bool ZoombiniConsole::CmdSub_Puzzle_Finish(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Trigger the active interactive puzzle's departure path.\n");
		debugPrintf("Usage: %s|%s %s\n", kCmdPage, kCmdPuzzle, kSubCmdPagePuzzleFinish);
		debugPrintf("\n");
		debugPrintf("This is a live debug action: it marks the game state as changed\n");
		debugPrintf("and invokes the active puzzle's normal departure handling. It\n");
		debugPrintf("requires a created game or a loaded save.\n");
		debugPrintf("\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (!requireGameStateReady(Common::String::format("%s %s", argv[0], argv[1])))
		return true;

	ZoombiniPage *page = _vm->getActivePage();
	if (!page) {
		debugPrintf("No active page.\n");
		debugPrintf("\n");
		return true;
	}

	ZoombiniPuzzle *puzzle = dynamic_cast<ZoombiniPuzzle *>(page);
	if (!puzzle) {
		debugPrintf("Current page is not a puzzle page.\n");
		debugPrintf("\n");
		return true;
	}

	puzzle->debugForceFinish();
	_vm->_state->markDebugStateMutation();
	debugPrintf("Departure triggered.\n");
	debugPrintf("\n");
	return true;
}

bool ZoombiniConsole::CmdSub_Puzzle_Answer(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Print the generated answer for the active puzzle.\n");
		debugPrintf("Usage: %s|%s %s\n", kCmdPage, kCmdPuzzle, kSubCmdPagePuzzleAnswer);
		debugPrintf("\n");
		debugPrintf("The answer is formatted for the debugger console and does not\n");
		debugPrintf("change game state. It requires a created game or a loaded save.\n");
		debugPrintf("\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (!requireGameStateReady(Common::String::format("%s %s", argv[0], argv[1])))
		return true;

	ZoombiniPage *page = _vm->getActivePage();
	if (!page) {
		debugPrintf("No active page.\n");
		debugPrintf("\n");
		return true;
	}

	ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
	if (!interactive) {
		debugPrintf("Current page is not an interactive page.\n");
		debugPrintf("\n");
		return true;
	}

	Common::String answer = formatZoombiniDebugAnswer(interactive->debugGetAnswer());
	debugPrintf("%s", answer.c_str());
	if (answer.empty() || answer[answer.size() - 1] != '\n')
		debugPrintf("\n");
	if (answer.size() < 2 || answer[answer.size() - 2] != '\n')
		debugPrintf("\n");
	return true;
}

bool ZoombiniConsole::CmdSub_Puzzle_Chances(int argc, const char **argv) {
	enum SubCmdKind {
		kSubHelp,
		kSubGet,
		kSubSet,
	};

	ZoombiniPage *page = _vm->getActivePage();
	ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
	const bool canSetChances = interactive && interactive->debugCanSetChances();

	SubCmdKind subCmdKind = kSubHelp;
	if (argc == 2 || (argc == 3 && scumm_stricmp(argv[2], kSubCmdPagePuzzleChanceGet) == 0)) {
		subCmdKind = kSubGet;
	} else if (argc >= 3 && scumm_stricmp(argv[2], kSubCmdSet) == 0) {
		subCmdKind = kSubSet;
	}

	if (subCmdKind == kSubHelp || hasHelpOption(argc, argv)) {
		debugPrintf("Show or adjust the active puzzle's chance information.\n");
		debugPrintf("Usage: %s|%s %s|%s [%s|%s <remaining>]\n", kCmdPage, kCmdPuzzle,
					kSubCmdPagePuzzleChance, kSubCmdPagePuzzleChances, kSubCmdPagePuzzleChanceGet, kSubCmdSet);
		debugPrintf("\n");
		debugPrintf("Built-in subcommands:\n");
		debugPrintf("  (default)\n");
		debugPrintf("      Print the number of remaining chances for the active puzzle.\n");
		debugPrintf("  %s\n", kSubCmdPagePuzzleChanceGet);
		debugPrintf("      Alias for the default chance query.\n");
		debugPrintf("  %s [remaining]\n", kSubCmdSet);
		if (canSetChances)
			debugPrintf("      Set the number of remaining chances for the active puzzle.\n");
		else
			debugPrintf("      Unsupported by the active page.\n");
		debugPrintf("\n");
		debugPrintf("Without an argument, print the chance model and current count.\n");
		debugPrintf("\n");
		if (canSetChances) {
			debugPrintf("With '%s [remaining]', set the number of chances left for the active puzzle.\n", kSubCmdSet);
			debugPrintf("The value must be non-negative and no greater than the puzzle's opportunity limit.\n");
		} else {
			debugPrintf("The '%s' subcommand is unsupported by the active page.\n", kSubCmdSet);
		}
		debugPrintf("\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (!requireGameStateReady(Common::String::format("%s %s", argv[0], argv[1])))
		return true;

	if (!page) {
		debugPrintf("No active page.\n");
		debugPrintf("\n");
		return true;
	}

	// Non-interactive pages, including transitions and the RodMap, use @ref ZmbChanceInfo::ZmbChanceType::kNone.
	ZmbChanceInfo info = ZmbChanceInfo();
	if (interactive)
		info = interactive->debugGetChances();

	if (subCmdKind == kSubSet) {
		if (!canSetChances || info.opportunities < 0) {
			debugPrintf("The active page does not support the '%s' subcommand.\n", kSubCmdSet);
			debugPrintf("\n");
			return true;
		}
		if (argc != 4) {
			debugPrintf("Usage: %s|%s %s|%s %s <remaining>\n", kCmdPage, kCmdPuzzle,
						kSubCmdPagePuzzleChance, kSubCmdPagePuzzleChances, kSubCmdSet);
			debugPrintf("\n");
			return true;
		}

		const char *remainingArg = argv[3];
		int32 remaining = -1;
		if (!ZmbResource::parseInt(remainingArg, remaining) || remaining < 0) {
			debugPrintf("Invalid remaining chances '%s'. Must be a non-negative integer.\n", remainingArg);
			debugPrintf("\n");
			return true;
		}
		if (info.opportunities < remaining) {
			debugPrintf("Remaining chances must be between 0 and %d.\n", info.opportunities);
			debugPrintf("\n");
			return true;
		}
		if (!interactive->debugSetChances(static_cast<int16>(remaining))) {
			debugPrintf("The active puzzle does not support adjusting chances.\n");
			debugPrintf("\n");
			return true;
		}

		info = interactive->debugGetChances();
		debugPrintf("Chances left set to %d.\n\n", info.chancesLeft());
	}

	debugPrintf("Chance type: %s\n", ZmbChanceInfo::typeName(info.type));

	switch (info.type) {
	case ZmbChanceInfo::ZmbChanceType::kMistake:
	case ZmbChanceInfo::ZmbChanceType::kSubmit:
		const char *unitName;
		if (info.unitName)
			unitName = info.unitName;
		else if (info.type == ZmbChanceInfo::ZmbChanceType::kMistake)
			unitName = "mistake";
		else
			unitName = "submit";
		debugPrintf("  One chance is used per [%s].\n", unitName);
		if (0 <= info.opportunities)
			debugPrintf("  Opportunities: %d\n", info.opportunities);
		else
			debugPrintf("  Opportunities: unbounded (no per-visit limit enforced)\n");
		if (0 <= info.chancesLeft())
			debugPrintf("  Chances left:  %d\n", info.chancesLeft());
		else
			debugPrintf("  Chances left:  N/A\n");
		if (0 <= info.used)
			debugPrintf("  Chances used:  %d\n", info.used);
		break;
	case ZmbChanceInfo::ZmbChanceType::kInfinite:
		debugPrintf("  The player can try without any chance limitation.\n");
		break;
	case ZmbChanceInfo::ZmbChanceType::kAmorphous:
		if (0 <= info.chancesLeft()) {
			debugPrintf("  This puzzle exposes a page-specific finite resource budget.\n");
			if (info.unitName)
				debugPrintf("  Resource unit: %s\n", info.unitName);
		} else {
			debugPrintf("  Cannot count the chances of this puzzle.\n");
		}
		break;
	case ZmbChanceInfo::ZmbChanceType::kNone:
	default:
		debugPrintf("  This page is not a puzzle.\n");
		break;
	}

	if (interactive) {
		Common::String details = interactive->debugGetChanceDetails();
		if (!details.empty())
			debugPrintf("%s", details.c_str());
	}
	debugPrintf("\n");
	return true;
}

// [*] Miscellaneous commands

bool ZoombiniConsole::Cmd_ManShortcuts(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Print the Zoombini gameplay shortcut manual.\n");
		debugPrintf("Usage: %s\n", kCmdManShortcuts);
		debugPrintf("\n");
		debugPrintf("Prints default keyboard shortcuts, as well as ScummVM remaps if it exists.\n");
		debugPrintf("\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 1) {
		debugPrintf("[Manual] Print Zoombinis gameplay shortcuts documented by the manuals or the original engine\n");
		debugPrintf("Usage: %s\n", kCmdManShortcuts);
		debugPrintf("\n");
		return true;
	}

	debugPrintf("Zoombinis keyboard shortcut help manual\n");
	debugPrintf("Modifier note: ScummVM currently recognizes Ctrl for the original Ctrl/Command shortcuts.\n");

	debugPrintf("\n[General Commands]\n");
	debugPrintf("  '/' or '?'  Open the Options dialog\n");
	printShortcutRemapping(Common::kStandardActionOpenSettings);
	debugPrintf("  Ctrl+N      Start a new game\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionNewGame);
	debugPrintf("  Ctrl+L      Open Load Game\n");
	printShortcutRemapping(Common::kStandardActionLoad);
	debugPrintf("  Ctrl+S      Open Save Game\n");
	printShortcutRemapping(Common::kStandardActionSave);
	debugPrintf("  Ctrl+R      Remove the selected game in the Load dialog\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionRemoveSave);
	debugPrintf("  Ctrl+Q      Quit\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionQuit);

	debugPrintf("\n[Options Toggles]\n");
	debugPrintf("  Ctrl+D      Toggle dialog, sound effects, and Zoombini voices\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionToggleDialogAndSfx);
	debugPrintf("  Ctrl+B      Toggle background music\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionToggleMusic);
	debugPrintf("  Ctrl+J      Toggle Sticky Mouse\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionToggleStickyMouse);
	debugPrintf("  Ctrl+U      Toggle Auto Sticky Mouse\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionToggleAutoStickyMouse);
	debugPrintf("  Ctrl+T      Toggle map transitions\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionToggleTransitions);
	debugPrintf("  Ctrl+G      Toggle Less/More Action background animation mode\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionToggleLessMoreAction);
	debugPrintf("  Ctrl+H      Hide or show the cursor\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionToggleCursor);

	debugPrintf("\n[Help Shortcuts]\n");
	debugPrintf("  F1          Replay the active puzzle's help voice, when one is available\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionReplayPuzzleInstructions);
	debugPrintf("  Ctrl+A      Toggle Help Audio in the TLC v2.0 release\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionToggleHelpDialogNarration);
	debugPrintf("  Ctrl+K      Toggle TouchSense in the TLC v2.0 release (hardware feedback is not implemented)\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionToggleTouchSense);

	debugPrintf("\n[Picker]\n");
	debugPrintf("  Ctrl+W      Cycle wave and boat animation: both moving, both frozen, waves only, or boat only\n");
	debugPrintf("  r           Show the Zoombini Update page the next time Help opens\n");

	debugPrintf("\n[Road Map]\n");
	debugPrintf("  Ctrl+P      Toggle Practice Mode on the map\n");
	printShortcutRemapping(MohawkMetaEngine_Zoombini::kActionPracticeMode);
	debugPrintf("  1-4         Select practice difficulty while Practice Mode is active on the map\n");

	debugPrintf("\n[Enhanced ScummVM-only Shortcuts] (%s)\n", _vm->useEnhancedKbdShortcuts() ? "enabled" : "disabled");
	if (_vm->useEnhancedKbdShortcuts()) {
		debugPrintf("  Help dialog: Enter/Esc closes, arrows/PageUp/PageDown change page\n");
		debugPrintf("  Save dialog: PageUp/PageDown scrolls by one page\n");
		debugPrintf("               Delete/Home/End edits the save-name text box\n");
		debugPrintf("  Load dialog: Up/Down changes selection\n");
		debugPrintf("               PageUp/PageDown scrolls by one page\n");
		debugPrintf("               Delete removes the selected save\n");
		debugPrintf("  Transfer:    Space/Enter/Esc skips the transition immediately, just like Click.\n");
		debugPrintf("  Credits:     '[' rewinds and ']' fast-forwards while held\n");
		debugPrintf("  Town:        Left/Right scrolls the town view\n");
	} else {
		debugPrintf("  Load dialog: Up/Down scrolls by one page\n");
	}

	debugPrintf("\n");
	return true;
}

bool ZoombiniConsole::Cmd_ManBuiltinDebug(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Print the Zoombinis built-in debug-key manual.\n");
		debugPrintf("Usage: %s\n\n", kCmdManBuiltinDebug);
		debugPrintf("The manual covers activation, shared diagnostics, MIDI test mode,\n");
		debugPrintf("and the original page-local keys exposed by ScummVM.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 1) {
		debugPrintf("Usage: %s\n\n", kCmdManBuiltinDebug);
		return true;
	}

	debugPrintf("Zoombinis built-in debug manual\n");
	printBuiltinDebugStatus();
	debugPrintf("\nNOTE: Some debug shortcuts may be shadowed by ScummVM global shortcuts.\n");
	debugPrintf("\n[Activation and ownership]\n");
	debugPrintf("  Ctrl+A, type lowercase 'go snoids', Space\n");
	debugPrintf("      Enable the serialized gameplay debug flag and show 'you got it'.\n");
	debugPrintf("  state set gameplay.debug on|off\n");
	debugPrintf("      Set the same flag through the ScummVM debugger.\n");
	debugPrintf("  Page-local keys run before the shared keys below.\n");
	debugPrintf("  Close this debugger before using an in-game key.\n");

	debugPrintf("\n[Recovery and reset]\n");
	debugPrintf("  If ] stops normal screen rendering, press [ to turn Step Mode off and resume it.\n");
	debugPrintf("  Then press = only if a full redraw is needed to clear direct diagnostics.\n");
	debugPrintf("  For a full transient reset, run 'debug disable', then 'debug enable'.\n");
	debugPrintf("  This reset does not change the independent built-in MIDI-test mode.\n");

	debugPrintf("\n[Shared keys: debug mode must be enabled]\n");
	debugPrintf("  [           Resume normal continuous screen rendering by turning Step Mode off\n");
	debugPrintf("  ]           Stop normal continuous screen rendering by turning Step Mode on;\n");
	debugPrintf("              later presses advance one pass or runner\n");
	debugPrintf("  Shift+N     Draw NODE points and the next PATH diagnostic\n");
	debugPrintf("  Shift+S     Toggle queued-sound start notifications\n");
	debugPrintf("  =           Request a full-screen redraw by merging the whole screen into the dirty region\n");
	debugPrintf("  &           Draw the 256-entry palette grid\n");
	debugPrintf("  *           Reset the fidget interval threshold\n");
	debugPrintf("  @           Set the original level-1 route-unlock bits\n");
	debugPrintf("  Ctrl+E/F    Draw all/Snoid-only click rectangles and render-state list positions\n");
	debugPrintf("  Ctrl+X/Y    Draw all/Snoid-only runner click rectangles and IDs\n");
	debugPrintf("  Ctrl+I      Force every available Zoombini into the active party\n");
	debugPrintf("  Ctrl+R      Toggle dragged-Zoombini coordinate notifications\n");
	debugPrintf("  Ctrl+Z      Toggle the palette-14 runner-debug backdrop\n");
	debugPrintf("  Debugger counterparts: debug -h\n");

	debugPrintf("\n[MIDI test mode: independent of gameplay.debug]\n");
	debugPrintf("  Ctrl+A, type lowercase 'midi', Space\n");
	if (_vm->isVersionFamilyTlcV2()) {
		debugPrintf("      Toggle the built-in MUSIC test selector.\n");
		debugPrintf("  Space       Show the current selector; only Picker's SND 30001 can enter the music queue\n");
		debugPrintf("  Shift+Space Advance with wraparound, then run the selected selector\n");
	} else {
		debugPrintf("      Toggle the built-in 18-entry MIDI test selector.\n");
		debugPrintf("  Space       Play the current test tMID unless MIDI is already playing\n");
		debugPrintf("  Shift+Space Advance with wraparound, then play the selected tMID\n");
	}
	debugPrintf("  Debugger counterparts: midi builtin_debug -h; state get|set midi.debug\n");

	debugPrintf("\n[Page-local keys: debug mode must be enabled unless noted]\n");
	debugPrintf("  RoadMap\n");
	debugPrintf("    +/-           Change the next generated practice pack by one (range 1-16)\n");
	debugPrintf("                  The current pack is unchanged\n");
	debugPrintf("    Shift+T, a-p  In practice mode, generate that pack and start XFER from source page a-p\n");
	debugPrintf("                  a-p: Picker, Bridge, Tunnels, Pizza, BC1 North, Ferry, Lilly, Slides,\n");
	debugPrintf("                       BC1 South, Fleens, Hotel, Net, BC2, Caves, Smoke, Maze\n");
	debugPrintf("                  Any non-selector key after Shift+T cancels the armed selector\n");
	debugPrintf("  Bridge\n");
	debugPrintf("    a             Draw the named lane and traits it accepts; the other lane accepts the complement\n");
	debugPrintf("                  This is read-only and is unavailable while a pack path walk is pending\n");
	debugPrintf("    r             Show the last rejection event type and selected page SCRS 1000-1019\n");
	debugPrintf("                  Values are -1 before the first rejection; wait for the next key or mouse button\n");
	debugPrintf("                  This is read-only and is unavailable while a pack path walk is pending\n");
	debugPrintf("  Tunnels\n");
	debugPrintf("    a             Draw the generated orientation and trait conditions (read-only)\n");
	debugPrintf("    Shift+C/F/I/O Load Crystal 4000-4038 / Ferrous 4200-4226 / Igno 4400-4423 / Onyx 4600-4617\n");
	debugPrintf("                  One shared index wraps per family; switching families starts at its first ID\n");
	debugPrintf("    Shift+H       Add four to the celebration target; eligible entered Zoombinis animate later\n");
	debugPrintf("    t             Queue from the nonrepeating 10-entry idle actor pool\n");
	debugPrintf("    Shift+T       Queue Igno 4420-4422 from the 3-entry post-game pool; do not finish the puzzle\n");
	debugPrintf("    e             Queue from the ready 7-entry, all-idle 8-entry, or partial 9-entry actor pool\n");
	debugPrintf("                  Does not press Go, move a Zoombini, or start departure\n");
	debugPrintf("    w             Queue the 4-entry first-open pool only at open count 1; otherwise the 8-entry pool\n");
	debugPrintf("    Shift+W       Set the engine-session open count to 1, then queue the first-open pool\n");
	debugPrintf("  Pizza\n");
	debugPrintf("    a             Draw active troll topping flags and the current meal (read-only)\n");
	debugPrintf("    b             Add an inert standard runner retaining absent retail SCRB ID 17\n");
	debugPrintf("                  It normally has no visible result\n");
	debugPrintf("    Shift+R/O/D   Arm private debug state 1, then 2, then 3; puzzle state is unchanged\n");
	debugPrintf("                  p, Space, and n/w/s do nothing before state 3\n");
	debugPrintf("    p             Load question reactions for present trolls; suppress narrator and leave Go disabled\n");
	debugPrintf("    Space         Restore the initial remaining-delivery count; do not reveal or change toppings\n");
	debugPrintf("    n/w/s         Cycle Arno 8000-8035 / Willa 9000-9034 / Shyler 10000-10038\n");
	debugPrintf("                  Willa exists on L2-4 and Shyler on L3-4; each selector wraps independently\n");
	debugPrintf("  Ferry\n");
	debugPrintf("    a             Rebuild the live placement matrix from seat bounds and draw its edges\n");
	debugPrintf("                  Later placements use it; level 4 adds the vertical-neighbor test\n");
	debugPrintf("    f             Retain the current Captain SCRB on first use, then replay that same SCRB\n");
	debugPrintf("                  Replays its frame sound and does not cycle beyond the retained ID 1800-1832\n");
	debugPrintf("    l             Show one-based route difficulty Level 1-4 without changing it\n");
	debugPrintf("  Slides\n");
	debugPrintf("    s-o-v-l-e     Advance a persistent lowercase sequence; wrong-order letters do not reset it\n");
	debugPrintf("                  Every level reaches state 5, but only L4 places the retained generated solution\n");
	debugPrintf("                  L4 also rebuilds accepted links, completes the round, increments matches, and enables Go\n");
	debugPrintf("  Fleens\n");
	debugPrintf("    l             Show one-based Route 3 difficulty Level 1-4 without changing state\n");
	debugPrintf("  Hotel\n");
	debugPrintf("    Space         Restore counter step 1 and the initial palette; levels 1-3 reload SCRB 6001\n");
	debugPrintf("                  Room assignments and generated rules remain unchanged\n");
	debugPrintf("    a             Draw generated F/N/E/H axis initials; level 4 adds the third axis (read-only)\n");
	debugPrintf("    h/i           H arms a trigger at 1; I advances it. H then I reaches 2\n");
	debugPrintf("                  On an empty level-2 board, restart all 15 active room displays and queue page SND 7012\n");
	debugPrintf("                  Other levels have no effect; a nonempty level-2 board retries after it becomes empty\n");
	debugPrintf("    Shift+R       Permanently bypass level 2-3 two-axis and level-4 three-axis checks for this page instance\n");
	debugPrintf("                  Level 1 validation is unchanged\n");
	debugPrintf("    w/e           Replace the shared room/guide runner with SCRB 7000-7009 / 7010-7017\n");
	debugPrintf("                  The first presses load 7000 / 7010; the two selectors wrap independently\n");
	debugPrintf("  Net\n");
	debugPrintf("    Space         Restore the initial remaining-shot budget and next feedback SCRB offset\n");
	debugPrintf("                  After rejection, clear it and restart selector/feedback setup\n");
	debugPrintf("                  The hidden rule, targets, and Zoombini positions remain unchanged\n");
	debugPrintf("    l             Draw rule axes: SC=SubColor, SH=Shape, MC=Mudball Color (read-only)\n");
	debugPrintf("                  Cells are wall row, wall column, then PR on L1-2 or the remaining axis on L3-4\n");
	debugPrintf("  Caves\n");
	debugPrintf("    1-4           Set the runtime level, rebuild hints, and reassign current Zoombinis\n");
	debugPrintf("                  Retains the rule categories, controller state, and current mistake count\n");
	debugPrintf("    Space         Reset the controller, mistake budget, and glyph panel; retain rule and seats\n");
	debugPrintf("    F3            Return every Zoombini, clear seats/Advance, then perform the Space reset\n");
	debugPrintf("    Shift+L       Show the one-based runtime level without changing it\n");
	debugPrintf("    F5-F8         Use Hair/Eyes/Nose/Feet as primary, rebuild hints, and reassign Zoombinis\n");
	debugPrintf("                  Does not reset the controller or current mistake count\n");
	debugPrintf("  Smoke\n");
	debugPrintf("    Shift+L       Show Level 1-4 without changing the difficulty\n");
	debugPrintf("    F4            Keep later L3/L4 filter sets in generated row order\n");
	debugPrintf("                  Applies when the next question is generated after the current L3 Zoombini or L4 pair\n");
	debugPrintf("                  No effect in L1/L2\n");
	debugPrintf("                  Does not move or auto-place the current filters\n");
	debugPrintf("    F5            Resume shuffling later L3/L4 filter home positions\n");
	debugPrintf("                  Does not reshuffle the current filters; no effect in L1/L2\n");
	debugPrintf("  Picker\n");
	debugPrintf("    Shift+Generate With debug enabled and Picker empty, set generated/Town counts to 624\n");
	debugPrintf("                   Then run one normal Generate for the final visible Zoombini; do not fill seats\n");
	debugPrintf("                   ScummVM blocks saving this synthetic state until Load or New Game\n");
	debugPrintf("  Town\n");
	debugPrintf("    Space         Show the 0-16 memorial data selector; 0 is OFF; does not open a card\n");
	debugPrintf("    f             Stop a nonempty pending celebration queue, or restart its retained count\n");
	debugPrintf("                  The retained count initially is 0, so initial f displays 0 but queues nothing\n");
	debugPrintf("    Shift+F       Stop a nonempty queue, or advance by 5 (5-100, wrapping) and start it\n");
	debugPrintf("                  Starting waits for one input; stopping affects pending, not spawned, walkers\n");
	debugPrintf("    x/z           Decrement / increment the 0-16 memorial data selector without wrapping\n");
	debugPrintf("                  Requires slot 16 occupied; an open card previews that record's route/level/date\n");
	debugPrintf("    .             Fill the first empty saved memorial using today's date and the next\n");
	debugPrintf("                  deterministic route/level pair; a full memorial is unchanged\n");
	debugPrintf("    0             Clear saved route/occupancy bytes and insertion order; keep unused dates/levels\n");
	debugPrintf("                  Both . and 0 schedule the debug-modified memorial state to be saved\n");
	debugPrintf("    Up/Down       Adjust runtime building-shape threshold by 5, clamped to 25-81\n");
	debugPrintf("                  Refresh overlays only; saved population/development/inhabitants do not change\n");

	debugPrintf("\n[Related direct page keys]\n");
	debugPrintf("  F1          Replay active help audio when the page supplies it\n");
	debugPrintf("  Picker:     Ctrl+W cycles both moving, both frozen, waves only, boat only\n");
	debugPrintf("              Frozen layers stay visible; no change occurs unless both SCRB 4104/4105 runners exist\n");
	debugPrintf("              r arms the next Help update header; body redirects only if that Help is still Picker\n");
	debugPrintf("              Neither key requires debug mode; Ctrl+W is consumed, lowercase r is not\n");
	debugPrintf("  Road Map:   Ctrl+P toggles Practice Mode\n");
	debugPrintf("  Page debugger counterparts: page builtin_debug <action>; use 'page -h' for the active page\n\n");
	return true;
}

// [*] Helper methods

bool ZoombiniConsole::isHelpOption(const char *arg) {
	return arg && (scumm_stricmp(arg, "--help") == 0 || scumm_stricmp(arg, "-h") == 0);
}

bool ZoombiniConsole::hasHelpOption(int argc, const char **argv) {
	for (int i = 1; i < argc; i++) {
		if (isHelpOption(argv[i]))
			return true;
	}

	return false;
}

void ZoombiniConsole::printHelpOption() {
	debugPrintf("  -h, --help  Show this help text and exit.\n");
	debugPrintf("\n");
}

void ZoombiniConsole::printShortcutRemapping(const char *actionId) {
	Common::Keymapper *keymapper = _vm->getEventManager()->getKeymapper();
	if (!keymapper)
		return;

	Common::Keymap *keymap = keymapper->getKeymap("zoombini");
	if (!keymap)
		return;

	Common::Action *action = nullptr;
	for (Common::Action *candidate : keymap->getActions()) {
		if (Common::String(candidate->id).equals(actionId)) {
			action = candidate;
			break;
		}
	}
	if (!action)
		return;

	Common::ConfigManager::Domain *activeDomain = ConfMan.getActiveDomain();
	if (!activeDomain)
		return;

	const Common::String configKey = Common::String::format("keymap_%s_%s", keymap->getId().c_str(), action->id);
	if (!activeDomain->contains(configKey))
		return;

	const Common::Array<Common::HardwareInput> mappedInputs = keymap->getActionMapping(action);
	Common::U32String mappedText;
	for (uint i = 0; i < mappedInputs.size(); i++) {
		if (!mappedText.empty())
			mappedText += U", ";
		mappedText += formatManualShortcutMapping(mappedInputs[i]);
	}
	if (mappedText.empty())
		mappedText = U"-";

	const Common::String mappedTextUtf8 = mappedText.encode(Common::kUtf8);
	debugPrintf("      (Remapped to %s)\n", mappedTextUtf8.c_str());
}

void ZoombiniConsole::printBuiltinDebugStatus() {
	debugPrintf("Built-in debug flag: %s (serialized).\n", _vm->isBuiltinDebugMode() ? "enabled" : "disabled");
	debugPrintf("MIDI test mode: %s (runtime only; current selector %u).\n",
				_vm->isBuiltinMidiTestMode() ? "enabled" : "disabled", _vm->getBuiltinMidiTestResource());
}

bool ZoombiniConsole::parseMemorialDate(const char *value, uint16 &year, byte &month, byte &day) {
	if (!value)
		return false;

	uint32 dateValue = 0;
	for (uint digitIdx = 0; digitIdx < 8; digitIdx += 1) {
		const char digit = value[digitIdx];
		if (digit < '0' || '9' < digit)
			return false;
		dateValue = dateValue * 10 + static_cast<uint32>(digit - '0');
	}
	if (value[8] != '\0')
		return false;

	year = static_cast<uint16>(dateValue / 10000);
	month = static_cast<byte>((dateValue / 100) % 100);
	day = static_cast<byte>(dateValue % 100);
	return 1 <= year && 1 <= month && month <= 12 && 1 <= day && day <= 31;
}

bool ZoombiniConsole::parseInt(const char *str, int32 &result) {
	bool success = ZmbResource::parseInt(str, result);
	if (!success)
		debugPrintf("Cannot parse integer(%s) (hex supported with 0x prefix)\n", str);
	return success;
}

bool ZoombiniConsole::parseResourceId(const char *str, ZmbResource &outRes) {
	bool success = ZmbResource::parse(str, outRes);
	if (!success) {
		debugPrintf("Cannot parse resourceId(%s), try <archive>:<ID> with ID in the 0-32767 range (hex supported with 0x prefix)\n", str);
		debugPrintf("\n");
	}
	return success;
}

bool ZoombiniConsole::requireGameStateReady(const Common::String &cmdBase) {
	if (!_vm->_state || !_vm->_state->isGameStateReady()) {
		debugPrintf("<%s> is unavailable until a new game is created or a saved game is loaded.\n", cmdBase.c_str());
		debugPrintf("\n");
		return false;
	}

	return true;
}

Common::String ZoombiniConsole::escapeTextDumpField(const Common::U32String &text) {
	Common::String utf8Text = text.encode(Common::kUtf8);
	Common::String escaped;
	for (uint index = 0; index < utf8Text.size(); index++) {
		switch (utf8Text[index]) {
		case '\\':
			escaped += "\\\\";
			break;
		case '\r':
			escaped += "\\r";
			break;
		case '\n':
			escaped += "\\n";
			break;
		case '\t':
			escaped += "\\t";
			break;
		default:
			escaped += utf8Text[index];
			break;
		}
	}
	return escaped;
}

Common::String ZoombiniConsole::quoteCsvDumpField(const Common::String &text) {
	Common::String quoted = "\"";
	for (uint index = 0; index < text.size(); index++) {
		if (text[index] == '"')
			quoted += "\"\"";
		else
			quoted += text[index];
	}
	quoted += "\"";
	return quoted;
}

Common::String ZoombiniConsole::quoteSimpleDumpField(const Common::String &text) {
	Common::String quoted = "\"";
	for (uint index = 0; index < text.size(); index++) {
		if (text[index] == '"')
			quoted += "\\\"";
		else
			quoted += text[index];
	}
	quoted += "\"";
	return quoted;
}

/**
 * Wrap debugger answers before the GUI console applies its own hard line break.
 *
 * The individual puzzles use indentation to group their generated rules.
 * Preserve that indentation and add two spaces on continuation lines so a
 * narrow console does not split words or lose the group relationship.
 */
Common::String ZoombiniConsole::formatZoombiniDebugAnswer(const Common::String &answer) {
	static constexpr uint32 kLineWidth = 88;
	Common::String formatted;
	size_t lineStart = 0;

	while (lineStart < answer.size()) {
		const size_t lineEnd = answer.findFirstOf('\n', lineStart);
		size_t lineLength;
		if (lineEnd == Common::String::npos)
			lineLength = answer.size() - lineStart;
		else
			lineLength = lineEnd - lineStart;
		Common::String line = answer.substr(lineStart, lineLength);
		const size_t indentationLength = line.findFirstNotOf(' ');
		size_t continuationLength;
		if (indentationLength == Common::String::npos)
			continuationLength = line.size();
		else
			continuationLength = indentationLength;
		Common::String continuationIndentation = line.substr(0, continuationLength);
		continuationIndentation += "  ";

		while (kLineWidth < line.size()) {
			size_t breakPosition = line.findLastOf(' ', kLineWidth);
			if (breakPosition == Common::String::npos || breakPosition <= indentationLength)
				breakPosition = line.findFirstOf(' ', kLineWidth);
			if (breakPosition == Common::String::npos)
				break;

			formatted += line.substr(0, breakPosition);
			formatted += '\n';
			const size_t nextWord = line.findFirstNotOf(' ', breakPosition);
			if (nextWord == Common::String::npos) {
				line.clear();
				break;
			}
			line = continuationIndentation + line.substr(nextWord);
		}

		formatted += line;
		if (lineEnd == Common::String::npos)
			break;
		formatted += '\n';
		lineStart = lineEnd + 1;
	}

	return formatted;
}

bool ZoombiniConsole::isDumpImageFormat(const char *arg) const {
	return scumm_stricmp(arg, "bmp") == 0 || scumm_stricmp(arg, "png") == 0;
}

bool ZoombiniConsole::parseDumpImageFormat(const char *arg, bool &exportAsPng) {
	if (scumm_stricmp(arg, "bmp") == 0) {
		exportAsPng = false;
		return true;
	}

	if (scumm_stricmp(arg, "png") == 0) {
#ifdef USE_PNG
		exportAsPng = true;
		return true;
#else
		debugPrintf("Cannot export tBMP resources to PNG due to lack of libpng support.\n");
		debugPrintf("\n");
		return false;
#endif
	}

	debugPrintf("Unknown export format '%s'. Use bmp or png.\n", arg);
	debugPrintf("\n");
	return false;
}

bool ZoombiniConsole::exportSurfaceToImage(const Common::String &filename, const Graphics::Surface *surface, const byte *palette, bool exportAsPng) {
	Common::String filepath = "dumps/" + filename;

	Common::DumpFile out;
	if (!out.open(Common::Path(filepath, '/'), true)) {
		debugPrintf("Failed to open file %s for writing\n", filepath.c_str());
		return false;
	}

	if (exportAsPng) {
#ifdef USE_PNG
		return Image::writePNG(out, *surface, palette);
#else
		debugPrintf("Cannot export tBMP resources to PNG due to lack of libpng support.\n");
		return false;
#endif
	}

	// @ref Image::writeBMP() expects a palette buffer with 256 colors or 768 bytes.
	return Image::writeBMP(out, *surface, palette);
}

const char *ZoombiniConsole::getLoadedFeatureListKindName(ZmbLoadedFeatureListKind listKind) {
	switch (listKind) {
	case ZmbLoadedFeatureListKind::kScrb:
		return "SCRB";
	case ZmbLoadedFeatureListKind::kSub:
		return "SUB";
	case ZmbLoadedFeatureListKind::kSnoid:
		return "SNOID";
	default:
		return "UNKNOWN";
	}
}

} // End of namespace Mohawk

#endif
