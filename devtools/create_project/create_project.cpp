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

#define ENABLE_XCODE

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "config.h"
#include "create_project.h"

#include "cmake.h"
#include "codeblocks.h"
#include "msvc.h"
#include "visualstudio.h"
#include "msbuild.h"
#include "xcode.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <algorithm>
#include <iomanip>
#include <iterator>

#include <cstring>
#include <cstdlib>
#include <ctime>

#if (defined(_WIN32) || defined(WIN32)) && !defined(__GNUC__)
#define USE_WIN32_API
#endif

#if (defined(_WIN32) || defined(WIN32))
#include <windows.h>
#else
#include <sstream>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#endif

namespace {
/**
 * Converts the given path to only use slashes as
 * delimiters.
 * This means that for example the path:
 *  foo/bar\test.txt
 * will be converted to:
 *  foo/bar/test.txt
 *
 * @param path Path string.
 * @return Converted path.
 */
std::string unifyPath(const std::string &path);

/**
 * Display the help text for the program.
 *
 * @param exe Name of the executable.
 */
void displayHelp(const char *exe);

/**
 * Build a list of options to enable or disable GCC warnings
 *
 * @param globalWarnings Resulting list of warnings
 */
void addGCCWarnings(StringList &globalWarnings);
} // End of anonymous namespace

enum ProjectType {
	kProjectNone,
	kProjectCMake,
	kProjectCodeBlocks,
	kProjectMSVC,
	kProjectXcode
};

