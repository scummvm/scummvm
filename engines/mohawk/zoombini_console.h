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

#ifndef MOHAWK_ZOOMBINI_CONSOLE_H
#define MOHAWK_ZOOMBINI_CONSOLE_H

#include "gui/debugger.h"

#ifdef ENABLE_ZOOMBINI

#include "mohawk/zoombini_page.h"
#include "mohawk/zoombini_resource.h"

namespace Graphics {

class Surface;

} // End of namespace Graphics

namespace Mohawk {

class MohawkEngine_Zoombini;

class ZoombiniConsole : public GUI::Debugger {
public:
	/** Create the Zoombini debugger and register its commands. */
	explicit ZoombiniConsole(MohawkEngine_Zoombini *vm);
	/** Release debugger-owned state. */
	~ZoombiniConsole() override;

private:
	/** Engine instance used by all debugger commands. */
	MohawkEngine_Zoombini *_vm;

	// Debugger command callbacks.
	// Every Cmd_* method in this section is registered with registerCmd().

	// CmdSub handlers receive the original debugger argc/argv without stripping argv[0].
	// argv[0] is the registered command, argv[1] is the subcommand, and argv[2] starts its arguments.

	// [*] Sound (tSND) commands
	/** Top-level debugger command for sound-effect playback. */
	static constexpr const char *kCmdSound = "sound";
	/** Sound-effect subcommand that plays a tWAV resource. */
	static constexpr const char *kSubCmdSoundPlay = "play";
	/** Sound-effect subcommand that stops playback. */
	static constexpr const char *kSubCmdSoundStop = "stop";
	/** Dispatch the sound debugger command to its selected subcommand. */
	bool Cmd_Sound(int argc, const char **argv);
	/** Handle the sound play debugger subcommand. */
	bool CmdSub_SoundPlay(int argc, const char **argv);
	/** Handle the sound stop debugger subcommand. */
	bool CmdSub_SoundStop(int argc, const char **argv);

	// [*] MIDI commands
	/** Top-level debugger command for MIDI playback. */
	static constexpr const char *kCmdMidi = "midi";
	/** MIDI subcommand that plays a page resource. */
	static constexpr const char *kSubCmdMidiPlay = "play";
	/** MIDI subcommand that stops playback. */
	static constexpr const char *kSubCmdMidiStop = "stop";
	/** MIDI subcommand that controls the built-in test selector. */
	static constexpr const char *kSubCmdMidiBuiltinDebug = "builtin_debug";
	/** Dispatch the MIDI debugger command to its selected subcommand. */
	bool Cmd_Midi(int argc, const char **argv);
	/** Handle the midi play debugger subcommand. */
	bool CmdSub_MidiPlay(int argc, const char **argv);
	/** Handle the midi stop debugger subcommand. */
	bool CmdSub_MidiStop(int argc, const char **argv);
	/** Handle the midi builtin_debug subcommand. */
	bool CmdSub_MidiBuiltinDebug(int argc, const char **argv);

	// [*] Draw commands
	/** Top-level debugger command for drawing resources. */
	static constexpr const char *kCmdDraw = "draw";
	/** Draw a cursor resource. */
	static constexpr const char *kSubCmdDrawCursor = "cursor";
	/** Draw an image resource. */
	static constexpr const char *kSubCmdDrawImage = "image";
	/** Draw one shape. */
	static constexpr const char *kSubCmdDrawShape = "shape";
	/** Draw a sequence of shapes. */
	static constexpr const char *kSubCmdDrawShapes = "shapes";
	/** Draw one SCRB feature. */
	static constexpr const char *kSubCmdDrawFeature = "feature";
	/** Render one localized text key in a Yes/No message box. */
	static constexpr const char *kSubCmdDrawMsgBox = "msgbox";
	/** Draw the active page through its terrain mask. */
	static constexpr const char *kSubCmdDrawTerrain = "terrain";
	/** Dispatch the draw command to its selected subcommand. */
	bool Cmd_Draw(int argc, const char **argv);
	/** Draw a cursor resource through the debugger. */
	bool CmdSub_DrawCursor(int argc, const char **argv);
	/** Draw an image resource through the debugger. */
	bool CmdSub_DrawImage(int argc, const char **argv);
	/** Draw one shape through the debugger. */
	bool CmdSub_DrawShape(int argc, const char **argv);
	/** Draw a shape sequence through the debugger. */
	bool CmdSub_DrawShapes(int argc, const char **argv);
	/** Draw one SCRB feature through the debugger. */
	bool CmdSub_DrawFeature(int argc, const char **argv);
	/** Queue one localized text key for a confirmation message box. */
	bool CmdSub_DrawMsgBox(int argc, const char **argv);
	/** Draw the active page through its terrain mask. */
	bool CmdSub_DrawTerrain(int argc, const char **argv);

