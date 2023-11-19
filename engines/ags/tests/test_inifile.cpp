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

#include "ags/shared/core/platform.h"
#include "ags/lib/std/algorithm.h"
//#include "ags/shared/debugging/assert.h"
// File not present??
#include "common/scummsys.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/ini_util.h"
#include "ags/shared/util/ini_file.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using namespace AGS::Shared;

#if AGS_PLATFORM_OS_WINDOWS
#define ENDL "\r\n"
#else
#define ENDL "\n"
#endif

const char *IniFileText = ""
                          "global_item=global_value" ENDL
                          "[section1]" ENDL
                          "item1" ENDL
                          "//this is comment" ENDL
                          "item2=" ENDL
                          "item3=value" ENDL
                          "item4=another value" ENDL
                          "[this_section_should_be_deleted]" ENDL
                          "item1=value1" ENDL
                          "item2=value2" ENDL
                          ";this is comment" ENDL
                          "[section3]" ENDL
                          "item_to_be_deleted=value" ENDL
                          "item_to_be_kept=another value" ENDL
                          "     [     section4     ]      " ENDL
                          "        item1     =     value  " ENDL;

const char *IniFileText2 = ""
                           "global_item=global_value" ENDL
                           "[section1]" ENDL
                           "item1=value1" ENDL
                           "//this is comment" ENDL
                           "item2=value2" ENDL
                           "item3=value3" ENDL
                           "new_item=new_value" ENDL
                           "[section3]" ENDL
                           "item_to_be_kept=another value" ENDL
                           "     [     section4     ]      " ENDL
                           "new_item1=new_value1" ENDL
                           "        item1     =     value  " ENDL
                           "new_item2=new_value2" ENDL
                           "[section5]" ENDL
                           "item5_1=value5_1" ENDL
                           "item5_2=value5_2" ENDL
                           "item5_3=value5_3" ENDL;

bool operator==(const StringOrderMap &lhs, const StringOrderMap &rhs) {
	if (lhs.size() != rhs.size())
		return false;

	StringOrderMap::const_iterator it1 = lhs.begin();
	StringOrderMap::const_iterator it2 = rhs.begin();

	for (; it1 != lhs.end(); ++it1, ++it2) {
		if (it1->_key || it2->_key || it1->_value != it2->_value)
			return false;
	}

	return true;
}

bool operator==(const ConfigTree &lhs, const ConfigTree &rhs) {
	if (lhs.size() != rhs.size())
		return false;

	ConfigTree::const_iterator it1 = lhs.begin();
	ConfigTree::const_iterator it2 = rhs.begin();

	for (; it1 != lhs.end(); ++it1, ++it2) {
		if (it1->_key || it2->_key || !(it1->_value == it2->_value))
			return false;
	}

	return true;
}

