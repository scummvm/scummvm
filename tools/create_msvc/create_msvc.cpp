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
#include <map>
#include <stack>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include <cassert>
#include <cctype>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <ctime>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#else
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

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
 * Structure for describing an FSNode. This is a very minimalistic
 * description, which includes everything we need.
 * It only contains the name of the node and whether it is a directory
 * or not.
 */
struct FSNode {
	FSNode() : name(), isDirectory(false) {}
	FSNode(const std::string &n, bool iD) : name(n), isDirectory(iD) {}

	std::string name; ///< Name of the file system node
	bool isDirectory; ///< Whether it is a directory or not
};

typedef std::list<FSNode> FileList;

class ProjectProvider {
public:
	typedef std::map<std::string, std::string> UUIDMap;

protected:
	const int _version;                                      ///< Target MSVC version
	std::string _globalWarnings;                             ///< Global warnings
	std::map<std::string, std::string> _projectWarnings;     ///< Per-project warnings

	UUIDMap _uuidMap;                                        ///< List of (project name, UUID) pairs

public:
	/**
	 * Instantiate new ProjectProvider class
	 *
	 * @param version Target MSVC version.
	 */
	ProjectProvider(const int version, std::string global_warnings, std::map<std::string, std::string> project_warnings);
	virtual ~ProjectProvider() {}

	/**
	 * Creates all MSVC build files: the solution
	 * for all projects, all projects itself and the
	 * global config files.
	 *
	 * @param setup Description of the desired build setup.
	 */
	void createMSVCProject(const BuildSetup &setup);

	/**
	 * Creates the main solution file "scummvm.sln" for a specific
	 * build setup.
	 *
	 * @param setup Description of the desired build.
	 */
	void createScummVMSolution(const BuildSetup &setup);

	/**
	 * Create a project file for the specified list of files.
	 *
	 * @param name Name of the project file.
	 * @param uuid UUID of the project file.
	 * @param setup Description of the desired build.
	 * @param moduleDir Path to the module.
	 * @param includeList Files to include (must have "moduleDir" as prefix).
	 * @param excludeList Files to exclude (must have "moduleDir" as prefix).
	 */
	virtual void createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
	                               const StringList &includeList, const StringList &excludeList) = 0;

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
	virtual void writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
	                                    const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix) = 0;

	/**
	 * Output a list of project references to the file stream
	 *
	 * @param output File stream to write to.
	 */
	virtual void writeReferences(std::ofstream &output) = 0;

	/**
	 * Outputs a property file based on the input parameters.
	 *
	 * It can be easily used to create different global properties files
	 * for a 64 bit and a 32 bit version. It will also take care that the
	 * two platform configurations will output their files into different
	 * directories.
	 *
	 * @param properties File stream in which to write the property settings.
	 * @param bits Number of bits the platform supports.
	 * @param defines Defines the platform needs to have set.
	 * @param prefix File prefix, used to add additional include paths.
	 */
	virtual void outputGlobalPropFile(std::ofstream &properties, int bits, const std::string &defines, const std::string &prefix) = 0;

	/**
	 * Generates the project properties for debug and release settings.
	 *
	 * @param setup Description of the desired build setup.
	 * @param isRelease Type of property file
	 * @param isWin32 Bitness of property file
	 */
	virtual void createBuildProp(const BuildSetup &setup, bool isRelease, bool isWin32) = 0;

	/**
	 * Get the file extension for project files
	 */
	virtual const char *getProjectExtension() = 0;

	/**
	 * Get the file extension for property files
	 */
	virtual const char *getPropertiesExtension() = 0;

	/**
	 * Get the Visual Studio version (used by the VS shell extension to launch the correct VS version)
	 */
	virtual int getVisualStudioVersion() = 0;

	/**
	 * Create the global project properties.
	 *
	 * @param setup Description of the desired build setup.
	 */
	void createGlobalProp(const BuildSetup &setup);

	/**
	 * Adds files of the specified directory recursively to given project file.
	 *
	 * @param dir Path to the directory.
	 * @param projectFile Output stream object, where all data should be written to.
	 * @param includeList Files to include (must have a relative directory as prefix).
	 * @param excludeList Files to exclude (must have a relative directory as prefix).
	 * @param filePrefix Prefix to use for relative path arguments.
	 */
	void addFilesToProject(const std::string &dir, std::ofstream &projectFile,
	                       const StringList &includeList, const StringList &excludeList,
	                       const std::string &filePrefix);

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
};

class VisualStudioProvider : public ProjectProvider {
public:
	VisualStudioProvider(const int version, std::string global_warnings, std::map<std::string, std::string> project_warnings);

	void createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
	                       const StringList &includeList, const StringList &excludeList);

	void writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
	                            const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix);

	void writeReferences(std::ofstream &output);

	void outputGlobalPropFile(std::ofstream &properties, int bits, const std::string &defines, const std::string &prefix);

	void createBuildProp(const BuildSetup &setup, bool isRelease, bool isWin32);

	const char *getProjectExtension();
	const char *getPropertiesExtension();
	int getVisualStudioVersion();
};

class MSBuildProvider : public ProjectProvider {
public:
	MSBuildProvider(const int version, std::string global_warnings, std::map<std::string, std::string> project_warnings);

	void createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
	                       const StringList &includeList, const StringList &excludeList);

	void outputProjectSettings(std::ofstream &project, const std::string &name, const BuildSetup &setup, bool isRelease, bool isWin32);

	void writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
	                            const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix);

	void writeReferences(std::ofstream &output);

	void outputGlobalPropFile(std::ofstream &properties, int bits, const std::string &defines, const std::string &prefix);

	void createBuildProp(const BuildSetup &setup, bool isRelease, bool isWin32);

	const char *getProjectExtension();
	const char *getPropertiesExtension();
	int getVisualStudioVersion();

private:
	struct FileEntry {
		std::string name;
		std::string path;
		std::string filter;
		std::string prefix;

		bool operator<(const FileEntry& rhs) {
			return path.compare(rhs.path) == -1;   // Not exactly right for alphabetical order, but good enough
		}
	};
	typedef std::list<FileEntry> FileEntries;

	std::list<std::string> _filters; // list of filters (we need to create a GUID for each filter id)
	FileEntries _compileFiles;
	FileEntries _includeFiles;
	FileEntries _otherFiles;
	FileEntries _asmFiles;
	FileEntries _resourceFiles;

	void computeFileList(const FileNode &dir, const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix);
	void createFiltersFile(const BuildSetup &setup, const std::string &name);
};

} // End of anonymous namespace

