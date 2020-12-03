/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// FIXME: Avoid using printf
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#define FORBIDDEN_SYMBOL_EXCEPTION_exit

#include <limits.h>

#include "engines/metaengine.h"
#include "base/commandLine.h"
#include "base/plugins.h"
#include "base/version.h"

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/rendermode.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/tokenizer.h"

#include "gui/ThemeEngine.h"

#include "audio/musicplugin.h"

#include "graphics/renderer.h"

#define DETECTOR_TESTING_HACK
#define UPGRADE_ALL_TARGETS_HACK

namespace Base {

#ifndef DISABLE_COMMAND_LINE

static const char USAGE_STRING[] =
	"%s: %s\n"
	"Usage: %s [OPTIONS]... [GAME]\n"
	"\n"
	"Try '%s --help' for more options.\n"
;

// DONT FIXME: DO NOT ORDER ALPHABETICALLY, THIS IS ORDERED BY IMPORTANCE/CATEGORY! :)
#if defined(__SYMBIAN32__) || defined(ANDROID) || defined(__DS__) || defined(__3DS__)
static const char HELP_STRING[] = "NoUsageString"; // save more data segment space
#else
static const char HELP_STRING[] =
	"ScummVM - Graphical Adventure Game Interpreter\n"
	"Usage: %s [OPTIONS]... [GAME]\n"
	"  -v, --version            Display ScummVM version information and exit\n"
	"  -h, --help               Display a brief help text and exit\n"
	"  -z, --list-games         Display list of supported games and exit\n"
	"  -t, --list-targets       Display list of configured targets and exit\n"
	"  --list-engines           Display list of suppported engines and exit\n"
	"  --list-saves             Display a list of saved games for the target specified\n"
	"                           with --game=TARGET, or all targets if none is specified\n"
	"  -a, --add                Add all games from current or specified directory.\n"
	"                           If --game=ID is passed only the game with id ID is added. See also --detect\n"
	"                           Use --path=PATH to specify a directory.\n"
	"  --detect                 Display a list of games with their ID from current or\n"
	"                           specified directory without adding it to the config.\n"
	"                           Use --path=PATH to specify a directory.\n"
	"  --game=ID                In combination with --add or --detect only adds or attempts to\n"
	"                           detect the game with id ID.\n"
	"  --auto-detect            Display a list of games from current or specified directory\n"
	"                           and start the first one. Use --path=PATH to specify a directory.\n"
	"  --recursive              In combination with --add or --detect recurse down all subdirectories\n"
#if defined(WIN32) && !defined(__SYMBIAN32__)
	"  --console                Enable the console window (default:enabled)\n"
#endif
	"\n"
	"  -c, --config=CONFIG      Use alternate configuration file\n"
#if defined(SDL_BACKEND)
	"  -l, --logfile=PATH       Use alternate path for log file\n"
#endif
	"  -p, --path=PATH          Path to where the game is installed\n"
	"  -x, --save-slot[=NUM]    Save game slot to load (default: autosave)\n"
	"  -f, --fullscreen         Force full-screen mode\n"
	"  -F, --no-fullscreen      Force windowed mode\n"
	"  -g, --gfx-mode=MODE      Select graphics scaler (1x,2x,3x,2xsai,super2xsai,\n"
	"                           supereagle,advmame2x,advmame3x,hq2x,hq3x,tv2x,\n"
	"                           dotmatrix)\n"
	"  --stretch-mode=MODE      Select stretch mode (center, integral, fit, stretch)\n"
	"  --filtering              Force filtered graphics mode\n"
	"  --no-filtering           Force unfiltered graphics mode\n"
	"  --gui-theme=THEME        Select GUI theme\n"
	"  --themepath=PATH         Path to where GUI themes are stored\n"
	"  --list-themes            Display list of all usable GUI themes\n"
	"  -e, --music-driver=MODE  Select music driver (see README for details)\n"
	"  --list-audio-devices     List all available audio devices\n"
	"  -q, --language=LANG      Select language (en,de,fr,it,pt,es,jp,zh,kr,se,gb,\n"
	"                           hb,ru,cz)\n"
	"  -m, --music-volume=NUM   Set the music volume, 0-255 (default: 192)\n"
	"  -s, --sfx-volume=NUM     Set the sfx volume, 0-255 (default: 192)\n"
	"  -r, --speech-volume=NUM  Set the speech volume, 0-255 (default: 192)\n"
	"  --midi-gain=NUM          Set the gain for MIDI playback, 0-1000 (default:\n"
	"                           100) (only supported by some MIDI drivers)\n"
	"  -n, --subtitles          Enable subtitles (use with games that have voice)\n"
	"  -b, --boot-param=NUM     Pass number to the boot script (boot param)\n"
	"  -d, --debuglevel=NUM     Set debug verbosity level\n"
	"  --debugflags=FLAGS       Enable engine specific debug flags\n"
	"                           (separated by commas)\n"
	"  --debug-channels-only    Show only the specified debug channels\n"
	"  -u, --dump-scripts       Enable script dumping if a directory called 'dumps'\n"
	"                           exists in the current directory\n"
	"\n"
	"  --cdrom=DRIVE            CD drive to play CD audio from; can either be a\n"
	"                           drive, path, or numeric index (default: 0 = best\n"
	"                           choice drive)\n"
	"  --joystick[=NUM]         Enable joystick input (default: 0 = first joystick)\n"
	"  --platform=WORD          Specify platform of game (allowed values: 2gs, 3do,\n"
	"                           acorn, amiga, atari, c64, fmtowns, nes, mac, pc, pc98,\n"
	"                           pce, segacd, wii, windows)\n"
	"  --savepath=PATH          Path to where saved games are stored\n"
	"  --extrapath=PATH         Extra path to additional game data\n"
	"  --soundfont=FILE         Select the SoundFont for MIDI playback (only\n"
	"                           supported by some MIDI drivers)\n"
	"  --multi-midi             Enable combination AdLib and native MIDI\n"
	"  --native-mt32            True Roland MT-32 (disable GM emulation)\n"
	"  --dump-midi              Dumps MIDI events to 'dump.mid', until quitting from game\n"
	"                           (if file already exists, it will be overwritten)\n"
	"  --enable-gs              Enable Roland GS mode for MIDI playback\n"
	"  --output-rate=RATE       Select output sample rate in Hz (e.g. 22050)\n"
	"  --opl-driver=DRIVER      Select AdLib (OPL) emulator (db, mame"
#ifndef DISABLE_NUKED_OPL
                                                                     ", nuked"
#endif
#ifdef ENABLE_OPL2LPT
                                                                     ", opl2lpt"
#endif
                                                                              ")\n"
	"  --show-fps               Set the turn on display FPS info in 3D games\n"
	"  --no-show-fps            Set the turn off display FPS info in 3D games\n"
	"  --renderer=RENDERER      Select 3D renderer (software, opengl, opengl_shaders)\n"
	"  --aspect-ratio           Enable aspect ratio correction\n"
	"  --[no-]dirtyrects        Enable dirty rectangles optimisation in software renderer\n"
	"                           (default: enabled)\n"
	"  --render-mode=MODE       Enable additional render modes (hercGreen, hercAmber,\n"
	"                           cga, ega, vga, amiga, fmtowns, pc9821, pc9801, 2gs,\n"
	"                           atari, macintosh)\n"
#ifdef ENABLE_EVENTRECORDER
	"  --record-mode=MODE       Specify record mode for event recorder (record, playback,\n"
	"                           passthrough [default])\n"
	"  --record-file-name=FILE  Specify record file name\n"
	"  --disable-display        Disable any gfx output. Used for headless events\n"
	"                           playback by Event Recorder\n"
#endif
	"\n"
#if defined(ENABLE_SKY) || defined(ENABLE_QUEEN)
	"  --alt-intro              Use alternative intro for CD versions of Beneath a\n"
	"                           Steel Sky and Flight of the Amazon Queen\n"
#endif
	"  --copy-protection        Enable copy protection in games, when\n"
	"                           ScummVM disables it by default.\n"
	"  --talkspeed=NUM          Set talk speed for games (default: 60)\n"
	"                           Grim Fandango or EMI (default: 179).\n"
#if defined(ENABLE_SCUMM) || defined(ENABLE_GROOVIE)
	"  --demo-mode              Start demo mode of Maniac Mansion or The 7th Guest\n"
#endif
#if defined(ENABLE_DIRECTOR)
	"  --start-movie=NAME@NUM   Start movie at frame for Director\n"
	"							Either can be specified without the other.\n"
#endif
#ifdef ENABLE_SCUMM
	"  --tempo=NUM              Set music tempo (in percent, 50-200) for SCUMM games\n"
	"                           (default: 100)\n"
#endif
#if (defined(ENABLE_SCUMM) && defined(ENABLE_SCUMM_7_8)) || defined(ENABLE_GRIM)
	"  --dimuse-tempo=NUM       Set internal Digital iMuse tempo (10 - 100) per second\n"
	"                           (default: 10)\n"
#endif
	"  --engine-speed=NUM       Set frame per second limit (0 - 100), 0 = no limit\n"
	"                           (default: 60)\n"
	"                           Grim Fandango or Escape from Monkey Island\n"
	"\n"
	"The meaning of boolean long options can be inverted by prefixing them with\n"
	"\"no-\", e.g. \"--no-aspect-ratio\".\n"
;
#endif

static const char *s_appName = "scummvm";

static void NORETURN_PRE usage(const char *s, ...) GCC_PRINTF(1, 2) NORETURN_POST;

static void usage(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

#if !(defined(__SYMBIAN32__) || defined(__DS__))
	printf(USAGE_STRING, s_appName, buf, s_appName, s_appName);
#endif
	exit(1);
}

static void ensureFirstCommand(const Common::String &existingCommand, const char *newCommand) {
	if (!existingCommand.empty())
		usage("--%s: Cannot accept more than one command (already found --%s).", newCommand, existingCommand.c_str());
}

static Common::String buildQualifiedGameName(const Common::String &engineId, const Common::String &gameId) {
	return Common::String::format("%s:%s", engineId.c_str(), gameId.c_str());
}

#endif // DISABLE_COMMAND_LINE


void registerDefaults() {

	// Graphics
	ConfMan.registerDefault("fullscreen", false);
	ConfMan.registerDefault("filtering", false);
	ConfMan.registerDefault("aspect_ratio", false);
	ConfMan.registerDefault("gfx_mode", "normal");
	ConfMan.registerDefault("render_mode", "default");
	ConfMan.registerDefault("desired_screen_aspect_ratio", "auto");
	ConfMan.registerDefault("stretch_mode", "default");
	ConfMan.registerDefault("shader", "default");
	ConfMan.registerDefault("show_fps", false);
	ConfMan.registerDefault("dirtyrects", true);
	ConfMan.registerDefault("vsync", true);

	// Sound & Music
	ConfMan.registerDefault("music_volume", 192);
	ConfMan.registerDefault("sfx_volume", 192);
	ConfMan.registerDefault("speech_volume", 192);

	ConfMan.registerDefault("music_mute", false);
	ConfMan.registerDefault("sfx_mute", false);
	ConfMan.registerDefault("speech_mute", false);
	ConfMan.registerDefault("mute", false);

	ConfMan.registerDefault("multi_midi", false);
	ConfMan.registerDefault("native_mt32", false);
	ConfMan.registerDefault("dump_midi", false);
	ConfMan.registerDefault("enable_gs", false);
	ConfMan.registerDefault("midi_gain", 100);

	ConfMan.registerDefault("music_driver", "auto");
	ConfMan.registerDefault("mt32_device", "null");
	ConfMan.registerDefault("gm_device", "null");
	ConfMan.registerDefault("opl2lpt_parport", "null");

	ConfMan.registerDefault("cdrom", 0);

	ConfMan.registerDefault("enable_unsupported_game_warning", true);

	// Game specific
	ConfMan.registerDefault("path", "");
	ConfMan.registerDefault("platform", Common::kPlatformDOS);
	ConfMan.registerDefault("language", "en");
	ConfMan.registerDefault("subtitles", false);
	ConfMan.registerDefault("boot_param", 0);
	ConfMan.registerDefault("dump_scripts", false);
	ConfMan.registerDefault("save_slot", -1);
	ConfMan.registerDefault("autosave_period", 5 * 60); // By default, trigger autosave every 5 minutes

#if defined(ENABLE_SCUMM) || defined(ENABLE_SWORD2)
	ConfMan.registerDefault("object_labels", true);
#endif

	ConfMan.registerDefault("copy_protection", false);
	ConfMan.registerDefault("talkspeed", 60);

#if defined(ENABLE_SCUMM) || defined(ENABLE_GROOVIE)
	ConfMan.registerDefault("demo_mode", false);
#endif
#ifdef ENABLE_SCUMM
	ConfMan.registerDefault("tempo", 0);
#endif
#if (defined(ENABLE_SCUMM) && defined(ENABLE_SCUMM_7_8)) || defined(ENABLE_GRIM)
	ConfMan.registerDefault("dimuse_tempo", 10);
#endif

#if defined(ENABLE_SKY) || defined(ENABLE_QUEEN)
	ConfMan.registerDefault("alt_intro", false);
#endif

	// Miscellaneous
	ConfMan.registerDefault("joystick_num", 0);
	ConfMan.registerDefault("confirm_exit", false);
	ConfMan.registerDefault("disable_sdl_parachute", false);

	ConfMan.registerDefault("disable_display", false);
	ConfMan.registerDefault("record_mode", "none");
	ConfMan.registerDefault("record_file_name", "record.bin");

	ConfMan.registerDefault("gui_saveload_chooser", "grid");
	ConfMan.registerDefault("gui_saveload_last_pos", "0");

	ConfMan.registerDefault("gui_browser_show_hidden", false);
	ConfMan.registerDefault("gui_browser_native", true);
	ConfMan.registerDefault("gui_return_to_launcher_at_exit", false);
	// Specify threshold for scanning directories in the launcher
	// If number of game entries in scummvm.ini exceeds the specified
	// number, then skip scanning. -1 = scan always
	ConfMan.registerDefault("gui_list_max_scan_entries", -1);
	ConfMan.registerDefault("game", "");

#ifdef USE_FLUIDSYNTH
	// The settings are deliberately stored the same way as in Qsynth. The
	// FluidSynth music driver is responsible for transforming them into
	// their appropriate values.
	ConfMan.registerDefault("fluidsynth_chorus_activate", true);
	ConfMan.registerDefault("fluidsynth_chorus_nr", 3);
	ConfMan.registerDefault("fluidsynth_chorus_level", 100);
	ConfMan.registerDefault("fluidsynth_chorus_speed", 30);
	ConfMan.registerDefault("fluidsynth_chorus_depth", 80);
	ConfMan.registerDefault("fluidsynth_chorus_waveform", "sine");

	ConfMan.registerDefault("fluidsynth_reverb_activate", true);
	ConfMan.registerDefault("fluidsynth_reverb_roomsize", 20);
	ConfMan.registerDefault("fluidsynth_reverb_damping", 0);
	ConfMan.registerDefault("fluidsynth_reverb_width", 1);
	ConfMan.registerDefault("fluidsynth_reverb_level", 90);

	ConfMan.registerDefault("fluidsynth_misc_interpolation", "4th");
#endif
}

static bool parseGameName(const Common::String &gameName, Common::String &engineId, Common::String &gameId) {
	Common::StringTokenizer tokenizer(gameName, ":");
	Common::String token1, token2;

	if (!tokenizer.empty()) {
		token1 = tokenizer.nextToken();
	}

	if (!tokenizer.empty()) {
		token2 = tokenizer.nextToken();
	}

	if (!tokenizer.empty()) {
		return false; // Stray colon
	}

	if (!token1.empty() && !token2.empty()) {
		engineId = token1;
		gameId = token2;
		return true;
	} else if (!token1.empty()) {
		engineId.clear();
		gameId = token1;
		return true;
	}

	return false;
}

static QualifiedGameDescriptor findGameMatchingName(const Common::String &name) {
	if (name.empty()) {
		return QualifiedGameDescriptor();
	}

	Common::String engineId;
	Common::String gameId;
	if (!parseGameName(name, engineId, gameId)) {
		return QualifiedGameDescriptor(); // Invalid name format
	}

	// Check if the name is a known game id
	QualifiedGameList games = EngineMan.findGamesMatching(engineId, gameId);
	if (games.size() != 1) {
		// Game not found or ambiguous name
		return QualifiedGameDescriptor();
	}

	return games[0];
}

static Common::String createTemporaryTarget(const Common::String &engineId, const Common::String &gameId) {
	Common::String domainName = gameId;

	ConfMan.addGameDomain(domainName);
	ConfMan.set("engineid", engineId, domainName);
	ConfMan.set("gameid", gameId, domainName);

	// We designate targets which come strictly from command line
	// so ConfMan will not save them to the configuration file.
	ConfMan.set("id_came_from_command_line", "1", domainName);

	return domainName;
}

//
// Various macros used by the command line parser.
//

#ifndef DISABLE_COMMAND_LINE

// Use this for options which have an *optional* value
#define DO_OPTION_OPT(shortCmd, longCmd, defaultVal) \
	if (isLongCmd ? (!strcmp(s + 2, longCmd) || !memcmp(s + 2, longCmd"=", sizeof(longCmd"=") - 1)) : (tolower(s[1]) == shortCmd)) { \
		s += 2; \
		if (isLongCmd) { \
			s += sizeof(longCmd) - 1; \
			if (*s == '=') \
				s++; \
		} \
		const char *option = s; \
		if (*s == '\0' && !isLongCmd) { option = s2; i++; } \
		if (!option || *option == '\0') option = defaultVal; \
		if (option) settings[longCmd] = option;

// Use this for options which have a required (string) value
#define DO_OPTION(shortCmd, longCmd) \
	DO_OPTION_OPT(shortCmd, longCmd, 0) \
	if (!option) usage("Option '%s' requires an argument", argv[isLongCmd ? i : i-1]);

// Use this for options which have a required integer value
// (we don't check ERANGE because WinCE doesn't support errno, so we're stuck just rejecting LONG_MAX/LONG_MIN..)
#define DO_OPTION_INT(shortCmd, longCmd) \
	DO_OPTION(shortCmd, longCmd) \
	char *endptr; \
	long int retval = strtol(option, &endptr, 0); \
	if (*endptr != '\0' || retval == LONG_MAX || retval == LONG_MIN) \
		usage("--%s: Invalid number '%s'", longCmd, option);

// Use this for boolean options; this distinguishes between "-x" and "-X",
// resp. between "--some-option" and "--no-some-option".
#define DO_OPTION_BOOL(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s + 2, longCmd) || !strcmp(s + 2, "no-" longCmd)) : (tolower(s[1]) == shortCmd)) { \
		bool boolValue = (Common::isLower(s[1]) != 0); \
		s += 2; \
		if (isLongCmd) { \
			boolValue = !strcmp(s, longCmd); \
			s += boolValue ? (sizeof(longCmd) - 1) : (sizeof("no-" longCmd) - 1); \
		} \
		if (*s != '\0') goto unknownOption; \
		const char *option = boolValue ? "true" : "false"; \
		settings[longCmd] = option;

// Use this for options which never have a value, i.e. for 'commands', like "--help".
#define DO_COMMAND(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s + 2, longCmd)) : (tolower(s[1]) == shortCmd)) { \
		s += 2; \
		if (isLongCmd) \
			s += sizeof(longCmd) - 1; \
		if (*s != '\0') goto unknownOption; \
		ensureFirstCommand(command, longCmd); \
		command = longCmd;


#define DO_LONG_OPTION_OPT(longCmd, d)  DO_OPTION_OPT(0, longCmd, d)
#define DO_LONG_OPTION(longCmd)         DO_OPTION(0, longCmd)
#define DO_LONG_OPTION_INT(longCmd)     DO_OPTION_INT(0, longCmd)
#define DO_LONG_OPTION_BOOL(longCmd)    DO_OPTION_BOOL(0, longCmd)
#define DO_LONG_COMMAND(longCmd)        DO_COMMAND(0, longCmd)

// End an option handler
#define END_OPTION \
		continue; \
	}

// End an option handler
#define END_COMMAND \
		continue; \
	}