	// [*] Dump commands
	/** Top-level debugger command for resource dumps. */
	static constexpr const char *kCmdDump = "dump";
	/** Dump MIDI resources. */
	static constexpr const char *kSubCmdDumpMidi = "midi";
	/** Dump one image resource. */
	static constexpr const char *kSubCmdDumpImage = "image";
	/** Dump shapes from an image resource. */
	static constexpr const char *kSubCmdDumpShapes = "shapes";
	/** Dump one SCRB feature, or all of them without an ID. */
	static constexpr const char *kSubCmdDumpFeature = "feature";
	/** Dump one SCRS snoid script, or all of them without an ID. */
	static constexpr const char *kSubCmdDumpScrs = "scrs";
	/** Dump localized text resources. */
	static constexpr const char *kSubCmdDumpTexts = "texts";
	/** Export the live game state in the raw save layout. */
	static constexpr const char *kSubCmdDumpSave = "save";
	/** Dispatch the dump command to its selected subcommand. */
	bool Cmd_DumpResources(int argc, const char **argv);
	/** Dump MIDI resources to diagnostic files. */
	bool CmdSub_DumpMidi(int argc, const char **argv);
	/** Dump one image resource to a diagnostic file. */
	bool CmdSub_DumpImage(int argc, const char **argv);
	/** Dump shapes from an image resource. */
	bool CmdSub_DumpShapes(int argc, const char **argv);
	/** Dump SCRB feature descriptions to a diagnostic file. */
	bool CmdSub_DumpFeature(int argc, const char **argv);
	/** Dump SCRS snoid script descriptions to a diagnostic file. */
	bool CmdSub_DumpScrs(int argc, const char **argv);
	/** Dump localized Zoombini text resources. */
	bool CmdSub_DumpTexts(int argc, const char **argv);
	/** Serialize the live game state to a raw save-layout file under dumps/. */
	bool CmdSub_DumpSave(int argc, const char **argv);

	// [*] Feature (SCRB, SCRS) commands
	/** Top-level debugger command for printing feature and snoid-script data. */
	static constexpr const char *kCmdPrint = "print";
	/** Print subcommand for SCRB feature descriptions. */
	static constexpr const char *kSubCmdPrintFeature = "feature";
	/** Print subcommand that lists loaded feature runners. */
	static constexpr const char *kSubCmdPrintRunners = "runners";
	/** Print subcommand for SCRS snoid scripts. */
	static constexpr const char *kSubCmdPrintScrs = "scrs";
	/** Dispatch the print command to its selected subcommand. */
	bool Cmd_Print(int argc, const char **argv);
	/** Print SCRB feature descriptions to the debugger console. */
	bool CmdSub_PrintFeature(int argc, const char **argv);
	/** List loaded feature runners on the active page. */
	bool CmdSub_PrintRunners(int argc, const char **argv);
	/** Print SCRS snoid script descriptions to the debugger console. */
	bool CmdSub_PrintScrs(int argc, const char **argv);

	// [*] Plot commands
	/** Top-level debugger command for diagnostic plotting. */
	static constexpr const char *kCmdPlot = "plot";
	/** Plot one point. */
	static constexpr const char *kSubCmdPlotPoint = "point";
	/** Plot one line. */
	static constexpr const char *kSubCmdPlotLine = "line";
	/** Plot one rectangle. */
	static constexpr const char *kSubCmdPlotRect = "rect";
	/** Dispatch the plot command to its selected subcommand. */
	bool Cmd_Plot(int argc, const char **argv);
	/** Plot one diagnostic point. */
	bool CmdSub_PlotPoint(int argc, const char **argv);
	/** Plot one diagnostic line. */
	bool CmdSub_PlotLine(int argc, const char **argv);
	/** Plot one diagnostic rectangle. */
	bool CmdSub_PlotRect(int argc, const char **argv);

