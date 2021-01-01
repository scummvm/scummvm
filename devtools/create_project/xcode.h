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

#ifndef TOOLS_CREATE_PROJECT_XCODE_H
#define TOOLS_CREATE_PROJECT_XCODE_H

#include "create_project.h"

#include <algorithm>
#include <vector>

namespace CreateProjectTool {

class XcodeProvider : public ProjectProvider {
public:
	XcodeProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version = 0);

protected:

	void createWorkspace(const BuildSetup &setup);

	void createOtherBuildFiles(const BuildSetup &setup);

	void addResourceFiles(const BuildSetup &setup, StringList &includeList, StringList &excludeList);

	void createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
	                       const StringList &includeList, const StringList &excludeList);

	void writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
	                            const std::string &objPrefix, const std::string &filePrefix);
private:
	enum {
		kSettingsAsList        = 0x01,
		kSettingsSingleItem    = 0x02,
		kSettingsNoQuote       = 0x04,
		kSettingsQuoteVariable = 0x08,
		kSettingsNoValue       = 0x10
	};

	// File properties
	struct FileProperty {
		std::string _fileEncoding;
		std::string _lastKnownFileType;
		std::string _fileName;
		std::string _filePath;
		std::string _sourceTree;

		FileProperty(const std::string &fileType = "", const std::string &name = "", const std::string &path = "", const std::string &source = "")
				: _fileEncoding(""), _lastKnownFileType(fileType), _fileName(name), _filePath(path), _sourceTree(source) {
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// XCObject and children
	typedef std::vector<std::string> ValueList;

	struct Entry {
		std::string _value;
		std::string _comment;

		Entry(std::string val, std::string cmt) : _value(val), _comment(cmt) {}
	};

	typedef std::vector<Entry> EntryList;

	struct Setting {
		EntryList _entries;
		int _flags;
		int _indent;
		int _order;

		Setting(std::string value = "", std::string comment = "", int flgs = 0, int idt = 0, int ord = -1) : _flags(flgs), _indent(idt), _order(ord) {
			_entries.push_back(Entry(value, comment));
		}

		Setting(ValueList values, int flgs = 0, int idt = 0, int ord = -1) : _flags(flgs), _indent(idt), _order(ord) {
			for (unsigned int i = 0; i < values.size(); i++)
				_entries.push_back(Entry(values[i], ""));
		}

		Setting(EntryList ents, int flgs = 0, int idt = 0, int ord = -1) : _entries(ents), _flags(flgs), _indent(idt), _order(ord) {}

		void addEntry(std::string value, std::string comment = "") {
			_entries.push_back(Entry(value, comment));
		}
	};

	typedef std::map<std::string, Setting> SettingList;
	typedef std::pair<std::string, Setting> SettingPair;
	typedef std::vector<SettingPair> OrderedSettingList;

	static bool OrderSortPredicate(const SettingPair &s1, const SettingPair &s2) {
		return s1.second._order < s2.second._order;
	}

	struct Property {
	public:
		SettingList _settings;
		int _flags;
		bool _hasOrder;

		Property() : _flags(0), _hasOrder(false) {}

		// Constructs a simple Property
		Property(std::string name, std::string value = "", std::string comment = "", int flgs = 0, int indent = 0, bool order = false) : _flags(flgs), _hasOrder(order) {
			_settings[name] = Setting(value, comment, _flags, indent);
		}

		Property(std::string name, ValueList values, int flgs = 0, int indent = 0, bool order = false) : _flags(flgs), _hasOrder(order) {
			_settings[name] = Setting(values, _flags, indent);
		}

		OrderedSettingList getOrderedSettingList() {
			OrderedSettingList list;

			// Prepare vector to sort
			for (SettingList::const_iterator setting = _settings.begin(); setting != _settings.end(); ++setting)
				list.push_back(SettingPair(setting->first, setting->second));

			// Sort vector using setting order
			if (_hasOrder)
				std::sort(list.begin(), list.end(), OrderSortPredicate);

			return list;
		}
	};

	typedef std::map<std::string, Property> PropertyList;

	// Main object struct
	// This is all a big hack unfortunately, but making everything all properly abstracted would
	// be overkill since we only have to generate a single project
	struct Object {
	public:
		std::string _id;                // Unique identifier for this object
		std::string _name;              // Name (may not be unique - for ex. configuration entries)
		std::string _refType;           // Type of object this references (if any)
		std::string _comment;           // Main comment (empty for no comment)

		PropertyList _properties;       // List of object properties, including output configuration

		// Constructs an object and add a default type property
		Object(XcodeProvider *objectParent, std::string objectId, std::string objectName, std::string objectType, std::string objectRefType = "", std::string objectComment = "")
		    : _id(objectId), _name(objectName), _refType(objectRefType), _comment(objectComment), _parent(objectParent) {
			assert(objectParent);
			assert(!objectId.empty());
			assert(!objectName.empty());
			assert(!objectType.empty());

			addProperty("isa", objectType, "", kSettingsNoQuote | kSettingsNoValue);
		}

		// Add a simple Property with just a name and a value
		void addProperty(std::string propName, std::string propValue, std::string propComment = "", int propFlags = 0, int propIndent = 0) {
			_properties[propName] = Property(propValue, "", propComment, propFlags, propIndent);
		}

		std::string toString(int flags = 0) {
			std::string output;
			output = "\t\t" + _parent->getHash(_id) + (_comment.empty() ? "" : " /* " + _comment + " */") + " = {";

			if (flags & kSettingsAsList)
				output += "\n";

			// Special case: always output the isa property first
			output += _parent->writeProperty("isa", _properties["isa"], flags);

			// Write each property
			for (PropertyList::iterator property = _properties.begin(); property != _properties.end(); ++property) {
				if (property->first == "isa")
					continue;

				output += _parent->writeProperty(property->first, property->second, flags);
			}

			if (flags & kSettingsAsList)
				output += "\t\t";

			output += "};\n";

			return output;
		}

		// Slight hack, to allow Group access to parent.
	protected:
		XcodeProvider *_parent;
	private:
		// Returns the type property (should always be the first in the properties map)
		std::string getType() {
			assert(!_properties.empty());
			assert(!_properties["isa"]._settings.empty());

			SettingList::iterator it = _properties["isa"]._settings.begin();

			return it->first;
		}
	};

	struct ObjectList {
	private:
		std::map<std::string, bool> _objectMap;

	public:
		std::vector<Object *> _objects;
		std::string _comment;
		int _flags;

		void add(Object *obj) {
			std::map<std::string, bool>::iterator it = _objectMap.find(obj->_id);
			if (it != _objectMap.end() && it->second == true)
				return;

			_objects.push_back(obj);
			_objectMap[obj->_id] = true;
		}

		Object *find(std::string id) {
			for (std::vector<Object *>::iterator it = _objects.begin(); it != _objects.end(); ++it) {
				if ((*it)->_id == id) {
					return *it;
				}
			}
			return NULL;
		}

		std::string toString() {
			std::string output;

			if (!_comment.empty())
				output = "\n/* Begin " + _comment + " section */\n";

			for (std::vector<Object *>::iterator object = _objects.begin(); object != _objects.end(); ++object)
				output += (*object)->toString(_flags);

			if (!_comment.empty())
				output += "/* End " + _comment + " section */\n";

			return output;
		}
	};

	// A class to maintain a folder-reference group-hierarchy, which together with the functionality below
	// allows for breaking up sub-paths into a chain of groups. This helps with merging engines into the
	// overall group-layout.
	class Group : public Object {
		int _childOrder;
		std::map<std::string, Group *> _childGroups;
		std::string _treeName;
		void addChildInternal(const std::string &id, const std::string &comment);
	public:
		Group(XcodeProvider *objectParent, const std::string &groupName, const std::string &uniqueName, const std::string &path);
		void addChildFile(const std::string &name);
		void addChildByHash(const std::string &hash, const std::string &name);
		// Should be passed the hash for the entry
		void addChildGroup(const Group *group);
		void ensureChildExists(const std::string &name);
		Group *getChildGroup(const std::string &name);
		std::string getHashRef() const { return _parent->getHash(_id); }
	};

	// The path used by the root-source group
	std::string _projectRoot;
	// The base source group, currently also re-purposed for containing the various support-groups.
	Group *_rootSourceGroup;
	// Helper function to create the chain of groups for the various subfolders. Necessary as
	// create_project likes to start in engines/
	Group *touchGroupsForPath(const std::string &path);
	// Functionality for adding file-refs and build-files, as Group-objects need to be able to do this.
	void addFileReference(const std::string &id, const std::string &name, FileProperty properties);
	void addProductFileReference(const std::string &id, const std::string &name);
	void addBuildFile(const std::string &id, const std::string &name, const std::string &fileRefId, const std::string &comment);
	// All objects
	std::map<std::string, std::string> _hashDictionnary;
	ValueList _defines;

	// Targets
	ValueList _targets;

	// Lists of objects
	ObjectList _buildFile;
	ObjectList _copyFilesBuildPhase;
	ObjectList _fileReference;
	ObjectList _frameworksBuildPhase;
	ObjectList _groups;
	ObjectList _nativeTarget;
	ObjectList _project;
	ObjectList _resourcesBuildPhase;
	ObjectList _sourcesBuildPhase;
	ObjectList _buildConfiguration;
	ObjectList _configurationList;

	void outputMainProjectFile(const BuildSetup &setup);

	// Setup objects
	void setupCopyFilesBuildPhase();
	void setupFrameworksBuildPhase(const BuildSetup &setup);
	void setupNativeTarget();
	void setupProject();
	void setupResourcesBuildPhase();
	void setupSourcesBuildPhase();
	void setupBuildConfiguration(const BuildSetup &setup);
	void setupImageAssetCatalog(const BuildSetup &setup);
	void setupAdditionalSources(std::string targetName, Property &files, int &order);

	// Misc
	void setupDefines(const BuildSetup &setup); // Setup the list of defines to be used on build configurations

	// Retrieve information
	ValueList& getResourceFiles() const;

	// Hash generation
	std::string getHash(std::string key);
#ifdef MACOSX
	std::string md5(std::string key);
#endif
	std::string newHash() const;

	// Output
	std::string writeProperty(const std::string &variable, Property &property, int flags = 0) const;
	std::string writeSetting(const std::string &variable, std::string name, std::string comment = "", int flags = 0, int indent = 0) const;
	std::string writeSetting(const std::string &variable, const Setting &setting) const;
};

} // End of CreateProjectTool namespace

#endif // TOOLS_CREATE_PROJECT_XCODE_H