Common::String parseCommandLine(Common::StringMap &settings, int argc, const char * const *argv) {
	const char *s, *s2;
	Common::String command;

	if (!argv)
		return command;

	// argv[0] contains the name of the executable.
	if (argv[0]) {
		s = strrchr(argv[0], '/');
		s_appName = s ? (s + 1) : argv[0];
	}

	// We store all command line settings into a string map.

	// Iterate over all command line arguments and parse them into our string map.
	for (int i = 1; i < argc; ++i) {
		s = argv[i];
		s2 = (i < argc-1) ? argv[i+1] : 0;

		if (s[0] != '-') {
			// The argument doesn't start with a dash, so it's not an option.
			// Hence it must be the target name. We currently enforce that
			// this always comes last.
			if (i != argc - 1)
				usage("Stray argument '%s'", s);

			// We defer checking whether this is a valid target to a later point.
			return s;
		} else {
			// On MacOS X prior to 10.9 the OS is sometimes adding a -psn_X_XXXXXX argument (where X are digits)
			// to pass the process serial number. We need to ignore it to avoid an error.
			// When using XCode it also adds -NSDocumentRevisionsDebugMode YES argument if XCode option
			// "Allow debugging when using document Versions Browser" is on (which is the default).
#ifdef MACOSX
			if (strncmp(s, "-psn_", 5) == 0)
				continue;
			if (strcmp(s, "-NSDocumentRevisionsDebugMode") == 0) {
				++i; // Also skip the YES that follows
				continue;
			}
#endif

			bool isLongCmd = (s[0] == '-' && s[1] == '-');

			DO_COMMAND('h', "help")
			END_COMMAND

			DO_COMMAND('v', "version")
			END_COMMAND

			DO_COMMAND('t', "list-targets")
			END_COMMAND

			DO_COMMAND('z', "list-games")
			END_COMMAND

			DO_LONG_COMMAND("list-engines")
			END_COMMAND

			DO_COMMAND('a', "add")
			END_COMMAND

			DO_LONG_COMMAND("detect")
			END_COMMAND

			DO_LONG_COMMAND("auto-detect")
			END_COMMAND

#ifdef DETECTOR_TESTING_HACK
			// HACK FIXME TODO: This command is intentionally *not* documented!
			DO_LONG_COMMAND("test-detector")
			END_COMMAND
#endif

#ifdef UPGRADE_ALL_TARGETS_HACK
			// HACK FIXME TODO: This command is intentionally *not* documented!
			DO_LONG_COMMAND("upgrade-targets")
			END_COMMAND
#endif

			DO_LONG_COMMAND("list-saves")
			END_COMMAND

			DO_OPTION('c', "config")
			END_OPTION

#if defined(SDL_BACKEND)
			DO_OPTION('l', "logfile")
			END_OPTION
#endif

			DO_OPTION_INT('b', "boot-param")
			END_OPTION

			DO_OPTION_OPT('d', "debuglevel", "0")
			END_OPTION

			DO_LONG_OPTION("debugflags")
			END_OPTION

			DO_LONG_OPTION_BOOL("debug-channels-only")
			END_OPTION

			DO_OPTION('e', "music-driver")
			END_OPTION

			DO_LONG_COMMAND("list-audio-devices")
			END_COMMAND

			DO_LONG_OPTION_INT("output-rate")
			END_OPTION

			DO_OPTION_BOOL('f', "fullscreen")
			END_OPTION

			DO_LONG_OPTION_BOOL("filtering")
			END_OPTION

#ifdef ENABLE_EVENTRECORDER
			DO_LONG_OPTION_INT("disable-display")
			END_OPTION

			DO_LONG_OPTION("record-mode")
			END_OPTION

			DO_LONG_OPTION("record-file-name")
			END_OPTION
#endif

			DO_LONG_OPTION("opl-driver")
			END_OPTION

			DO_OPTION('g', "gfx-mode")
			END_OPTION

			DO_LONG_OPTION("stretch-mode")
			END_OPTION

			DO_LONG_OPTION("shader")
			END_OPTION

			DO_OPTION_INT('m', "music-volume")
			END_OPTION

			DO_OPTION_BOOL('n', "subtitles")
			END_OPTION

			DO_OPTION('p', "path")
				Common::FSNode path(option);
				if (!path.exists()) {
					usage("Non-existent game path '%s'", option);
				} else if (!path.isReadable()) {
					usage("Non-readable game path '%s'", option);
				}
			END_OPTION

			DO_OPTION('q', "language")
				if (Common::parseLanguage(option) == Common::UNK_LANG)
					usage("Unrecognized language '%s'", option);
			END_OPTION

			DO_OPTION_INT('s', "sfx-volume")
			END_OPTION

			DO_OPTION_INT('r', "speech-volume")
			END_OPTION

			DO_LONG_OPTION_INT("midi-gain")
			END_OPTION

			DO_OPTION_BOOL('u', "dump-scripts")
			END_OPTION

			DO_OPTION_OPT('x', "save-slot", "0")
			END_OPTION

			DO_LONG_OPTION_INT("cdrom")
			END_OPTION

			DO_LONG_OPTION_OPT("joystick", "0")
				settings["joystick_num"] = option;
				settings.erase("joystick");
			END_OPTION

			DO_LONG_OPTION("platform")
				int platform = Common::parsePlatform(option);
				if (platform == Common::kPlatformUnknown)
					usage("Unrecognized platform '%s'", option);
			END_OPTION

			DO_LONG_OPTION("soundfont")
				Common::FSNode path(option);
				if (!path.exists()) {
					usage("Non-existent soundfont path '%s'", option);
				} else if (!path.isReadable()) {
					usage("Non-readable soundfont path '%s'", option);
				}
			END_OPTION

			DO_LONG_OPTION_BOOL("disable-sdl-parachute")
			END_OPTION

			DO_LONG_OPTION_BOOL("multi-midi")
			END_OPTION

			DO_LONG_OPTION_BOOL("native-mt32")
			END_OPTION

			DO_LONG_OPTION_BOOL("dump-midi")
			END_OPTION

			DO_LONG_OPTION_BOOL("enable-gs")
			END_OPTION

			DO_LONG_OPTION_BOOL("aspect-ratio")
			END_OPTION

			DO_LONG_OPTION("render-mode")
				int renderMode = Common::parseRenderMode(option);
				if (renderMode == Common::kRenderDefault)
					usage("Unrecognized render mode '%s'", option);
			END_OPTION

			DO_LONG_OPTION_BOOL("dirtyrects")
			END_OPTION

			DO_LONG_OPTION("gamma")
			END_OPTION

			DO_LONG_OPTION("renderer")
				Graphics::RendererType renderer = Graphics::parseRendererTypeCode(option);
				if (renderer == Graphics::kRendererTypeDefault)
					usage("Unrecognized renderer type '%s'", option);
			END_OPTION

			DO_LONG_OPTION_BOOL("show-fps")
			END_OPTION

			DO_LONG_OPTION("savepath")
				Common::FSNode path(option);
				if (!path.exists()) {
					usage("Non-existent saved games path '%s'", option);
				} else if (!path.isWritable()) {
					usage("Non-writable saved games path '%s'", option);
				}
			END_OPTION

			DO_LONG_OPTION("extrapath")
				Common::FSNode path(option);
				if (!path.exists()) {
					usage("Non-existent extra path '%s'", option);
				} else if (!path.isReadable()) {
					usage("Non-readable extra path '%s'", option);
				}
			END_OPTION

			DO_LONG_OPTION_INT("talkspeed")
			END_OPTION

			DO_LONG_OPTION_BOOL("copy-protection")
			END_OPTION

			DO_LONG_OPTION("gui-theme")
			END_OPTION

			DO_LONG_OPTION("game")
			END_OPTION

			DO_LONG_OPTION_BOOL("recursive")
			END_OPTION

			DO_LONG_OPTION("themepath")
				Common::FSNode path(option);
				if (!path.exists()) {
					usage("Non-existent theme path '%s'", option);
				} else if (!path.isReadable()) {
					usage("Non-readable theme path '%s'", option);
				}
			END_OPTION

			DO_LONG_COMMAND("list-themes")
			END_COMMAND

			DO_LONG_OPTION("target-md5")
			END_OPTION

#ifdef ENABLE_SCUMM
			DO_LONG_OPTION_INT("tempo")
			END_OPTION
#endif
#if (defined(ENABLE_SCUMM) && defined(ENABLE_SCUMM_7_8)) || defined(ENABLE_GRIM)
			DO_LONG_OPTION_INT("dimuse-tempo")
			END_OPTION
#endif
#if defined(ENABLE_SCUMM) || defined(ENABLE_GROOVIE)
			DO_LONG_OPTION_BOOL("demo-mode")
			END_OPTION
#endif

#if defined(ENABLE_SKY) || defined(ENABLE_QUEEN)
			DO_LONG_OPTION_BOOL("alt-intro")
			END_OPTION
#endif

			DO_LONG_OPTION_INT("engine-speed")
			END_OPTION

#ifdef IPHONE
			// This is automatically set when launched from the Springboard.
			DO_LONG_OPTION_OPT("launchedFromSB", 0)
			END_OPTION
#endif

#if defined(WIN32) && !defined(__SYMBIAN32__)
			// Optional console window on Windows (default: enabled)
			DO_LONG_OPTION_BOOL("console")
			END_OPTION
#endif

#if defined(ENABLE_DIRECTOR)
			DO_LONG_OPTION("start-movie")
			END_OPTION
#endif

unknownOption:
			// If we get till here, the option is unhandled and hence unknown.
			usage("Unrecognized option '%s'", argv[i]);
		}
	}

	return command;
}