int main(int argc, char *argv[]) {
#if !(defined(_WIN32) || defined(WIN32))
	// Initialize random number generator for UUID creation
	std::srand(std::time(0));
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

	setup.engines = parseConfigure(setup.srcDir);

	if (setup.engines.empty()) {
		std::cout << "WARNING: No engines found in configure file or configure file missing in \"" << setup.srcDir << "\"\n";
		return 0;
	}

	setup.features = getAllFeatures();

	int msvcVersion = 9;
	// Parse command line arguments
	using std::cout;
	for (int i = 2; i < argc; ++i) {
		if (!std::strcmp(argv[i], "--list-engines")) {
			cout << " The following enables are available in the ScummVM source destribution\n"
			        " located at \"" << srcDir << "\":\n";

			cout << "   state  |       name      |     description\n\n";
			cout.setf(std::ios_base::left, std::ios_base::adjustfield);
			for (EngineDescList::const_iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
				cout << ' ' << (j->enable ? " enabled" : "disabled") << " | " << std::setw(15) << j->name << std::setw(0) << " | " << j->desc << "\n";
			cout.setf(std::ios_base::right, std::ios_base::adjustfield);

			return 0;
		} else if (!std::strcmp(argv[i], "--msvc-version")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"version\" parameter for \"--msvc-version\"!\n";
				return -1;
			}

			msvcVersion = atoi(argv[++i]);

			if (msvcVersion != 8 && msvcVersion != 9 && msvcVersion != 10) {
				std::cerr << "ERROR: Unsupported version: \"" << msvcVersion << "\" passed to \"--msvc-version\"!\n";
				return -1;
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
			} else if (!setEngineBuildState(name, setup.engines, true)) {
				// If none found, we'll try the features list
				if (!setFeatureBuildState(name, setup.features, true)) {
					std::cerr << "ERROR: \"" << name << "\" is neither an engine nor a feature!\n";
					return -1;
				}
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
			} else if (!setEngineBuildState(name, setup.engines, false)) {
				// If none found, we'll try the features list
				if (!setFeatureBuildState(name, setup.features, false)) {
					std::cerr << "ERROR: \"" << name << "\" is neither an engine nor a feature!\n";
					return -1;
				}
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
				std::cerr << "ERROR: Missing \"path\" parameter for \"--output-dirx\"!\n";
				return -1;
			}

			setup.outputDir = unifyPath(argv[++i]);
			if (setup.outputDir.at(setup.outputDir.size() - 1) == '/')
				setup.outputDir.erase(setup.outputDir.size() - 1);
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


	// List of global warnings and a map for project-specific warnings
	std::string globalWarnings;
	std::map<std::string, std::string> projectWarnings;

	////////////////////////////////////////////////////////////////////////////
	// Initialize global & project-specific warnings
	//
	// Tracker reference:
	// https://sourceforge.net/tracker/?func=detail&aid=2909981&group_id=37116&atid=418822
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
	// 4310 (cast truncates constant value)
	//   used in some engines
	//
	// 4351 (new behavior: elements of array 'array' will be default initialized)
	//   a change in behavior in Visual Studio 2005. We want the new behavior, so it can be disabled
	//
	// 4512 ('class' : assignment operator could not be generated)
	//   some classes use const items and the default assignment operator cannot be generated
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
	////////////////////////////////////////////////////////////////////////////
	//
	// 4189 (local variable is initialized but not referenced)
	//   false positive in lure engine
	//
	// 4355 ('this' : used in base member initializer list)
	//   only disabled for specific engines where it is used in a safe way
	//
	// 4510 ('class' : default constructor could not be generated)
	//
	// 4610 (object 'class' can never be instantiated - user-defined constructor required)
	//   "correct" but harmless (as is 4510)
	//
	////////////////////////////////////////////////////////////////////////////

	globalWarnings = "4068;4100;4103;4127;4244;4250;4310;4351;4512;4702;4706;4800;4996";

	projectWarnings["agi"] = "4510;4610";
	projectWarnings["lure"] = "4189;4355";
	projectWarnings["kyra"] = "4355";
	projectWarnings["m4"] = "4355";

	ProjectProvider *provider = NULL;

	if (msvcVersion == 8 || msvcVersion == 9)
		provider = new VisualStudioProvider(msvcVersion, globalWarnings, projectWarnings);
	else
		provider = new MSBuildProvider(msvcVersion, globalWarnings, projectWarnings);

	provider->createMSVCProject(setup);

	delete provider;
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
	        " --msvc-version version   set the targeted MSVC version. Possible values:\n"
	        "                           8 stands for \"Visual Studio 2005\"\n"
	        "                           9 stands for \"Visual Studio 2008\"\n"
	        "                           10 stands for \"Visual Studio 2010\" (Experimental)\n"
	        "                           The default is \"9\", thus \"Visual Studio 2008\"\n"
	        " --file-prefix prefix     allow overwriting of relative file prefix in the\n"
	        "                          MSVC project files. By default the prefix is the\n"
	        "                          \"path\\to\\source\" argument\n"
	        " --output-dir path        overwrite path, where the project files are placed\n"
	        "                          By default this is \".\", i.e. the current working\n"
	        "                          directory\n"
	        "\n"
	        "ScummVM engine settings:\n"
	        " --list-engines           list all available engines and their default state\n"
	        " --enable-engine          enable building of the engine with the name \"engine\"\n"
	        " --disable-engine         disable building of the engine with the name \"engine\"\n"
	        " --enable-all-engines     enable building of all engines\n"
	        " --disable-all-engines    disable building of all engines\n"
	        "\n"
	        "ScummVM optional feature settings:\n"
	        " --enable-name            enable inclusion of the feature \"name\"\n"
	        " --disable-name           disable inclusion of the feature \"name\"\n"
	        "\n"
	        " There are the following features available:\n"
	        "\n";

	cout << "   state  |       name      |     description\n\n";
	const FeatureList features = getAllFeatures();
	cout.setf(std::ios_base::left, std::ios_base::adjustfield);
	for (FeatureList::const_iterator i = features.begin(); i != features.end(); ++i)
		cout << ' ' << (i->enable ? " enabled" : "disabled") << " | " << std::setw(15) << i->name << std::setw(0) << " | " << i->description << '\n';
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
			error("Failed while reading from " + configureFile);

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
	{    "flac",      "USE_FLAC", "libFLAC_static.lib", true, "FLAC support" },
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

bool setFeatureBuildState(const std::string &name, FeatureList &features, bool enable) {
	FeatureList::iterator i = std::find(features.begin(), features.end(), name);
	if (i != features.end()) {
		i->enable = enable;
		return true;
	} else {
		return false;
	}
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
#if defined(_WIN32) || defined(WIN32)
	UUID uuid;
	if (UuidCreate(&uuid) != RPC_S_OK)
		error("UuidCreate failed");

	unsigned char *string = 0;
	if (UuidToStringA(&uuid, &string) != RPC_S_OK)
		error("UuidToStringA failed");

	std::string result = std::string((char *)string);
	std::transform(result.begin(), result.end(), result.begin(), toupper);
	RpcStringFreeA(&string);
	return result;
#else
	unsigned char uuid[16];

	for (int i = 0; i < 16; ++i)
		uuid[i] = (unsigned char)((std::rand() / (double)(RAND_MAX)) * 0xFF);

	uuid[8] &= 0xBF; uuid[8] |= 0x80;
	uuid[6] &= 0x4F; uuid[6] |= 0x40;

	std::stringstream uuidString;
	uuidString << std::hex << std::uppercase << std::setfill('0');
	for (int i = 0; i < 16; ++i) {
		uuidString << std::setw(2) << (int)uuid[i];
		if (i == 3 || i == 5 || i == 7 || i == 9) {
			uuidString << std::setw(0) << '-';
		}
	}

	return uuidString.str();
#endif
}

//////////////////////////////////////////////////////////////////////////
// Project Provider methods
//////////////////////////////////////////////////////////////////////////
ProjectProvider::ProjectProvider(const int version, std::string global_warnings, std::map<std::string, std::string> project_warnings)
	: _version(version), _globalWarnings(global_warnings), _projectWarnings(project_warnings) {
}

void ProjectProvider::createMSVCProject(const BuildSetup &setup) {
	_uuidMap = createUUIDMap(setup);

	// We also need to add the UUID of the main project file.
	const std::string svmUUID = _uuidMap["scummvm"] = createUUID();

	createScummVMSolution(setup);

	StringList in, ex;

	// Create engine project files
	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == "scummvm")
			continue;

		in.clear(); ex.clear();
		const std::string moduleDir = setup.srcDir + "/engines/" + i->first;

		createModuleList(moduleDir, setup.defines, in, ex);
		createProjectFile(i->first, i->second, setup, moduleDir, in, ex);
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

	// Create the scummvm project file.
	createProjectFile("scummvm", svmUUID, setup, setup.srcDir, in, ex);

	// Create the global property file
	createGlobalProp(setup);

	// Create the configuration property files (for Debug and Release with 32 and 64bits versions)
	createBuildProp(setup, true, false);
	createBuildProp(setup, true, true);
	createBuildProp(setup, false, false);
	createBuildProp(setup, false, true);
}

void ProjectProvider::createScummVMSolution(const BuildSetup &setup) {
	UUIDMap::const_iterator svmUUID = _uuidMap.find("scummvm");
	if (svmUUID == _uuidMap.end())
		error("No UUID for \"scummvm\" project created");

	const std::string svmProjectUUID = svmUUID->second;
	assert(!svmProjectUUID.empty());

	std::string solutionUUID = createUUID();

	std::ofstream solution((setup.outputDir + '/' + "scummvm.sln").c_str());
	if (!solution)
		error("Could not open \"" + setup.outputDir + '/' + "scummvm.sln\" for writing");

	solution << "Microsoft Visual Studio Solution File, Format Version " << _version + 1 << ".00\n";
	solution << "# Visual Studio " << getVisualStudioVersion() << "\n";

	solution << "Project(\"{" << solutionUUID << "}\") = \"scummvm\", \"scummvm" << getProjectExtension() << "\", \"{" << svmProjectUUID << "}\"\n";

	// Project dependencies are moved to vcxproj files in Visual Studio 2010
	if (_version < 10)
		writeReferences(solution);

	solution << "EndProject\n";

	// Note we assume that the UUID map only includes UUIDs for enabled engines!
	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == "scummvm")
			continue;

		solution << "Project(\"{" << solutionUUID << "}\") = \"" << i->first << "\", \"" << i->first << getProjectExtension() << "\", \"{" << i->second << "}\"\n"
		         << "EndProject\n";
	}

	solution << "Global\n"
	            "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
	            "\t\tDebug|Win32 = Debug|Win32\n"
	            "\t\tRelease|Win32 = Release|Win32\n"
	             "\t\tDebug|x64 = Debug|x64\n"
	            "\t\tRelease|x64 = Release|x64\n"
	            "\tEndGlobalSection\n"
	            "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n";

	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		solution << "\t\t{" << i->second << "}.Debug|Win32.ActiveCfg = Debug|Win32\n"
		         << "\t\t{" << i->second << "}.Debug|Win32.Build.0 = Debug|Win32\n"
		         << "\t\t{" << i->second << "}.Release|Win32.ActiveCfg = Release|Win32\n"
		         << "\t\t{" << i->second << "}.Release|Win32.Build.0 = Release|Win32\n"
		         << "\t\t{" << i->second << "}.Debug|x64.ActiveCfg = Debug|x64\n"
		         << "\t\t{" << i->second << "}.Debug|x64.Build.0 = Debug|x64\n"
		         << "\t\t{" << i->second << "}.Release|x64.ActiveCfg = Release|x64\n"
		         << "\t\t{" << i->second << "}.Release|x64.Build.0 = Release|x64\n";
	}

	solution << "\tEndGlobalSection\n"
	            "\tGlobalSection(SolutionProperties) = preSolution\n"
	            "\t\tHideSolutionNode = FALSE\n"
	            "\tEndGlobalSection\n"
	            "EndGlobal\n";
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
 * Returns a list of all files and directories in the specified
 * path.
 *
 * @param dir Directory which should be listed.
 * @return List of all children.
 */
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
#endif
	return result;
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

