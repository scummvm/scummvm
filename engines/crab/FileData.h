#pragma once

#include "XMLDoc.h"
#include "common_header.h"
#include "loaders.h"

namespace pyrodactyl {
namespace ui {
class FileData {
public:
	std::string name, path, last_modified;
	FileData(const boost::filesystem::path &filepath);
};

class SaveFileData : public FileData {
public:
	std::string loc_id, loc_name, char_name, diff, time, preview;

	// This is to account for the first save slot, called "New Save", which doesn't actually have a file
	bool blank;

	SaveFileData(const boost::filesystem::path &filepath);
	SaveFileData(const bool empty);
};

class ModFileData : public FileData {
public:
	std::string author, version, info, website, preview;
	ModFileData(boost::filesystem::path filepath);
};

// The types of data shown about the save file
enum { DATA_SAVENAME,
	   DATA_LASTMODIFIED,
	   DATA_BUTTON_TOTAL };

// Both of these are capped at DATA_HOVER_TOTAL
enum { DATA_LOCNAME,
	   DATA_DIFFICULTY,
	   DATA_TIMEPLAYED,
	   DATA_PLAYERNAME };
enum { DATA_AUTHOR,
	   DATA_VERSION,
	   DATA_INFO,
	   DATA_WEBSITE };
const int DATA_HOVER_TOTAL = 4;
} // End of namespace ui
} // End of namespace pyrodactyl