/** List all supported game IDs, i.e. all games which any loaded plugin supports. */
static void listGames() {
	printf("Game ID                        Full Title                                                 \n"
	       "------------------------------ -----------------------------------------------------------\n");

	const PluginList &plugins = EngineMan.getPlugins();
	for (PluginList::const_iterator iter = plugins.begin(); iter != plugins.end(); ++iter) {
		const MetaEngineDetection &metaengine = (*iter)->get<MetaEngineDetection>();

		PlainGameList list = metaengine.getSupportedGames();
		for (PlainGameList::const_iterator v = list.begin(); v != list.end(); ++v) {
			printf("%-30s %s\n", buildQualifiedGameName(metaengine.getEngineId(), v->gameId).c_str(), v->description);
		}
	}
}

/** List all supported engines, i.e. all loaded plugins. */
static void listEngines() {
	printf("Engine ID       Engine Name                                           \n"
	       "--------------- ------------------------------------------------------\n");

	const PluginList &plugins = EngineMan.getPlugins();
	for (PluginList::const_iterator iter = plugins.begin(); iter != plugins.end(); ++iter) {
		const MetaEngineDetection &metaEngine = (*iter)->get<MetaEngineDetection>();
		printf("%-15s %s\n", metaEngine.getEngineId(), metaEngine.getName());
	}
}