int main(int argc, char *argv[]) {
#ifndef USE_WIN32_API
	// Initialize random number generator for UUID creation
	std::srand((unsigned int)std::time(0));
#endif

	if (argc < 2) {
		displayHelp(argv[0]);
		return -1;
	}

	const std::string srcDir = argv[1];

	BuildSetup setup;
	setup.srcDir = unifyPath(srcDir);

	if (setup.srcDir.at(setup.srcDir.size() - 1) == '/')
		setup.srcDir.erase(setup.srcDir.size() - 1);

	setup.filePrefix = setup.srcDir;
	setup.outputDir = '.';

	setup.engines = parseEngines(setup.srcDir);

	if (setup.engines.empty()) {
		std::cout << "WARNING: No engines found in configure file or configure file missing in \"" << setup.srcDir << "\"\n";
		return 0;
	}

	setup.features = getAllFeatures();

	ProjectType projectType = kProjectNone;
	const MSVCVersion* msvc = NULL;
	int msvcVersion = 0;

	// Parse command line arguments
	using std::cout;
	for (int i = 2; i < argc; ++i) {
		if (!std::strcmp(argv[i], "--list-engines")) {
			cout << " The following enables are available in the " PROJECT_DESCRIPTION " source distribution\n"
			        " located at \"" << srcDir << "\":\n";

			cout << "   state  |       name      |     description\n\n";
			cout.setf(std::ios_base::left, std::ios_base::adjustfield);
			for (EngineDescList::const_iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
				cout << ' ' << (j->enable ? " enabled" : "disabled") << " | " << std::setw((std::streamsize)15) << j->name << std::setw((std::streamsize)0) << " | " << j->desc << "\n";
			cout.setf(std::ios_base::right, std::ios_base::adjustfield);

			return 0;

		} else if (!std::strcmp(argv[i], "--cmake")) {
			if (projectType != kProjectNone) {
				std::cerr << "ERROR: You cannot pass more than one project type!\n";
				return -1;
			}

			projectType = kProjectCMake;

		} else if (!std::strcmp(argv[i], "--codeblocks")) {
			if (projectType != kProjectNone) {
				std::cerr << "ERROR: You cannot pass more than one project type!\n";
				return -1;
			}

			projectType = kProjectCodeBlocks;

		} else if (!std::strcmp(argv[i], "--msvc")) {
			if (projectType != kProjectNone) {
				std::cerr << "ERROR: You cannot pass more than one project type!\n";
				return -1;
			}

			projectType = kProjectMSVC;

#ifdef ENABLE_XCODE
		} else if (!std::strcmp(argv[i], "--xcode")) {
			if (projectType != kProjectNone) {
				std::cerr << "ERROR: You cannot pass more than one project type!\n";
				return -1;
			}

			projectType = kProjectXcode;
#endif

		} else if (!std::strcmp(argv[i], "--msvc-version")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"version\" parameter for \"--msvc-version\"!\n";
				return -1;
			}

			msvcVersion = atoi(argv[++i]);

		} else if (!strncmp(argv[i], "--enable-engine=", 16)) {
			const char *names = &argv[i][16];
			if (!*names) {
				std::cerr << "ERROR: Invalid command \"" << argv[i] << "\"\n";
				return -1;
			}

			TokenList tokens = tokenize(names, ',');
			TokenList::const_iterator token = tokens.begin();
			while (token != tokens.end()) {
				std::string name = *token++;
				if (!setEngineBuildState(name, setup.engines, true)) {
					std::cerr << "ERROR: \"" << name << "\" is not a known engine!\n";
					return -1;
				}
			}
		} else if (!strncmp(argv[i], "--disable-engine=", 17)) {
			const char *names = &argv[i][17];
			if (!*names) {
				std::cerr << "ERROR: Invalid command \"" << argv[i] << "\"\n";
				return -1;
			}

			TokenList tokens = tokenize(names, ',');
			TokenList::const_iterator token = tokens.begin();
			while (token != tokens.end()) {
				std::string name = *token++;
				if (!setEngineBuildState(name, setup.engines, false)) {
					std::cerr << "ERROR: \"" << name << "\" is not a known engine!\n";
					return -1;
				}
			}
		} else if (!strncmp(argv[i], "--enable-", 9)) {
			const char *name = &argv[i][9];
			if (!*name) {
				std::cerr << "ERROR: Invalid command \"" << argv[i] << "\"\n";
				return -1;
			}

			if (!std::strcmp(name, "all-engines")) {
				for (EngineDescList::iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
					j->enable = true;
			} else if (!setFeatureBuildState(name, setup.features, true)) {
				std::cerr << "ERROR: \"" << name << "\" is not a feature!\n";
				return -1;
			}
		} else if (!strncmp(argv[i], "--disable-", 10)) {
			const char *name = &argv[i][10];
			if (!*name) {
				std::cerr << "ERROR: Invalid command \"" << argv[i] << "\"\n";
				return -1;
			}

			if (!std::strcmp(name, "all-engines")) {
				for (EngineDescList::iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
					j->enable = false;
			} else if (!setFeatureBuildState(name, setup.features, false)) {
				std::cerr << "ERROR: \"" << name << "\" is not a feature!\n";
				return -1;
			}
		} else if (!std::strcmp(argv[i], "--file-prefix")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"prefix\" parameter for \"--file-prefix\"!\n";
				return -1;
			}

			setup.filePrefix = unifyPath(argv[++i]);
			if (setup.filePrefix.at(setup.filePrefix.size() - 1) == '/')
				setup.filePrefix.erase(setup.filePrefix.size() - 1);
		} else if (!std::strcmp(argv[i], "--output-dir")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"path\" parameter for \"--output-dir\"!\n";
				return -1;
			}

			setup.outputDir = unifyPath(argv[++i]);
			if (setup.outputDir.at(setup.outputDir.size() - 1) == '/')
				setup.outputDir.erase(setup.outputDir.size() - 1);

		} else if (!std::strcmp(argv[i], "--build-events")) {
			setup.runBuildEvents = true;
		} else if (!std::strcmp(argv[i], "--installer")) {
			setup.runBuildEvents  = true;
			setup.createInstaller = true;
		} else if (!std::strcmp(argv[i], "--tools")) {
			setup.devTools = true;
		} else if (!std::strcmp(argv[i], "--tests")) {
			setup.tests = true;
		} else if (!std::strcmp(argv[i], "--sdl1")) {
			setup.useSDL2 = false;
		} else {
			std::cerr << "ERROR: Unknown parameter \"" << argv[i] << "\"\n";
			return -1;
		}
	}

	// When building tests, disable some features
	if (setup.tests) {
		setFeatureBuildState("mt32emu", setup.features, false);
		setFeatureBuildState("eventrecorder", setup.features, false);

		for (EngineDescList::iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
			j->enable = false;
	}

	// Disable engines for which we are missing dependencies
	for (EngineDescList::const_iterator i = setup.engines.begin(); i != setup.engines.end(); ++i) {
		if (i->enable) {
			for (StringList::const_iterator ef = i->requiredFeatures.begin(); ef != i->requiredFeatures.end(); ++ef) {
				FeatureList::iterator feature = std::find(setup.features.begin(), setup.features.end(), *ef);
				if (feature != setup.features.end() && !feature->enable) {
					setEngineBuildState(i->name, setup.engines, false);
					break;
				}
			}
		}
	}

	// HACK: Vorbis and Tremor can not be enabled simultaneously
	if (getFeatureBuildState("tremor", setup.features)) {
		setFeatureBuildState("vorbis", setup.features, false);
	}

	// Print status
	cout << "Enabled engines:\n\n";
	for (EngineDescList::const_iterator i = setup.engines.begin(); i != setup.engines.end(); ++i) {
		if (i->enable)
			cout << "    " << i->desc << '\n';
	}

	cout << "\nDisabled engines:\n\n";
	for (EngineDescList::const_iterator i = setup.engines.begin(); i != setup.engines.end(); ++i) {
		if (!i->enable)
			cout << "    " << i->desc << '\n';
	}

	cout << "\nEnabled features:\n\n";
	for (FeatureList::const_iterator i = setup.features.begin(); i != setup.features.end(); ++i) {
		if (i->enable)
			cout << "    " << i->description << '\n';
	}

	cout << "\nDisabled features:\n\n";
	for (FeatureList::const_iterator i = setup.features.begin(); i != setup.features.end(); ++i) {
		if (!i->enable)
			cout << "    " << i->description << '\n';
	}

	// Check if the keymapper and the event recorder are enabled simultaneously
	bool keymapperEnabled = false;
	for (FeatureList::const_iterator i = setup.features.begin(); i != setup.features.end(); ++i) {
		if (i->enable && !strcmp(i->name, "keymapper"))
			keymapperEnabled = true;
		if (i->enable && !strcmp(i->name, "eventrecorder") && keymapperEnabled) {
			std::cerr << "ERROR: The keymapper and the event recorder cannot be enabled simultaneously currently, please disable one of the two\n";
			return -1;
		}
	}

	// Check if tools and tests are enabled simultaneously
	if (setup.devTools && setup.tests) {
		std::cerr << "ERROR: The tools and tests projects cannot be created simultaneously\n";
		return -1;
	}

	// Setup defines and libraries
	setup.defines = getEngineDefines(setup.engines);
	setup.libraries = getFeatureLibraries(setup.features);

	// Add features
	StringList featureDefines = getFeatureDefines(setup.features);
	setup.defines.splice(setup.defines.begin(), featureDefines);

	bool backendWin32 = false;
	if (projectType == kProjectXcode) {
		setup.defines.push_back("POSIX");
		// Define both MACOSX, and IPHONE, but only one of them will be associated to the
		// correct target by the Xcode project provider.
		// This define will help catching up target dependend files, like "browser_osx.mm"
		// The suffix ("_osx", or "_ios") will be used by the project provider to filter out
		// the files, according to the target.
		setup.defines.push_back("MACOSX");
		setup.defines.push_back("IPHONE");
	} else if (projectType == kProjectMSVC || projectType == kProjectCodeBlocks) {
		setup.defines.push_back("WIN32");
		backendWin32 = true;
	} else {
		// As a last resort, select the backend files to build based on the platform used to build create_project.
		// This is broken when cross compiling.
#if defined(_WIN32) || defined(WIN32)
		setup.defines.push_back("WIN32");
		backendWin32 = true;
#else
		setup.defines.push_back("POSIX");
#endif
	}

	bool updatesEnabled = false, curlEnabled = false, sdlnetEnabled = false, ttsEnabled = false;
	for (FeatureList::const_iterator i = setup.features.begin(); i != setup.features.end(); ++i) {
		if (i->enable) {
			if (!strcmp(i->name, "updates"))
				updatesEnabled = true;
			else if (!strcmp(i->name, "libcurl"))
				curlEnabled = true;
			else if (!strcmp(i->name, "sdlnet"))
				sdlnetEnabled = true;
			else if (!strcmp(i->name, "tts"))
				ttsEnabled = true;
		}
	}

	if (updatesEnabled) {
		setup.defines.push_back("USE_SPARKLE");
		if (backendWin32)
			setup.libraries.push_back("winsparkle");
		else
			setup.libraries.push_back("sparkle");
	}

	if (backendWin32) {
		if (curlEnabled) {
			setup.defines.push_back("CURL_STATICLIB");
			setup.libraries.push_back("ws2_32");
			setup.libraries.push_back("wldap32");
			setup.libraries.push_back("crypt32");
			setup.libraries.push_back("normaliz");
		}
		if (sdlnetEnabled) {
			setup.libraries.push_back("iphlpapi");
		}
		setup.libraries.push_back("winmm");
	}

	if (ttsEnabled) {
		setup.libraries.push_back("sapi");
	}

	setup.defines.push_back("SDL_BACKEND");
	if (!setup.useSDL2) {
		cout << "\nBuilding against SDL 1.2\n\n";
		setup.libraries.push_back("sdl");
	} else {
		cout << "\nBuilding against SDL 2.0\n\n";
		// TODO: This also defines USE_SDL2 in the preprocessor, we don't do
		// this in our configure/make based build system. Adapt create_project
		// to replicate this behavior.
		setup.defines.push_back("USE_SDL2");
		setup.libraries.push_back("sdl2");
	}

	// Add additional project-specific library
#ifdef ADDITIONAL_LIBRARY
	setup.libraries.push_back(ADDITIONAL_LIBRARY);
#endif

	// List of global warnings and map of project-specific warnings
	// FIXME: As shown below these two structures have different behavior for
	// Code::Blocks and MSVC. In Code::Blocks this is used to enable *and*
	// disable certain warnings (and some other not warning related flags
	// actually...). While in MSVC this is solely for disabling warnings.
	// That is really not nice. We should consider a nicer way of doing this.
	StringList globalWarnings;
	std::map<std::string, StringList> projectWarnings;

	CreateProjectTool::ProjectProvider *provider = NULL;

	switch (projectType) {
	default:
	case kProjectNone:
		std::cerr << "ERROR: No project type has been specified!\n";
		return -1;

	case kProjectCMake:
		if (setup.devTools || setup.tests) {
			std::cerr << "ERROR: Building tools or tests is not supported for the CMake project type!\n";
			return -1;
		}

		addGCCWarnings(globalWarnings);

		provider = new CreateProjectTool::CMakeProvider(globalWarnings, projectWarnings);

		break;

	case kProjectCodeBlocks:
		if (setup.devTools || setup.tests) {
			std::cerr << "ERROR: Building tools or tests is not supported for the CodeBlocks project type!\n";
			return -1;
		}

		addGCCWarnings(globalWarnings);

		provider = new CreateProjectTool::CodeBlocksProvider(globalWarnings, projectWarnings);


		// Those libraries are automatically added by MSVC, but we need to add them manually with mingw
		setup.libraries.push_back("ole32");
		setup.libraries.push_back("uuid");

		break;

	case kProjectMSVC:
		// Auto-detect if no version is specified
		if (msvcVersion == 0) {
			msvcVersion = getInstalledMSVC();
			if (msvcVersion == 0) {
				std::cerr << "ERROR: No Visual Studio versions found, please specify one with \"--msvc-version\"\n";
				return -1;
			} else {
				cout << "Visual Studio " << msvcVersion << " detected\n\n";
			}
		}

		msvc = getMSVCVersion(msvcVersion);
		if (!msvc) {
			std::cerr << "ERROR: Unsupported version: \"" << msvcVersion << "\" passed to \"--msvc-version\"!\n";
			return -1;
		}

		////////////////////////////////////////////////////////////////////////////
		// For Visual Studio, all warnings are on by default in the project files,
		// so we pass a list of warnings to disable globally or per-project
		//
		////////////////////////////////////////////////////////////////////////////
		//
		// 4068 (unknown pragma)
		//   only used in scumm engine to mark code sections
		//
		// 4100 (unreferenced formal parameter)
		//
		// 4103 (alignment changed after including header, may be due to missing #pragma pack(pop))
		//   used by pack-start / pack-end
		//
		// 4127 (conditional expression is constant)
		//   used in a lot of engines
		//
		// 4244 ('conversion' conversion from 'type1' to 'type2', possible loss of data)
		//   throws tons and tons of warnings, most of them false positives
		//
		// 4250 ('class1' : inherits 'class2::member' via dominance)
		//   two or more members have the same name. Should be harmless
		//
		// 4267 ('var' : conversion from 'size_t' to 'type', possible loss of data)
		//   throws tons and tons of warnings (no immediate plan to fix all usages)
		//
		// 4310 (cast truncates constant value)
		//   used in some engines
		//
		// 4345 (behavior change: an object of POD type constructed with an
		// initializer of the form () will be default-initialized)
		//   used in Common::Array(), and it basically means that newer VS
		//   versions adhere to the standard in this case. Can be safely
		//   disabled.
		//
		// 4351 (new behavior: elements of array 'array' will be default initialized)
		//   a change in behavior in Visual Studio 2005. We want the new behavior, so it can be disabled
		//
		// 4512 ('class' : assignment operator could not be generated)
		//   some classes use const items and the default assignment operator cannot be generated
		//
		// 4577 ('noexcept' used with no exception handling mode specified)
		//
		// 4702 (unreachable code)
		//   mostly thrown after error() calls (marked as NORETURN)
		//
		// 4706 (assignment within conditional expression)
		//   used in a lot of engines
		//
		// 4800 ('type' : forcing value to bool 'true' or 'false' (performance warning))
		//
		// 4996 ('function': was declared deprecated)
		//   disabling it removes all the non-standard unsafe functions warnings (strcpy_s, etc.)
		//
		// 6211 (Leaking memory <pointer> due to an exception. Consider using a local catch block to clean up memory)
		//   we disable exceptions
		//
		// 6204 (possible buffer overrun in call to <function>: use of unchecked parameter <variable>)
		// 6385 (invalid data: accessing <buffer name>, the readable size is <size1> bytes, but <size2> bytes may be read)
		// 6386 (buffer overrun: accessing <buffer name>, the writable size is <size1> bytes, but <size2> bytes may be written)
		//   give way too many false positives
		//
		////////////////////////////////////////////////////////////////////////////
		//
		// 4189 (local variable is initialized but not referenced)
		//   false positive in lure engine
		//
		// 4355 ('this' : used in base member initializer list)
		//   only disabled for specific engines where it is used in a safe way
		//
		// 4373 (previous versions of the compiler did not override when parameters only differed by const/volatile qualifiers)
		//
		// 4510 ('class' : default constructor could not be generated)
		//
		// 4511 ('class' : copy constructor could not be generated)
		//
		// 4610 (object 'class' can never be instantiated - user-defined constructor required)
		//   "correct" but harmless (as is 4510)
		//
		////////////////////////////////////////////////////////////////////////////

		globalWarnings.push_back("4068");
		globalWarnings.push_back("4100");
		globalWarnings.push_back("4103");
		globalWarnings.push_back("4127");
		globalWarnings.push_back("4244");
		globalWarnings.push_back("4250");
		globalWarnings.push_back("4310");
		globalWarnings.push_back("4345");
		globalWarnings.push_back("4351");
		globalWarnings.push_back("4512");
		globalWarnings.push_back("4702");
		globalWarnings.push_back("4706");
		globalWarnings.push_back("4800");
		globalWarnings.push_back("4996");
		globalWarnings.push_back("6204");
		globalWarnings.push_back("6211");
		globalWarnings.push_back("6385");
		globalWarnings.push_back("6386");

		if (msvcVersion >= 14) {
			globalWarnings.push_back("4267");
			globalWarnings.push_back("4577");
		}

		projectWarnings["agi"].push_back("4510");
		projectWarnings["agi"].push_back("4610");

		projectWarnings["agos"].push_back("4511");

		projectWarnings["dreamweb"].push_back("4355");

		projectWarnings["lure"].push_back("4189");
		projectWarnings["lure"].push_back("4355");

		projectWarnings["kyra"].push_back("4355");
		projectWarnings["kyra"].push_back("4510");
		projectWarnings["kyra"].push_back("4610");

		projectWarnings["m4"].push_back("4355");

		projectWarnings["sci"].push_back("4373");

		if (msvcVersion == 9)
			provider = new CreateProjectTool::VisualStudioProvider(globalWarnings, projectWarnings, msvcVersion, *msvc);
		else
			provider = new CreateProjectTool::MSBuildProvider(globalWarnings, projectWarnings, msvcVersion, *msvc);

		break;

	case kProjectXcode:
		if (setup.devTools || setup.tests) {
			std::cerr << "ERROR: Building tools or tests is not supported for the XCode project type!\n";
			return -1;
		}

		////////////////////////////////////////////////////////////////////////////
		// Xcode is also using GCC behind the scenes. See Code::Blocks comment
		// for info on all warnings
		////////////////////////////////////////////////////////////////////////////
		globalWarnings.push_back("-Wall");
		globalWarnings.push_back("-Wno-long-long");
		globalWarnings.push_back("-Wno-multichar");
		globalWarnings.push_back("-Wno-unknown-pragmas");
		globalWarnings.push_back("-Wno-reorder");
		globalWarnings.push_back("-Wpointer-arith");
		globalWarnings.push_back("-Wcast-qual");
		globalWarnings.push_back("-Wcast-align");
		globalWarnings.push_back("-Wshadow");
		globalWarnings.push_back("-Wimplicit");
		globalWarnings.push_back("-Wnon-virtual-dtor");
		globalWarnings.push_back("-Wwrite-strings");
		// The following are not warnings at all... We should consider adding them to
		// a different list of parameters.
#if !NEEDS_RTTI
		globalWarnings.push_back("-fno-rtti");
#endif
		globalWarnings.push_back("-fno-exceptions");
		globalWarnings.push_back("-fcheck-new");

		provider = new CreateProjectTool::XcodeProvider(globalWarnings, projectWarnings);
		break;
	}

	// Setup project name and description
	setup.projectName        = PROJECT_NAME;
	setup.projectDescription = PROJECT_DESCRIPTION;

	if (setup.devTools) {
		setup.projectName        += "-tools";
		setup.projectDescription += "Tools";
	}

	if (setup.tests) {
		setup.projectName += "-tests";
		setup.projectDescription += "Tests";
	}

	provider->createProject(setup);

	delete provider;
}

namespace {
std::string unifyPath(const std::string &path) {
	std::string result = path;
	std::replace(result.begin(), result.end(), '\\', '/');
	return result;
}

void displayHelp(const char *exe) {
	using std::cout;

	cout << "Usage:\n"
	     << exe << " path\\to\\source [optional options]\n"
	     << "\n"
	     << " Creates project files for the " PROJECT_DESCRIPTION " source located at \"path\\to\\source\".\n"
	        " The project files will be created in the directory where tool is run from and\n"
	        " will include \"path\\to\\source\" for relative file paths, thus be sure that you\n"
	        " pass a relative file path like \"..\\..\\trunk\".\n"
	        "\n"
	        " Additionally there are the following switches for changing various settings:\n"
	        "\n"
	        "Project specific settings:\n"
	        " --cmake                  build CMake project files\n"
	        " --codeblocks             build Code::Blocks project files\n"
	        " --msvc                   build Visual Studio project files\n"
	        " --xcode                  build XCode project files\n"
	        " --file-prefix prefix     allow overwriting of relative file prefix in the\n"
	        "                          MSVC project files. By default the prefix is the\n"
	        "                          \"path\\to\\source\" argument\n"
	        " --output-dir path        overwrite path, where the project files are placed\n"
	        "                          By default this is \".\", i.e. the current working\n"
	        "                          directory\n"
	        "\n"
	        "MSVC specific settings:\n"
	        " --msvc-version version   set the targeted MSVC version. Possible values:\n";

	const MSVCList msvc = getAllMSVCVersions();
	for (MSVCList::const_iterator i = msvc.begin(); i != msvc.end(); ++i)
		cout << "                           " << i->version << " stands for \"" << i->name << "\"\n";

	cout << "                           If no version is set, the latest installed version is used\n"
	        " --build-events           Run custom build events as part of the build\n"
	        "                          (default: false)\n"
	        " --installer              Create installer after the build (implies --build-events)\n"
	        "                          (default: false)\n"
	        " --tools                  Create project files for the devtools\n"
	        "                          (ignores --build-events and --installer, as well as engine settings)\n"
	        "                          (default: false)\n"
	        " --tests                  Create project files for the tests\n"
	        "                          (ignores --build-events and --installer, as well as engine settings)\n"
	        "                          (default: false)\n"
	        "\n"
	        "Engines settings:\n"
	        " --list-engines           list all available engines and their default state\n"
	        " --enable-engine=<name>   enable building of the engine with the name \"name\"\n"
	        " --disable-engine=<name>  disable building of the engine with the name \"name\"\n"
	        " --enable-all-engines     enable building of all engines\n"
	        " --disable-all-engines    disable building of all engines\n"
	        "\n"
	        "Optional features settings:\n"
	        " --enable-<name>          enable inclusion of the feature \"name\"\n"
	        " --disable-<name>         disable inclusion of the feature \"name\"\n"
	        "\n"
	        "SDL settings:\n"
	        " --sdl1                   link to SDL 1.2, instead of SDL 2.0\n"
	        "\n"
	        " There are the following features available:\n"
	        "\n";

	cout << "   state  |       name      |     description\n\n";
	const FeatureList features = getAllFeatures();
	cout.setf(std::ios_base::left, std::ios_base::adjustfield);
	for (FeatureList::const_iterator i = features.begin(); i != features.end(); ++i)
		cout << ' ' << (i->enable ? " enabled" : "disabled") << " | " << std::setw((std::streamsize)15) << i->name << std::setw((std::streamsize)0) << " | " << i->description << '\n';
	cout.setf(std::ios_base::right, std::ios_base::adjustfield);
}

void addGCCWarnings(StringList &globalWarnings) {
	////////////////////////////////////////////////////////////////////////////
	//
	// -Wall
	//   enable all warnings
	//
	// -Wno-long-long -Wno-multichar -Wno-unknown-pragmas -Wno-reorder
	//   disable annoying and not-so-useful warnings
	//
	// -Wpointer-arith -Wcast-qual -Wcast-align
	// -Wshadow -Wimplicit -Wnon-virtual-dtor -Wwrite-strings
	//   enable even more warnings...
	//
	// -fno-exceptions -fcheck-new
	//   disable exceptions, and enable checking of pointers returned by "new"
	//
	////////////////////////////////////////////////////////////////////////////

	globalWarnings.push_back("-Wall");
	globalWarnings.push_back("-Wno-long-long");
	globalWarnings.push_back("-Wno-multichar");
	globalWarnings.push_back("-Wno-unknown-pragmas");
	globalWarnings.push_back("-Wno-reorder");
	globalWarnings.push_back("-Wpointer-arith");
	globalWarnings.push_back("-Wcast-qual");
	globalWarnings.push_back("-Wcast-align");
	globalWarnings.push_back("-Wshadow");
	globalWarnings.push_back("-Wnon-virtual-dtor");
	globalWarnings.push_back("-Wwrite-strings");
	// The following are not warnings at all... We should consider adding them to
	// a different list of parameters.
	globalWarnings.push_back("-fno-exceptions");
	globalWarnings.push_back("-fcheck-new");
}

/**
 * Parse the configure.engine file of a given engine directory and return a
 * list of all defined engines.
 *
 * @param engineDir The directory of the engine.
 * @return The list of all defined engines.
 */
EngineDescList parseEngineConfigure(const std::string &engineDir);

/**
 * Compares two FSNode entries in a strict-weak fashion based on the name.
 *
 * @param left  The first operand.
 * @param right The second operand.
 * @return "true" when the name of the left operand is strictly smaller than
 *         the name of the second operand. "false" otherwise.
 */
bool compareFSNode(const CreateProjectTool::FSNode &left, const CreateProjectTool::FSNode &right);

#ifdef FIRST_ENGINE
/**
 * Compares two FSNode entries in a strict-weak fashion based on engine name
 * order.
 *
 * @param left  The first operand.
 * @param right The second operand.
 * @return "true" when the name of the left operand is strictly smaller than
 *         the name of the second operand. "false" otherwise.
 */
bool compareEngineNames(const CreateProjectTool::FSNode &left, const CreateProjectTool::FSNode &right);
#endif
} // End of anonymous namespace

EngineDescList parseEngines(const std::string &srcDir) {
	using CreateProjectTool::FileList;
	using CreateProjectTool::listDirectory;

	EngineDescList engineList;

	FileList engineFiles = listDirectory(srcDir + "/engines/");

#ifdef FIRST_ENGINE
	// In case we want to sort an engine to the front of the list we will
	// use some manual sorting predicate which assures that.
	engineFiles.sort(&compareEngineNames);
#else
	// Otherwise, we simply sort the file list alphabetically this allows
	// for a nicer order in --list-engines output, for example.
	engineFiles.sort(&compareFSNode);
#endif

	for (FileList::const_iterator i = engineFiles.begin(), end = engineFiles.end(); i != end; ++i) {
		// Each engine requires its own sub directory thus we will skip all
		// non directory file nodes here.
		if (!i->isDirectory) {
			continue;
		}

		// Retrieve all engines defined in this sub directory and add them to
		// the list of all engines.
		EngineDescList list = parseEngineConfigure(srcDir + "/engines/" + i->name);
		engineList.splice(engineList.end(), list);
	}

	return engineList;
}

bool isSubEngine(const std::string &name, const EngineDescList &engines) {
	for (EngineDescList::const_iterator i = engines.begin(); i != engines.end(); ++i) {
		if (std::find(i->subEngines.begin(), i->subEngines.end(), name) != i->subEngines.end())
			return true;
	}

	return false;
}

bool setEngineBuildState(const std::string &name, EngineDescList &engines, bool enable) {
	if (enable && isSubEngine(name, engines)) {
		// When we enable a sub engine, we need to assure that the parent is also enabled,
		// thus we enable both sub engine and parent over here.
		EngineDescList::iterator engine = std::find(engines.begin(), engines.end(), name);
		if (engine != engines.end()) {
			engine->enable = enable;

			for (engine = engines.begin(); engine != engines.end(); ++engine) {
				if (std::find(engine->subEngines.begin(), engine->subEngines.end(), name) != engine->subEngines.end()) {
					engine->enable = true;
					break;
				}
			}

			return true;
		}
	} else {
		EngineDescList::iterator engine = std::find(engines.begin(), engines.end(), name);
		if (engine != engines.end()) {
			engine->enable = enable;

			// When we disable an engine, we also need to disable all the sub engines.
			if (!enable && !engine->subEngines.empty()) {
				for (StringList::const_iterator j = engine->subEngines.begin(); j != engine->subEngines.end(); ++j) {
					EngineDescList::iterator subEngine = std::find(engines.begin(), engines.end(), *j);
					if (subEngine != engines.end())
						subEngine->enable = false;
				}
			}

			return true;
		}
	}

	return false;
}

StringList getEngineDefines(const EngineDescList &engines) {
	StringList result;

	for (EngineDescList::const_iterator i = engines.begin(); i != engines.end(); ++i) {
		if (i->enable) {
			std::string define = "ENABLE_" + i->name;
			std::transform(define.begin(), define.end(), define.begin(), toupper);
			result.push_back(define);
		}
	}

	return result;
}

namespace {
/**
 * Try to parse a given line and create an engine definition
 * out of the result.
 *
 * This may take *any* input line, when the line is not used
 * to define an engine the result of the function will be "false".
 *
 * Note that the contents of "engine" are undefined, when this
 * function returns "false".
 *
 * @param line Text input line.
 * @param engine Reference to an object, where the engine information
 *               is to be stored in.
 * @return "true", when parsing succeeded, "false" otherwise.
 */
bool parseEngine(const std::string &line, EngineDesc &engine) {
	// Format:
	// add_engine engine_name "Readable Description" enable_default ["SubEngineList"] ["base games"] ["dependencies"]
	TokenList tokens = tokenize(line);

	if (tokens.size() < 4)
		return false;

	TokenList::const_iterator token = tokens.begin();

	if (*token != "add_engine")
		return false;
	++token;

	engine.name = *token; ++token;
	engine.desc = *token; ++token;
	engine.enable = (*token == "yes"); ++token;
	if (token != tokens.end()) {
		engine.subEngines = tokenize(*token);
		++token;
		if (token != tokens.end())
			++token;
		if (token != tokens.end())
			engine.requiredFeatures = tokenize(*token);
	}

	return true;
}

EngineDescList parseEngineConfigure(const std::string &engineDir) {
	std::string configureFile = engineDir + "/configure.engine";

	std::ifstream configure(configureFile.c_str());
	if (!configure)
		return EngineDescList();

	std::string line;
	EngineDescList engines;

	for (;;) {
		std::getline(configure, line);
		if (configure.eof())
			break;

		if (configure.fail())
			error("Failed while reading from " + configureFile);

		EngineDesc desc;
		if (parseEngine(line, desc))
			engines.push_back(desc);
	}

	return engines;
}

bool compareFSNode(const CreateProjectTool::FSNode &left, const CreateProjectTool::FSNode &right) {
	return left.name < right.name;
}

#ifdef FIRST_ENGINE
bool compareEngineNames(const CreateProjectTool::FSNode &left, const CreateProjectTool::FSNode &right) {
	if (left.name == FIRST_ENGINE) {
		return right.name != FIRST_ENGINE;
	} else if (right.name == FIRST_ENGINE) {
		return false;
	} else {
		return compareFSNode(left, right);
	}
}
#endif
} // End of anonymous namespace

TokenList tokenize(const std::string &input, char separator) {
	TokenList result;

	std::string::size_type sIdx = input.find_first_not_of(" \t");
	std::string::size_type nIdx = std::string::npos;

	if (sIdx == std::string::npos)
		return result;

	do {
		if (input.at(sIdx) == '\"') {
			++sIdx;
			nIdx = input.find_first_of('\"', sIdx);
		} else {
			nIdx = input.find_first_of(separator, sIdx);
		}

		if (nIdx != std::string::npos) {
			result.push_back(input.substr(sIdx, nIdx - sIdx));
			if (separator == ' ')
				sIdx = input.find_first_not_of(" \t", nIdx + 1);
			else
				sIdx = input.find_first_not_of(separator, nIdx + 1);
		} else {
			result.push_back(input.substr(sIdx));
			break;
		}
	} while (sIdx != std::string::npos);

	return result;
}

namespace {
const Feature s_features[] = {
	// Libraries
	{      "libz",        "USE_ZLIB", "zlib",             true,  "zlib (compression) support" },
	{       "mad",         "USE_MAD", "libmad",           true,  "libmad (MP3) support" },
	{       "ogg",         "USE_OGG", "libogg_static",    true,  "Ogg support" },
	{    "vorbis",      "USE_VORBIS", "libvorbisfile_static libvorbis_static", true, "Vorbis support" },
	{    "tremor",      "USE_TREMOR", "libtremor", false, "Tremor support" },
	{      "flac",        "USE_FLAC", "libFLAC_static win_utf8_io_static",   true, "FLAC support" },
	{       "png",         "USE_PNG", "libpng16",         true,  "libpng support" },
	{      "faad",        "USE_FAAD", "libfaad",          false, "AAC support" },
	{     "mpeg2",       "USE_MPEG2", "libmpeg2",         false, "MPEG-2 support" },
	{    "theora",   "USE_THEORADEC", "libtheora_static", true, "Theora decoding support" },
	{  "freetype",   "USE_FREETYPE2", "freetype",         true, "FreeType support" },
	{      "jpeg",        "USE_JPEG", "jpeg-static",      true, "libjpeg support" },
	{"fluidsynth",  "USE_FLUIDSYNTH", "libfluidsynth",    true, "FluidSynth support" },
	{   "libcurl",     "USE_LIBCURL", "libcurl",          true, "libcurl support" },
	{    "sdlnet",     "USE_SDL_NET", "SDL_net",          true, "SDL_net support" },

	// Feature flags
	{            "bink",                      "USE_BINK",  "", true,  "Bink video support" },
	{         "scalers",                   "USE_SCALERS",  "", true,  "Scalers" },
	{       "hqscalers",                "USE_HQ_SCALERS",  "", true,  "HQ scalers" },
	{           "16bit",                 "USE_RGB_COLOR",  "", true,  "16bit color support" },
	{         "highres",                   "USE_HIGHRES",  "", true,  "high resolution" },
	{         "mt32emu",                   "USE_MT32EMU",  "", true,  "integrated MT-32 emulator" },
	{             "lua",                       "USE_LUA",  "", true,  "lua" },
	{            "nasm",                      "USE_NASM",  "", true,  "IA-32 assembly support" }, // This feature is special in the regard, that it needs additional handling.
	{          "opengl",                    "USE_OPENGL",  "", true,  "OpenGL support" },
	{        "opengles",                      "USE_GLES",  "", true,  "forced OpenGL ES mode" },
	{         "taskbar",                   "USE_TASKBAR",  "", true,  "Taskbar integration support" },
	{           "cloud",                     "USE_CLOUD",  "", true,  "Cloud integration support" },
	{     "translation",               "USE_TRANSLATION",  "", true,  "Translation support" },
	{          "vkeybd",                 "ENABLE_VKEYBD",  "", false, "Virtual keyboard support"},
	{       "keymapper",              "ENABLE_KEYMAPPER",  "", false, "Keymapper support"},
	{   "eventrecorder",          "ENABLE_EVENTRECORDER",  "", false, "Event recorder support"},
	{         "updates",                   "USE_UPDATES",  "", false, "Updates support"},
	{         "dialogs",                "USE_SYSDIALOGS",  "", true,  "System dialogs support"},
	{      "langdetect",                "USE_DETECTLANG",  "", true,  "System language detection support" }, // This feature actually depends on "translation", there
	                                                                                                         // is just no current way of properly detecting this...
	{    "text-console", "USE_TEXT_CONSOLE_FOR_DEBUGGER",  "", false, "Text console debugger" }, // This feature is always applied in xcode projects
	{             "tts",                       "USE_TTS",  "", false, "Text to speech support"}
};

const Tool s_tools[] = {
	{ "create_cryo",         true},
	{ "create_drascula",     true},
	{ "create_hugo",         true},
	{ "create_kyradat",      true},
	{ "create_lure",         true},
	{ "create_neverhood",    true},
	{ "create_teenagent",    true},
	{ "create_titanic",      true},
	{ "create_tony",         true},
	{ "create_toon",         true},
	{ "create_translations", true},
	{ "qtable",              true}
};

const MSVCVersion s_msvc[] = {
//    Ver    Name                     Solution                     Project    Toolset    LLVM
	{  9,    "Visual Studio 2008",    "10.00",          "2008",     "4.0",     "v90",    "LLVM-vs2008" },
	{ 10,    "Visual Studio 2010",    "11.00",          "2010",     "4.0",    "v100",    "LLVM-vs2010" },
	{ 11,    "Visual Studio 2012",    "11.00",          "2012",     "4.0",    "v110",    "LLVM-vs2012" },
	{ 12,    "Visual Studio 2013",    "12.00",          "2013",    "12.0",    "v120",    "LLVM-vs2013" },
	{ 14,    "Visual Studio 2015",    "12.00",            "14",    "14.0",    "v140",    "LLVM-vs2014" },
	{ 15,    "Visual Studio 2017",    "12.00",            "15",    "15.0",    "v141",    "llvm"        },
	{ 16,    "Visual Studio 2019",    "12.00",    "Version 16",    "16.0",    "v142",    "llvm"        }
};
} // End of anonymous namespace

FeatureList getAllFeatures() {
	const size_t featureCount = sizeof(s_features) / sizeof(s_features[0]);

	FeatureList features;
	for (size_t i = 0; i < featureCount; ++i)
		features.push_back(s_features[i]);

	return features;
}

StringList getFeatureDefines(const FeatureList &features) {
	StringList defines;

	for (FeatureList::const_iterator i = features.begin(); i != features.end(); ++i) {
		if (i->enable && i->define && i->define[0])
			defines.push_back(i->define);
	}

	return defines;
}

StringList getFeatureLibraries(const FeatureList &features) {
	StringList libraries;

	for (FeatureList::const_iterator i = features.begin(); i != features.end(); ++i) {
		if (i->enable && i->libraries && i->libraries[0]) {
			StringList fLibraries = tokenize(i->libraries);
			libraries.splice(libraries.end(), fLibraries);
		}
	}

	return libraries;
}

bool setFeatureBuildState(const std::string &name, FeatureList &features, bool enable) {
	FeatureList::iterator i = std::find(features.begin(), features.end(), name);
	if (i != features.end()) {
		i->enable = enable;
		return true;
	} else {
		return false;
	}
}

bool getFeatureBuildState(const std::string &name, FeatureList &features) {
	FeatureList::iterator i = std::find(features.begin(), features.end(), name);
	if (i != features.end()) {
		return i->enable;
	} else {
		return false;
	}
}

ToolList getAllTools() {
	const size_t toolCount = sizeof(s_tools) / sizeof(s_tools[0]);

	ToolList tools;
	for (size_t i = 0; i < toolCount; ++i)
		tools.push_back(s_tools[i]);

	return tools;
}

MSVCList getAllMSVCVersions() {
	const size_t msvcCount = sizeof(s_msvc) / sizeof(s_msvc[0]);

	MSVCList msvcVersions;
	for (size_t i = 0; i < msvcCount; ++i)
		msvcVersions.push_back(s_msvc[i]);

	return msvcVersions;
}

const MSVCVersion *getMSVCVersion(int version) {
	const size_t msvcCount = sizeof(s_msvc) / sizeof(s_msvc[0]);

	for (size_t i = 0; i < msvcCount; ++i) {
		if (s_msvc[i].version == version)
			return &s_msvc[i];
	}

	return NULL;
}

int getInstalledMSVC() {
	int latest = 0;
#if defined(_WIN32) || defined(WIN32)
	// Use the Visual Studio Installer to get the latest version
	const char *vsWhere = "\"\"%PROGRAMFILES(X86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe\" -latest -legacy -property installationVersion\"";
	FILE *pipe = _popen(vsWhere, "rt");
	if (pipe != NULL) {
		char version[50];
		if (fgets(version, 50, pipe) != NULL) {
			latest = atoi(version);
		}
		_pclose(pipe);
	}

	// Use the registry to get the latest version
	if (latest == 0) {
		HKEY key;
		LSTATUS err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7", 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &key);
		if (err == ERROR_SUCCESS && key != NULL) {
			const MSVCList msvc = getAllMSVCVersions();
			for (MSVCList::const_reverse_iterator i = msvc.rbegin(); i != msvc.rend(); ++i) {
				std::ostringstream version;
				version << i->version << ".0";
				err = RegQueryValueEx(key, version.str().c_str(), NULL, NULL, NULL, NULL);
				if (err == ERROR_SUCCESS) {
					latest = i->version;
					break;
				}
			}
			RegCloseKey(key);
		}
	}
#endif
	return latest;
}

namespace CreateProjectTool {

//////////////////////////////////////////////////////////////////////////
// Utilities
//////////////////////////////////////////////////////////////////////////

std::string convertPathToWin(const std::string &path) {
	std::string result = path;
	std::replace(result.begin(), result.end(), '/', '\\');
	return result;
}

std::string getIndent(const int indentation) {
	std::string result;
	for (int i = 0; i < indentation; ++i)
		result += '\t';
	return result;
}

void splitFilename(const std::string &fileName, std::string &name, std::string &ext) {
	const std::string::size_type dot = fileName.find_last_of('.');
	name = (dot == std::string::npos) ? fileName : fileName.substr(0, dot);
	ext = (dot == std::string::npos) ? std::string() : fileName.substr(dot + 1);
}

std::string basename(const std::string &fileName) {
	const std::string::size_type slash = fileName.find_last_of('/');
	if (slash == std::string::npos) return fileName;
	return fileName.substr(slash + 1);
}

bool producesObjectFile(const std::string &fileName) {
	std::string n, ext;
	splitFilename(fileName, n, ext);

	if (ext == "cpp" || ext == "c" || ext == "asm" || ext == "m" || ext == "mm")
		return true;
	else
		return false;
}

std::string toString(int num) {
	std::ostringstream os;
	os << num;
	return os.str();
}

/**
 * Checks whether the give file in the specified directory is present in the given
 * file list.
 *
 * This function does as special match against the file list. Object files (.o) are
 * excluded by default and it will not take file extensions into consideration,
 * when the extension of a file in the specified directory is one of "h", "cpp",
 * "c" or "asm".
 *
 * @param dir Parent directory of the file.
 * @param fileName File name to match.
 * @param fileList List of files to match against.
 * @return "true" when the file is in the list, "false" otherwise.
 */
bool isInList(const std::string &dir, const std::string &fileName, const StringList &fileList) {
	std::string compareName, extensionName;
	splitFilename(fileName, compareName, extensionName);

	if (!extensionName.empty())
		compareName += '.';

	for (StringList::const_iterator i = fileList.begin(); i != fileList.end(); ++i) {
		if (i->compare(0, dir.size(), dir))
			continue;

		// When no comparison name is given, we try to match whether a subset of
		// the given directory should be included. To do that we must assure that
		// the first character after the substring, having the same size as dir, must
		// be a path delimiter.
		if (compareName.empty()) {
			if (i->size() >= dir.size() + 1 && i->at(dir.size()) == '/')
				return true;
			else
				continue;
		}

		const std::string lastPathComponent = ProjectProvider::getLastPathComponent(*i);
		if (extensionName == "o") {
			return false;
		} else if (!producesObjectFile(fileName) && extensionName != "h") {
			if (fileName == lastPathComponent)
				return true;
		} else {
			if (!lastPathComponent.compare(0, compareName.size(), compareName))
				return true;
		}
	}

	return false;
}

/**
 * A strict weak compare predicate for sorting a list of
 * "FileNode *" entries.
 *
 * It will sort directory nodes before file nodes.
 *
 * @param l Left-hand operand.
 * @param r Right-hand operand.
 * @return "true" if and only if l should be sorted before r.
 */
bool compareNodes(const FileNode *l, const FileNode *r) {
	if (!l) {
		return false;
	} else if (!r) {
		return true;
	} else {
		if (l->children.empty() && !r->children.empty()) {
			return false;
		} else if (!l->children.empty() && r->children.empty()) {
			return true;
		} else {
			return l->name < r->name;
		}
	}
}

FileList listDirectory(const std::string &dir) {
	FileList result;
#if defined(_WIN32) || defined(WIN32)
	WIN32_FIND_DATA fileInformation;
	HANDLE fileHandle = FindFirstFile((dir + "/*").c_str(), &fileInformation);

	if (fileHandle == INVALID_HANDLE_VALUE)
		return result;

	do {
		if (fileInformation.cFileName[0] == '.')
			continue;

		result.push_back(FSNode(fileInformation.cFileName, (fileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0));
	} while (FindNextFile(fileHandle, &fileInformation) == TRUE);

	FindClose(fileHandle);
#else
	DIR *dirp = opendir(dir.c_str());
	struct dirent *dp = NULL;

	if (dirp == NULL)
		return result;

	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_name[0] == '.')
			continue;

		struct stat st;
		if (stat((dir + '/' + dp->d_name).c_str(), &st))
			continue;

		result.push_back(FSNode(dp->d_name, S_ISDIR(st.st_mode)));
	}

	closedir(dirp);
#endif
	return result;
}

void createDirectory(const std::string &dir) {
#if defined(_WIN32) || defined(WIN32)
	if (!CreateDirectory(dir.c_str(), NULL)) {
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			error("Could not create folder \"" + dir + "\"");
		}
	}
#else
	if (mkdir(dir.c_str(), 0777) == -1) {
		if (errno == EEXIST) {
			// Try to open as a folder (might be a file / symbolic link)
			DIR *dirp = opendir(dir.c_str());
			if (dirp == NULL) {
				error("Could not create folder \"" + dir + "\"");
			} else {
				// The folder exists, just close the stream and return
				closedir(dirp);
			}
		} else {
			error("Could not create folder \"" + dir + "\"");
		}
	}
#endif

}