	// [*] Game state commands
	/** Top-level debugger command for direct game navigation. */
	static constexpr const char *kCmdGo = "go";
	/** Start a route-transfer page. */
	static constexpr const char *kSubCmdGoXfer = "xfer";
	/** Start a practice route. */
	static constexpr const char *kSubCmdGoPractice = "practice";
	/** Top-level debugger command for game-state inspection. */
	static constexpr const char *kCmdState = "state";
	/** State subcommand that prints usage. */
	static constexpr const char *kSubCmdHelp = "help";
	/** State subcommand that prints the current state. */
	static constexpr const char *kSubCmdGet = "get";
	/** State subcommand that changes one value. */
	static constexpr const char *kSubCmdSet = "set";
	/** State key for the gameplay sound-effects toggle. */
	static constexpr const char *kStateKeyOptionToggleSfx = "option.toggle.sfx";
	/** State key for the gameplay music toggle. */
	static constexpr const char *kStateKeyOptionToggleBgm = "option.toggle.bgm";
	/** State key for the gameplay sticky-mouse toggle. */
	static constexpr const char *kStateKeyOptionToggleStickyMouse = "option.toggle.stickyMouse";
	/** State key for the gameplay cursor toggle. */
	static constexpr const char *kStateKeyOptionToggleCursor = "option.toggle.cursor";
	/** State key for the gameplay automatic sticky-mouse toggle. */
	static constexpr const char *kStateKeyOptionToggleAutoStickyMouse = "option.toggle.autoStickyMouse";
	/** State key for the gameplay reduced-action toggle. */
	static constexpr const char *kStateKeyOptionToggleLessAction = "option.toggle.lessAction";
	/** State key for the release-specific gameplay transition toggle. */
	static constexpr const char *kStateKeyOptionToggleTransitions = "option.toggle.transitions";
	/** State key for the gameplay touch-sense toggle. */
	static constexpr const char *kStateKeyOptionToggleTouchSense = "option.toggle.touchSense";
	/** State key for the gameplay help-audio toggle. */
	static constexpr const char *kStateKeyOptionToggleHelpAudio = "option.toggle.helpAudio";
	/** State key for route one's player-facing difficulty level. */
	static constexpr const char *kStateKeyProgressRoute1Level = "progress.route1.level";
	/** State key for route two's player-facing difficulty level. */
	static constexpr const char *kStateKeyProgressRoute2Level = "progress.route2.level";
	/** State key for route three's player-facing difficulty level. */
	static constexpr const char *kStateKeyProgressRoute3Level = "progress.route3.level";
	/** State key for route four's player-facing difficulty level. */
	static constexpr const char *kStateKeyProgressRoute4Level = "progress.route4.level";
	/** State key for route one perfect-completion progress. */
	static constexpr const char *kStateKeyProgressRoute1PerfectCounter = "progress.route1.perfectCounter";
	/** State key for route two perfect-completion progress. */
	static constexpr const char *kStateKeyProgressRoute2PerfectCounter = "progress.route2.perfectCounter";
	/** State key for route three perfect-completion progress. */
	static constexpr const char *kStateKeyProgressRoute3PerfectCounter = "progress.route3.perfectCounter";
	/** State key for route four perfect-completion progress. */
	static constexpr const char *kStateKeyProgressRoute4PerfectCounter = "progress.route4.perfectCounter";
	/** State-key pattern for memorial active flags. */
	static constexpr const char *kStateKeyMemorialActivePattern = "memorial.route[1-4].level[1-4].active";
	/** State-key pattern for memorial achievement dates. */
	static constexpr const char *kStateKeyMemorialDatePattern = "memorial.route[1-4].level[1-4].date";
	/** State key for the serialized gameplay debug-mode flag. */
	static constexpr const char *kStateKeyGameplayDebug = "gameplay.debug";
	/** State key for the runtime-only MIDI test mode. */
	static constexpr const char *kStateKeyMidiDebug = "midi.debug";
	/** State key for Fleens hair-value rotation. */
	static constexpr const char *kStateKeyGameplayFleensHairValueRotation = "gameplay.fleens.hair.valueRotation";
	/** State key for Fleens eye-value rotation. */
	static constexpr const char *kStateKeyGameplayFleensEyesValueRotation = "gameplay.fleens.eyes.valueRotation";
	/** State key for Fleens nose-value rotation. */
	static constexpr const char *kStateKeyGameplayFleensNoseValueRotation = "gameplay.fleens.nose.valueRotation";
	/** State key for Fleens feet-value rotation. */
	static constexpr const char *kStateKeyGameplayFleensFeetValueRotation = "gameplay.fleens.feet.valueRotation";
	/** State key for the Fleens hair destination slot. */
	static constexpr const char *kStateKeyGameplayFleensHairDestSlot = "gameplay.fleens.hair.destSlot";
	/** State key for the Fleens eye destination slot. */
	static constexpr const char *kStateKeyGameplayFleensEyesDestSlot = "gameplay.fleens.eyes.destSlot";
	/** State key for the Fleens nose destination slot. */
	static constexpr const char *kStateKeyGameplayFleensNoseDestSlot = "gameplay.fleens.nose.destSlot";
	/** State key for the Fleens feet destination slot. */
	static constexpr const char *kStateKeyGameplayFleensFeetDestSlot = "gameplay.fleens.feet.destSlot";
	/** State key for the gameplay maze next-layout index. */
	static constexpr const char *kStateKeyGameplayMazeNextLayout = "gameplay.maze.nextLayout";
	/** Parsed state key category shared by state get and state set. */
	enum StateKeyKind : uint16 {
		kStateKindUnknown = 0,