/** List all targets which are configured in the config file. */
static void listTargets() {
	printf("Target               Description                                           \n"
	       "-------------------- ------------------------------------------------------\n");

	const Common::ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	Common::ConfigManager::DomainMap::const_iterator iter;

	Common::Array<Common::String> targets;
	targets.reserve(domains.size());

	for (iter = domains.begin(); iter != domains.end(); ++iter) {
		Common::String name(iter->_key);
		Common::String description(iter->_value.getVal("description"));

		// If there's no description, fallback on the default description.
		if (description.empty()) {
			QualifiedGameDescriptor g = EngineMan.findTarget(name);
			if (!g.description.empty())
				description = g.description;
		}
		// If there's still no description, we cannot come up with one. Insert some dummy text.
		if (description.empty())
			description = "<Unknown game>";

		targets.push_back(Common::String::format("%-20s %s", name.c_str(), description.c_str()));
	}

	Common::sort(targets.begin(), targets.end());

	for (Common::Array<Common::String>::const_iterator i = targets.begin(), end = targets.end(); i != end; ++i)
		printf("%s\n", i->c_str());
}

/** List all saves states for the given target. */
static Common::Error listSaves(const Common::String &singleTarget) {
	Common::Error result = Common::kNoError;

	// If no target is specified, list save games for all known targets
	Common::Array<Common::String> targets;
	if (!singleTarget.empty())
		targets.push_back(singleTarget);
	else {
		const Common::ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
		Common::ConfigManager::DomainMap::const_iterator iter;

		targets.reserve(domains.size());
		for (iter = domains.begin(); iter != domains.end(); ++iter)
			targets.push_back(iter->_key);
	}

	// FIXME HACK
	g_system->initBackend();

	Common::String oldDomain = ConfMan.getActiveDomainName();

	bool atLeastOneFound = false;
	for (Common::Array<Common::String>::const_iterator i = targets.begin(), end = targets.end(); i != end; ++i) {
		// Check whether there is either a game domain (i.e. a target) matching
		// the specified game name, or alternatively whether there is a matching game id.
		Common::String currentTarget;
		QualifiedGameDescriptor game;

		const Plugin *metaEnginePlugin = nullptr;
		const Plugin *enginePlugin = nullptr;

		if (ConfMan.hasGameDomain(*i)) {
			// The name is a known target
			currentTarget = *i;
			EngineMan.upgradeTargetIfNecessary(*i);
			game = EngineMan.findTarget(*i, &metaEnginePlugin);
		} else if (game = findGameMatchingName(*i), !game.gameId.empty()) {
			// The name is a known game id
			metaEnginePlugin = EngineMan.findPlugin(game.engineId);
			currentTarget = createTemporaryTarget(game.engineId, game.gameId);
		} else {
			return Common::Error(Common::kEnginePluginNotFound, Common::String::format("target '%s'", singleTarget.c_str()));
		}

		// If we actually found a domain, we're going to change the domain
		ConfMan.setActiveDomain(currentTarget);

		if (!metaEnginePlugin) {
			// If the target was specified, treat this as an error, and otherwise skip it.
			if (!singleTarget.empty())
				return Common::Error(Common::kMetaEnginePluginNotFound,
				                     Common::String::format("target '%s'", i->c_str()));
			printf("MetaEnginePlugin could not be loaded for target '%s'\n", i->c_str());
			continue;
		} else {
			enginePlugin = PluginMan.getEngineFromMetaEngine(metaEnginePlugin);

			if (!enginePlugin) {
				// If the target was specified, treat this as an error, and otherwise skip it.
				if (!singleTarget.empty())
					return Common::Error(Common::kEnginePluginNotFound,
				                     	 Common::String::format("target '%s'", i->c_str()));
				printf("EnginePlugin could not be loaded for target '%s'\n", i->c_str());
				continue;
			}
		}

		const MetaEngine &metaEngine = enginePlugin->get<MetaEngine>();
		Common::String qualifiedGameId = buildQualifiedGameName(game.engineId, game.gameId);

		if (!metaEngine.hasFeature(MetaEngine::kSupportsListSaves)) {
			// If the target was specified, treat this as an error, and otherwise skip it.
			if (!singleTarget.empty())
				// TODO: Include more info about the target (desc, engine name, ...) ???
				return Common::Error(Common::kEnginePluginNotSupportSaves,
				                     Common::String::format("target '%s', gameid '%s'", i->c_str(), qualifiedGameId.c_str()));
			continue;
		}

		// Query the plugin for a list of saved games
		SaveStateList saveList = metaEngine.listSaves(i->c_str());

		if (!saveList.empty()) {
			// TODO: Include more info about the target (desc, engine name, ...) ???
			if (atLeastOneFound)
				printf("\n");
			printf("Save states for target '%s' (gameid '%s'):\n", i->c_str(), qualifiedGameId.c_str());
			printf("  Slot Description                                           \n"
					   "  ---- ------------------------------------------------------\n");

			for (SaveStateList::const_iterator x = saveList.begin(); x != saveList.end(); ++x) {
				printf("  %-4d %s\n", x->getSaveSlot(), x->getDescription().encode().c_str());
				// TODO: Could also iterate over the full hashmap, printing all key-value pairs
			}
			atLeastOneFound = true;
		} else {
			// If the target was specified, indicate no save games were found for it. Otherwise just skip it.
			if (!singleTarget.empty())
				printf("There are no save states for target '%s' (gameid '%s'):\n", i->c_str(), qualifiedGameId.c_str());
		}
	}

	// Revert to the old active domain
	ConfMan.setActiveDomain(oldDomain);

	if (!atLeastOneFound && singleTarget.empty())
		printf("No save states could be found.\n");

	return result;
}