/**
 * Scans the specified directory against files, which should be included
 * in the project files. It will not include files present in the exclude list.
 *
 * @param dir Directory in which to search for files.
 * @param includeList Files to include in the project.
 * @param excludeList Files to exclude from the project.
 * @return Returns a file node for the specific directory.
 */
FileNode *scanFiles(const std::string &dir, const StringList &includeList, const StringList &excludeList) {
	FileList files = listDirectory(dir);

	if (files.empty())
		return 0;

	FileNode *result = new FileNode(dir);
	assert(result);

	for (FileList::const_iterator i = files.begin(); i != files.end(); ++i) {
		if (i->isDirectory) {
			const std::string subDirName = dir + '/' + i->name;
			if (!isInList(subDirName, std::string(), includeList))
				continue;

			FileNode *subDir = scanFiles(subDirName, includeList, excludeList);

			if (subDir) {
				subDir->name = i->name;
				result->children.push_back(subDir);
			}
			continue;
		}

		if (isInList(dir, i->name, excludeList))
			continue;

		std::string name, ext;
		splitFilename(i->name, name, ext);

		if (ext != "h") {
			if (!isInList(dir, i->name, includeList))
				continue;
		}

		FileNode *child = new FileNode(i->name);
		assert(child);
		result->children.push_back(child);
	}

	if (result->children.empty()) {
		delete result;
		return 0;
	} else {
		result->children.sort(compareNodes);
		return result;
	}
}

