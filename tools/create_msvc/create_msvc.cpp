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

#include "create_msvc.h"

#include <fstream>
#include <iostream>
#include <cassert>
#include <map>
#include <stack>
#include <cctype>
#include <sstream>

namespace {
/**
 * Converts the given path to only use backslashes.
 * This means that for example the path:
 *  foo/bar\test.txt
 * will be converted to:
 *  foo\bar\test.txt
 *
 * @param path Path string.
 * @return Converted path.
 */
std::string convertPathToWin(const std::string &path);

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
 * Returns the last path component.
 *
 * @param path Path string.
 * @return Last path component.
 */
std::string getLastPathComponent(const std::string &path);

/**
 * Display the help text for the program.
 *
 * @param exe Name of the executable.
 */
void displayHelp(const char *exe);
} // End of anonymous namespace

int main(int argc, char *argv[]) {
	if (argc < 2) {
		displayHelp(argv[0]);
		return -1;
	}

	const std::string srcDir = argv[1];

	BuildSetup setup;
	setup.srcDir = unifyPath(srcDir);
	setup.engines = parseConfigure(setup.srcDir);
	setup.features = getAllFeatures();

	int msvcVersion = 9;
	// Parse command line arguments
	using std::cout;
	for (int i = 2; i < argc; ++i) {
		if (!strcmp(argv[i], "--list-engines")) {
			cout << " The following enables are available in the ScummVM source destribution\n"
			        " located at \"" << srcDir << "\":\n";

			cout << "   state  |       name      |     description\n\n";
			cout.setf(std::ios_base::left, std::ios_base::adjustfield);
			for (EngineDescList::const_iterator j = setup.engines.begin(); j != setup.engines.end(); ++j) {
				cout << ' ' << (j->enable ? " enabled" : "disabled") << " | ";
				cout.width(15);
				cout << j->name;
				cout.width(0);
				cout << " | " << j->desc << "\n";
			}
			cout.setf(std::ios_base::right, std::ios_base::adjustfield);

			return 0;
		} else if (!strcmp(argv[i], "--msvc-version")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"version\" parameter for \"--msvc-version\"!\n";
				return -1;
			}

			msvcVersion = atoi(argv[++i]);

			if (msvcVersion != 8 && msvcVersion != 9) {
				std::cerr << "ERROR: Unsupported version: \"" << msvcVersion << "\" passed to \"--msvc-version\"!\n";
				return -1;
			}
		} else if (!strncmp(argv[i], "--enable-", 9)) {
			const char *name = &argv[i][9];
			if (!*name) {
				std::cerr << "ERROR: Invalid command \"" << argv[i] << "\"\n";
				return -1;
			}

			if (!strcmp(name, "all-engines")) {
				for (EngineDescList::iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
					j->enable = true;
			} else if (!setEngineBuildState(name, setup.engines, true)) {
				FeatureList::iterator feature = std::find(setup.features.begin(), setup.features.end(), name);
				if (feature == setup.features.end()) {
					std::cerr << "ERROR: \"" << name << "\" is neither an engine nor a feature!\n";
					return -1;
				}

				feature->enable = true;
			}
		} else if (!strncmp(argv[i], "--disable-", 10)) {
			const char *name = &argv[i][10];
			if (!*name) {
				std::cerr << "ERROR: Invalid command \"" << argv[i] << "\"\n";
				return -1;
			}

			if (!strcmp(name, "all-engines")) {
				for (EngineDescList::iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
					j->enable = false;
			} else if (!setEngineBuildState(name, setup.engines, false)) {
				// If none found, we'll try the features list
				FeatureList::iterator feature = std::find(setup.features.begin(), setup.features.end(), name);
				if (feature == setup.features.end()) {
					std::cerr << "ERROR: \"" << name << "\" is neither an engine nor a feature!\n";
					return -1;
				}

				feature->enable = false;
			}
		} else {
			std::cerr << "ERROR: Unknown parameter \"" << argv[i] << "\"\n";
			return -1;
		}
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

	// Creation...
	setup.defines = getEngineDefines(setup.engines);
	StringList featureDefines = getFeatureDefines(setup.features);
	setup.defines.splice(setup.defines.begin(), featureDefines);
	
	setup.libraries = getFeatureLibraries(setup.features);

	setup.libraries.push_back("winmm.lib");
	setup.libraries.push_back("sdl.lib");

	try {
		createMSVCProject(setup, msvcVersion);
	} catch (const std::string &error) {
		std::cerr << "ERROR: " << error << "!" << std::endl;
	} catch (const std::exception &exp) {
		std::cerr << "ERROR: " << exp.what() << "!" << std::endl;
	}
}

namespace {
std::string convertPathToWin(const std::string &path) {
	std::string result = path;
	std::replace(result.begin(), result.end(), '/', '\\');
	return result;
}

std::string unifyPath(const std::string &path) {
	std::string result = path;
	std::replace(result.begin(), result.end(), '\\', '/');
	return result;
}

std::string getLastPathComponent(const std::string &path) {
	std::string::size_type pos = path.find_last_of('/');
	if (pos == std::string::npos)
		return path;
	else
		return path.substr(pos + 1);
}

void displayHelp(const char *exe) {
	using std::cout;

	cout << "Usage:\n"
	     << exe << " path\\to\\source [optional options]\n"
	     << "\n"
	     << " Creates MSVC project files for the ScummVM source locatd at \"path\\to\\source\".\n"
		    " The project files will be created in the directory where tool is run from and\n"
			" will include \"path\\to\\source\" for relative file paths, thus be sure that you\n"
			" pass a relative file path like \"..\\..\\trunk\".\n"
			"\n"
			" Additionally there are the following switches for changing various settings:\n"
			"\n"
			"MSVC specifc settings:\n"
			" --msvc-version version   sets the targeted MSVC version. Possible values:\n"
			"                          8 stands for \"Visual Studio 2005\"\n"
			"                          9 stands for \"Visual Studio 2008\"\n"
			"                         The default is \"9\", thus \"Visual Studio 2008\"\n"
			"\n"
			"ScummVM engine settings:\n"
			" --list-engines           lists all available engines and their default state\n"
			" --enable-engine          enables building of the engine with the name \"engine\"\n"
			" --disable-engine         disables building of the engine with the name \"engine\"\n"
			" --enable-all-engines     enables building of all engines\n"
			" --disable-all-engines    disables building of all engines\n"
			"\n"
			"ScummVM optional feature settings:\n"
			" --enable-name            enables inclusion of the feature \"name\"\n"
			" --disable-name           disables inclusion of the feature \"name\"\n"
			"\n"
			" There are the following features available:\n"
			"\n";

	cout << "   state  |       name      |     description\n\n";
	const FeatureList features = getAllFeatures();
	cout.setf(std::ios_base::left, std::ios_base::adjustfield);
	for (FeatureList::const_iterator i = features.begin(); i != features.end(); ++i) {
		cout << ' ' << (i->enable ? " enabled" : "disabled") << " | ";
		cout.width(15);
		cout << i->name;
		cout.width(0);
		cout << " | " << i->description << '\n';
	}
	cout.setf(std::ios_base::right, std::ios_base::adjustfield);
}

typedef StringList TokenList;

/**
 * Takes a given input line and creates a list of tokens out of it.
 *
 * A token in this context is seperated by whitespaces. A special case
 * are quotation marks though. A string inside quotation marks is treated
 * as single token, even when it contains whitespaces.
 *
 * Thus for example the input:
 * foo bar "1 2 3 4" ScummVM
 * will create a list with the following entries:
 * "foo", "bar", "1 2 3 4", "ScummVM"
 * As you can see the quotation marks will get *removed* too.
 *
 * @param input The text to be tokenized.
 * @return A list of tokens.
 */
TokenList tokenize(const std::string &input);

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
bool parseEngine(const std::string &line, EngineDesc &engine);
} // End of anonymous namespace

EngineDescList parseConfigure(const std::string &srcDir) {
	std::string configureFile = srcDir + "/configure";

	std::ifstream configure(configureFile.c_str());
	if (!configure)
		return EngineDescList();

	std::string line;
	EngineDescList engines;

	while (true) {
		std::getline(configure, line);
		if (configure.eof())
			break;

		if (configure.fail())
			throw std::string("Failed while reading from " + configureFile);

		EngineDesc desc;
		if (parseEngine(line, desc))
			engines.push_back(desc);
	}

	return engines;
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

			// When we disable an einge, we also need to disable all the sub engines.
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
bool parseEngine(const std::string &line, EngineDesc &engine) {
	// Format:
	// add_engine engine_name "Readable Description" enable_default ["SubEngineList"]
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
	if (token != tokens.end())
		engine.subEngines = tokenize(*token);

	return true;
}

TokenList tokenize(const std::string &input) {
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
			nIdx = input.find_first_of(' ', sIdx);
		}

		if (nIdx != std::string::npos) {
			result.push_back(input.substr(sIdx, nIdx - sIdx));
			sIdx = input.find_first_not_of(" \t", nIdx + 1);
		} else {
			result.push_back(input.substr(sIdx));
			break;
		}
	} while (sIdx != std::string::npos);

	return result;
}
} // End of anonymous namespace