void ProjectProvider::createGlobalProp(const BuildSetup &setup) {
	std::ofstream properties((setup.outputDir + '/' + "ScummVM_Global" + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + "ScummVM_Global" + getPropertiesExtension() + "\" for writing");

	std::string defines;
	for (StringList::const_iterator i = setup.defines.begin(); i != setup.defines.end(); ++i) {
		if (i != setup.defines.begin())
			defines += ';';
		defines += *i;
	}

	outputGlobalPropFile(properties, 32, defines, convertPathToWin(setup.filePrefix));
	properties.close();

	properties.open((setup.outputDir + '/' + "ScummVM_Global64" + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + "ScummVM_Global64" + getPropertiesExtension() + "\" for writing");

	// HACK: We must disable the "nasm" feature for x64. To achieve that we must duplicate the feature list and
	// recreate a define list.
	FeatureList x64Features = setup.features;
	setFeatureBuildState("nasm", x64Features, false);
	StringList x64Defines = getFeatureDefines(x64Features);
	StringList x64EngineDefines = getEngineDefines(setup.engines);
	x64Defines.splice(x64Defines.end(), x64EngineDefines);

	defines.clear();
	for (StringList::const_iterator i = x64Defines.begin(); i != x64Defines.end(); ++i) {
		if (i != x64Defines.begin())
			defines += ';';
		defines += *i;
	}

	outputGlobalPropFile(properties, 64, defines, convertPathToWin(setup.filePrefix));
}

void ProjectProvider::addFilesToProject(const std::string &dir, std::ofstream &projectFile,
                       const StringList &includeList, const StringList &excludeList,
                       const std::string &filePrefix) {
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

	writeFileListToProject(*files, projectFile, 0, duplicate, std::string(), filePrefix + '/');

	delete files;
}

void ProjectProvider::createModuleList(const std::string &moduleDir, const StringList &defines, StringList &includeList, StringList &excludeList) {
	const std::string moduleMkFile = moduleDir + "/module.mk";
	std::ifstream moduleMk(moduleMkFile.c_str());
	if (!moduleMk)
		error(moduleMkFile + " is not present");

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
				error("Malformed ifdef in " + moduleMkFile);
			++i;

			if (std::find(defines.begin(), defines.end(), *i) == defines.end())
				shouldInclude.push(false);
			else
				shouldInclude.push(true);
		} else if (*i == "ifndef") {
			if (tokens.size() < 2)
				error("Malformed ifndef in " + moduleMkFile);
			++i;

			if (std::find(defines.begin(), defines.end(), *i) == defines.end())
				shouldInclude.push(true);
			else
				shouldInclude.push(false);
		} else if (*i == "else") {
			shouldInclude.top() = !shouldInclude.top();
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

//////////////////////////////////////////////////////////////////////////
// Visual Studio Provider
//////////////////////////////////////////////////////////////////////////

VisualStudioProvider::VisualStudioProvider(const int version, std::string global_warnings, std::map<std::string, std::string> project_warnings)
	: ProjectProvider(version, global_warnings, project_warnings) {
}

const char *VisualStudioProvider::getProjectExtension() {
	return ".vcproj";
}

const char *VisualStudioProvider::getPropertiesExtension() {
	return ".vsprops";
}

int VisualStudioProvider::getVisualStudioVersion() {
	if (_version == 9)
		return 2008;

	if (_version == 8)
		return 2005;

	error("Unsupported version passed to createScummVMSolution");
	return 0;
}

void VisualStudioProvider::createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
                                             const StringList &includeList, const StringList &excludeList) {
	const std::string projectFile = setup.outputDir + '/' + name + getProjectExtension();
	std::ofstream project(projectFile.c_str());
	if (!project)
		error("Could not open \"" + projectFile + "\" for writing");

	project << "<?xml version=\"1.0\" encoding=\"windows-1252\"?>\n"
	           "<VisualStudioProject\n"
	           "\tProjectType=\"Visual C++\"\n"
	           "\tVersion=\"" << _version << ".00\"\n"
	           "\tName=\"" << name << "\"\n"
	           "\tProjectGUID=\"{" << uuid << "}\"\n"
	           "\tRootNamespace=\"" << name << "\"\n"
	           "\tKeyword=\"Win32Proj\"\n";

	if (_version >= 9)
		project << "\tTargetFrameworkVersion=\"131072\"\n";

	project << "\t>\n"
	           "\t<Platforms>\n"
	           "\t\t<Platform Name=\"Win32\" />\n"
	           "\t\t<Platform Name=\"x64\" />\n"
	           "\t</Platforms>\n"
	           "\t<Configurations>\n";

	// Check for project-specific warnings:
	std::map<std::string, std::string>::iterator warnings = _projectWarnings.find(name);

	if (name == "scummvm") {
		std::string libraries;

		for (StringList::const_iterator i = setup.libraries.begin(); i != setup.libraries.end(); ++i)
			libraries += ' ' + *i;

		// Win32
		project << "\t\t<Configuration Name=\"Debug|Win32\" ConfigurationType=\"1\" InheritedPropertySheets=\".\\ScummVM_Debug.vsprops\">\n"
		           "\t\t\t<Tool\tName=\"VCCLCompilerTool\" DisableLanguageExtensions=\"false\" />\n"
		           "\t\t\t<Tool\tName=\"VCLinkerTool\" OutputFile=\"$(OutDir)/scummvm.exe\"\n"
		           "\t\t\t\tAdditionalDependencies=\"" << libraries << "\"\n"
		           "\t\t\t/>\n"
		           "\t\t</Configuration>\n"
		           "\t\t<Configuration Name=\"Release|Win32\" ConfigurationType=\"1\" InheritedPropertySheets=\".\\ScummVM_Release.vsprops\">\n"
		           "\t\t\t<Tool\tName=\"VCCLCompilerTool\" DisableLanguageExtensions=\"false\" />\n"
		           "\t\t\t<Tool\tName=\"VCLinkerTool\" OutputFile=\"$(OutDir)/scummvm.exe\"\n"
		           "\t\t\t\tAdditionalDependencies=\"" << libraries << "\"\n"
		           "\t\t\t/>\n"
		           "\t\t</Configuration>\n";

		// x64
		// For 'x64' we must disable NASM support. Usually we would need to disable the "nasm" feature for that and
		// re-create the library list, BUT since NASM doesn't link any additional libraries, we can just use the
		// libraries list created for IA-32. If that changes in the future, we need to adjust this part!
		project << "\t\t<Configuration Name=\"Debug|x64\" ConfigurationType=\"1\" InheritedPropertySheets=\".\\ScummVM_Debug64.vsprops\">\n"
		           "\t\t\t<Tool\tName=\"VCCLCompilerTool\" DisableLanguageExtensions=\"false\" />\n"
		           "\t\t\t<Tool\tName=\"VCLinkerTool\" OutputFile=\"$(OutDir)/scummvm.exe\"\n"
		           "\t\t\t\tAdditionalDependencies=\"" << libraries << "\"\n"
		           "\t\t\t/>\n"
		           "\t\t</Configuration>\n"
		           "\t\t<Configuration Name=\"Release|x64\" ConfigurationType=\"1\" InheritedPropertySheets=\".\\ScummVM_Release64.vsprops\">\n"
		           "\t\t\t<Tool\tName=\"VCCLCompilerTool\" DisableLanguageExtensions=\"false\" />\n"
		           "\t\t\t<Tool\tName=\"VCLinkerTool\" OutputFile=\"$(OutDir)/scummvm.exe\"\n"
		           "\t\t\t\tAdditionalDependencies=\"" << libraries << "\"\n"
		           "\t\t\t/>\n"
		           "\t\t</Configuration>\n";
	} else if (warnings != _projectWarnings.end()) {
		// Win32
		project << "\t\t<Configuration Name=\"Debug|Win32\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Debug.vsprops\">\n"
		           "\t\t\t<Tool Name=\"VCCLCompilerTool\" DisableSpecificWarnings=\"" << warnings->second << "\" />\n"
		           "\t\t</Configuration>\n"
		           "\t\t<Configuration Name=\"Release|Win32\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Release.vsprops\">\n"
		           "\t\t\t<Tool Name=\"VCCLCompilerTool\" DisableSpecificWarnings=\"" << warnings->second << "\" />\n"
		           "\t\t</Configuration>\n";
		// x64
		project << "\t\t<Configuration Name=\"Debug|x64\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Debug64.vsprops\">\n"
		           "\t\t\t<Tool Name=\"VCCLCompilerTool\" DisableSpecificWarnings=\"" << warnings->second << "\" />\n"
		           "\t\t</Configuration>\n"
		           "\t\t<Configuration Name=\"Release|x64\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Release64.vsprops\">\n"
		           "\t\t\t<Tool Name=\"VCCLCompilerTool\" DisableSpecificWarnings=\"" << warnings->second << "\" />\n"
		           "\t\t</Configuration>\n";
	} else if (name == "tinsel") {
		// Win32
		project << "\t\t<Configuration Name=\"Debug|Win32\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Debug.vsprops\">\n"
		           "\t\t\t<Tool Name=\"VCCLCompilerTool\" DebugInformationFormat=\"3\" />\n"
		           "\t\t</Configuration>\n"
		           "\t\t<Configuration Name=\"Release|Win32\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Release.vsprops\" />\n";
		// x64
		project << "\t\t<Configuration Name=\"Debug|x64\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Debug64.vsprops\">\n"
		           "\t\t\t<Tool Name=\"VCCLCompilerTool\" DebugInformationFormat=\"3\" />\n"
		           "\t\t</Configuration>\n"
		           "\t\t<Configuration Name=\"Release|x64\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Release64.vsprops\" />\n";
	} else {
		// Win32
		project << "\t\t<Configuration Name=\"Debug|Win32\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Debug.vsprops\" />\n"
		           "\t\t<Configuration Name=\"Release|Win32\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Release.vsprops\" />\n";
		// x64
		project << "\t\t<Configuration Name=\"Debug|x64\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Debug64.vsprops\" />\n"
		           "\t\t<Configuration Name=\"Release|x64\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_Release64.vsprops\" />\n";
	}
	project << "\t</Configurations>\n"
	           "\t<Files>\n";

	std::string modulePath;
	if (!moduleDir.compare(0, setup.srcDir.size(), setup.srcDir)) {
		modulePath = moduleDir.substr(setup.srcDir.size());
		if (!modulePath.empty() && modulePath.at(0) == '/')
			modulePath.erase(0, 1);
	}

	if (modulePath.size())
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix + '/' + modulePath);
	else
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix);

	project << "\t</Files>\n"
	           "</VisualStudioProject>\n";
}