//////////////////////////////////////////////////////////////////////////
// Project Provider methods
//////////////////////////////////////////////////////////////////////////
ProjectProvider::ProjectProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: _version(version), _globalWarnings(global_warnings), _projectWarnings(project_warnings) {
}

void ProjectProvider::createProject(BuildSetup &setup) {
	std::string targetFolder;

	if (setup.devTools) {
		_uuidMap = createToolsUUIDMap();
		targetFolder = "/devtools/";
	} else if (!setup.tests) {
		_uuidMap = createUUIDMap(setup);
		targetFolder = "/engines/";
	}

	// We also need to add the UUID of the main project file.
	const std::string svmUUID = _uuidMap[setup.projectName] = createUUID(setup.projectName);

	createWorkspace(setup);

	StringList in, ex;

	// Create project files
	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == setup.projectName)
			continue;
		// Retain the files between engines if we're creating a single project
		in.clear(); ex.clear();

		const std::string moduleDir = setup.srcDir + targetFolder + i->first;

		createModuleList(moduleDir, setup.defines, setup.testDirs, in, ex);
		createProjectFile(i->first, i->second, setup, moduleDir, in, ex);
	}

	if (setup.tests) {
		// Create the main project file.
		in.clear(); ex.clear();
		createModuleList(setup.srcDir + "/backends", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/backends/platform/sdl", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/base", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/common", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/engines", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/graphics", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/gui", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/audio", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/test", setup.defines, setup.testDirs, in, ex);

		createProjectFile(setup.projectName, svmUUID, setup, setup.srcDir, in, ex);
	} else if (!setup.devTools) {
		// Last but not least create the main project file.
		in.clear(); ex.clear();
		// File list for the Project file
		createModuleList(setup.srcDir + "/backends", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/backends/platform/sdl", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/base", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/common", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/engines", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/graphics", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/gui", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/audio", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/audio/softsynth/mt32", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/video", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/image", setup.defines, setup.testDirs, in, ex);

		// Resource files
		addResourceFiles(setup, in, ex);

		// Various text files
		in.push_back(setup.srcDir + "/AUTHORS");
		in.push_back(setup.srcDir + "/COPYING");
		in.push_back(setup.srcDir + "/COPYING.LGPL");
		in.push_back(setup.srcDir + "/COPYING.BSD");
		in.push_back(setup.srcDir + "/COPYING.FREEFONT");
		in.push_back(setup.srcDir + "/COPYING.OFL");
		in.push_back(setup.srcDir + "/COPYRIGHT");
		in.push_back(setup.srcDir + "/NEWS");
		in.push_back(setup.srcDir + "/README");
		in.push_back(setup.srcDir + "/TODO");

		// Create the main project file.
		createProjectFile(setup.projectName, svmUUID, setup, setup.srcDir, in, ex);
	}

	// Create other misc. build files
	createOtherBuildFiles(setup);

	// In case we create the main ScummVM project files we will need to
	// generate engines/plugins_table.h too.
	if (!setup.tests && !setup.devTools) {
		createEnginePluginsTable(setup);
	}
}