/** Lists all usable themes */
static void listThemes() {
	typedef Common::List<GUI::ThemeEngine::ThemeDescriptor> ThList;
	ThList thList;
	GUI::ThemeEngine::listUsableThemes(thList);

	printf("Theme          Description\n");
	printf("-------------- ------------------------------------------------\n");

	for (ThList::const_iterator i = thList.begin(); i != thList.end(); ++i)
		printf("%-14s %s\n", i->id.c_str(), i->name.c_str());
}

/** Lists all output devices */
static void listAudioDevices() {
	PluginList pluginList = MusicMan.getPlugins();

	printf("ID                             Description\n");
	printf("------------------------------ ------------------------------------------------\n");

	for (PluginList::const_iterator i = pluginList.begin(), iend = pluginList.end(); i != iend; ++i) {
		const MusicPluginObject &musicObject = (*i)->get<MusicPluginObject>();
		MusicDevices deviceList = musicObject.getDevices();
		for (MusicDevices::iterator j = deviceList.begin(), jend = deviceList.end(); j != jend; ++j) {
			printf("%-30s %s\n", Common::String::format("\"%s\"", j->getCompleteId().c_str()).c_str(), j->getCompleteName().c_str());
		}
	}
}

/** Display all games in the given directory, or current directory if empty */
static DetectedGames getGameList(const Common::FSNode &dir) {
	Common::FSList files;

	// Collect all files from directory
	if (!dir.getChildren(files, Common::FSNode::kListAll)) {
		printf("Path %s does not exist or is not a directory.\n", dir.getPath().c_str());
		return DetectedGames();
	}

	// detect Games
	DetectionResults detectionResults = EngineMan.detectGames(files);

	if (detectionResults.foundUnknownGames()) {
		Common::U32String report = detectionResults.generateUnknownGameReport(false, 80);
		g_system->logMessage(LogMessageType::kInfo, report.encode().c_str());
	}

	return detectionResults.listRecognizedGames();
}