namespace {
const Feature s_features[] = {
	// Libraries
	{    "libz",      "USE_ZLIB", "zlib.lib", true, "zlib (compression) support" },
	{     "mad",       "USE_MAD", "libmad.lib", true, "libmad (MP3) support" },
	{  "vorbis",    "USE_VORBIS", "libvorbisfile_static.lib libvorbis_static.lib libogg_static.lib", true, "Ogg Vorbis support" },
	{    "flac",      "USE_FLAC", "libFLAC_static.lib", false, "FLAC support" },
	{   "mpeg2",     "USE_MPEG2", "libmpeg2.lib", false, "mpeg2 codec for cutscenes" },

	// ScummVM feature flags
	{   "16bit", "USE_RGB_COLOR", "", true, "16bit color support" },
	{ "mt32emu",   "USE_MT32EMU", "", true, "integrated MT-32 emulator" },
	{    "nasm",     "HAVE_NASM", "", true, "IA-32 assembly support" }, // This feature is special in the regard, that it needs additional handling.
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

namespace {
typedef std::map<std::string, std::string> UUIDMap;

/**
 * Creates an UUID for every enabled engine of the
 * passed build description.
 *
 * @param setup Description of the desired build.
 * @return A map, which includes UUIDs for all enabled engines.
 */
UUIDMap createUUIDMap(const BuildSetup &setup);

/**
 * Creates an UUID and returns it in string representation.
 *
 * @return A new UUID as string.
 */
std::string createUUID();

/**
 * Creates the main solution file "scummvm.sln" for a specific
 * build setup.
 *
 * @param setup Description of the desired build.
 * @param uuids Map of all project file UUIDs.
 * @param version Target MSVC version.
 */
void createScummVMSolution(const BuildSetup &setup, const UUIDMap &uuids, const int version);

/**
 * Create a project file for the specified list of files.
 *
 * @param name Name of the project file.
 * @param uuid UUID of the project file.
 * @param setup Description of the desired build.
 * @param moduleDir Path to the module.
 * @param includeList Files to include (must have "moduleDir" as prefix).
 * @param excludeList Files to exclude (must have "moduleDir" as prefix).
 * @param version Target MSVC version.
 */
void createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
                       const StringList &includeList, const StringList &excludeList, const int version);

/**
 * Adds files of the specified directory recursively to given project file.
 *
 * @param dir Path to the directory.
 * @param projectFile Output stream object, where all data should be written to.
 * @param includeList Files to include (must have a relative directory as prefix).
 * @param excludeList Files to exclude (must have a relative directory as prefix).
 */
void addFilesToProject(const std::string &dir, std::ofstream &projectFile,
                       const StringList &includeList, const StringList &excludeList);

/**
 * Create the global project properties.
 *
 * @param setup Description of the desired build setup.
 * @param version Target MSVC version.
 */
void createGlobalProp(const BuildSetup &setup, const int version);

/**
 * Generates the project properties for debug and release settings.
 *
 * @param setup Description of the desired build setup.
 * @param version Target MSVC version.
 */
void createBuildProp(const BuildSetup &setup, const int version);

/**
 * Creates a list of files of the specified module. This also
 * creates a list of files, which should not be included.
 * All filenames will have "moduleDir" as prefix.
 *
 * @param moduleDir Path to the module.
 * @param defines List of set defines.
 * @param includeList Reference to a list, where included files should be added.
 * @param excludeList Reference to a list, where excluded files should be added.
 */
void createModuleList(const std::string &moduleDir, const StringList &defines, StringList &includeList, StringList &excludeList);
} // End of anonymous namespace

void createMSVCProject(const BuildSetup &setup, const int version) {
	UUIDMap uuidMap = createUUIDMap(setup);

	// We also need to add the UUID of the main project file.
	const std::string svmUUID = uuidMap["scummvm"] = createUUID();

	createScummVMSolution(setup, uuidMap, version);

	StringList in, ex;

	// Create engine project files
	for (UUIDMap::const_iterator i = uuidMap.begin(); i != uuidMap.end(); ++i) {
		if (i->first == "scummvm")
			continue;

		in.clear(); ex.clear();
		const std::string moduleDir = setup.srcDir + "/engines/" + i->first;

		createModuleList(moduleDir, setup.defines, in, ex);
		createProjectFile(i->first, i->second, setup, moduleDir, in, ex, version);
	}

	// Last but not least create the main ScummVM project file.
	in.clear(); ex.clear();

	// File list for the ScummVM project file
	createModuleList(setup.srcDir + "/backends", setup.defines, in, ex);
	createModuleList(setup.srcDir + "/backends/platform/sdl", setup.defines, in, ex);
	createModuleList(setup.srcDir + "/base", setup.defines, in, ex);
	createModuleList(setup.srcDir + "/common", setup.defines, in, ex);
	createModuleList(setup.srcDir + "/engines", setup.defines, in, ex);
	createModuleList(setup.srcDir + "/graphics", setup.defines, in, ex);
	createModuleList(setup.srcDir + "/gui", setup.defines, in, ex);
	createModuleList(setup.srcDir + "/sound", setup.defines, in, ex);
	createModuleList(setup.srcDir + "/sound/softsynth/mt32", setup.defines, in, ex);

	// Resource files
	in.push_back(setup.srcDir + "/icons/scummvm.ico");
	in.push_back(setup.srcDir + "/dists/scummvm.rc");

	// Various text files
	in.push_back(setup.srcDir + "/AUTHORS");
	in.push_back(setup.srcDir + "/COPYING");
	in.push_back(setup.srcDir + "/COPYING.LGPL");
	in.push_back(setup.srcDir + "/COPYRIGHT");
	in.push_back(setup.srcDir + "/NEWS");
	in.push_back(setup.srcDir + "/README");
	in.push_back(setup.srcDir + "/TODO");

	// Create the "scummvm.vcproj" file.
	createProjectFile("scummvm", svmUUID, setup, setup.srcDir, in, ex, version);

	// Create the global property file
	createGlobalProp(setup, version);

	// Create the configuration property files
	createBuildProp(setup, version);
}

namespace {
UUIDMap createUUIDMap(const BuildSetup &setup) {
	UUIDMap result;

	for (EngineDescList::const_iterator i = setup.engines.begin(); i != setup.engines.end(); ++i) {
		if (!i->enable || isSubEngine(i->name, setup.engines))
			continue;

		result[i->name] = createUUID();
	}

	return result;
}

std::string createUUID() {
	UUID uuid;
	if (UuidCreate(&uuid) != RPC_S_OK)
		throw std::string("UuidCreate failed");

	unsigned char *string = 0;
	if (UuidToStringA(&uuid, &string) != RPC_S_OK)
		throw std::string("UuidToStringA failed");

	std::string result = std::string((char *)string);
	std::transform(result.begin(), result.end(), result.begin(), toupper);
	RpcStringFreeA(&string);
	return result;
}

void createScummVMSolution(const BuildSetup &/*setup*/, const UUIDMap &uuids, const int version) {
	UUIDMap::const_iterator svmUUID = uuids.find("scummvm");
	if (svmUUID == uuids.end())
		throw std::string("No UUID for \"scummvm\" project created");

	const std::string svmProjectUUID = svmUUID->second;
	assert(!svmProjectUUID.empty());

	std::string solutionUUID = createUUID();

	std::ofstream solution("scummvm.sln");
	if (!solution)
		throw std::string("Could not open \"scummvm.sln\" for writing");

	solution << "Microsoft Visual Studio Solution File, Format Version " << version + 1 << ".00\n";
	if (version == 9)
		solution << "# Visual Studio 2008\n";
	else if (version == 8)
		solution << "# Visual Studio 2005\n";
	else
		throw std::string("Unsupported version passed to createScummVMSolution");

	solution << "Project(\"{" << solutionUUID << "}\") = \"scummvm\", \"scummvm.vcproj\", \"{" << svmProjectUUID << "}\"\n"
	         << "\tProjectSection(ProjectDependencies) = postProject\n";
	for (UUIDMap::const_iterator i = uuids.begin(); i != uuids.end(); ++i) {
		if (i->first == "scummvm")
			continue;

		solution << "\t\t{" << i->second << "} = {" << i->second << "}\n";
	}

	solution << "\tEndProjectSection\n"
	         << "EndProject\n";

	// Note we assume that the UUID map only includes UUIDs for enabled engines!
	for (UUIDMap::const_iterator i = uuids.begin(); i != uuids.end(); ++i) {
		if (i->first == "scummvm")
			continue;

		solution << "Project(\"{" << solutionUUID << "}\") = \"" << i->first << "\", \"" << i->first << ".vcproj\", \"{" << i->second << "}\"\n"
		         << "EndProject\n";
	}

	solution << "Global\n"
	            "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
	            "\t\tDebug|Win32 = Debug|Win32\n"
	            "\t\tRelease|Win32 = Release|Win32\n"
	            "\tEndGlobalSection\n"
	            "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n";

	for (UUIDMap::const_iterator i = uuids.begin(); i != uuids.end(); ++i) {
		solution << "\t\t{" << i->second << "}.Debug|Win32.ActiveCfg = Debug|Win32\n"
		         << "\t\t{" << i->second << "}.Debug|Win32.Build.0 = Debug|Win32\n"
		         << "\t\t{" << i->second << "}.Release|Win32.ActiveCfg = Release|Win32\n"
		         << "\t\t{" << i->second << "}.Release|Win32.Build.0 = Release|Win32\n";
	}

	solution << "\tEndGlobalSection\n"
	            "\tGlobalSection(SolutionProperties) = preSolution\n"
	            "\t\tHideSolutionNode = FALSE\n"
	            "\tEndGlobalSection\n"
	            "EndGlobal\n";
}

void createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
					   const StringList &includeList, const StringList &excludeList, const int version) {
	const std::string projectFile = name + ".vcproj";
	std::ofstream project(projectFile.c_str());
	if (!project)
		throw std::string("Could not open \"" + projectFile + "\" for writing");

	project << "<?xml version=\"1.0\" encoding=\"windows-1252\"?>\n"
	           "<VisualStudioProject\n"
	           "\tProjectType=\"Visual C++\"\n"
	           "\tVersion=\"" << version << ".00\"\n"
	           "\tName=\"" << name << "\"\n"
	           "\tProjectGUID=\"{" << uuid << "}\"\n"
	           "\tRootNamespace=\"" << name << "\"\n"
	           "\tKeyword=\"Win32Proj\"\n";

	if (version >= 9)
		project << "\tTargetFrameworkVersion=\"131072\"\n";

	project << "\t>\n"
	           "\t<Platforms>\n"
	           "\t\t<Platform Name=\"Win32\" />\n"
			   "\t</Platforms>\n"
	           "\t<Configurations>\n";

	if (name == "scummvm") {
		std::string libraries;

		for (StringList::const_iterator i = setup.libraries.begin(); i != setup.libraries.end(); ++i)
			libraries += ' ' + *i;

		project << "\t\t<Configuration Name=\"Debug|Win32\" ConfigurationType=\"1\" InheritedPropertySheets=\".\\ScummVM_Debug.vsprops\">\n"
		           "\t\t\t<Tool\tName=\"VCLinkerTool\" OutputFile=\"$(OutDir)/scummvm.exe\"\n"
				   "\t\t\t\tAdditionalDependencies=\"" << libraries << "\"\n"
				   "\t\t\t/>\n"
				   "\t\t</Configuration>\n"
				   "\t\t<Configuration Name=\"Release|Win32\" ConfigurationType=\"1\" InheritedPropertySheets=\".\\ScummVM_Release.vsprops\">\n"
		           "\t\t\t<Tool\tName=\"VCLinkerTool\" OutputFile=\"$(OutDir)/scummvm.exe\"\n"
		           "\t\t\t\tAdditionalDependencies=\"" << libraries << "\"\n"
		           "\t\t\t/>\n"
		           "\t\t</Configuration>\n";
	} else if (name == "tinsel") {
		project << "\t\t<Configuration Name=\"Debug|Win32\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Debug.vsprops\">\n"
		           "\t\t\t<Tool Name=\"VCCLCompilerTool\" DebugInformationFormat=\"3\" />\n"
				   "\t\t</Configuration>\n"
	               "\t\t<Configuration Name=\"Release|Win32\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Release.vsprops\" />\n";
	} else {
		project << "\t\t<Configuration Name=\"Debug|Win32\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Debug.vsprops\" />\n"
	               "\t\t<Configuration Name=\"Release|Win32\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Release.vsprops\" />\n";
	}
	project << "\t</Configurations>\n"
	           "\t<Files>\n";

	addFilesToProject(moduleDir, project, includeList, excludeList);

	project << "\t</Files>\n"
	           "</VisualStudioProject>\n";
}

void createGlobalProp(const BuildSetup &setup, const int /*version*/) {
	std::ofstream properties("ScummVM_Global.vsprops");
	if (!properties)
		throw std::string("Could not open \"ScummVM_Global.vsprops\" for writing");

	std::string defines;
	for (StringList::const_iterator i = setup.defines.begin(); i != setup.defines.end(); ++i) {
		if (i != setup.defines.begin())
			defines += ';';
		defines += *i;
	}

	properties << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
	              "<VisualStudioPropertySheet\n"
	              "\tProjectType=\"Visual C++\"\n"
	              "\tVersion=\"8.00\"\n"
	              "\tName=\"ScummVM_Global\"\n"
	              "\tOutputDirectory=\"$(ConfigurationName)\"\n"
	              "\tIntermediateDirectory=\"$(ConfigurationName)/$(ProjectName)\"\n"
	              "\t>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCCLCompilerTool\"\n"
	              "\t\tDisableSpecificWarnings=\"4068;4100;4103;4121;4127;4189;4201;4221;4244;4250;4310;4351;4355;4510;4511;4512;4610;4701;4702;4706;4800;4996\"\n"
	              "\t\tAdditionalIncludeDirectories=\"" << setup.srcDir << ";" << setup.srcDir << "\\engines\"\n"
	              "\t\tPreprocessorDefinitions=\"" << defines << "\"\n"
	              "\t\tExceptionHandling=\"0\"\n"
	              "\t\tRuntimeTypeInfo=\"false\"\n"
	              "\t\tWarningLevel=\"4\"\n"
	              "\t\tWarnAsError=\"false\"\n"
	              "\t\tCompileAs=\"0\"\n"
	              "\t\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCLibrarianTool\"\n"
	              "\t\tIgnoreDefaultLibraryNames=\"\"\n"
	              "\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCLinkerTool\"\n"
	              "\t\tIgnoreDefaultLibraryNames=\"\"\n"
	              "\t\tSubSystem=\"1\"\n"
	              "\t\tEntryPointSymbol=\"WinMainCRTStartup\"\n"
	              "\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCResourceCompilerTool\"\n"
	              "\t\tPreprocessorDefinitions=\"HAS_INCLUDE_SET\"\n"
	              "\t\tAdditionalIncludeDirectories=\"" << setup.srcDir << "\"\n"
	              "\t/>\n"
	              "</VisualStudioPropertySheet>\n";
}

void createBuildProp(const BuildSetup &/*setup*/, const int /*version*/) {
	std::ofstream properties("ScummVM_Debug.vsprops");
	if (!properties)
		throw std::string("Could not open \"ScummVM_Debug.vsprops\" for writing");

	properties << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
	              "<VisualStudioPropertySheet\n"
	              "\tProjectType=\"Visual C++\"\n"
	              "\tVersion=\"8.00\"\n"
	              "\tName=\"ScummVM_Debug32\"\n"
	              "\tInheritedPropertySheets=\".\\ScummVM_Global.vsprops\"\n"
	              "\t>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCCLCompilerTool\"\n"
	              "\t\tOptimization=\"0\"\n"
	              "\t\tPreprocessorDefinitions=\"WIN32\"\n"
	              "\t\tMinimalRebuild=\"true\"\n"
	              "\t\tBasicRuntimeChecks=\"3\"\n"
	              "\t\tRuntimeLibrary=\"1\"\n"
	              "\t\tEnableFunctionLevelLinking=\"true\"\n"
	              "\t\tWarnAsError=\"false\"\n"
	              "\t\tDebugInformationFormat=\"4\"\n"
	              "\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCLinkerTool\"\n"
	              "\t\tLinkIncremental=\"2\"\n"
	              "\t\tGenerateDebugInformation=\"true\"\n"
	              "\t\tIgnoreDefaultLibraryNames=\"libcmt.lib\"\n"
	              "\t/>\n"
	              "</VisualStudioPropertySheet>\n";

	properties.flush();
	properties.close();

	properties.open("ScummVM_Release.vsprops");
	if (!properties)
		throw std::string("Could not open \"ScummVM_Release.vsprops\" for writing");

	properties << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
	              "<VisualStudioPropertySheet\n"
	              "\tProjectType=\"Visual C++\"\n"
	              "\tVersion=\"8.00\"\n"
	              "\tName=\"ScummVM_Release32\"\n"
	              "\tInheritedPropertySheets=\".\\ScummVM_Global.vsprops\"\n"
	              "\t>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCCLCompilerTool\"\n"
	              "\t\tEnableIntrinsicFunctions=\"true\"\n"
	              "\t\tWholeProgramOptimization=\"true\"\n"
	              "\t\tPreprocessorDefinitions=\"WIN32\"\n"
	              "\t\tStringPooling=\"true\"\n"
	              "\t\tBufferSecurityCheck=\"false\"\n"
	              "\t\tDebugInformationFormat=\"0\"\n"
	              "\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCLinkerTool\"\n"
	              "\t\tLinkIncremental=\"1\"\n"
	              "\t\tIgnoreDefaultLibraryNames=\"\"\n"
	              "\t\tSetChecksum=\"true\"\n"
	              "\t/>\n"
	              "</VisualStudioPropertySheet>\n";
}

/**
 * Gets a proper sequence of \t characters for the given
 * indentation level.
 *
 * For example with an indentation level of 2 this will
 * produce:
 *  \t\t
 *
 * @param indentation The indentation level
 * @return Sequence of \t characters.
 */
std::string getIndent(const int indentation) {
	std::string result;
	for (int i = 0; i < indentation; ++i)
		result += '\t';
	return result;
}

/**
 * Splits a file name into name and extension.
 * The file name must be only the filename, no
 * additional path name.
 *
 * @param fileName Filename to split
 * @param name Reference to a string, where to store the name.
 * @param ext Reference to a string, where to store the extension.
 */
void splitFilename(const std::string &fileName, std::string &name, std::string &ext) {
	const std::string::size_type dot = fileName.find_last_of('.');
	name = (dot == std::string::npos) ? fileName : fileName.substr(0, dot);
	ext = (dot == std::string::npos) ? std::string() : fileName.substr(dot + 1);
}

/**
 * Checks whether the given file will produce an object file or not.
 *
 * @param fileName Name of the file.
 * @return "true" when it will produce a file, "false" otherwise.
 */
bool producesObjectFile(const std::string &fileName) {
	std::string n, ext;
	splitFilename(fileName, n, ext);

	if (ext == "cpp" || ext == "c" || ext == "asm")
		return true;
	else
		return false;
}

/**
 * Checks whether the give file in the specified directory is present in the given
 * file list.
 *
 * This function does as special match against the file list. It will not take file
 * extensions into consideration, when the extension of a file in the specified
 * directory is one of "h", "cpp", "c" or "asm".
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

		const std::string lastPathComponent = getLastPathComponent(*i);
		if (!producesObjectFile(fileName) && extensionName != "h") {
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
 * Structure representing a file tree. This contains two
 * members: name and children. "name" holds the name of
 * the node. "children" does contain all the node's children.
 * When the list "children" is empty, the node is a file entry,
 * otherwise it's a directory.
 */
struct FileNode {
	typedef std::list<FileNode *> NodeList;

