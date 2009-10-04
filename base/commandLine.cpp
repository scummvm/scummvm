/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/metaengine.h"
#include "base/commandLine.h"
#include "base/plugins.h"
#include "base/version.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/fs.h"

#include "gui/ThemeEngine.h"

namespace Base {

#ifndef DISABLE_COMMAND_LINE

static const char USAGE_STRING[] =
	"%s: %s\n"
	"Usage: %s [OPTIONS]... [GAME]\n"
	"\n"
	"Try '%s --help' for more options.\n"
;

// DONT FIXME: DO NOT ORDER ALPHABETICALLY, THIS IS ORDERED BY IMPORTANCE/CATEGORY! :)
#if defined(PALMOS_MODE) || defined(__SYMBIAN32__) || defined(__GP32__)
static const char HELP_STRING[] = "NoUsageString"; // save more data segment space
#else
static const char HELP_STRING[] =
	"Residual - Virtual machine to run 3D adventure games\n"
	"Usage: %s [OPTIONS]... [GAME]\n"
	"  -v, --version            Display Residual version information and exit\n"
	"  -h, --help               Display a brief help text and exit\n"
	"  -z, --list-games         Display list of supported games and exit\n"
	"  -t, --list-targets       Display list of configured targets and exit\n"

	"\n"
	"  -c, --config=CONFIG      Use alternate configuration file\n"
	"  -p, --path=PATH          Path to where the game is installed\n"
	"  -f, --fullscreen         Force full-screen mode\n"
	"  -q, --language=LANG      Select language (en,de,fr,it,pt,es,jp,zh,kr,se,gb,\n"
	"                           hb,ru,cz)\n"
	"  --gui-theme=THEME        Select GUI theme\n"
	"  --themepath=PATH         Path to where GUI themes are stored\n"
	"  --list-themes            Display list of all usable GUI themes\n"
	"  -m, --music-volume=NUM   Set the music volume, 0-127 (default: 127)\n"
	"  -s, --sfx-volume=NUM     Set the sfx volume, 0-127 (default: 127)\n"
	"  -r, --speech-volume=NUM  Set the speech volume, 0-127 (default: 127)\n"
	"  --speech-mode=NUM        Set the mode of speech 1-Text only, 2-Speech Only, 3-Speech and Text\n"
	"  --soft-renderer=BOOL     Set the turn on/off software 3D renderer: true/false\n"
	"  -d, --debuglevel=NUM     Set debug verbosity level\n"
	"  --debugflags=FLAGS       Enables engine specific debug flags\n"
	"  --savepath=PATH          Path to where savegames are stored\n"
	"  --extrapath=PATH         Extra path to additional game data\n"
	"  --output-rate=RATE       Select output sample rate in Hz (e.g. 22050)\n"
	"  --show-fps=BOOL          Set the turn on/off display FPS info: true/false\n"
	"\n"
;
#endif

static const char *s_appName = "residual";

static void usage(const char *s, ...) GCC_PRINTF(1, 2);

static void usage(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

#if !(defined(__GP32__) || defined (__SYMBIAN32__))
	printf(USAGE_STRING, s_appName, buf, s_appName, s_appName);
#endif
	exit(1);
}

#endif // DISABLE_COMMAND_LINE


void registerDefaults() {
	ConfMan.registerDefault("platform", Common::kPlatformPC);
	ConfMan.registerDefault("language", "en");
	ConfMan.registerDefault("autosave_period", 5 * 60);	// By default, trigger autosave every 5 minutes

	ConfMan.registerDefault("music_volume", 127);
	ConfMan.registerDefault("sfx_volume", 127);
	ConfMan.registerDefault("speech_volume", 127);
	ConfMan.registerDefault("speech_mode", "3");

	ConfMan.registerDefault("path", ".");

	ConfMan.registerDefault("soft_renderer", "true");
	ConfMan.registerDefault("fullscreen", "false");
	ConfMan.registerDefault("show_fps", "false");

	ConfMan.registerDefault("disable_sdl_parachute", false);

	ConfMan.registerDefault("record_mode", "none");
	ConfMan.registerDefault("record_file_name", "record.bin");
	ConfMan.registerDefault("record_temp_file_name", "record.tmp");
	ConfMan.registerDefault("record_time_file_name", "record.time");
}

//
// Various macros used by the command line parser.
//

#ifndef DISABLE_COMMAND_LINE

// Use this for options which have an *optional* value
#define DO_OPTION_OPT(shortCmd, longCmd, defaultVal) \
	if (isLongCmd ? (!strcmp(s+2, longCmd) || !memcmp(s+2, longCmd"=", sizeof(longCmd"=") - 1)) : (tolower(s[1]) == shortCmd)) { \
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
#define DO_OPTION_INT(shortCmd, longCmd) \
	DO_OPTION(shortCmd, longCmd) \
	char *endptr = 0; \
	int intValue; intValue = (int)strtol(option, &endptr, 0); \
	if (endptr == NULL || *endptr != 0) usage("--%s: Invalid number '%s'", longCmd, option);

// Use this for boolean options; this distinguishes between "-x" and "-X",
// resp. between "--some-option" and "--no-some-option".
#define DO_OPTION_BOOL(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s+2, longCmd) || !strcmp(s+2, "no-"longCmd)) : (tolower(s[1]) == shortCmd)) { \
		bool boolValue = (islower(s[1]) != 0); \
		s += 2; \
		if (isLongCmd) { \
			boolValue = !strcmp(s, longCmd); \
			s += boolValue ? (sizeof(longCmd) - 1) : (sizeof("no-"longCmd) - 1); \
		} \
		if (*s != '\0') goto unknownOption; \
		const char *option = boolValue ? "true" : "false"; \
		settings[longCmd] = option;

// Use this for options which never have a value, i.e. for 'commands', like "--help".
#define DO_COMMAND(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s+2, longCmd)) : (tolower(s[1]) == shortCmd)) { \
		s += 2; \
		if (isLongCmd) \
			s += sizeof(longCmd) - 1; \
		if (*s != '\0') goto unknownOption; \
		return longCmd;


#define DO_LONG_OPTION_OPT(longCmd, d)  DO_OPTION_OPT(0, longCmd, d)
#define DO_LONG_OPTION(longCmd)         DO_OPTION(0, longCmd)
#define DO_LONG_OPTION_INT(longCmd)     DO_OPTION_INT(0, longCmd)
#define DO_LONG_OPTION_BOOL(longCmd)    DO_OPTION_BOOL(0, longCmd)
#define DO_LONG_COMMAND(longCmd)        DO_COMMAND(0, longCmd)

// End an option handler
#define END_OPTION \
		continue; \
	}


Common::String parseCommandLine(Common::StringMap &settings, int argc, const char * const *argv) {
	const char *s, *s2;

	// argv[0] contains the name of the executable.
	if (argv && argv[0]) {
		s = strrchr(argv[0], '/');
		s_appName = s ? (s+1) : argv[0];
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

			bool isLongCmd = (s[0] == '-' && s[1] == '-');

			DO_COMMAND('h', "help")
			END_OPTION

			DO_COMMAND('v', "version")
			END_OPTION

			DO_COMMAND('t', "list-targets")
			END_OPTION

			DO_COMMAND('z', "list-games")
			END_OPTION

			DO_OPTION('c', "config")
			END_OPTION

			DO_OPTION_OPT('d', "debuglevel", "0")
			END_OPTION

			DO_LONG_OPTION("debugflags")
			END_OPTION

			DO_LONG_OPTION_INT("output-rate")
			END_OPTION

			DO_OPTION_BOOL('f', "fullscreen")
			END_OPTION

			DO_OPTION_INT('m', "music-volume")
			END_OPTION

			DO_OPTION('s', "sfx-volume")
			END_OPTION

			DO_OPTION('r', "speech-volume")
			END_OPTION

			DO_OPTION('q', "language")
				if (Common::parseLanguage(option) == Common::UNK_LANG)
					usage("Unrecognized language '%s'", option);
			END_OPTION

			DO_LONG_OPTION_OPT("joystick", "0")
				settings["joystick_num"] = option;
				settings.erase("joystick");
			END_OPTION

			DO_LONG_OPTION("show-fps")
			END_OPTION

			DO_LONG_OPTION("soft-renderer")
			END_OPTION

			DO_LONG_OPTION("engine-speed")
			END_OPTION

			DO_LONG_OPTION("gamma")
			END_OPTION

			DO_OPTION('p', "path")
				Common::FSNode path(option);
				if (!path.exists()) {
					usage("Non-existent game path '%s'", option);
				} else if (!path.isReadable()) {
					usage("Non-readable game path '%s'", option);
				}
			END_OPTION

			DO_LONG_OPTION_BOOL("disable-sdl-parachute")
			END_OPTION

			DO_LONG_OPTION("savepath")
				Common::FSNode path(option);
				if (!path.exists()) {
					usage("Non-existent savegames path '%s'", option);
				} else if (!path.isWritable()) {
					usage("Non-writable savegames path '%s'", option);
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

			DO_LONG_OPTION("gui-theme")
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
			END_OPTION

			DO_LONG_OPTION("text-speed")
			END_OPTION

			DO_LONG_OPTION("speech-mode")
			END_OPTION

			DO_LONG_OPTION("record-mode")
			END_OPTION

			DO_LONG_OPTION("record-file-name")
			END_OPTION

			DO_LONG_OPTION("record-temp-file-name")
			END_OPTION

			DO_LONG_OPTION("record-time-file-name")
			END_OPTION

#ifdef IPHONE
			// This is automatically set when launched from the Springboard.
			DO_LONG_OPTION_OPT("launchedFromSB", 0)
			END_OPTION
#endif

unknownOption:
			// If we get till here, the option is unhandled and hence unknown.
			usage("Unrecognized option '%s'", argv[i]);
		}
	}

	return Common::String();
}

/** List all supported game IDs, i.e. all games which any loaded plugin supports. */
static void listGames() {
	printf("Game ID              Full Title                                            \n"
	       "-------------------- ------------------------------------------------------\n");

	const EnginePlugin::List &plugins = EngineMan.getPlugins();
	EnginePlugin::List::const_iterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		GameList list = (**iter)->getSupportedGames();
		for (GameList::iterator v = list.begin(); v != list.end(); ++v) {
			printf("%-20s %s\n", v->gameid().c_str(), v->description().c_str());
		}
	}
}