static DetectedGames recListGames(const Common::FSNode &dir, const Common::String &engineId, const Common::String &gameId, bool recursive) {
	DetectedGames list = getGameList(dir);

	if (recursive) {
		Common::FSList files;
		dir.getChildren(files, Common::FSNode::kListDirectoriesOnly);
		for (Common::FSList::const_iterator file = files.begin(); file != files.end(); ++file) {
			DetectedGames rec = recListGames(*file, engineId, gameId, recursive);
			for (DetectedGames::const_iterator game = rec.begin(); game != rec.end(); ++game) {
				if ((game->engineId == engineId && game->gameId == gameId)
				    || gameId.empty())
					list.push_back(*game);
			}
		}
	}

	return list;
}

/** Display all games in the given directory, return ID of first detected game */
static Common::String detectGames(const Common::String &path, const Common::String &engineId, const Common::String &gameId, bool recursive) {
	bool noPath = path.empty();
	//Current directory
	Common::FSNode dir(path);
	DetectedGames candidates = recListGames(dir, engineId, gameId, recursive);

	if (candidates.empty()) {
		printf("WARNING: ScummVM could not find any game in %s\n", dir.getPath().c_str());
		if (noPath) {
			printf("WARNING: Consider using --path=<path> to specify a directory\n");
		}
		if (!recursive) {
			printf("WARNING: Consider using --recursive to search inside subdirectories\n");
		}
		return Common::String();
	}
	// TODO this is not especially pretty
	printf("GameID                         Description                                                Full Path\n");
	printf("------------------------------ ---------------------------------------------------------- ---------------------------------------------------------\n");
	for (DetectedGames::const_iterator v = candidates.begin(); v != candidates.end(); ++v) {
		printf("%-30s %-58s %s\n",
		       buildQualifiedGameName(v->engineId, v->gameId).c_str(),
		       v->description.c_str(),
		       v->path.c_str());
	}

	return buildQualifiedGameName(candidates[0].engineId, candidates[0].gameId);
}