void VisualStudioProvider::writeReferences(std::ofstream &output) {
	output << "\tProjectSection(ProjectDependencies) = postProject\n";

	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == "scummvm")
			continue;

		output << "\t\t{" << i->second << "} = {" << i->second << "}\n";
	}

	output << "\tEndProjectSection\n";
}

void VisualStudioProvider::outputGlobalPropFile(std::ofstream &properties, int bits, const std::string &defines, const std::string &prefix) {
	properties << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
	              "<VisualStudioPropertySheet\n"
	              "\tProjectType=\"Visual C++\"\n"
	              "\tVersion=\"8.00\"\n"
	              "\tName=\"ScummVM_Global\"\n"
	              "\tOutputDirectory=\"$(ConfigurationName)" << bits << "\"\n"
	              "\tIntermediateDirectory=\"$(ConfigurationName)" << bits << "/$(ProjectName)\"\n"
	              "\t>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCCLCompilerTool\"\n"
	              "\t\tDisableLanguageExtensions=\"true\"\n"
	              "\t\tDisableSpecificWarnings=\"" << _globalWarnings << "\"\n"
	              "\t\tAdditionalIncludeDirectories=\"" << prefix << ";" << prefix << "\\engines\"\n"
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
	              "\t\tAdditionalIncludeDirectories=\"" << prefix << "\"\n"
	              "\t/>\n"
	              "</VisualStudioPropertySheet>\n";

	properties.flush();
}