	FileNode(const std::string &n) : name(n), children() {}

	~FileNode() {
		for (NodeList::iterator i = children.begin(); i != children.end(); ++i)
			delete *i;
	}

	std::string name;  ///< Name of the node
	NodeList children; ///< List of children for the node
};

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
	WIN32_FIND_DATA fileInformation;
	HANDLE fileHandle = FindFirstFile((dir + "/*").c_str(), &fileInformation);

	if (fileHandle == INVALID_HANDLE_VALUE)
		return 0;

	FileNode *result = new FileNode(dir);

	do {
		if (fileInformation.cFileName[0] == '.')
			continue;

		if ((fileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			const std::string subDirName = dir + '/' + fileInformation.cFileName;
			if (!isInList(subDirName, std::string(), includeList))
				continue;

			FileNode *subDir = scanFiles(subDirName, includeList, excludeList);

			if (subDir) {
				subDir->name = fileInformation.cFileName;
				result->children.push_back(subDir);
			}
			continue;
		}

		if (isInList(dir, fileInformation.cFileName, excludeList))
			continue;

		std::string name, ext;
		splitFilename(fileInformation.cFileName, name, ext);

		if (ext != "h") {
			if (!isInList(dir, fileInformation.cFileName, includeList))
				continue;
		}

		result->children.push_back(new FileNode(fileInformation.cFileName));
	} while (FindNextFile(fileHandle, &fileInformation) == TRUE);

	CloseHandle(fileHandle);

	if (result->children.empty()) {
		delete result;
		return 0;
	} else {
		result->children.sort(compareNodes);
		return result;
	}
}

/**
 * Writes file entries for the specified directory node into
 * the given project file. It will also take care of duplicate
 * object files.
 *
 * @param dir Directory node.
 * @param projectFile File stream to write to.
 * @param indentation Indentation level to use.
 * @param duplicate List of duplicate object file names.
 * @param objPrefix Prefix to use for object files, which would name clash.
 * @param filePrefix Generic prefix to all files of the node.
 */
void writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
							const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix) {
	const std::string indentString = getIndent(indentation + 2);

	if (indentation)
		projectFile << getIndent(indentation + 1) << "<Filter\tName=\"" << dir.name << "\">\n";

	for (FileNode::NodeList::const_iterator i = dir.children.begin(); i != dir.children.end(); ++i) {
		const FileNode *node = *i;

		if (!node->children.empty()) {
			writeFileListToProject(*node, projectFile, indentation + 1, duplicate, objPrefix + node->name + '_', filePrefix + node->name + '/');
		} else {
			if (producesObjectFile(node->name)) {
				std::string name, ext;
				splitFilename(node->name, name, ext);
				const bool isDuplicate = (std::find(duplicate.begin(), duplicate.end(), name + ".o") != duplicate.end());

				if (ext == "asm") {
					std::string objFileName = "$(IntDir)\\";
					if (isDuplicate)
						objFileName += objPrefix;
					objFileName += "$(InputName).obj";

					const std::string toolLine = indentString + "\t\t<Tool Name=\"VCCustomBuildTool\" CommandLine=\"nasm.exe -f win32 -g -o &quot;" + objFileName + "&quot; &quot;$(InputPath)&quot;&#x0D;&#x0A;\" Outputs=\"" + objFileName + "\" />\n";

					projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\">\n"
					            << indentString << "\t<FileConfiguration Name=\"Debug|Win32\">\n"
					            << toolLine
					            << indentString << "\t</FileConfiguration>\n"
					            << indentString << "\t<FileConfiguration Name=\"Release|Win32\">\n"
					            << toolLine
					            << indentString << "\t</FileConfiguration>\n"
					            << indentString << "</File>\n";
				} else {
					if (isDuplicate) {
						const std::string toolLine = indentString + "\t\t<Tool Name=\"VCCLCompilerTool\" ObjectFile=\"$(IntDir)\\" + objPrefix + "$(InputName).obj\" XMLDocumentationFileName=\"$(IntDir)\\" + objPrefix + "$(InputName).xdc\" />\n";

						projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\">\n"
						            << indentString << "\t<FileConfiguration Name=\"Debug|Win32\">\n"
						            << toolLine
						            << indentString << "\t</FileConfiguration>\n"
						            << indentString << "\t<FileConfiguration Name=\"Release|Win32\">\n"
						            << toolLine
						            << indentString << "\t</FileConfiguration>\n"
						            << indentString << "</File>\n";
					} else {
						projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\" />\n";
					}
				}
			} else {
				projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\" />\n";
			}
		}
	}

	if (indentation)
		projectFile << getIndent(indentation + 1) << "</Filter>\n";
}