static int recAddGames(const Common::FSNode &dir, const Common::String &engineId, const Common::String &gameId, bool recursive) {
	int count = 0;
	DetectedGames list = getGameList(dir);
	for (DetectedGames::const_iterator v = list.begin(); v != list.end(); ++v) {
		if ((v->engineId != engineId || v->gameId != gameId)
		    && !gameId.empty()) {
			printf("Found %s, only adding %s per --game option, ignoring...\n",
			       buildQualifiedGameName(v->engineId, v->gameId).c_str(),
			       buildQualifiedGameName(engineId, gameId).c_str());
		} else if (ConfMan.hasGameDomain(v->preferredTarget)) {
			// TODO Better check for game already added?
			printf("Found %s, but has already been added, skipping\n",
			       buildQualifiedGameName(v->engineId, v->gameId).c_str());
		} else {
			Common::String target = EngineMan.createTargetForGame(*v);
			count++;

			// Display added game info
			printf("Game Added: \n  Target:   %s\n  GameID:   %s\n  Name:     %s\n  Language: %s\n  Platform: %s\n",
			       target.c_str(),
			       buildQualifiedGameName(v->engineId, v->gameId).c_str(),
			       v->description.c_str(),
			       Common::getLanguageDescription(v->language),
			       Common::getPlatformDescription(v->platform)
			);
		}
	}

	if (recursive) {
		Common::FSList files;
		if (dir.getChildren(files, Common::FSNode::kListDirectoriesOnly)) {
			for (Common::FSList::const_iterator file = files.begin(); file != files.end(); ++file) {
				count += recAddGames(*file, engineId, gameId, recursive);
			}
		}
	}

	return count;
}

static bool addGames(const Common::String &path, const Common::String &engineId, const Common::String &gameId, bool recursive) {
	//Current directory
	Common::FSNode dir(path);
	int added = recAddGames(dir, engineId, gameId, recursive);
	printf("Added %d games\n", added);
	if (added == 0 && !recursive) {
		printf("Consider using --recursive to search inside subdirectories\n");
	}
	ConfMan.flushToDisk();
	return true;
}

#ifdef DETECTOR_TESTING_HACK
static void runDetectorTest() {
	// HACK: The following code can be used to test the detection code of our
	// engines. Basically, it loops over all targets, and calls the detector
	// for the given path. It then prints out the result and also checks
	// whether the result agrees with the settings of the target.

	const Common::ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	Common::ConfigManager::DomainMap::const_iterator iter = domains.begin();
	int success = 0, failure = 0;
	for (iter = domains.begin(); iter != domains.end(); ++iter) {
		Common::String name(iter->_key);
		Common::String gameid(iter->_value.getVal("gameid"));
		Common::String path(iter->_value.getVal("path"));
		printf("Looking at target '%s', gameid '%s', path '%s' ...\n",
				name.c_str(), gameid.c_str(), path.c_str());
		if (path.empty()) {
			printf(" ... no path specified, skipping\n");
			continue;
		}
		if (gameid.empty()) {
			gameid = name;
		}

		Common::FSNode dir(path);
		Common::FSList files;
		if (!dir.getChildren(files, Common::FSNode::kListAll)) {
			printf(" ... invalid path, skipping\n");
			continue;
		}

		DetectionResults detectionResults = EngineMan.detectGames(files);
		DetectedGames candidates = detectionResults.listRecognizedGames();

		bool gameidDiffers = false;
		DetectedGames::const_iterator x;
		for (x = candidates.begin(); x != candidates.end(); ++x) {
			gameidDiffers |= !gameid.equalsIgnoreCase(x->gameId);
		}

		if (candidates.empty()) {
			printf(" FAILURE: No games detected\n");
			failure++;
		} else if (candidates.size() > 1) {
			if (gameidDiffers) {
				printf(" WARNING: Multiple games detected, some/all with wrong gameid\n");
			} else {
				printf(" WARNING: Multiple games detected, but all have matching gameid\n");
			}
			failure++;
		} else if (gameidDiffers) {
			printf(" FAILURE: Wrong gameid detected\n");
			failure++;
		} else {
			printf(" SUCCESS: Game was detected correctly\n");
			success++;
		}

		for (x = candidates.begin(); x != candidates.end(); ++x) {
			printf("    gameid '%s', desc '%s', language '%s', platform '%s'\n",
				   x->gameId.c_str(),
				   x->description.c_str(),
				   Common::getLanguageDescription(x->language),
				   Common::getPlatformDescription(x->platform));
		}
	}
	int total = domains.size();
	printf("Detector test run: %d fail, %d success, %d skipped, out of %d\n",
			failure, success, total - failure - success, total);
}
#endif