void VisualStudioProvider::createBuildProp(const BuildSetup &setup, bool isRelease, bool isWin32) {
	const std::string outputType = (isRelease ? "Release" : "Debug");
	const std::string outputBitness = (isWin32 ? "32" : "64");

	std::ofstream properties((setup.outputDir + '/' + "ScummVM_" + outputType + (isWin32 ? "" : "64") + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + "ScummVM_" + outputType + (isWin32 ? "" : "64") + getPropertiesExtension() + "\" for writing");

	properties << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
	              "<VisualStudioPropertySheet\n"
	              "\tProjectType=\"Visual C++\"\n"
	              "\tVersion=\"8.00\"\n"
	              "\tName=\"ScummVM_" << outputType << outputBitness << "\"\n"
	              "\tInheritedPropertySheets=\".\\ScummVM_Global" << (isWin32 ? "" : "64") << ".vsprops\"\n"
	              "\t>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCCLCompilerTool\"\n";

	if (isRelease) {
		properties << "\t\tEnableIntrinsicFunctions=\"true\"\n"
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
		              "\t\tSetChecksum=\"true\"\n";
	} else {
		properties << "\t\tOptimization=\"0\"\n"
	              "\t\tPreprocessorDefinitions=\"WIN32\"\n"
	              "\t\tMinimalRebuild=\"true\"\n"
	              "\t\tBasicRuntimeChecks=\"3\"\n"
	              "\t\tRuntimeLibrary=\"1\"\n"
	              "\t\tEnableFunctionLevelLinking=\"true\"\n"
	              "\t\tWarnAsError=\"false\"\n"
		              "\t\tDebugInformationFormat=\"" << (isWin32 ? "4" : "3") << "\"\n"	// For x64 format "4" (Edit and continue) is not supported, thus we default to "3"
	              "\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCLinkerTool\"\n"
	              "\t\tLinkIncremental=\"2\"\n"
	              "\t\tGenerateDebugInformation=\"true\"\n"
		              "\t\tIgnoreDefaultLibraryNames=\"libcmt.lib\"\n";
	}

	properties << "\t/>\n"
	              "</VisualStudioPropertySheet>\n";

	properties.flush();
	properties.close();
}

void VisualStudioProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
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

					// NASM is not supported for x64, thus we do not need to add additional entries here :-).
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
						            << indentString << "\t<FileConfiguration Name=\"Debug|x64\">\n"
						            << toolLine
						            << indentString << "\t</FileConfiguration>\n"
						            << indentString << "\t<FileConfiguration Name=\"Release|x64\">\n"
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

//////////////////////////////////////////////////////////////////////////
// MSBuild Provider (Visual Studio 2010)
//////////////////////////////////////////////////////////////////////////

MSBuildProvider::MSBuildProvider(const int version, std::string global_warnings, std::map<std::string, std::string> project_warnings)
	: ProjectProvider(version, global_warnings, project_warnings) {

}

const char *MSBuildProvider::getProjectExtension() {
	return ".vcxproj";
}

const char *MSBuildProvider::getPropertiesExtension() {
	return ".props";
}

int MSBuildProvider::getVisualStudioVersion() {
	return 2010;
}

#define OUTPUT_CONFIGURATION_MSBUILD(config, platform) \
	project << "\t\t<ProjectConfiguration Include=\"" << config << "|" << platform << "\">\n" \
	           "\t\t\t<Configuration>" << config << "</Configuration>\n" \
	           "\t\t\t<Platform>" << platform << "</Platform>\n" \
	           "\t\t</ProjectConfiguration>\n"

#define OUTPUT_CONFIGURATION_TYPE_MSBUILD(config) \
	project << "\t<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config << "'\" Label=\"Configuration\">\n" \
	           "\t\t<ConfigurationType>" << (name == "scummvm" ? "Application" : "StaticLibrary") << "</ConfigurationType>\n" \
	           "\t</PropertyGroup>\n"

#define OUTPUT_PROPERTIES_MSBUILD(config, properties) \
	project << "\t<ImportGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config << "'\" Label=\"PropertySheets\">\n" \
	           "\t\t<Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" />\n" \
	           "\t\t<Import Project=\"" << properties << "\" />\n" \
	           "\t</ImportGroup>\n"

void MSBuildProvider::createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
                                        const StringList &includeList, const StringList &excludeList) {
	const std::string projectFile = setup.outputDir + '/' + name + getProjectExtension();
	std::ofstream project(projectFile.c_str());
	if (!project)
		error("Could not open \"" + projectFile + "\" for writing");

	project << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	           "<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	           "\t<ItemGroup Label=\"ProjectConfigurations\">\n";

	OUTPUT_CONFIGURATION_MSBUILD("Debug", "Win32");
	OUTPUT_CONFIGURATION_MSBUILD("Debug", "x64");
	OUTPUT_CONFIGURATION_MSBUILD("Release", "Win32");
	OUTPUT_CONFIGURATION_MSBUILD("Release", "x64");

	project << "\t</ItemGroup>\n";

	// Project name & Guid
	project << "\t<PropertyGroup Label=\"Globals\">\n"
	           "\t\t<ProjectGuid>" << uuid << "</ProjectGuid>\n"
	           "\t\t<RootNamespace>" << name << "</RootNamespace>\n"
	           "\t\t<Keyword>Win32Proj</Keyword>\n"
	           "\t</PropertyGroup>\n";

	// Shared configuration
	project << "\t<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n";

	OUTPUT_CONFIGURATION_TYPE_MSBUILD("Release|Win32");
	OUTPUT_CONFIGURATION_TYPE_MSBUILD("Debug|Win32");
	OUTPUT_CONFIGURATION_TYPE_MSBUILD("Release|x64");
	OUTPUT_CONFIGURATION_TYPE_MSBUILD("Debug|x64");

	project << "\t<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n"
	           "\t<ImportGroup Label=\"ExtensionSettings\">\n"
	           "\t</ImportGroup>\n";

	OUTPUT_PROPERTIES_MSBUILD("Release|Win32", "ScummVM_Release.props");
	OUTPUT_PROPERTIES_MSBUILD("Debug|Win32", "ScummVM_Debug.props");
	OUTPUT_PROPERTIES_MSBUILD("Release|x64", "ScummVM_Release64.props");
	OUTPUT_PROPERTIES_MSBUILD("Debug|x64", "ScummVM_Debug64.props");

	project << "\t<PropertyGroup Label=\"UserMacros\" />\n";

	// Project version number
	project << "\t<PropertyGroup>\n"
	           "\t\t<_ProjectFileVersion>10.0.21006.1</_ProjectFileVersion>\n"; // FIXME: update temporary entry _ProjectFileVersion

	if (name == "scummvm")
		project << "<ExecutablePath>$(SCUMMVM_LIBS)\\bin;$(VCInstallDir)bin;$(WindowsSdkDir)bin\\NETFX 4.0 Tools;$(WindowsSdkDir)bin;$(VSInstallDir)Common7\\Tools\\bin;$(VSInstallDir)Common7\\tools;$(VSInstallDir)Common7\\ide;$(ProgramFiles)\\HTML Help Workshop;$(FrameworkSDKDir)\\bin;$(MSBuildToolsPath32);$(VSInstallDir);$(SystemRoot)\\SysWow64;$(FxCopDir);$(PATH)</ExecutablePath>\n"
		           "<IncludePath>$(SCUMMVM_LIBS)\\include;$(VCInstallDir)include;$(VCInstallDir)atlmfc\\include;$(WindowsSdkDir)include;$(FrameworkSDKDir)\\include;</IncludePath>\n"
		           "<LibraryPath>$(SCUMMVM_LIBS)\\lib;$(VCInstallDir)lib;$(VCInstallDir)atlmfc\\lib;$(WindowsSdkDir)lib;$(FrameworkSDKDir)\\lib</LibraryPath>\n";

	project << "\t</PropertyGroup>\n";

	// Project-specific settings
	outputProjectSettings(project, name, setup, false, true);
	outputProjectSettings(project, name, setup, true, true);
	outputProjectSettings(project, name, setup, false, false);
	outputProjectSettings(project, name, setup, true, false);

	// Files
	std::string modulePath;
	if (!moduleDir.compare(0, setup.srcDir.size(), setup.srcDir)) {
		modulePath = moduleDir.substr(setup.srcDir.size());
		if (!modulePath.empty() && modulePath.at(0) == '/')
			modulePath.erase(0, 1);
	}

	if (modulePath.size())
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix + '/' + modulePath);
	else
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix);

	// Output references for scummvm project
	if (name == "scummvm")
		writeReferences(project);

	project << "\t<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
	           "\t<ImportGroup Label=\"ExtensionTargets\">\n"
	           "\t</ImportGroup>\n"
	           "</Project>\n";

	// Output filter file if necessary
	createFiltersFile(setup, name);
}