ProjectProvider::UUIDMap ProjectProvider::createUUIDMap(const BuildSetup &setup) const {
	UUIDMap result;

	for (EngineDescList::const_iterator i = setup.engines.begin(); i != setup.engines.end(); ++i) {
		if (!i->enable || isSubEngine(i->name, setup.engines))
			continue;

		result[i->name] = createUUID(i->name);
	}

	return result;
}

ProjectProvider::UUIDMap ProjectProvider::createToolsUUIDMap() const {
	UUIDMap result;

	ToolList tools = getAllTools();
	for (ToolList::const_iterator i = tools.begin(); i != tools.end(); ++i) {
		if (!i->enable)
			continue;

		result[i->name] = createUUID(i->name);
	}

	return result;
}

const int kUUIDLen = 16;

std::string ProjectProvider::createUUID() const {
#ifdef USE_WIN32_API
	UUID uuid;
	RPC_STATUS status = UuidCreateSequential(&uuid);
	if (status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY)
		error("UuidCreateSequential failed");

	unsigned char *string = 0;
	if (UuidToStringA(&uuid, &string) != RPC_S_OK)
		error("UuidToStringA failed");

	std::string result = std::string((char *)string);
	std::transform(result.begin(), result.end(), result.begin(), toupper);
	RpcStringFreeA(&string);
	return result;
#else
	unsigned char uuid[kUUIDLen];

	for (int i = 0; i < kUUIDLen; ++i)
		uuid[i] = (unsigned char)((std::rand() / (double)(RAND_MAX)) * 0xFF);

	uuid[8] &= 0xBF; uuid[8] |= 0x80;
	uuid[6] &= 0x4F; uuid[6] |= 0x40;

	return UUIDToString(uuid);
#endif
}

