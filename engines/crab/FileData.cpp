#include "stdafx.h"
#include "FileData.h"
#include <iomanip>

using namespace pyrodactyl::ui;

FileData::FileData(const boost::filesystem::path &filepath)
{
	if (boost::filesystem::exists(filepath))
	{
		name = boost::filesystem::basename(filepath);
		path = filepath.string();

#if defined(__WIN32__) || defined(__APPLE__)
		std::time_t temp = boost::filesystem::last_write_time(filepath);
		last_modified = NumberToString(std::put_time(std::localtime(&temp), "%d %b %Y %H:%M:%S"));
#else
		//Workaround for GNU C++ not having implemented std::put_time
		std::time_t temp = boost::filesystem::last_write_time(filepath);
		char timestr[100];

		std::strftime(timestr, sizeof(timestr), "%d %b %Y %H:%M:%S", std::localtime(&temp));
		last_modified = timestr;
#endif
	}
	else
		name = "New Save";
}

SaveFileData::SaveFileData(const boost::filesystem::path &filepath) : FileData(filepath)
{
	if (boost::filesystem::exists(filepath))
	{
		XMLDoc conf(filepath.string());
		if (conf.ready())
		{
			rapidxml::xml_node<char> * node = conf.Doc()->first_node("save");
			if (NodeValid(node))
			{
				LoadStr(diff, "diff", node);
				LoadStr(loc_id, "loc_id", node);
				LoadStr(loc_name, "loc_name", node);
				LoadStr(char_name, "char_name", node);
				LoadStr(time, "time", node);
				LoadStr(preview, "preview", node);
				blank = false;
			}
		}
	}
}

ModFileData::ModFileData(boost::filesystem::path filepath) : FileData(filepath)
{
	if (boost::filesystem::exists(filepath))
	{
		XMLDoc conf(filepath.string());
		if (conf.ready())
		{
			rapidxml::xml_node<char> * node = conf.Doc()->first_node("config");
			if (NodeValid(node))
			{
				LoadStr(author, "author", node);
				LoadStr(version, "version", node);
				LoadStr(info, "info", node);
				LoadStr(website, "website", node);
				LoadStr(preview, "preview", node);
			}
		}
	}
}