#define OUTPUT_FILTER_MSBUILD(files, action) \
	if (!files.empty()) { \
		filters << "\t<ItemGroup>\n"; \
		for (std::list<FileEntry>::const_iterator entry = files.begin(); entry != files.end(); ++entry) { \
			if ((*entry).filter != "") { \
				filters << "\t\t<" action " Include=\"" << (*entry).path << "\">\n" \
				           "\t\t\t<Filter>" << (*entry).filter << "</Filter>\n" \
				           "\t\t</" action ">\n"; \
			} else { \
				filters << "\t\t<" action " Include=\"" << (*entry).path << "\" />\n"; \
			} \
		} \
		filters << "\t</ItemGroup>\n"; \
	}

void MSBuildProvider::createFiltersFile(const BuildSetup &setup, const std::string &name) {
	// No filters => no need to create a filter file
	if (_filters.empty())
		return;

	// Sort all list alphabetically
	_filters.sort();
	_compileFiles.sort();
	_includeFiles.sort();
	_otherFiles.sort();
	_resourceFiles.sort();
	_asmFiles.sort();

	const std::string filtersFile = setup.outputDir + '/' + name + getProjectExtension() + ".filters";
	std::ofstream filters(filtersFile.c_str());
	if (!filters)
		error("Could not open \"" + filtersFile + "\" for writing");

	filters << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	           "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n";

	// Output the list of filters
	filters << "\t<ItemGroup>\n";
	for (std::list<std::string>::iterator filter = _filters.begin(); filter != _filters.end(); ++filter) {
		filters << "\t\t<Filter Include=\"" << *filter << "\">\n"
		           "\t\t\t<UniqueIdentifier>" << createUUID() << "</UniqueIdentifier>\n"
		           "\t\t</Filter>\n";
	}
	filters << "\t</ItemGroup>\n";

	// Output files
	OUTPUT_FILTER_MSBUILD(_compileFiles, "ClCompile")
	OUTPUT_FILTER_MSBUILD(_includeFiles, "ClInclude")
	OUTPUT_FILTER_MSBUILD(_otherFiles, "None")
	OUTPUT_FILTER_MSBUILD(_resourceFiles, "ResourceCompile")
	OUTPUT_FILTER_MSBUILD(_asmFiles, "CustomBuild")

	filters << "</Project>";
}