/** List all targets which are configured in the config file. */
static void listTargets() {
	printf("Target               Description                                           \n"
	       "-------------------- ------------------------------------------------------\n");

	using namespace Common;
	const ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	ConfigManager::DomainMap::const_iterator iter;
	for (iter = domains.begin(); iter != domains.end(); ++iter) {
		Common::String name(iter->_key);
		Common::String description(iter->_value.get("description"));

		if (description.empty()) {
			// FIXME: At this point, we should check for a "gameid" override
			// to find the proper desc. In fact, the platform probably should
			// be taken into account, too.
			Common::String gameid(name);
			GameDescriptor g = EngineMan.findGame(gameid);
			if (g.description().size() > 0)
				description = g.description();
		}

		printf("%-20s %s\n", name.c_str(), description.c_str());

	}
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

#else // DISABLE_COMMAND_LINE


Common::String parseCommandLine(Common::StringMap &settings, int argc, const char * const *argv) {
	return Common::String();
}


#endif // DISABLE_COMMAND_LINE


bool processSettings(Common::String &command, Common::StringMap &settings) {

#ifndef DISABLE_COMMAND_LINE

	// Handle commands passed via the command line (like --list-targets and
	// --list-games). This must be done after the config file and the plugins
	// have been loaded.
	if (command == "list-targets") {
		listTargets();
		return false;
	} else if (command == "list-games") {
		listGames();
		return false;
	} else if (command == "list-themes") {
		listThemes();
		return false;
	} else if (command == "version") {
		printf("%s\n", gResidualFullVersion);
		printf("Features compiled in: %s\n", gResidualFeatures);
		return false;
	} else if (command == "help") {
		printf(HELP_STRING, s_appName);
		return false;
	}

#endif // DISABLE_COMMAND_LINE


	// If a target was specified, check whether there is either a game
	// domain (i.e. a target) matching this argument, or alternatively
	// whether there is a gameid matching that name.
	if (!command.empty()) {
		GameDescriptor gd = EngineMan.findGame(command);
		if (ConfMan.hasGameDomain(command) || !gd.gameid().empty()) {
			bool idCameFromCommandLine = false;

			// WORKAROUND: Fix for bug #1719463: "DETECTOR: Launching
			// undefined target adds launcher entry"
			//
			// We designate gameids which come strictly from command line
			// so AdvancedDetector will not save config file with invalid
			// gameid in case target autoupgrade was performed
			if (!ConfMan.hasGameDomain(command)) {
				idCameFromCommandLine = true;
			}

			ConfMan.setActiveDomain(command);

			if (idCameFromCommandLine)
				ConfMan.set("id_came_from_command_line", "1");

		} else {
#ifndef DISABLE_COMMAND_LINE
			usage("Unrecognized game target '%s'", command.c_str());
#endif // DISABLE_COMMAND_LINE
		}
	}


	// The user can override the savepath with the SCUMMVM_SAVEPATH
	// environment variable. This is weaker than a --savepath on the
	// command line, but overrides the default savepath, hence it is
	// handled here, just before the command line gets parsed.
#if !defined(MACOS_CARBON) && !defined(_WIN32_WCE) && !defined(PALMOS_MODE) && !defined(__GP32__)
	if (!settings.contains("savepath")) {
		const char *dir = getenv("RESIDUAL_SAVEPATH");
		if (dir && *dir && strlen(dir) < MAXPATHLEN) {
			Common::FSNode saveDir(dir);
			if (!saveDir.exists()) {
				warning("Non-existent RESIDUAL_SAVEPATH save path. It will be ignored.");
			} else if (!saveDir.isWritable()) {
				warning("Non-writable RESIDUAL_SAVEPATH save path. It will be ignored.");
			} else {
				settings["savepath"] = dir;
			}
		}
	}
#endif

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

	return true;
}

} // End of namespace Base