		kStateKindSfx = 100,
		kStateKindBgm,
		kStateKindStickyMouse,
		kStateKindCursor,
		kStateKindAutoStickyMouse,
		kStateKindLessAction,
		kStateKindTransitions,
		kStateKindTouchSense,
		kStateKindHelpAudio,

		kStateKindRoute1Level = 200,
		kStateKindRoute2Level,
		kStateKindRoute3Level,
		kStateKindRoute4Level,
		kStateKindRoute1PerfectCounter,
		kStateKindRoute2PerfectCounter,
		kStateKindRoute3PerfectCounter,
		kStateKindRoute4PerfectCounter,
		kStateKindMemorialActive,
		kStateKindMemorialDate,

		kStateKindDebug = 300,
		kStateKindFleensTraitValueRotation,
		kStateKindFleensTraitDestSlot,
		kStateKindMazeNextLayout,

		kStateKindMidiDebug = 400,
	};
	/** Dispatch direct navigation to its selected subcommand. */
	bool Cmd_Go(int argc, const char **argv);
	/** Start a route transfer page from the debugger. */
	bool CmdSub_GoXfer(int argc, const char **argv);
	/** Start a practice route from the debugger. */
	bool CmdSub_GoPractice(int argc, const char **argv);
	/** Inspect or modify debugger-exposed game state. */
	bool Cmd_State(int argc, const char **argv);
	/** Set one debugger-exposed game-state key. */
	bool CmdSub_StateSet(int argc, const char **argv);
	/** Top-level command exposing shared built-in debug actions. */
	static constexpr const char *kCmdBuiltinDebug = "builtin_debug";
	/** Dispatch a shared built-in debug action to the active interactive page. */
	bool Cmd_BuiltinDebug(int argc, const char **argv);
	/** Print the state keys available for the active release family. */
	void printAvailableStateKeys();
	/** Print the warning and responsibility disclaimer for state-set writes. */
	void printStateSetWarning();
	/** Parse a debugger state key into its shared state category. */
	static StateKeyKind parseStateKey(const char *key);
	/** Parse one route/level memorial key and return its zero-based route index. */
	static StateKeyKind parseMemorialStateKey(const char *key, uint &routeIndex, uint &difficultyLevel);
	/** Parse one Fleens trait key and return its zero-based source trait index. */
	static StateKeyKind parseFleensTraitStateKey(const char *key, uint &traitIndex);
	/** Return whether a state key exists in the active release family. */
	bool isStateKeyAvailable(StateKeyKind keyKind) const;
	/** Return whether route state writes are allowed on the active non-puzzle page. */
	bool isRouteStateMutationAllowed() const;
	/** Print the value represented by one state key. */
	void printStateKeyValue(const char *key, StateKeyKind keyKind, uint memorialRouteIndex = 0, uint memorialLevel = 0,
							uint fleensTraitIndex = 0);

private:
	/** Format route completion bits for the state command. */
	static Common::String formatCompletionFlags(uint8 flags);