void MSBuildProvider::writeReferences(std::ofstream &output) {
	output << "\t<ItemGroup>\n";

	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == "scummvm")
			continue;

		output << "\t<ProjectReference Include=\"" << i->first << ".vcxproj\">\n"
		          "\t\t<Project>{" << i->second << "}</Project>\n"
		          "\t</ProjectReference>\n";
	}

	output << "\t</ItemGroup>\n";
}

void MSBuildProvider::outputProjectSettings(std::ofstream &project, const std::string &name, const BuildSetup &setup, bool isRelease, bool isWin32) {
	// Check for project-specific warnings:
	std::map<std::string, std::string>::iterator warnings = _projectWarnings.find(name);

	// Nothing to add here, move along!
	if (name != "scummvm" && name != "tinsel" && warnings == _projectWarnings.end())
		return;

	project << "\t<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='" << (isRelease ? "Release" : "Debug") << "|" << (isWin32 ? "Win32" : "x64") << "'\">\n"
	           "\t\t<ClCompile>\n";

	// Compile configuration
	if (name == "scummvm") {
		project << "\t\t\t<DisableLanguageExtensions>false</DisableLanguageExtensions>\n";
	} else {
		if (name == "tinsel" && !isRelease)
			project << "\t\t\t<DebugInformationFormat>ProgramDatabase</DebugInformationFormat>\n";

		if (warnings != _projectWarnings.end())
			project << "\t\t\t<DisableSpecificWarnings>" << warnings->second << ";%(DisableSpecificWarnings)</DisableSpecificWarnings>\n";
	}

	project << "\t\t</ClCompile>\n";

	// Link configuration for scummvm project
	if (name == "scummvm") {
		std::string libraries;

		for (StringList::const_iterator i = setup.libraries.begin(); i != setup.libraries.end(); ++i)
			libraries += *i + ';';

		project << "\t\t<Link>\n"
		           "\t\t\t<OutputFile>$(OutDir)scummvm.exe</OutputFile>\n"
		           "\t\t\t<AdditionalDependencies>" << libraries << "%(AdditionalDependencies)</AdditionalDependencies>\n"
		           "\t\t</Link>\n";
	}

	project << "\t</ItemDefinitionGroup>\n";
}

void MSBuildProvider::outputGlobalPropFile(std::ofstream &properties, int bits, const std::string &defines, const std::string &prefix) {
	// FIXME: update temporary entries _ProjectFileVersion & _PropertySheetDisplayName
	properties << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	              "<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	              "<PropertyGroup>\n"
	              "<_ProjectFileVersion>10.0.21006.1</_ProjectFileVersion>\n"
	              "<_PropertySheetDisplayName>ScummVM_Global</_PropertySheetDisplayName>\n"
	              "<OutDir>$(Configuration)" << bits << "\\</OutDir>\n"
	              "<IntDir>$(Configuration)" << bits << "/$(ProjectName)\\</IntDir>\n"
	              "</PropertyGroup>\n"
	              "<ItemDefinitionGroup>\n"
	              "<ClCompile>\n"
	              "<DisableLanguageExtensions>true</DisableLanguageExtensions>\n"
	              "<DisableSpecificWarnings>" << _globalWarnings << ";%(DisableSpecificWarnings)</DisableSpecificWarnings>\n"
	              "<AdditionalIncludeDirectories>" << prefix << ";" << prefix << "\\engines;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n"
	              "<PreprocessorDefinitions>" << defines << ";%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
	              "<ExceptionHandling>\n"
	              "</ExceptionHandling>\n"
	              "<RuntimeTypeInfo>false</RuntimeTypeInfo>\n"
	              "<WarningLevel>Level4</WarningLevel>\n"
	              "<TreatWarningAsError>false</TreatWarningAsError>\n"
	              "<CompileAs>Default</CompileAs>\n"
	              "</ClCompile>\n"
	              "<Link>\n"
	              "<IgnoreSpecificDefaultLibraries>%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>\n"
	              "<SubSystem>Console</SubSystem>\n"
	              "<EntryPointSymbol>WinMainCRTStartup</EntryPointSymbol>\n"
	              "</Link>\n"
	              "<ResourceCompile>\n"
	              "<PreprocessorDefinitions>HAS_INCLUDE_SET;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
	              "<AdditionalIncludeDirectories>" << prefix << ";%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n"
	              "</ResourceCompile>\n"
	              "</ItemDefinitionGroup>\n"
	              "</Project>\n";

	properties.flush();
}