std::string ProjectProvider::createUUID(const std::string &name) const {
#ifdef USE_WIN32_API
	HCRYPTPROV hProv = NULL;
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		error("CryptAcquireContext failed");
	}
	
	// Use MD5 hashing algorithm
	HCRYPTHASH hHash = NULL;
	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
		CryptReleaseContext(hProv, 0);
		error("CryptCreateHash failed");
	}

	// Hash unique ScummVM namespace {5f5b43e8-35ff-4f1e-ad7e-a2a87e9b5254}
	const BYTE uuidNs[kUUIDLen] =
		{ 0x5f, 0x5b, 0x43, 0xe8, 0x35, 0xff, 0x4f, 0x1e, 0xad, 0x7e, 0xa2, 0xa8, 0x7e, 0x9b, 0x52, 0x54 };
	if (!CryptHashData(hHash, uuidNs, kUUIDLen, 0)) {
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		error("CryptHashData failed");
	}

	// Hash project name
	if (!CryptHashData(hHash, (const BYTE *)name.c_str(), name.length(), 0)) {
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		error("CryptHashData failed");
	}

	// Get resulting UUID
	BYTE uuid[kUUIDLen];
	DWORD len = kUUIDLen;
	if (!CryptGetHashParam(hHash, HP_HASHVAL, uuid, &len, 0)) {
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		error("CryptGetHashParam failed");
	}

	// Add version and variant
	uuid[6] &= 0x0F; uuid[6] |= 0x30;
	uuid[8] &= 0x3F; uuid[8] |= 0x80;

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);

	return UUIDToString(uuid);