	// [*] Page/Puzzle commands
	/** Top-level debugger command for page controls. */
	static constexpr const char *kCmdPage = "page";
	/** Top-level debugger command for puzzle controls. */
	static constexpr const char *kCmdPuzzle = "puzzle";
	/** Page subcommand exposing built-in page-local debug actions. */
	static constexpr const char *kSubCmdPageBuiltinDebug = "builtin_debug";
	/** Finish the current puzzle. */
	static constexpr const char *kSubCmdPagePuzzleFinish = "finish";
	/** Inspect or set the current puzzle answer. */
	static constexpr const char *kSubCmdPagePuzzleAnswer = "answer";
	/** Inspect or set the current puzzle chances. */
	static constexpr const char *kSubCmdPagePuzzleChance = "chance";
	/** Alias for the default current puzzle chance query. */
	static constexpr const char *kSubCmdPagePuzzleChanceGet = "get";
	/** Inspect or set the current puzzle chances. */
	static constexpr const char *kSubCmdPagePuzzleChances = "chances";
	/** Dispatch page puzzle controls to their selected subcommand. */
	bool Cmd_PagePuzzle(int argc, const char **argv);
	/** Dispatch a built-in page-local debug action to its owning page. */
	bool CmdSub_PageBuiltinDebug(int argc, const char **argv);
	/** Finish the current puzzle through its owning page. */
	bool CmdSub_Puzzle_Finish(int argc, const char **argv);
	/** Set or display the current puzzle answer through the debugger. */
	bool CmdSub_Puzzle_Answer(int argc, const char **argv);
	/** Set or display the current puzzle chance or resource budget. */
	bool CmdSub_Puzzle_Chances(int argc, const char **argv);

	// [*] Miscellaneous commands
	/** Print the debugger shortcut manual. */
	static constexpr const char *kCmdManShortcuts = "man_shortcuts";
	/** Print the compact debugger shortcut manual. */
	bool Cmd_ManShortcuts(int argc, const char **argv);
	/** Print the built-in debug-key manual. */
	static constexpr const char *kCmdManBuiltinDebug = "man_builtin_debug";
	/** Print activation, shared, MIDI, and page-local built-in debug keys. */
	bool Cmd_ManBuiltinDebug(int argc, const char **argv);

	// [*] Helper methods
	/** Return whether @p arg requests command help. */
	static bool isHelpOption(const char *arg);
	/** Return whether any command argument requests help. */
	static bool hasHelpOption(int argc, const char **argv);
	/** Print the standard help-option usage line. */
	void printHelpOption();
	/** Print the active saved override for one shortcut action. */
	void printShortcutRemapping(const char *actionId);
	/** Print the serialized debug flag and runtime-only MIDI test state. */
	void printBuiltinDebugStatus();
	/** Parse a signed integer debugger argument. */
	bool parseInt(const char *str, int32 &result);
	/** Parse a memorial date in yyyyMMdd form. */
	static bool parseMemorialDate(const char *value, uint16 &year, byte &month, byte &day);
	/** Parse a qualified Zoombini resource argument. */
	bool parseResourceId(const char *str, ZmbResource &outRes);
	/** Build the text description of one parsed SCRB feature. */
	Common::String buildScrbDescription(const ZmbResource &resource);
	/** Build the text description of one parsed SCRS snoid script. */
	Common::String buildScrsDescription(const ZmbResource &resource);
	/** Ensure the engine has an active state before a state command runs. */
	bool requireGameStateReady(const Common::String &cmdBase);
	/** Escape one text field for the plain resource dump format. */
	static Common::String escapeTextDumpField(const Common::U32String &text);
	/** Quote one text field for CSV output. */
	static Common::String quoteCsvDumpField(const Common::String &text);
	/** Quote one text field for simple line-oriented output. */
	static Common::String quoteSimpleDumpField(const Common::String &text);
	/** Format a puzzle answer for debugger output. */
	static Common::String formatZoombiniDebugAnswer(const Common::String &answer);
	/** @return True if an export was successful. */
	bool isDumpImageFormat(const char *arg) const;
	/** Parse an image dump format and return whether PNG output was requested. */
	bool parseDumpImageFormat(const char *arg, bool &exportAsPng);
	/** Export a surface with the requested image format and palette. */
	bool exportSurfaceToImage(const Common::String &filename, const Graphics::Surface *surface, const byte *palette, bool exportAsPng);
	/** Return the printable name of a loaded-feature list kind. */
	static const char *getLoadedFeatureListKindName(ZmbLoadedFeatureListKind listKind);
};

} // End of namespace Mohawk

#endif

#endif