void MSBuildProvider::createBuildProp(const BuildSetup &setup, bool isRelease, bool isWin32) {
	const std::string outputType = (isRelease ? "Release" : "Debug");
	const std::string outputBitness = (isWin32 ? "32" : "64");

	std::ofstream properties((setup.outputDir + '/' + "ScummVM_" + outputType + (isWin32 ? "" : "64") + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + "ScummVM_" + outputType + (isWin32 ? "" : "64") + getPropertiesExtension() + "\" for writing");

	// FIXME: update temporary entries _ProjectFileVersion & _PropertySheetDisplayName
	properties << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	              "<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	              "\t<ImportGroup Label=\"PropertySheets\">\n"
	              "\t\t<Import Project=\"ScummVM_Global" << (isWin32 ? "" : "64") << ".props\" />\n"
	              "\t</ImportGroup>\n"
	              "\t<PropertyGroup>\n"
	              "\t\t<_ProjectFileVersion>10.0.21006.1</_ProjectFileVersion>\n"
	              "\t\t<_PropertySheetDisplayName>ScummVM_" << outputType << outputBitness << "</_PropertySheetDisplayName>\n"
	              "\t\t<LinkIncremental>" << (isRelease ? "false" : "true") << "</LinkIncremental>\n"
	              "\t</PropertyGroup>\n"
	              "\t<ItemDefinitionGroup>\n"
	              "\t\t<ClCompile>\n";

	if (isRelease) {
		properties << "\t\t\t<IntrinsicFunctions>true</IntrinsicFunctions>\n"
		              "\t\t\t<WholeProgramOptimization>true</WholeProgramOptimization>\n"
		              "\t\t\t<PreprocessorDefinitions>WIN32;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
		              "\t\t\t<StringPooling>true</StringPooling>\n"
		              "\t\t\t<BufferSecurityCheck>false</BufferSecurityCheck>\n"
		              "\t\t\t<DebugInformationFormat></DebugInformationFormat>\n"
		              "\t\t</ClCompile>\n"
		              "\t\t<Link>\n"
		              "\t\t\t<IgnoreSpecificDefaultLibraries>%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>\n"
		              "\t\t\t<SetChecksum>true</SetChecksum>\n";
	} else {
		properties << "\t\t\t<Optimization>Disabled</Optimization>\n"
		              "\t\t\t<PreprocessorDefinitions>WIN32;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
		              "\t\t\t<MinimalRebuild>true</MinimalRebuild>\n"
		              "\t\t\t<BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>\n"
		              "\t\t\t<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>\n"
		              "\t\t\t<FunctionLevelLinking>true</FunctionLevelLinking>\n"
		              "\t\t\t<TreatWarningAsError>false</TreatWarningAsError>\n"
		              "\t\t\t<DebugInformationFormat>" << (isWin32 ? "EditAndContinue" : "ProgramDatabase") << "</DebugInformationFormat>\n" // For x64 format Edit and continue is not supported, thus we default to Program Database
		              "\t\t</ClCompile>\n"
		              "\t\t<Link>\n"
		              "\t\t\t<GenerateDebugInformation>true</GenerateDebugInformation>\n"
		              "\t\t\t<IgnoreSpecificDefaultLibraries>libcmt.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>\n";
	}

	properties << "\t\t</Link>\n"
	              "\t</ItemDefinitionGroup>\n"
	              "</Project>\n";

	properties.flush();
	properties.close();
}

#define OUTPUT_NASM_COMMAND_MSBUILD(config) \
	projectFile << "\t\t\t<Command Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|Win32'\">nasm.exe -f win32 -g -o \"$(IntDir)" << (isDuplicate ? (*entry).prefix : "") << "%(FileName).obj\" \"%(FullPath)\"</Command>\n" \
	               "\t\t\t<Outputs Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|Win32'\">$(IntDir)" << (isDuplicate ? (*entry).prefix : "") << "%(FileName).obj;%(Outputs)</Outputs>\n";

#define OUPUT_OBJECT_FILENAME_MSBUILD(config, platform, prefix) \
	projectFile << "\t\t<ObjectFileName Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|" << platform << "'\">$(IntDir)" << prefix << "%(FileName).obj</ObjectFileName>\n" \
	               "\t\t<XMLDocumentationFileName Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|" << platform << "'\">$(IntDir)" << prefix << "%(FileName).xdc</XMLDocumentationFileName>\n";

#define OUPUT_FILES_MSBUILD(files, action) \
	if (!files.empty()) { \
		projectFile << "\t<ItemGroup>\n"; \
		for (std::list<FileEntry>::const_iterator entry = files.begin(); entry != files.end(); ++entry) { \
			projectFile << "\t\t<" action " Include=\"" << (*entry).path << "\" />\n"; \
		} \
		projectFile << "\t</ItemGroup>\n"; \
	}

void MSBuildProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int, const StringList &duplicate,
                                             const std::string &objPrefix, const std::string &filePrefix) {
	// Reset lists
	_filters.clear();
	_compileFiles.clear();
	_includeFiles.clear();
	_otherFiles.clear();
	_resourceFiles.clear();
	_asmFiles.clear();

	// Compute the list of files
	_filters.push_back(""); // init filters
	computeFileList(dir, duplicate, objPrefix, filePrefix);
	_filters.pop_back();    // remove last empty filter

	// Output compile files
	if (!_compileFiles.empty()) {
		projectFile << "\t<ItemGroup>\n";
		for (std::list<FileEntry>::const_iterator entry = _compileFiles.begin(); entry != _compileFiles.end(); ++entry) {
			const bool isDuplicate = (std::find(duplicate.begin(), duplicate.end(), (*entry).name + ".o") != duplicate.end());

			// Deal with duplicated file names
			if (isDuplicate) {
				projectFile << "\t\t<ClCompile Include=\"" << (*entry).path << "\">\n";
				OUPUT_OBJECT_FILENAME_MSBUILD("Debug", "Win32", (*entry).prefix)
				OUPUT_OBJECT_FILENAME_MSBUILD("Debug", "x64", (*entry).prefix)
				OUPUT_OBJECT_FILENAME_MSBUILD("Release", "Win32", (*entry).prefix)
				OUPUT_OBJECT_FILENAME_MSBUILD("Release", "x64", (*entry).prefix)
				projectFile << "\t\t</ClCompile>\n";
			} else {
				projectFile << "\t\t<ClCompile Include=\"" << (*entry).path << "\" />\n";
			}
		}
		projectFile << "\t</ItemGroup>\n";
	}

	// Output include, other and resource files
	OUPUT_FILES_MSBUILD(_includeFiles, "ClInclude")
	OUPUT_FILES_MSBUILD(_otherFiles, "None")
	OUPUT_FILES_MSBUILD(_resourceFiles, "ResourceCompile")

	// Output asm files
	if (!_asmFiles.empty()) {
		projectFile << "\t<ItemGroup>\n";
		for (std::list<FileEntry>::const_iterator entry = _asmFiles.begin(); entry != _asmFiles.end(); ++entry) {

			const bool isDuplicate = (std::find(duplicate.begin(), duplicate.end(), (*entry).name + ".o") != duplicate.end());

			projectFile << "\t\t<CustomBuild Include=\"" << (*entry).path << "\">\n"
			               "\t\t\t<FileType>Document</FileType>\n";

			OUTPUT_NASM_COMMAND_MSBUILD("Debug")
			OUTPUT_NASM_COMMAND_MSBUILD("Release")

			projectFile << "\t\t</CustomBuild>\n";
		}
		projectFile << "\t</ItemGroup>\n";
	}
}

void MSBuildProvider::computeFileList(const FileNode &dir, const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix) {
	for (FileNode::NodeList::const_iterator i = dir.children.begin(); i != dir.children.end(); ++i) {
		const FileNode *node = *i;

		if (!node->children.empty()) {
			// Update filter
			std::string _currentFilter = _filters.back();
			_filters.back().append((_filters.back() == "" ? "" : "\\") + node->name);

			computeFileList(*node, duplicate, objPrefix + node->name + '_', filePrefix + node->name + '/');

			// Reset filter
			_filters.push_back(_currentFilter);
		} else {
			// Filter files by extension
			std::string name, ext;
			splitFilename(node->name, name, ext);

			FileEntry entry;
			entry.name = name;
			entry.path = convertPathToWin(filePrefix + node->name);
			entry.filter = _filters.back();
			entry.prefix = objPrefix;

			if (ext == "cpp" || ext == "c")
				_compileFiles.push_back(entry);
			else if (ext == "h")
				_includeFiles.push_back(entry);
			else if (ext == "rc")
				_resourceFiles.push_back(entry);
			else if (ext == "asm")
				_asmFiles.push_back(entry);
			else
				_otherFiles.push_back(entry);
		}
	}
}

} // End of anonymous namespace

void error(const std::string &message) {
	std::cerr << "ERROR: " << message << "!" << std::endl;
	std::exit(-1);
}

