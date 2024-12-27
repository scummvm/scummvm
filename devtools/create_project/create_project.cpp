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

#define ENABLE_XCODE

#if (defined(_WIN32) || defined(WIN32)) && !defined(__GNUC__)
#define USE_WIN32_API
#endif

#if (defined(_WIN32) || defined(WIN32))
#define _WIN32_WINNT 0x0502
#include <windows.h>
#else
#include <dirent.h>
#include <errno.h>
#include <sstream>
#include <sys/param.h>
#include <sys/stat.h>
#endif

#include "create_project.h"
#include "config.h"

#include "cmake.h"
#include "codeblocks.h"
#include "msbuild.h"
#include "msvc.h"
#include "xcode.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stack>
#include <utility>

#include <cstdlib>
#include <cstring>
#include <ctime>

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
 * Removes trailing slash from path if it exists
 *
 * @param path Path string.
 */
void removeTrailingSlash(std::string& path);

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

std::map<std::string, bool> isEngineEnabled;

int main(int argc, char *argv[]) {
#ifndef USE_WIN32_API
	// Initialize random number generator for UUID creation
	std::srand((unsigned int)std::time(nullptr));
#endif

	if (argc < 2) {
		displayHelp(argv[0]);
		return -1;
	}

	const std::string srcDir = argv[1];

	BuildSetup setup;
	setup.srcDir = unifyPath(srcDir);
	removeTrailingSlash(setup.srcDir);

	setup.filePrefix = setup.srcDir;
	setup.outputDir = '.';

	setup.engines = parseEngines(setup.srcDir);

	if (setup.engines.empty()) {
		std::cout << "WARNING: No engines found in configure file or configure file missing in \"" << setup.srcDir << "\"\n";
		return 0;
	}

	setup.features = getAllFeatures();
	setup.components = getAllComponents(setup.srcDir, setup.features);

	ProjectType projectType = kProjectNone;
	const MSVCVersion *msvc = nullptr;
	int msvcVersion = 0;

	// Parse command line arguments
	using std::cout;
	for (int i = 2; i < argc; ++i) {
		if (!std::strcmp(argv[i], "--list-engines")) {
			cout << " The following enables are available in the " PROJECT_DESCRIPTION " source distribution\n"
			     << " located at \"" << srcDir << "\":\n";

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
			removeTrailingSlash(setup.filePrefix);
		} else if (!std::strcmp(argv[i], "--output-dir")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"path\" parameter for \"--output-dir\"!\n";
				return -1;
			}

			setup.outputDir = unifyPath(argv[++i]);
			removeTrailingSlash(setup.outputDir);
		} else if (!std::strcmp(argv[i], "--include-dir")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"path\" parameter for \"--include-dir\"!\n";
				return -1;
			}
			std::string includeDir = unifyPath(argv[++i]);
			removeTrailingSlash(includeDir);
			setup.includeDirs.push_back(includeDir);
		} else if (!std::strcmp(argv[i], "--library-dir")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"path\" parameter for \"--library-dir\"!\n";
				return -1;
			}
			std::string libraryDir = unifyPath(argv[++i]);
			removeTrailingSlash(libraryDir);
			setup.libraryDirs.push_back(libraryDir);
		} else if (!std::strcmp(argv[i], "--build-events")) {
			setup.runBuildEvents = true;
		} else if (!std::strcmp(argv[i], "--installer")) {
			setup.runBuildEvents = true;
			setup.createInstaller = true;
		} else if (!std::strcmp(argv[i], "--tools")) {
			setup.devTools = true;
		} else if (!std::strcmp(argv[i], "--tests")) {
			setup.tests = true;
		} else if (!std::strcmp(argv[i], "--sdl1")) {
			setup.useSDL2 = false;
		} else if (!std::strcmp(argv[i], "--use-canonical-lib-names")) {
			// Deprecated: Kept here so it doesn't error
		} else if (!std::strcmp(argv[i], "--use-windows-unicode")) {
			setup.useWindowsUnicode = true;
		} else if (!std::strcmp(argv[i], "--use-windows-ansi")) {
			setup.useWindowsUnicode = false;
		} else if (!std::strcmp(argv[i], "--use-windows-subsystem")) {
			setup.useWindowsSubsystem = true;
		} else if (!std::strcmp(argv[i], "--use-xcframework")) {
			setup.useXCFramework = true;
		} else if (!std::strcmp(argv[i], "--vcpkg")) {
			setup.useVcpkg = true;
		} else if (!std::strcmp(argv[i], "--libs-path")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"path\" parameter for \"--libs-path\"!\n";
				return -1;
			}
			std::string libsDir = unifyPath(argv[++i]);
			removeTrailingSlash(libsDir);
			setup.libsDir = libsDir;
		} else if (!std::strcmp(argv[i], "--list-components")) {
			for (ComponentList::const_iterator j = setup.components.begin(); j != setup.components.end(); ++j)
				cout << ' ' << j->description << "\n";

			return 0;
		} else {
			std::cerr << "ERROR: Unknown parameter \"" << argv[i] << "\"\n";
			return -1;
		}
	}

	// When building tests, disable some features
	if (setup.tests) {
		setup.useStaticDetection = false;
		setFeatureBuildState("mt32emu", setup.features, false);
		setFeatureBuildState("eventrecorder", setup.features, false);

		for (EngineDescList::iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
			j->enable = false;
	} else if (setup.devTools) {
		setup.useStaticDetection = false;
	}

	if (!getFeatureBuildState("detection-static", setup.features)) {
		setup.useStaticDetection = false;
	}

	// HACK: Vorbis and Tremor can not be enabled simultaneously
	if (getFeatureBuildState("tremor", setup.features)) {
		setFeatureBuildState("vorbis", setup.features, false);
	}

	// HACK: Fluidsynth and Fluidlite can not be enabled simultaneously
	if (getFeatureBuildState("fluidlite", setup.features)) {
		setFeatureBuildState("fluidsynth", setup.features, false);
	}

	// HACK: OpenMPT and Mikmod can not be enabled simultaneously
	if (getFeatureBuildState("openmpt", setup.features)) {
		setFeatureBuildState("mikmod", setup.features, false);
	}

	// HACK: These features depend on OpenGL
	if (!getFeatureBuildState("opengl", setup.features)) {
		setFeatureBuildState("opengl_game_classic", setup.features, false);
		setFeatureBuildState("opengl_game_shaders", setup.features, false);
	}

	// HACK: Check IMGUI dependencies
	if (!getFeatureBuildState("opengl", setup.features) ||
		!getFeatureBuildState("freetype2", setup.features) ||
		!setup.useSDL2) {
		std::cerr << "WARNING: imgui requires opengl, freetype2 and sdl2\n";
		setFeatureBuildState("imgui", setup.features, false);
	}
	// HACK: IMGUI is not available on Xcode
#ifdef ENABLE_XCODE
	if (projectType == kProjectXcode) {
		setFeatureBuildState("imgui", setup.features, false);
	}
#endif

	// Calculate 3D feature state
	setFeatureBuildState("3d", setup.features,
			getFeatureBuildState("tinygl", setup.features) ||
			getFeatureBuildState("opengl_game_classic", setup.features) ||
			getFeatureBuildState("opengl_game_shaders", setup.features));

	// Disable engines for which we are missing dependencies and mark components as needed
	for (EngineDescList::const_iterator i = setup.engines.begin(); i != setup.engines.end(); ++i) {
		if (!i->enable) {
			continue;
		}

		bool enabled = true;
		std::list<FeatureList::iterator> missingFeatures;
		for (StringList::const_iterator ef = i->requiredFeatures.begin(); ef != i->requiredFeatures.end(); ++ef) {
			FeatureList::iterator feature = std::find(setup.features.begin(), setup.features.end(), *ef);
			if (feature == setup.features.end()) {
				std::cerr << "ERROR: Missing feature " << *ef << " from engine " << i->name << '\n';
				return -1;
			} else if (!feature->enable) {
				enabled = false;
				missingFeatures.push_back(feature);
			}
		}
		isEngineEnabled[i->name] = enabled;
		if (!enabled) {
			setEngineBuildState(i->name, setup.engines, false);
			std::cout << "WARNING: Disabling engine " << i->desc << " because the following dependencies are unmet:";
			for (std::list<FeatureList::iterator>::iterator itr = missingFeatures.begin(); itr != missingFeatures.end(); itr++) {
				std::cout << " " << (*itr)->description;
			}
			std::cout << "\n";
			continue;
		}
		// Mark components as needed now the engine is definitely enabled
		for (StringList::const_iterator ef = i->requiredFeatures.begin(); ef != i->requiredFeatures.end(); ++ef) {
			ComponentList::iterator component = std::find(setup.components.begin(), setup.components.end(), *ef);
			if (component == setup.components.end()) {
				continue;
			}
			component->needed = true;
		}
		for (StringList::const_iterator ef = i->wishedComponents.begin(); ef != i->wishedComponents.end(); ++ef) {
			ComponentList::iterator component = std::find(setup.components.begin(), setup.components.end(), *ef);
			if (component == setup.components.end()) {
				std::cerr << "ERROR: Missing component " << *ef << " from engine " << i->name << '\n';
				return -1;
			}
			component->needed = true;
		}
	}

	// Disable unused features / components
	disableComponents(setup.components);

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

	// Check if tools and tests are enabled simultaneously
	if (setup.devTools && setup.tests) {
		std::cerr << "ERROR: The tools and tests projects cannot be created simultaneously\n";
		return -1;
	}

	// Setup defines and libraries
	setup.defines = getEngineDefines(setup.engines);

	// Add features
	StringList featureDefines = getFeatureDefines(setup.features);
	setup.defines.splice(setup.defines.begin(), featureDefines);

	if (projectType == kProjectXcode) {
		setup.defines.push_back("POSIX");
		// Define both MACOSX, and IPHONE, but only one of them will be associated to the
		// correct target by the Xcode project provider.
		// This define will help catching up target-dependent files, like "browser_osx.mm"
		// The suffix ("_osx", or "_ios") will be used by the project provider to filter out
		// the files, according to the target.
		setup.defines.push_back("MACOSX");
		setup.defines.push_back("IPHONE");
		setup.defines.push_back("SCUMMVM_NEON");
	} else if (projectType == kProjectMSVC || projectType == kProjectCodeBlocks) {
		setup.defines.push_back("WIN32");
		setup.win32 = true;
	} else {
		// As a last resort, select the backend files to build based on the platform used to build create_project.
		// This is broken when cross compiling.
#if defined(_WIN32) || defined(WIN32)
		setup.defines.push_back("WIN32");
		setup.win32 = true;
#else
		setup.defines.push_back("POSIX");
#endif
	}

	for (FeatureList::const_iterator i = setup.features.begin(); i != setup.features.end(); ++i) {
		if (i->enable) {
			if (!strcmp(i->name, "updates"))
				setup.defines.push_back("USE_SPARKLE");
			else if (setup.win32 && !strcmp(i->name, "libcurl"))
				setup.defines.push_back("CURL_STATICLIB");
			else if (!strcmp(i->name, "fluidlite"))
				setup.defines.push_back("USE_FLUIDSYNTH");
		}
	}

	setup.defines.push_back("SDL_BACKEND");
	if (!setup.useSDL2) {
		cout << "\nBuilding against SDL 1.2\n\n";
	} else {
		cout << "\nBuilding against SDL 2.0\n\n";
		setup.defines.push_back("USE_SDL2");
	}

	if (setup.useStaticDetection) {
		setup.defines.push_back("DETECTION_STATIC");
	}

	if (getFeatureBuildState("opengl", setup.features)) {
		setup.defines.push_back("USE_GLAD");
	}

	// HACK: Add IMGUI SDL Renderer support
	// This needs SDL 2.0.18+
	if (getFeatureBuildState("imgui", setup.features)) {
		setup.defines.push_back("USE_IMGUI_SDLRENDERER2");
	}

	// List of global warnings and map of project-specific warnings
	// FIXME: As shown below these two structures have different behavior for
	// Code::Blocks and MSVC. In Code::Blocks this is used to enable *and*
	// disable certain warnings (and some other not warning related flags
	// actually...). While in MSVC this is solely for disabling warnings.
	// That is really not nice. We should consider a nicer way of doing this.
	StringList globalWarnings;
	StringList globalErrors;
	std::map<std::string, StringList> projectWarnings;

	CreateProjectTool::ProjectProvider *provider = nullptr;

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

		provider = new CreateProjectTool::CMakeProvider(globalWarnings, projectWarnings, globalErrors);

		break;

	case kProjectCodeBlocks:
		if (setup.devTools || setup.tests) {
			std::cerr << "ERROR: Building tools or tests is not supported for the CodeBlocks project type!\n";
			return -1;
		}

		addGCCWarnings(globalWarnings);

		provider = new CreateProjectTool::CodeBlocksProvider(globalWarnings, projectWarnings, globalErrors);

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
		// 4589 (Constructor of abstract class 'type' ignores initializer for virtual base class 'type')
		//   caused by Common::Stream virtual inheritance, should be harmless
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
		// 4324 (structure was padded due to alignment specifier)
		//
		////////////////////////////////////////////////////////////////////////////

		globalWarnings.push_back("4068");
		globalWarnings.push_back("4100");
		globalWarnings.push_back("4103");
		globalWarnings.push_back("4127");
		globalWarnings.push_back("4244");
		globalWarnings.push_back("4250");
		globalWarnings.push_back("4310");
		globalWarnings.push_back("4324");
		globalWarnings.push_back("4345");
		globalWarnings.push_back("4351");
		globalWarnings.push_back("4512");
		globalWarnings.push_back("4589");
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

		globalErrors.push_back("4701"); // potential use of uninitialized local variable
		globalErrors.push_back("4703"); // potential use of uninitialized local pointer
		globalErrors.push_back("4456"); // declaration hides previous local declaration
		globalErrors.push_back("4003"); // not enough arguments for function-like macro invocation
		globalErrors.push_back("4840"); // use of non-trivial class as an argument to a variadic function
		globalErrors.push_back("4805"); // comparison of bool to non-bool, unsafe mix of bool and int in arithmetic or bitwise operation
		globalErrors.push_back("4305"); // truncation of double to float or int to bool
		globalErrors.push_back("4366"); // address taken of unaligned field
		globalErrors.push_back("4315"); // unaligned field has constructor that expects to be aligned

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

		projectWarnings["grim"].push_back("4611");

		provider = new CreateProjectTool::MSBuildProvider(globalWarnings, projectWarnings, globalErrors, msvcVersion, *msvc);

		break;

	case kProjectXcode:
		if (setup.devTools || setup.tests) {
			std::cerr << "ERROR: Building tools or tests is not supported for the XCode project type!\n";
			return -1;
		}

		addGCCWarnings(globalWarnings);

		provider = new CreateProjectTool::XcodeProvider(globalWarnings, projectWarnings, globalErrors);
		break;
	}

	// Setup project name and description
	setup.projectName = PROJECT_NAME;
	setup.projectDescription = PROJECT_DESCRIPTION;

	if (setup.devTools) {
		setup.projectName += "-tools";
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

void removeTrailingSlash(std::string& path) {
	if (path.size() > 0 && path.at(path.size() - 1) == '/')
		path.erase(path.size() - 1);
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
	        " --cmake                    build CMake project files\n"
	        " --codeblocks               build Code::Blocks project files\n"
	        " --msvc                     build Visual Studio project files\n"
	        " --xcode                    build XCode project files\n"
	        " --file-prefix prefix       allow overwriting of relative file prefix in the\n"
	        "                            MSVC project files. By default the prefix is the\n"
	        "                            \"path\\to\\source\" argument\n"
	        " --output-dir path          overwrite path, where the project files are placed\n"
	        "                            By default this is \".\", i.e. the current working\n"
	        "                            directory\n"
			" --include-dir path         add a path to the include search path\n"
			" --library-dir path         add a path to the library search path\n"
	        "\n"
	        "MSVC specific settings:\n"
	        " --msvc-version version     set the targeted MSVC version. Possible values:\n";

	const MSVCList msvc = getAllMSVCVersions();
	for (MSVCList::const_iterator i = msvc.begin(); i != msvc.end(); ++i)
		cout << "                           " << i->version << " stands for \"" << i->name << "\"\n";

	cout << "                            If no version is set, the latest installed version is used\n"
	        " --build-events             Run custom build events as part of the build\n"
	        "                            (default: false)\n"
	        " --installer                Create installer after the build (implies --build-events)\n"
	        "                            (default: false)\n"
	        " --tools                    Create project files for the devtools\n"
	        "                            (ignores --build-events and --installer, as well as engine settings)\n"
	        "                            (default: false)\n"
	        " --tests                    Create project files for the tests\n"
	        "                            (ignores --build-events and --installer, as well as engine settings)\n"
	        "                            (default: false)\n"
	        " --use-windows-unicode      Use Windows Unicode APIs\n"
	        "                            (default: true)\n"
	        " --use-windows-ansi         Use Windows ANSI APIs\n"
	        "                            (default: false)\n"
	        " --use-windows-subsystem    Use Windows subsystem instead of Console\n"
	        " --libs-path path           Specify the path of pre-built libraries instead of using the\n"
			"                            " LIBS_DEFINE " environment variable\n "
	        " --vcpkg                    Use vcpkg-provided libraries instead of pre-built libraries\n"
	        "                            (default: false)\n"
	        "\n"
	        "Engines settings:\n"
	        " --list-engines             list all available engines and their default state\n"
	        " --enable-engine=<name>     enable building of the engine with the name \"name\"\n"
	        " --disable-engine=<name>    disable building of the engine with the name \"name\"\n"
	        " --enable-all-engines       enable building of all engines\n"
	        " --disable-all-engines      disable building of all engines\n"
	        "\n"
	        "Optional features settings:\n"
	        " --enable-<name>            enable inclusion of the feature \"name\"\n"
	        " --disable-<name>           disable inclusion of the feature \"name\"\n"
	        "\n"
	        "SDL settings:\n"
	        " --sdl1                     link to SDL 1.2, instead of SDL 2.0\n"
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
#if !NEEDS_RTTI
	globalWarnings.push_back("-fno-rtti");
#endif
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
		if (i->enable)
			result.push_back("ENABLE_" + CreateProjectTool::toUpper(i->name));
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
	// add_engine engine_name "Readable Description" enable_default ["SubEngineList"] ["base games"] ["dependencies"] ["components"]
	TokenList tokens = tokenize(line);

	if (tokens.size() < 4)
		return false;

	TokenList::const_iterator token = tokens.begin();

	if (*token != "add_engine")
		return false;
	++token;

	engine.name = *token;
	++token;
	engine.desc = *token;
	++token;
	engine.enable = (*token == "yes");
	++token;
	if (token != tokens.end()) {
		engine.subEngines = tokenize(*token);
		++token;
		if (token != tokens.end())
			++token;
		if (token != tokens.end()) {
			engine.requiredFeatures = tokenize(*token);
			++token;
		}
		if (token != tokens.end())
			engine.wishedComponents = tokenize(*token);
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
// clang-format off
const Feature s_features[] = {
	// Libraries (must be added in generators)
	{      "zlib",        "USE_ZLIB", true, true,  "zlib (compression) support" },
	{       "mad",         "USE_MAD", true, true,  "libmad (MP3) support" },
	{   "fribidi",     "USE_FRIBIDI", true, true,  "BiDi support" },
	{       "ogg",         "USE_OGG", true, true,  "Ogg support" },
	{    "vorbis",      "USE_VORBIS", true, true,  "Vorbis support" },
	{    "tremor",      "USE_TREMOR", true, false, "Tremor support" },
	{      "flac",        "USE_FLAC", true, true,  "FLAC support" },
	{       "png",         "USE_PNG", true, true,  "libpng support" },
	{       "gif",         "USE_GIF", true, false, "libgif support" },
	{      "faad",        "USE_FAAD", true, false, "AAC support" },
	{    "mikmod",      "USE_MIKMOD", true, false, "libmikmod support" },
	{   "openmpt",     "USE_OPENMPT", true, false, "libopenmpt support" },
	{     "mpeg2",       "USE_MPEG2", true, true,  "MPEG-2 support" },
	{ "theoradec",   "USE_THEORADEC", true, true,  "Theora decoding support" },
	{       "vpx",         "USE_VPX", true, false, "VP8/VP9 decoding support" },
	{ "freetype2",   "USE_FREETYPE2", true, true,  "FreeType support" },
	{      "jpeg",        "USE_JPEG", true, true,  "libjpeg support" },
	{"fluidsynth",  "USE_FLUIDSYNTH", true, true,  "FluidSynth support" },
	{ "fluidlite",   "USE_FLUIDLITE", true, false, "FluidLite support" },
	{   "libcurl",     "USE_LIBCURL", true, true,  "libcurl support" },
	{    "sdlnet",     "USE_SDL_NET", true, true,  "SDL_net support" },
	{   "discord",     "USE_DISCORD", true, false, "Discord support" },
	{ "retrowave",   "USE_RETROWAVE", true, false, "RetroWave OPL3 support" },
	{       "a52",         "USE_A52", true, false, "ATSC A/52 support" },
	{       "mpc",      "USE_MPCDEC", true, false, "Musepack support" },

	// Feature flags
	{               "bink",                      "USE_BINK", false, true,  "Bink video support" },
	{            "scalers",                   "USE_SCALERS", false, true,  "Scalers" },
	{          "hqscalers",                "USE_HQ_SCALERS", false, true,  "HQ scalers" },
	{        "edgescalers",              "USE_EDGE_SCALERS", false, true,  "Edge scalers" },
	{             "aspect",                    "USE_ASPECT", false, true,  "Aspect ratio correction" },
	{              "16bit",                 "USE_RGB_COLOR", false, true,  "16bit color support" },
	{                 "3d",                              "", false, true,  "3D rendering" },
	{            "highres",                   "USE_HIGHRES", false, true,  "high resolution" },
	{               "nasm",                      "USE_NASM", false, true,  "IA-32 assembly support" }, // This feature is special in the regard, that it needs additional handling.
	{             "opengl",                    "USE_OPENGL", false, true,  "OpenGL support" },
	{"opengl_game_classic",               "USE_OPENGL_GAME", false, true,  "OpenGL support (classic) in 3d games" },
	{"opengl_game_shaders",            "USE_OPENGL_SHADERS", false, true,  "OpenGL support (shaders) in 3d games" },
	{            "taskbar",                   "USE_TASKBAR", false, true,  "Taskbar integration support" },
	{              "cloud",                     "USE_CLOUD", false, true,  "Cloud integration support" },
	{               "enet",                      "USE_ENET", false, true,  "ENet networking support" },
	{        "translation",               "USE_TRANSLATION", false, true,  "Translation support" },
	{             "vkeybd",                 "ENABLE_VKEYBD", false, false, "Virtual keyboard support"},
	{      "eventrecorder",          "ENABLE_EVENTRECORDER", false, false, "Event recorder support"},
	{            "updates",                   "USE_UPDATES", false, false, "Updates support"},
	{            "dialogs",                "USE_SYSDIALOGS", false, true,  "System dialogs support"},
	{         "langdetect",                "USE_DETECTLANG", false, true,  "System language detection support" }, // This feature actually depends on "translation", there
	                                                                                                           // is just no current way of properly detecting this...
	{       "text-console", "USE_TEXT_CONSOLE_FOR_DEBUGGER", false, false, "Text console debugger" }, // This feature is always applied in xcode projects
	{                "tts",                       "USE_TTS", false, true,  "Text to speech support"},
	{  "builtin-resources",             "BUILTIN_RESOURCES", false, true,  "include resources (e.g. engine data, fonts) into the binary"},
	{     "detection-full",                "DETECTION_FULL", false, true,  "Include detection objects for all engines" },
	{   "detection-static", "USE_DETECTION_FEATURES_STATIC", false, true,  "Static linking of detection objects for engines."},
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
	{ 10,    "Visual Studio 2010",    "11.00",          "2010",     "4.0",    "v100",    "LLVM-vs2010" },
	{ 11,    "Visual Studio 2012",    "11.00",          "2012",     "4.0",    "v110",    "LLVM-vs2012" },
	{ 12,    "Visual Studio 2013",    "12.00",          "2013",    "12.0",    "v120",    "LLVM-vs2013" },
	{ 14,    "Visual Studio 2015",    "12.00",            "14",    "14.0",    "v140",    "LLVM-vs2014" },
	{ 15,    "Visual Studio 2017",    "12.00",            "15",    "15.0",    "v141",    "llvm"        },
	{ 16,    "Visual Studio 2019",    "12.00",    "Version 16",    "16.0",    "v142",    "llvm"        },
	{ 17,    "Visual Studio 2022",    "12.00",    "Version 17",    "17.0",    "v143",    "llvm"        }
};

const char *s_msvc_arch_names[] = {"arm64", "x86", "x64"};
const char *s_msvc_config_names[] = {"arm64", "Win32", "x64"};
// clang-format on
} // End of anonymous namespace

// An array of buffers for the features
// created out of the components (which use char pointers)
std::list<std::string> s_stash_features;
FeatureList::iterator addDynamicFeature(FeatureList &features, const std::string &name, const std::string &description, const std::string &define) {
	// Add a new entry in our stash and fill it
	s_stash_features.push_back(std::string());
	std::string &buffer = s_stash_features.back();

	buffer += name;
	buffer += '\0';
	buffer += define;
	buffer += '\0';
	buffer += description;

	// Starting from now the buffer must be read-only

	const char *ptr = buffer.c_str();
	Feature feature = {
		ptr,
		ptr + name.size() + 1,
		false,
		true,
		ptr + name.size() + define.size() + 2
	};

	features.push_back(feature);
	return --features.end();
}

std::string getMSVCArchName(MSVC_Architecture arch) {
	return s_msvc_arch_names[arch];
}

std::string getMSVCConfigName(MSVC_Architecture arch) {
	return s_msvc_config_names[arch];
}

FeatureList getAllFeatures() {
	const size_t featureCount = sizeof(s_features) / sizeof(s_features[0]);

	FeatureList features;
	for (size_t i = 0; i < featureCount; ++i)
		features.push_back(s_features[i]);

	return features;
}

ComponentList getAllComponents(const std::string &srcDir, FeatureList &features) {
	std::string configureFile = srcDir + "/configure";

	std::ifstream configure(configureFile.c_str());
	if (!configure)
		return ComponentList();

	std::string line;
	ComponentList components;
	bool seenComponents = false;

	for (;;) {
		std::getline(configure, line);
		if (configure.eof())
			break;

		if (configure.fail())
			error("Failed while reading from " + configureFile);

		TokenList tokens = tokenize(line);

		if (tokens.size() < 4)
			continue;

		TokenList::const_iterator token = tokens.begin();

		// add_component lua "lua" "_lua" "USE_LUA"
		if (*token != "add_component") {
			if (seenComponents)	// No need to read whole file
				break;
			else
				continue;
		}

		seenComponents = true;
		++token;
		std::string name = *token;
		++token;
		std::string description = *token;
		++token;
		++token;
		std::string define = *token;

		FeatureList::iterator itr = std::find(features.begin(), features.end(), name);
		if (itr == features.end()) {
			// Create a new feature on the fly
			itr = addDynamicFeature(features, name, description, define);
		}

		Component comp = { name, define, *itr, description, false };
		components.push_back(comp);
	}

	return components;
}

void disableComponents(const ComponentList &components) {
	bool disabled = false;
	for (ComponentList::const_iterator i = components.begin(); i != components.end(); ++i) {
		if (!i->needed) {
			i->feature.enable = false;
			disabled = true;
			std::cout << "Feature " << i->feature.description << " is disabled as unused by enabled engines\n";
		}
	}
	if (disabled) {
		std::cout << "\n";
	}
}

StringList getFeatureDefines(const FeatureList &features) {
	StringList defines;

	for (FeatureList::const_iterator i = features.begin(); i != features.end(); ++i) {
		if (i->enable && i->define && i->define[0])
			defines.push_back(i->define);
	}

	return defines;
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

bool getFeatureBuildState(const std::string &name, const FeatureList &features) {
	FeatureList::const_iterator i = std::find(features.begin(), features.end(), name);
	if (i != features.end()) {
		return i->enable;
	} else {
		return false;
	}
}

BuildSetup removeFeatureFromSetup(BuildSetup setup, const std::string &feature) {
	// TODO: disable feature instead of removing from setup
	for (FeatureList::iterator i = setup.features.begin(); i != setup.features.end(); ++i) {
		if (i->enable && feature == i->name) {
			if (i->define && i->define[0]) {
				setup.defines.remove(i->define);
			}
			setup.features.erase(i);
			break;
		}
	}
	return setup;
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

	return nullptr;
}

int getInstalledMSVC() {
	int latest = 0;
#if defined(_WIN32) || defined(WIN32)
	// Use the Visual Studio Installer to get the latest version
	const char *vsWhere = "\"\"%PROGRAMFILES(X86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe\" -latest -legacy -property installationVersion\"";
	FILE *pipe = _popen(vsWhere, "rt");
	if (pipe != nullptr) {
		char version[50];
		if (fgets(version, 50, pipe) != nullptr) {
			latest = atoi(version);
		}
		_pclose(pipe);
	}

	// Use the registry to get the latest version
	if (latest == 0) {
		HKEY key;
		LONG err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7"), 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &key);
		if (err == ERROR_SUCCESS && key != nullptr) {
			const MSVCList msvc = getAllMSVCVersions();
			for (MSVCList::const_reverse_iterator i = msvc.rbegin(); i != msvc.rend(); ++i) {
				std::ostringstream version;
				version << i->version << ".0";
				err = RegQueryValueExA(key, version.str().c_str(), nullptr, nullptr, nullptr, nullptr);
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

void splitPath(const std::string &path, std::string &dir, std::string &file) {
	const std::string::size_type sep = path.find_last_of('/');
	dir = (sep == std::string::npos) ? path : path.substr(0, sep);
	file = (sep == std::string::npos) ? std::string() : path.substr(sep + 1);
}

bool calculatePchPaths(const std::string &sourceFilePath, const std::string &pchIncludeRoot, const StringList &pchDirs, const StringList &pchExclude, char separator, std::string &outPchIncludePath, std::string &outPchFilePath, std::string &outPchFileName) {
	std::string compareName, extensionName;
	splitFilename(sourceFilePath, compareName, extensionName);

	// Is this file excluded?
	if (std::find(pchExclude.begin(), pchExclude.end(), compareName) != pchExclude.end())
		return false;

	size_t lastDelimiter = sourceFilePath.find_last_of(separator);
	if (lastDelimiter == std::string::npos)
		lastDelimiter = 0;

	std::string pchDirectory = sourceFilePath.substr(0, lastDelimiter);

	if (std::find(pchDirs.begin(), pchDirs.end(), pchDirectory) == pchDirs.end())
		return false;

	// This file uses a PCH
	if (pchDirectory.size() < pchIncludeRoot.size() || pchDirectory.substr(0, pchIncludeRoot.size()) != pchIncludeRoot) {
		error("PCH prefix for file '" + sourceFilePath + "' wasn't located under PCH include root '" + pchIncludeRoot + "'");
	}

	size_t pchDirNamePos = pchDirectory.find_last_of(separator);
	if (pchDirNamePos == std::string::npos)
		pchDirNamePos = 0;
	else
		pchDirNamePos++;

	std::string pchFileName = pchDirectory.substr(pchDirNamePos) + "_pch.h";

	std::string pchPath = (pchDirectory + separator + pchFileName);

	// Convert to the local file prefix
	std::string includePath = pchPath.substr(pchIncludeRoot.size());

	if (separator != '/') {
		for (std::string::iterator ch = includePath.begin(), chEnd = includePath.end(); ch != chEnd; ++ch) {
			if (*ch == separator)
				*ch = '/';
		}
	}

	outPchIncludePath = includePath;
	outPchFilePath = pchPath;
	outPchFileName = pchFileName;

	return true;
}

std::string basename(const std::string &fileName) {
	const std::string::size_type slash = fileName.find_last_of('/');
	if (slash == std::string::npos)
		return fileName;
	return fileName.substr(slash + 1);
}

bool producesObjectExtension(const std::string &ext) {
	return (ext == "cpp" || ext == "c" || ext == "asm" || ext == "m" || ext == "mm");
}

bool producesObjectFile(const std::string &fileName) {
	std::string n, ext;
	splitFilename(fileName, n, ext);
	return producesObjectExtension(ext);
}

std::string toString(int num) {
	std::ostringstream os;
	os << num;
	return os.str();
}

std::string toUpper(const std::string &str) {
	std::string res;
	std::transform(str.begin(), str.end(), std::back_inserter(res), toupper);
	return res;
}

/**
 * Checks whether the give file in the specified directory is present in the given
 * file list.
 *
 * This function does as special match against the file list.
 * By default object files (.o) are excluded, header files (.h) are included,
 * and it will not take file extensions into consideration, when the extension
 * of a file in the specified directory is one of "m", "cpp", "c" or "asm".
 *
 * @param dir Parent directory of the file.
 * @param fileName File name to match.
 * @param fileList List of files to match against.
 * @return "true" when the file is in the list, "false" otherwise.
 */
bool isInList(const std::string &dir, const std::string &fileName, const StringList &fileList) {
	if (fileList.empty())
		return false;

	std::string compareName, extensionName;
	splitFilename(fileName, compareName, extensionName);

	if (!extensionName.empty())
		compareName += '.';

	for (StringList::const_iterator i = fileList.begin(); i != fileList.end(); ++i) {

		// When no comparison name is given, we try to match whether a subset of
		// the given directory should be included. To do that we must assure that
		// the first character after the substring, having the same size as dir, must
		// be a path delimiter.
		if (compareName.empty()) {
			if (i->compare(0, dir.size(), dir))
				continue;
			if (i->size() >= dir.size() + 1 && i->at(dir.size()) == '/')
				return true;
			else
				continue;
		}

		std::string listDir, listFile;
		splitPath(*i, listDir, listFile);
		if (dir.compare(0, listDir.size(), listDir))
			continue;

		if (extensionName == "o") {
			return false;
		} else if (extensionName == "h") {
			return true;
		} else if (!producesObjectExtension(extensionName)) {
			if (fileName == listFile)
				return true;
		} else {
			if (!listFile.compare(0, compareName.size(), compareName))
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
	WIN32_FIND_DATAA fileInformation;
	HANDLE fileHandle = FindFirstFileA((dir + "/*").c_str(), &fileInformation);

	if (fileHandle == INVALID_HANDLE_VALUE)
		return result;

	do {
		if (fileInformation.cFileName[0] == '.')
			continue;

		result.push_back(FSNode(fileInformation.cFileName, (fileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0));
	} while (FindNextFileA(fileHandle, &fileInformation) == TRUE);

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
	if (!CreateDirectoryA(dir.c_str(), nullptr)) {
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
		return nullptr;

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

		std::string name, ext;
		splitFilename(i->name, name, ext);

		if (ext != "h" && isInList(dir, i->name, excludeList))
			continue;

		if (!isInList(dir, i->name, includeList))
			continue;

		FileNode *child = new FileNode(i->name);
		assert(child);
		result->children.push_back(child);
	}

	if (result->children.empty()) {
		delete result;
		return nullptr;
	} else {
		result->children.sort(compareNodes);
		return result;
	}
}

//////////////////////////////////////////////////////////////////////////
// Project Provider methods
//////////////////////////////////////////////////////////////////////////
ProjectProvider::ProjectProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, StringList &global_errors, const int version)
	: _version(version), _globalWarnings(global_warnings), _projectWarnings(project_warnings), _globalErrors(global_errors) {
}

void ProjectProvider::createProject(BuildSetup &setup) {
	std::string targetFolder;

	if (setup.devTools) {
		_engineUuidMap = createToolsUUIDMap();
		targetFolder = "/devtools/";
	} else if (!setup.tests) {
		_engineUuidMap = createUUIDMap(setup);
		targetFolder = "/engines/";
	}

	_allProjUuidMap = _engineUuidMap;

	// We also need to add the UUID of the main project file.
	const std::string svmUUID = _allProjUuidMap[setup.projectName] = createUUID(setup.projectName);
	// Add the uuid of the detection project
	const std::string detProject = setup.projectName + "-detection";
	const std::string detUUID = createUUID(detProject);
	if (setup.useStaticDetection) {
		_allProjUuidMap[detProject] = _engineUuidMap[detProject] = detUUID;
	}

	// Scan for resources
	for (int i = 0; i < kEngineDataGroupCount; i++) {
		createDataFilesList(static_cast<EngineDataGroup>(i), setup.srcDir, setup.defines, _engineDataGroupDefs[i].dataFiles, _engineDataGroupDefs[i].winHeaderPath);
	}

	createWorkspace(setup);

	StringList in, ex, pchDirs, pchEx;

	// Create project files
	for (UUIDMap::const_iterator i = _engineUuidMap.begin(); i != _engineUuidMap.end(); ++i) {
		if (i->first == detProject)
			continue;
		// Retain the files between engines if we're creating a single project
		in.clear();
		ex.clear();
		pchDirs.clear();
		pchEx.clear();

		const std::string moduleDir = setup.srcDir + targetFolder + i->first;

		createModuleList(moduleDir, setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createProjectFile(i->first, i->second, setup, moduleDir, in, ex, setup.srcDir + targetFolder, pchDirs, pchEx);
	}

	// Create engine-detection submodules.
	if (setup.useStaticDetection) {
		in.clear();
		ex.clear();
		std::vector<std::string> detectionModuleDirs;
		detectionModuleDirs.reserve(setup.engines.size());
		bool detectAllEngines = getFeatureBuildState("detection-full", setup.features);

		for (EngineDescList::const_iterator i = setup.engines.begin(), end = setup.engines.end(); i != end; ++i) {
			// We ignore all sub engines here because they require no special handling.
			if (isSubEngine(i->name, setup.engines)) {
				continue;
			}
			// If we're not detecting all engines then ignore the disabled ones
			if (!(detectAllEngines || i->enable)) {
				continue;
			}
			detectionModuleDirs.push_back(setup.srcDir + "/engines/" + i->name);
		}

		for (std::vector<std::string>::const_iterator i = detectionModuleDirs.begin(), end = detectionModuleDirs.end(); i != end; ++i) {
			StringList tempPchDirs, tempSchEx;	// No PCH for detection
			createModuleList(*i, setup.defines, setup.testDirs, in, ex, tempPchDirs, tempSchEx, true);
		}

		createProjectFile(detProject, detUUID, setup, setup.srcDir + "/engines", in, ex, "", StringList(), StringList());
	}

	if (!setup.devTools) {
		// Last but not least create the main project file.
		in.clear();
		ex.clear();
		pchDirs.clear();
		pchEx.clear();
		// File list for the Project file
		createModuleList(setup.srcDir + "/backends", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/backends/platform/sdl", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/base", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/common", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/common/compression", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/common/formats", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/common/lua", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/engines", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/graphics", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/gui", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/audio", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/video", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/image", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		createModuleList(setup.srcDir + "/math", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);

		if (getFeatureBuildState("mt32emu", setup.features))
			createModuleList(setup.srcDir + "/audio/softsynth/mt32", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);

		if (setup.tests) {
			createModuleList(setup.srcDir + "/test", setup.defines, setup.testDirs, in, ex, pchDirs, pchEx);
		} else {
			// Resource files
			addResourceFiles(setup, in, ex);
			if (setup.win32) {
				for (const EngineDataGroupDef &groupDef : _engineDataGroupDefs)
					in.push_back(setup.srcDir + "/" + groupDef.winHeaderPath);
			}

			// Various text files
			in.push_back(setup.srcDir + "/AUTHORS");
			in.push_back(setup.srcDir + "/COPYING");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.Apache");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.BSD");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.BSL");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.FREEFONT");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.GLAD");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.ISC");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.LGPL");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.LUA");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.MIT");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.MKV");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.MPL");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.OFL");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.TINYGL");
			in.push_back(setup.srcDir + "/LICENSES/CatharonLicense.txt");
			in.push_back(setup.srcDir + "/COPYRIGHT");
			in.push_back(setup.srcDir + "/NEWS.md");
			in.push_back(setup.srcDir + "/README.md");
		}

		// Create the main project file.
		createProjectFile(setup.projectName, svmUUID, setup, setup.srcDir, in, ex, setup.srcDir + '/', pchDirs, pchEx);
	}

	// Create other misc. build files
	createOtherBuildFiles(setup);
	createResourceEmbeds(setup);

	// In case we create the main ScummVM project files we will need to
	// generate engines/plugins_table.h & engines/detection_table.h
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

	uuid[8] &= 0xBF;
	uuid[8] |= 0x80;
	uuid[6] &= 0x4F;
	uuid[6] |= 0x40;

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
	    {0x5f, 0x5b, 0x43, 0xe8, 0x35, 0xff, 0x4f, 0x1e, 0xad, 0x7e, 0xa2, 0xa8, 0x7e, 0x9b, 0x52, 0x54};
	if (!CryptHashData(hHash, uuidNs, kUUIDLen, 0)) {
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		error("CryptHashData failed");
	}

	// Hash project name
	if (!CryptHashData(hHash, (const BYTE *)name.c_str(), (DWORD)name.length(), 0)) {
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
	uuid[6] &= 0x0F;
	uuid[6] |= 0x30;
	uuid[8] &= 0x3F;
	uuid[8] |= 0x80;

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

void ProjectProvider::addFilesToProject(const std::string &dir, std::ostream &projectFile,
										const StringList &includeList, const StringList &excludeList,
										const std::string &pchIncludeRoot, const StringList &pchDirs, const StringList &pchExclude,
										const std::string &filePrefix) {
	FileNode *files = scanFiles(dir, includeList, excludeList);
	if (files == nullptr) {
		return;
	}

	writeFileListToProject(*files, projectFile, 0, std::string(), filePrefix + '/', pchIncludeRoot, pchDirs, pchExclude);

	delete files;
}

void ProjectProvider::createModuleList(const std::string &moduleDir, const StringList &defines, StringList &testDirs, StringList &includeList, StringList &excludeList, StringList &pchDirs, StringList &pchExclude, bool forDetection) const {
	const std::string moduleMkFile = moduleDir + "/module.mk";
	std::ifstream moduleMk(moduleMkFile.c_str());
	if (!moduleMk)
		error(moduleMkFile + " is not present");

	includeList.push_back(moduleMkFile);

	std::stack<bool> shouldInclude;
	shouldInclude.push(true);

	StringList filesInVariableList;
	std::string moduleRootDir;

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
			if (forDetection) {
				moduleRootDir = moduleRoot;
				break;
			}
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
		} else if (*i == "MODULE_PCH_DIRS") {
			if (tokens.size() < 3)
				error("Malformed MODULE_PCH_DIRS definition in " + moduleMkFile);
			++i;

			// This is not exactly correct, for example an ":=" would usually overwrite
			// all already added files, but since we do only save the files inside
			// includeList or excludeList currently, we couldn't handle such a case easily.
			// (includeList and excludeList should always preserve their entries, not added
			// by this function, thus we can't just clear them on ":=" or "=").
			// But hopefully our module.mk files will never do such things anyway.
			if (*i != ":=" && *i != "+=" && *i != "=")
				error("Malformed MODULE_PCH_DIRS definition in " + moduleMkFile);

			++i;

			while (i != tokens.end()) {
				if (*i == "\\") {
					std::getline(moduleMk, line);
					tokens = tokenize(line);
					i = tokens.begin();
				} else {
					std::string filename = moduleDir;
					if ((*i) != ".")
						filename += "/" + unifyPath(*i);

					if (shouldInclude.top())
						pchDirs.push_back(filename);

					++i;
				}
			}
		} else if (*i == "MODULE_PCH_EXCLUDE") {
			if (tokens.size() < 3)
				error("Malformed MODULE_PCH_EXCLUDE definition in " + moduleMkFile);
			++i;

			// This is not exactly correct, for example an ":=" would usually overwrite
			// all already added files, but since we do only save the files inside
			// includeList or excludeList currently, we couldn't handle such a case easily.
			// (includeList and excludeList should always preserve their entries, not added
			// by this function, thus we can't just clear them on ":=" or "=").
			// But hopefully our module.mk files will never do such things anyway.
			if (*i != ":=" && *i != "+=" && *i != "=")
				error("Malformed MODULE_PCH_EXCLUDE definition in " + moduleMkFile);

			++i;

			while (i != tokens.end()) {
				if (*i == "\\") {
					std::getline(moduleMk, line);
					tokens = tokenize(line);
					i = tokens.begin();
				} else {
					const std::string filename = moduleDir + "/" + unifyPath(*i);

					if (shouldInclude.top())
						pchExclude.push_back(filename);

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
		} else if (*i == "ifeq" || *i == "ifneq") {
			//XXX
			shouldInclude.push(false);
		}
	}

	if (forDetection) {
		std::string::size_type p = moduleRootDir.find('/');
		std::string engineName = moduleRootDir.substr(p + 1);
		std::string engineNameUpper = toUpper(engineName);

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

			if (*i != "DETECT_OBJS" && *i != "ifneq") {
				continue;
			}

			if (*i == "ifneq") {
				++i;
				if (*i != ("($(ENABLE_" + engineNameUpper + "),")) {
					continue;
				}

				// If the engine is already enabled, skip the additional
				// dependencies for detection objects.
				if (isEngineEnabled[engineName]) {
					bool breakEarly = false;
					while (true) {
						std::getline(moduleMk, line);
						if (moduleMk.eof()) {
							error("Unexpected EOF found, while parsing for " + engineName + " engine's module file.");
						} else if (line != "endif") {
							continue;
						} else {
							breakEarly = true;
							break;
						}
					}
					if (breakEarly) {
						break;
					}
				}

				while (*i != "DETECT_OBJS") {
					std::getline(moduleMk, line);
					if (moduleMk.eof()) {
						break;
					}

					tokens = tokenize(line);

					if (tokens.empty())
						continue;
					i = tokens.begin();
				}
			}


			if (tokens.size() < 3)
				error("Malformed DETECT_OBJS definition in " + moduleMkFile);
			++i;

			if (*i != "+=")
				error("Malformed DETECT_OBJS definition in " + moduleMkFile);

			++i;

			p = (*i).find('/');
			const std::string filename = moduleDir + "/" + (*i).substr(p + 1);

			includeList.push_back(filename);
		}
	}
	if (shouldInclude.size() != 1)
		error("Malformed file " + moduleMkFile);
}

static EngineDataGroupResolution s_engineDataResolutions[] = {
	{kEngineDataGroupNormal,	"dists/engine-data/engine_data.mk",			"dists/scummvm_rc_engine_data.rh"},
	{kEngineDataGroupBig,		"dists/engine-data/engine_data_big.mk",		"dists/scummvm_rc_engine_data_big.rh"},
	{kEngineDataGroupCore,		"dists/engine-data/engine_data_core.mk",	"dists/scummvm_rc_engine_data_core.rh"},
};

void ProjectProvider::createDataFilesList(EngineDataGroup engineDataGroup, const std::string &baseDir, const StringList &defines, StringList &outDataFiles, std::string &outWinHeaderPath) const {
	outDataFiles.clear();

	const EngineDataGroupResolution *resolution = nullptr;

	for (const EngineDataGroupResolution &resolutionCandidate : s_engineDataResolutions) {
		if (resolutionCandidate.engineDataGroup == engineDataGroup) {
			resolution = &resolutionCandidate;
			break;
		}
	}

	if (!resolution)
		error("Engine data group resolution wasn't defined");

	std::string mkFile = baseDir + "/" + resolution->mkFilePath;
	std::ifstream moduleMk(mkFile.c_str());
	if (!moduleMk)
		error(mkFile + " is not present");

	outWinHeaderPath = resolution->winHeaderPath;

	std::stack<bool> shouldInclude;
	shouldInclude.push(true);

	std::string line;
	for (;;) {
		std::getline(moduleMk, line);

		if (moduleMk.eof())
			break;

		if (moduleMk.fail())
			error(std::string("Failed while reading from ") + mkFile);

		TokenList tokens = tokenize(line);
		if (tokens.empty())
			continue;

		TokenList::const_iterator i = tokens.begin();
		if (*i == "DIST_FILES_LIST") {
			if (tokens.size() < 3)
				error("Malformed DIST_FILES_LIST definition in " + mkFile);
			++i;

			if (*i != "+=")
				error("Malformed DIST_FILES_LIST definition in " + mkFile);

			++i;

			while (i != tokens.end()) {
				if (*i == "\\") {
					std::getline(moduleMk, line);
					tokens = tokenize(line);
					i = tokens.begin();
				} else {
					const std::string filename = unifyPath(*i);

					if (shouldInclude.top()) {
						outDataFiles.push_back(filename);
					}
					++i;
				}
			}
		} else if (*i == "ifdef") {
			if (tokens.size() < 2)
				error("Malformed ifdef in " + mkFile);
			++i;

			if (std::find(defines.begin(), defines.end(), *i) == defines.end())
				shouldInclude.push(false);
			else
				shouldInclude.push(true && shouldInclude.top());
		} else if (*i == "ifndef") {
			if (tokens.size() < 2)
				error("Malformed ifndef in " + mkFile);
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
				error("endif without ifdef found in " + mkFile);
			shouldInclude.pop();
		} else if (*i == "elif") {
			error("Unsupported operation 'elif' in " + mkFile);
		} else if (*i == "ifeq" || *i == "ifneq") {
			// XXX
			shouldInclude.push(false);
		} else if (*i == "#") {
			// Comment, ignore
		} else
			error("Unknown definition line in " + mkFile);
	}

	if (shouldInclude.size() != 1)
		error("Malformed file " + mkFile);
}

void ProjectProvider::createEnginePluginsTable(const BuildSetup &setup) {
	// First we need to create the "engines" directory.
	createDirectory(setup.outputDir + "/engines");

	// Then, we can generate the actual "plugins_table.h" & "detection_table.h" file.
	const std::string enginePluginsTableFile = setup.outputDir + "/engines/plugins_table.h";
	const std::string detectionTableFile = setup.outputDir + "/engines/detection_table.h";

	std::ofstream enginePluginsTable(enginePluginsTableFile.c_str());
	std::ofstream detectionTable(detectionTableFile.c_str());

	if (!enginePluginsTable) {
		error("Could not open \"" + enginePluginsTableFile + "\" for writing");
	}

	if (!detectionTable) {
		error("Could not open \"" + detectionTableFile + "\" for writing");
	}

	enginePluginsTable << "/* This file is automatically generated by create_project */\n"
	                   << "/* DO NOT EDIT MANUALLY */\n"
	                   << "// This file is being included by \"base/plugins.cpp\"\n";

	detectionTable	   << "/* This file is automatically generated by create_project */\n"
	                   << "/* DO NOT EDIT MANUALLY */\n"
	                   << "// This file is being included by \"base/plugins.cpp\"\n";

	for (EngineDescList::const_iterator i = setup.engines.begin(), end = setup.engines.end(); i != end; ++i) {
		// We ignore all sub engines here because they require no special
		// handling.
		if (isSubEngine(i->name, setup.engines)) {
			continue;
		}

		// Make the engine name all uppercase.
		const std::string engineName = toUpper(i->name);

		enginePluginsTable << "#if PLUGIN_ENABLED_STATIC(" << engineName << ")\n"
		                   << "LINK_PLUGIN(" << engineName << ")\n"
		                   << "#endif\n";

		detectionTable << "#if defined(ENABLE_" << engineName << ") || defined(DETECTION_FULL)\n"
					   << "LINK_PLUGIN(" << engineName << "_DETECTION)\n"
					   << "#endif\n";
	}
}

void ProjectProvider::createResourceEmbeds(const BuildSetup &setup) const {
	if (!setup.win32)
		return;

	for (int i = 0; i < kEngineDataGroupCount; i++) {
		const EngineDataGroupDef &groupDef = _engineDataGroupDefs[i];

		std::string outPath = setup.srcDir + "/" + groupDef.winHeaderPath;

		std::ofstream resEmbedFile(outPath.c_str());

		if (!resEmbedFile || !resEmbedFile.is_open()) {
			error("Could not open \"" + outPath + "\" for writing");
			return;
		}

		resEmbedFile << "// This file was generated by create_project" << std::endl;
		resEmbedFile << "// Do not edit this file manually" << std::endl;
		resEmbedFile << std::endl;

		for (const std::string &fileName : groupDef.dataFiles) {
			size_t lastSlashPos = fileName.find_last_of('/');
			if (lastSlashPos == std::string::npos)
				error("Data file definition " + fileName + " wasn't located in a subdirectory");

			std::string shortName = fileName.substr(lastSlashPos + 1);

			resEmbedFile << shortName << " FILE \"" << fileName << "\"" << std::endl;
		}
	}
}
} // namespace CreateProjectTool

void error(const std::string &message) {
	std::cerr << "ERROR: " << message << "!" << std::endl;
	std::exit(-1);
}

bool BuildSetup::featureEnabled(const std::string &feature) const {
	return getFeature(feature).enable;
}

Feature BuildSetup::getFeature(const std::string &feature) const {
	FeatureList::const_iterator itr = std::find(features.begin(), features.end(), feature);
	if (itr == features.end()) {
		error("invalid feature request: " + feature);
	}

	return *itr;
}