void Test_IniFile() {
	Stream *fs = File::CreateFile("test.ini");
	fs->Write(IniFileText, strlen(IniFileText));
	delete fs;

	IniFile ini;
	fs = File::OpenFileRead("test.ini");
	ini.Read(fs);
	delete fs;

	// there are explicit sections and 1 implicit global one
	const size_t section_count = 5;
	// Test reading from the custom ini file
	{
		assert(ini.GetSectionCount() == section_count);
		IniFile::ConstSectionIterator sec = ini.CBegin();

		assert(sec->GetItemCount() == 1u);
		IniFile::ConstItemIterator item = sec->CBegin();
		assert(item->GetKey() == "global_item");
		assert(item->GetValue() == "global_value");

		++sec;
		assert(sec->GetName() == "section1");
		assert(sec->GetItemCount() == 5u);
		item = sec->CBegin();
		assert(item->GetKey() == "item1");
		assert(item->GetValue() == "");
		++item;
		assert(item->GetLine() == "//this is comment");
		++item;
		assert(item->GetKey() == "item2");
		assert(item->GetValue() == "");
		++item;
		assert(item->GetKey() == "item3");
		assert(item->GetValue() == "value");
		++item;
		assert(item->GetKey() == "item4");
		assert(item->GetValue() == "another value");

		++sec;
		assert(sec->GetName() == "this_section_should_be_deleted");
		assert(sec->GetItemCount() == 3u);
		item = sec->CBegin();
		assert(item->GetKey() == "item1");
		assert(item->GetValue() == "value1");
		++item;
		assert(item->GetKey() == "item2");
		assert(item->GetValue() == "value2");
		++item;
		assert(item->GetLine() == ";this is comment");

		++sec;
		assert(sec->GetName() == "section3");
		assert(sec->GetItemCount() == 2u);
		item = sec->CBegin();
		assert(item->GetKey() == "item_to_be_deleted");
		assert(item->GetValue() == "value");
		++item;
		assert(item->GetKey() == "item_to_be_kept");
		assert(item->GetValue() == "another value");

		++sec;
		assert(sec->GetName() == "section4");
		assert(sec->GetItemCount() == 1u);
		item = sec->CBegin();
		assert(item->GetKey() == "item1");
		assert(item->GetValue() == "value");
	}

	// Test altering INI data and saving to file
	{
		// Modiying item values
		IniFile::SectionIterator sec = ini.Begin();
		++sec;
		IniFile::ItemIterator item = sec->Begin();
		item->SetValue("value1");
		++item;
		++item;
		item->SetValue("value2");
		++item;
		item->SetValue("value3");
		++item;
		item->SetKey("new_item");
		item->SetValue("new_value");

		// Removing a section
		sec = ini.Begin();
		++sec;
		++sec;
		ini.RemoveSection(sec);
		assert(ini.GetSectionCount() == section_count - 1);

		// Removing an item
		sec = ini.Begin();
		++sec;
		++sec;
		assert(sec->GetName() == "section3");
		item = sec->Begin();
		assert(item->GetKey() == "item_to_be_deleted");
		sec->EraseItem(item);

		// Inserting new items
		++sec;
		assert(sec->GetName() == "section4");
		ini.InsertItem(sec, sec->Begin(), "new_item1", "new_value1");
		ini.InsertItem(sec, sec->End(), "new_item2", "new_value2");

		// Append new section
		sec = ini.InsertSection(ini.End(), "section5");
		ini.InsertItem(sec, sec->End(), "item5_1", "value5_1");
		ini.InsertItem(sec, sec->End(), "item5_2", "value5_2");
		ini.InsertItem(sec, sec->End(), "item5_3", "value5_3");

		fs = File::CreateFile("test.ini");
		ini.Write(fs);
		delete fs;

		fs = File::OpenFileRead("test.ini");
		String ini_content;
		ini_content.ReadCount(fs, static_cast<size_t>(fs->GetLength()));

		assert(ini_content == IniFileText2);
	}

	// Test creating KeyValueTree from existing ini file
	{
		ConfigTree tree;
		IniUtil::Read("test.ini", tree);

		assert(tree.size() == 5u);
		assert(tree.find("") != tree.end()); // global section
		assert(tree.find("section1") != tree.end());
		assert(tree.find("section3") != tree.end());
		assert(tree.find("section4") != tree.end());
		assert(tree.find("section5") != tree.end());
		StringOrderMap &sub_tree = tree[""];
		assert(sub_tree.size() == 1u);
		assert(sub_tree.find("global_item") != sub_tree.end());
		assert(sub_tree["global_item"] == "global_value");
		sub_tree = tree["section1"];
		assert(sub_tree.size() == 4u);
		assert(sub_tree.find("item1") != sub_tree.end());
		assert(sub_tree.find("item2") != sub_tree.end());
		assert(sub_tree.find("item3") != sub_tree.end());
		assert(sub_tree.find("new_item") != sub_tree.end());
		assert(sub_tree["item1"] == "value1");
		assert(sub_tree["item2"] == "value2");
		assert(sub_tree["item3"] == "value3");
		assert(sub_tree["new_item"] == "new_value");
		sub_tree = tree["section3"];
		assert(sub_tree.size() == 1u);
		assert(sub_tree.find("item_to_be_kept") != sub_tree.end());
		assert(sub_tree["item_to_be_kept"] == "another value");
		sub_tree = tree["section4"];
		assert(sub_tree.size() == 3u);
		assert(sub_tree.find("new_item1") != sub_tree.end());
		assert(sub_tree.find("item1") != sub_tree.end());
		assert(sub_tree.find("new_item2") != sub_tree.end());
		assert(sub_tree["new_item1"] == "new_value1");
		assert(sub_tree["item1"] == "value");
		assert(sub_tree["new_item2"] == "new_value2");
		sub_tree = tree["section5"];
		assert(sub_tree.size() == 3u);
		assert(sub_tree.find("item5_1") != sub_tree.end());
		assert(sub_tree.find("item5_2") != sub_tree.end());
		assert(sub_tree.find("item5_3") != sub_tree.end());
		assert(sub_tree["item5_1"] == "value5_1");
		assert(sub_tree["item5_2"] == "value5_2");
		assert(sub_tree["item5_3"] == "value5_3");
	}

	// Test self-serialization
	ConfigTree tree1;
	{
		ConfigTree tree2;

		// construct the tree
		{
			StringOrderMap &audio_tree = tree1["audio"];
			audio_tree["volume"] = "100.0";
			audio_tree["driver"] = "midi";
			StringOrderMap &video_tree = tree1["video"];
			video_tree["gfx_mode"] = "standard mode";
			video_tree["gamma"] = "1.0";
			video_tree["vsync"] = "false";
		}

		IniUtil::Write("test.ini", tree1);
		IniUtil::Read("test.ini", tree2);

		// Assert, that tree2 has exactly same items as tree1
		assert(tree1 == tree2);
	}

	// Test merging
	{
		ConfigTree tree3;
		ConfigTree tree4;

		// Try merging altered tree into existing file
		tree3 = tree1;
		{
			StringOrderMap &audio_tree = tree3["audio"];
			audio_tree["extra_option1"] = "extra value 1";
			audio_tree["extra_option2"] = "extra value 2";
			audio_tree["extra_option3"] = "extra value 3";
			StringOrderMap &video_tree = tree3["video"];
			video_tree["gfx_mode"] = "alternate mode";
			video_tree["gamma"] = "2.0";
			StringOrderMap &new_tree = tree3["other1"];
			new_tree["item1_1"] = "value1_1";
			new_tree["item1_2"] = "value1_2";
			new_tree["item1_3"] = "value1_3";
			StringOrderMap &new_tree2 = tree3["other2"];
			new_tree2["item2_1"] = "value2_1";
			new_tree2["item2_2"] = "value2_2";
			new_tree2["item2_3"] = "value2_3";
		}

		IniUtil::Merge("test.ini", tree3);
		IniUtil::Read("test.ini", tree4);

		// Assert, that tree4 has all the items from tree3
		assert(tree3 == tree4);
	}

	File::DeleteFile("test.ini");
}

} // namespace AGS3