#else
	// Fallback to random UUID
	return createUUID();
#endif
}

std::string ProjectProvider::UUIDToString(unsigned char *uuid) const {
	std::stringstream uuidString;
	uuidString << std::hex << std::uppercase << std::setfill('0');
	for (int i = 0; i < kUUIDLen; ++i) {
		uuidString << std::setw(2) << (int)uuid[i];
		if (i == 3 || i == 5 || i == 7 || i == 9) {
			uuidString << std::setw(0) << '-';
		}
	}
	return uuidString.str();
}

std::string ProjectProvider::getLastPathComponent(const std::string &path) {
	std::string::size_type pos = path.find_last_of('/');
	if (pos == std::string::npos)
		return path;
	else
		return path.substr(pos + 1);
}

void ProjectProvider::addFilesToProject(const std::string &dir, std::ofstream &projectFile,
                                        const StringList &includeList, const StringList &excludeList,
                                        const std::string &filePrefix) {
	// Check for duplicate object file names
	StringList duplicate;

	for (StringList::const_iterator i = includeList.begin(); i != includeList.end(); ++i) {
		std::string fileName = getLastPathComponent(*i);
		std::transform(fileName.begin(), fileName.end(), fileName.begin(), tolower);

		// Leave out non object file names.
		if (fileName.size() < 2 || fileName.compare(fileName.size() - 2, 2, ".o"))
			continue;

		// Check whether an duplicate has been found yet
		if (std::find(duplicate.begin(), duplicate.end(), fileName) != duplicate.end())
			continue;

		// Search for duplicates
		StringList::const_iterator j = i; ++j;
		for (; j != includeList.end(); ++j) {
			std::string candidateFileName = getLastPathComponent(*j);
			std::transform(candidateFileName.begin(), candidateFileName.end(), candidateFileName.begin(), tolower);
			if (fileName == candidateFileName) {
				duplicate.push_back(fileName);
				break;
			}
		}
	}

	FileNode *files = scanFiles(dir, includeList, excludeList);

	writeFileListToProject(*files, projectFile, 0, duplicate, std::string(), filePrefix + '/');

	delete files;
}