void addFilesToProject(const std::string &dir, std::ofstream &projectFile,
					   const StringList &includeList, const StringList &excludeList) {
	// Check for duplicate object file names
	StringList duplicate;

	for (StringList::const_iterator i = includeList.begin(); i != includeList.end(); ++i) {
		const std::string fileName = getLastPathComponent(*i);
		
		// Leave out non object file names.
		if (fileName.size() < 2 || fileName.compare(fileName.size() - 2, 2, ".o"))
			continue;

		// Check whether an duplicate has been found yet
		if (std::find(duplicate.begin(), duplicate.end(), fileName) != duplicate.end())
			continue;

		// Search for duplicates
		StringList::const_iterator j = i; ++j;
		for (; j != includeList.end(); ++j) {
			if (fileName == getLastPathComponent(*j)) {
				duplicate.push_back(fileName);
				break;
			}
		}
	}

	FileNode *files = scanFiles(dir, includeList, excludeList);

	writeFileListToProject(*files, projectFile, 0, duplicate, std::string(), dir + '/');

	delete files;
}

void createModuleList(const std::string &moduleDir, const StringList &defines, StringList &includeList, StringList &excludeList) {
	const std::string moduleMkFile = moduleDir + "/module.mk";
	std::ifstream moduleMk(moduleMkFile.c_str());
	if (!moduleMk)
		throw std::string(moduleMkFile + " is not present");

	includeList.push_back(moduleMkFile);

	std::stack<bool> shouldInclude;
	shouldInclude.push(true);

	bool hadModule = false;
	std::string line;
	while (true) {
		std::getline(moduleMk, line);

		if (moduleMk.eof())
			break;

		if (moduleMk.fail())
			throw std::string("Failed while reading from " + moduleMkFile);

		TokenList tokens = tokenize(line);
		if (tokens.empty())
			continue;

		TokenList::const_iterator i = tokens.begin();
		if (*i == "MODULE") {
			if (hadModule)
				throw std::string("More than one MODULE definition in " + moduleMkFile);
			// Format: "MODULE := path/to/module"
			if (tokens.size() < 3)
				throw std::string("Malformed MODULE definition in " + moduleMkFile);
			++i;
			if (*i != ":=")
				throw std::string("Malformed MODULE definition in " + moduleMkFile);
			++i;

			std::string moduleRoot = unifyPath(*i);
			if (moduleDir.compare(moduleDir.size() - moduleRoot.size(), moduleRoot.size(), moduleRoot))
				throw std::string("MODULE root " + moduleRoot + " does not match base dir " + moduleDir);

			hadModule = true;
		} else if (*i == "MODULE_OBJS") {
			if (tokens.size() < 3)
				throw std::string("Malformed MODULE_OBJS definition in " + moduleMkFile);
			++i;

			// This is not exactly correct, for example an ":=" would usually overwrite
			// all already added files, but since we do only save the files inside
			// includeList or excludeList currently, we couldn't handle such a case easily.
			// (includeList and excludeList should always preserve their entries, not added
			// by this function, thus we can't just clear them on ":=" or "=").
			// But hopefully our module.mk files will never do such things anyway.
			if (*i != ":=" && *i != "+=" && *i != "=")
				throw std::string("Malformed MODULE_OBJS definition in " + moduleMkFile);

			++i;

			while (i != tokens.end()) {
				if (*i == "\\") {
					std::getline(moduleMk, line);
					tokens = tokenize(line);
					i = tokens.begin();
				} else {
					if (shouldInclude.top())
						includeList.push_back(moduleDir + "/" + unifyPath(*i));
					else
						excludeList.push_back(moduleDir + "/" + unifyPath(*i));
					++i;
				}
			}
		} else if (*i == "ifdef") {
			if (tokens.size() < 2)
				throw std::string("Malformed ifdef in " + moduleMkFile);
			++i;

			if (std::find(defines.begin(), defines.end(), *i) == defines.end())
				shouldInclude.push(false);
			else
				shouldInclude.push(true);
		} else if (*i == "ifndef") {
			if (tokens.size() < 2)
				throw std::string("Malformed ifndef in " + moduleMkFile);
			++i;

			if (std::find(defines.begin(), defines.end(), *i) == defines.end())
				shouldInclude.push(true);
			else
				shouldInclude.push(false);
		} else if (*i == "else") {
			shouldInclude.top() = !shouldInclude.top();
		} else if (*i == "endif") {
			if (shouldInclude.size() <= 1)
				throw std::string("endif without ifdef found in " + moduleMkFile);
			shouldInclude.pop();
		} else if (*i == "elif") {
			throw std::string("Unsupported operation 'elif' in " + moduleMkFile);
		} else if (*i == "ifeq") {
			//XXX
			shouldInclude.push(false);
		}
	}

	if (shouldInclude.size() != 1)
		throw std::string("Malformed file " + moduleMkFile);
}
} // End of anonymous namespace