#ifdef UPGRADE_ALL_TARGETS_HACK
void upgradeTargets() {
	// HACK: The following upgrades all your targets to the latest and
	// greatest. Right now that means updating the guioptions and (optionally)
	// also the game descriptions.
	// Basically, it loops over all targets, and calls the detector for the
	// given path. It then compares the result with the settings of the target.
	// If the basics seem to match, it updates the guioptions.

	printf("Upgrading all your existing targets\n");

	Common::ConfigManager::DomainMap::iterator iter = ConfMan.beginGameDomains();
	for (; iter != ConfMan.endGameDomains(); ++iter) {
		Common::ConfigManager::Domain &dom = iter->_value;
		Common::String name(iter->_key);
		Common::String gameid(dom.getVal("gameid"));
		Common::String path(dom.getVal("path"));
		printf("Looking at target '%s', gameid '%s' ...\n",
				name.c_str(), gameid.c_str());
		if (path.empty()) {
			printf(" ... no path specified, skipping\n");
			continue;
		}
		if (gameid.empty()) {
			gameid = name;
		}
		gameid.toLowercase(); // TODO: Is this paranoia? Maybe we should just assume all lowercase, always?

		Common::FSNode dir(path);
		Common::FSList files;
		if (!dir.getChildren(files, Common::FSNode::kListAll)) {
			printf(" ... invalid path, skipping\n");
			continue;
		}

		Common::Language lang = Common::parseLanguage(dom.getVal("language"));
		Common::Platform plat = Common::parsePlatform(dom.getVal("platform"));
		Common::String desc(dom.getVal("description"));

		DetectionResults detectionResults = EngineMan.detectGames(files);
		DetectedGames candidates = detectionResults.listRecognizedGames();

		DetectedGame *g = 0;

		// We proceed as follows:
		// * If detection failed to produce candidates, skip.
		// * If there is a unique detector match, trust it.
		// * If there are multiple match, run over them comparing gameid, language and platform.
		//   If we end up with a unique match, use it. Otherwise, skip.
		if (candidates.empty()) {
			printf(" ... failed to detect game, skipping\n");
			continue;
		}
		if (candidates.size() > 1) {
			// Scan over all candidates, check if there is a unique match for gameid, language and platform
			DetectedGames::iterator x;
			int matchesFound = 0;
			for (x = candidates.begin(); x != candidates.end(); ++x) {
				if (x->gameId == gameid && x->language == lang && x->platform == plat) {
					matchesFound++;
					g = &(*x);
				}
			}
			if (matchesFound != 1) {
				printf(" ... detected multiple games, could not establish unique match, skipping\n");
				continue;
			}
		} else {
			// Unique match -> use it
			g = &candidates[0];
		}

		// At this point, g points to a GameDescriptor which we can use to update
		// the target referred to by dom. We update several things

		// Always set the engine ID and game ID explicitly (in case of legacy targets)
		dom["engineid"] = g->engineId;
		dom["gameid"] = g->gameId;

		// Always set the GUI options. The user should not modify them, and engines might
		// gain more features over time, so we want to keep this list up-to-date.
		if (!g->getGUIOptions().empty()) {
			printf("  -> update guioptions to '%s'\n", g->getGUIOptions().c_str());
			dom["guioptions"] = g->getGUIOptions();
		} else if (dom.contains("guioptions")) {
			dom.erase("guioptions");
		}

		// Update the language setting but only if none has been set yet.
		if (lang == Common::UNK_LANG && g->language != Common::UNK_LANG) {
			printf("  -> set language to '%s'\n", Common::getLanguageCode(g->language));
			dom["language"] = Common::getLanguageCode(g->language);
		}

		// Update the platform setting but only if none has been set yet.
		if (plat == Common::kPlatformUnknown && g->platform != Common::kPlatformUnknown) {
			printf("  -> set platform to '%s'\n", Common::getPlatformCode(g->platform));
			dom["platform"] = Common::getPlatformCode(g->platform);
		}

		// TODO: We could also update the description. But not everybody will want that.
		// Esp. because for some games (e.g. the combined Zak/Loom FM-TOWNS demo etc.)
		// ScummVM still generates an incorrect description string. So, the description
		// should only be updated if the user explicitly requests this.
#if 0
		if (desc != g->description) {
			printf("  -> update desc from '%s' to\n                      '%s' ?\n", desc.c_str(), g->description.c_str());
			dom["description"] = g->description;
		}
#endif
	}

	// Finally, save our changes to disk
	ConfMan.flushToDisk();
}
#endif

#else // DISABLE_COMMAND_LINE


Common::String parseCommandLine(Common::StringMap &settings, int argc, const char * const *argv) {
	return Common::String();
}


#endif // DISABLE_COMMAND_LINE


bool processSettings(Common::String &command, Common::StringMap &settings, Common::Error &err) {
	err = Common::kNoError;

#ifndef DISABLE_COMMAND_LINE

	// Check the --game argument refers to a known game id.
	QualifiedGameDescriptor gameOption;
	if (settings.contains("game")) {
		gameOption = findGameMatchingName(settings["game"]);
		if (gameOption.gameId.empty()) {
			usage("Unrecognized game '%s'. Use the --list-games command for a list of accepted values.\n", settings["game"].c_str());
		}
	}

	// Handle commands passed via the command line (like --list-targets and
	// --list-games). This must be done after the config file and the plugins
	// have been loaded.
	if (command == "list-targets") {
		listTargets();
		return true;
	} else if (command == "list-games") {
		listGames();
		return true;
	} else if (command == "list-engines") {
		listEngines();
		return true;
	} else if (command == "list-saves") {
		err = listSaves(settings["game"]);
		return true;
	} else if (command == "list-themes") {
		listThemes();
		return true;
	} else if (command == "list-audio-devices") {
		listAudioDevices();
		return true;
	} else if (command == "version") {
		printf("%s\n", gScummVMFullVersion);
		printf("Features compiled in: %s\n", gScummVMFeatures);
		return true;
	} else if (command == "help") {
		printf(HELP_STRING, s_appName);
		return true;
	} else if (command == "auto-detect") {
		bool resursive = settings["recursive"] == "true";
		// If auto-detects fails (returns an empty ID) return true to close ScummVM.
		// If we get a non-empty ID, we store it in command so that it gets processed together with the
		// other command line options below.
		if (resursive) {
			printf("ERROR: Autodetection not supported with --recursive; are you sure you didn't want --detect?\n");
			err = Common::kUnknownError;
			return true;
			// There is not a particularly good technical reason for this.
			// From an UX point of view, however, it might get confusing.
			// Consider removing this if consensus says otherwise.
		} else {
			command = detectGames(settings["path"], gameOption.engineId, gameOption.gameId, resursive);
			if (command.empty()) {
				err = Common::kNoGameDataFoundError;
				return true;
			}
		}
	} else if (command == "detect") {
		detectGames(settings["path"], gameOption.engineId, gameOption.gameId, settings["recursive"] == "true");
		return true;
	} else if (command == "add") {
		addGames(settings["path"], gameOption.engineId, gameOption.gameId, settings["recursive"] == "true");
		return true;
	}
#ifdef DETECTOR_TESTING_HACK
	else if (command == "test-detector") {
		runDetectorTest();
		return true;
	}
#endif
#ifdef UPGRADE_ALL_TARGETS_HACK
	else if (command == "upgrade-targets") {
		upgradeTargets();
		return true;
	}
#endif

#endif // DISABLE_COMMAND_LINE


	// If a target was specified, check whether there is either a game
	// domain (i.e. a target) matching this argument, or alternatively
	// whether there is a gameid matching that name.
	if (!command.empty()) {
		QualifiedGameDescriptor gd;
		if (ConfMan.hasGameDomain(command)) {
			// Command is a known target
			ConfMan.setActiveDomain(command);
		} else if (gd = findGameMatchingName(command), !gd.gameId.empty()) {
			// Command is a known game ID
			Common::String domainName = createTemporaryTarget(gd.engineId, gd.gameId);
			ConfMan.setActiveDomain(domainName);
		} else {
#ifndef DISABLE_COMMAND_LINE
			usage("Unrecognized game '%s'. Use the --list-targets and --list-games commands for a list of accepted values.", command.c_str());
#endif // DISABLE_COMMAND_LINE
		}
	}


	// Finally, store the command line settings into the config manager.
	for (Common::StringMap::const_iterator x = settings.begin(); x != settings.end(); ++x) {
		Common::String key(x->_key);
		Common::String value(x->_value);

		// Replace any "-" in the key by "_" (e.g. change "save-slot" to "save_slot").
		for (Common::String::iterator c = key.begin(); c != key.end(); ++c)
			if (*c == '-')
				*c = '_';

		// Store it into ConfMan.
		ConfMan.set(key, value, Common::ConfigManager::kTransientDomain);
	}

	return false;
}

} // End of namespace Base