void ProjectProvider::createModuleList(const std::string &moduleDir, const StringList &defines, StringList &testDirs, StringList &includeList, StringList &excludeList) const {
	const std::string moduleMkFile = moduleDir + "/module.mk";
	std::ifstream moduleMk(moduleMkFile.c_str());
	if (!moduleMk)
		error(moduleMkFile + " is not present");

	includeList.push_back(moduleMkFile);

	std::stack<bool> shouldInclude;
	shouldInclude.push(true);

	StringList filesInVariableList;

	bool hadModule = false;
	std::string line;
	for (;;) {
		std::getline(moduleMk, line);

		if (moduleMk.eof())
			break;

		if (moduleMk.fail())
			error("Failed while reading from " + moduleMkFile);

		TokenList tokens = tokenize(line);
		if (tokens.empty())
			continue;

		TokenList::const_iterator i = tokens.begin();
		if (*i == "MODULE") {
			if (hadModule)
				error("More than one MODULE definition in " + moduleMkFile);
			// Format: "MODULE := path/to/module"
			if (tokens.size() < 3)
				error("Malformed MODULE definition in " + moduleMkFile);
			++i;
			if (*i != ":=")
				error("Malformed MODULE definition in " + moduleMkFile);
			++i;

			std::string moduleRoot = unifyPath(*i);
			if (moduleDir.compare(moduleDir.size() - moduleRoot.size(), moduleRoot.size(), moduleRoot))
				error("MODULE root " + moduleRoot + " does not match base dir " + moduleDir);

			hadModule = true;
		} else if (*i == "MODULE_OBJS") {
			if (tokens.size() < 3)
				error("Malformed MODULE_OBJS definition in " + moduleMkFile);
			++i;

			// This is not exactly correct, for example an ":=" would usually overwrite
			// all already added files, but since we do only save the files inside
			// includeList or excludeList currently, we couldn't handle such a case easily.
			// (includeList and excludeList should always preserve their entries, not added
			// by this function, thus we can't just clear them on ":=" or "=").
			// But hopefully our module.mk files will never do such things anyway.
			if (*i != ":=" && *i != "+=" && *i != "=")
				error("Malformed MODULE_OBJS definition in " + moduleMkFile);

			++i;

			while (i != tokens.end()) {
				if (*i == "\\") {
					std::getline(moduleMk, line);
					tokens = tokenize(line);
					i = tokens.begin();
				} else if (*i == "$(KYRARPG_COMMON_OBJ)") {
					// HACK to fix EOB/LOL compilation in the kyra engine:
					// replace the variable name with the stored files.
					// This assumes that the file list has already been defined.
					if (filesInVariableList.size() == 0)
						error("$(KYRARPG_COMMON_OBJ) found, but the variable hasn't been set before it");
					// Construct file list and replace the variable
					for (StringList::iterator j = filesInVariableList.begin(); j != filesInVariableList.end(); ++j) {
						const std::string filename = *j;

						if (shouldInclude.top()) {
							// In case we should include a file, we need to make
							// sure it is not in the exclude list already. If it
							// is we just drop it from the exclude list.
							excludeList.remove(filename);

							includeList.push_back(filename);
						} else if (std::find(includeList.begin(), includeList.end(), filename) == includeList.end()) {
							// We only add the file to the exclude list in case it
							// has not yet been added to the include list.
							excludeList.push_back(filename);
						}
					}
					++i;
				} else {
					const std::string filename = moduleDir + "/" + unifyPath(*i);

					if (shouldInclude.top()) {
						// In case we should include a file, we need to make
						// sure it is not in the exclude list already. If it
						// is we just drop it from the exclude list.
						excludeList.remove(filename);

						includeList.push_back(filename);
					} else if (std::find(includeList.begin(), includeList.end(), filename) == includeList.end()) {
						// We only add the file to the exclude list in case it
						// has not yet been added to the include list.
						excludeList.push_back(filename);
					}
					++i;
				}
			}
		} else if (*i == "KYRARPG_COMMON_OBJ") {
			// HACK to fix EOB/LOL compilation in the kyra engine: add the
			// files defined in the KYRARPG_COMMON_OBJ variable in a list
			if (tokens.size() < 3)
				error("Malformed KYRARPG_COMMON_OBJ definition in " + moduleMkFile);
			++i;

			if (*i != ":=" && *i != "+=" && *i != "=")
				error("Malformed KYRARPG_COMMON_OBJ definition in " + moduleMkFile);

			++i;

			while (i != tokens.end()) {
				if (*i == "\\") {
					std::getline(moduleMk, line);
					tokens = tokenize(line);
					i = tokens.begin();
				} else {
					const std::string filename = moduleDir + "/" + unifyPath(*i);
					filesInVariableList.push_back(filename);
					++i;
				}
			}
		} else if (*i == "TESTS") {
			if (tokens.size() < 3)
				error("Malformed TESTS definition in " + moduleMkFile);
			++i;

			if (*i != ":=" && *i != "+=" && *i != "=")
				error("Malformed TESTS definition in " + moduleMkFile);
			++i;

			while (i != tokens.end()) {
				// Read input
				std::string folder = unifyPath(*i);

				// Get include folder
				const std::string source_dir = "$(srcdir)/";
				const std::string selector = getLastPathComponent(folder);
				const std::string module = getLastPathComponent(moduleDir);

				folder.replace(folder.find(source_dir), source_dir.length(), "");
				folder.replace(folder.find(selector), selector.length(), "");
				folder.replace(folder.find(module), module.length(), moduleDir);

				// Scan all files in the include folder
				FileList files = listDirectory(folder);

				if (files.empty())
					continue;

				// Add to list of test folders
				testDirs.push_back(folder);

				for (FileList::const_iterator f = files.begin(); f != files.end(); ++f) {
					if (f->isDirectory)
						continue;

					std::string filename = folder + f->name;

					if (shouldInclude.top()) {
						// In case we should include a file, we need to make
						// sure it is not in the exclude list already. If it
						// is we just drop it from the exclude list.
						excludeList.remove(filename);

						includeList.push_back(filename);
					} else if (std::find(includeList.begin(), includeList.end(), filename) == includeList.end()) {
						// We only add the file to the exclude list in case it
						// has not yet been added to the include list.
						excludeList.push_back(filename);
					}
				}

				++i;
			}
		} else if (*i == "ifdef") {
			if (tokens.size() < 2)
				error("Malformed ifdef in " + moduleMkFile);
			++i;

			if (std::find(defines.begin(), defines.end(), *i) == defines.end())
				shouldInclude.push(false);
			else
				shouldInclude.push(true && shouldInclude.top());
		} else if (*i == "ifndef") {
			if (tokens.size() < 2)
				error("Malformed ifndef in " + moduleMkFile);
			++i;

			if (std::find(defines.begin(), defines.end(), *i) == defines.end())
				shouldInclude.push(true && shouldInclude.top());
			else
				shouldInclude.push(false);
		} else if (*i == "else") {
			bool last = shouldInclude.top();
			shouldInclude.pop();
			shouldInclude.push(!last && shouldInclude.top());
		} else if (*i == "endif") {
			if (shouldInclude.size() <= 1)
				error("endif without ifdef found in " + moduleMkFile);
			shouldInclude.pop();
		} else if (*i == "elif") {
			error("Unsupported operation 'elif' in " + moduleMkFile);
		} else if (*i == "ifeq") {
			//XXX
			shouldInclude.push(false);
		}
	}

	if (shouldInclude.size() != 1)
		error("Malformed file " + moduleMkFile);
}

void ProjectProvider::createEnginePluginsTable(const BuildSetup &setup) {
	// First we need to create the "engines" directory.
	createDirectory(setup.outputDir + "/engines");

	// Then, we can generate the actual "plugins_table.h" file.
	const std::string enginePluginsTableFile = setup.outputDir + "/engines/plugins_table.h";
	std::ofstream enginePluginsTable(enginePluginsTableFile.c_str());
	if (!enginePluginsTable) {
		error("Could not open \"" + enginePluginsTableFile + "\" for writing");
	}

	enginePluginsTable << "/* This file is automatically generated by create_project */\n"
	                   << "/* DO NOT EDIT MANUALLY */\n"
	                   << "// This file is being included by \"base/plugins.cpp\"\n";

	for (EngineDescList::const_iterator i = setup.engines.begin(), end = setup.engines.end(); i != end; ++i) {
		// We ignore all sub engines here because they require no special
		// handling.
		if (isSubEngine(i->name, setup.engines)) {
			continue;
		}

		// Make the engine name all uppercase.
		std::string engineName;
		std::transform(i->name.begin(), i->name.end(), std::back_inserter(engineName), toupper);

		enginePluginsTable << "#if PLUGIN_ENABLED_STATIC(" << engineName << ")\n"
		                   << "LINK_PLUGIN(" << engineName << ")\n"
		                   << "#endif\n";
	}
}
} // End of anonymous namespace

void error(const std::string &message) {
	std::cerr << "ERROR: " << message << "!" << std::endl;
	std::exit(-1);
}
