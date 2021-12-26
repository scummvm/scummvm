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

#include "ags/lib/std/memory.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/ini_util.h"
#include "ags/shared/util/ini_file.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/text_stream_writer.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

typedef std::unique_ptr<Stream>       UStream;
typedef IniFile::SectionIterator      SectionIterator;
typedef IniFile::ConstSectionIterator CSectionIterator;
typedef IniFile::ItemIterator         ItemIterator;
typedef IniFile::ConstItemIterator    CItemIterator;

static bool ReadIni(const String &file, IniFile &ini) {
	UStream fs(File::OpenFileRead(file));
	if (fs.get()) {
		ini.Read(fs.get());
		return true;
	}
	return false;
}

bool IniUtil::Read(const String &file, ConfigTree &tree) {
	// Read ini content
	IniFile ini;
	if (!ReadIni(file, ini))
		return false;

	// Copy items into key-value tree
	for (CSectionIterator sec = ini.CBegin(); sec != ini.CEnd(); ++sec) {
		if (!sec->GetItemCount())
			continue; // skip empty sections
		StringOrderMap &subtree = tree[sec->GetName()];
		for (CItemIterator item = sec->CBegin(); item != sec->CEnd(); ++item) {
			if (!item->IsKeyValue())
				continue; // skip non key-value items
			subtree[item->GetKey()] = item->GetValue();
		}
	}
	return true;
}

void IniUtil::Write(const String &file, const ConfigTree &tree) {
	UStream fs(File::CreateFile(file));
	TextStreamWriter writer(fs.get());

	for (ConfigNode it_sec = tree.begin(); it_sec != tree.end(); ++it_sec) {
		const String &sec_key = it_sec->_key;
		const StringOrderMap &sec_tree = it_sec->_value;

		if (!sec_tree.size())
			continue; // skip empty sections
		// write section name
		if (!sec_key.IsEmpty()) {
			writer.WriteFormat("[%s]", sec_key.GetCStr());
			writer.WriteLineBreak();
		}
		// write all items
		for (StrStrOIter keyval = sec_tree.begin(); keyval != sec_tree.end(); ++keyval) {
			const String &item_key = keyval->_key;
			const String &item_value = keyval->_value;

			writer.WriteFormat("%s = %s", item_key.GetCStr(), item_value.GetCStr());
			writer.WriteLineBreak();
		}
	}

	writer.ReleaseStream();
}

void IniUtil::WriteToString(String &s, const ConfigTree &tree) {
	for (ConfigNode it_sec = tree.begin(); it_sec != tree.end(); ++it_sec) {
		const String &sec_key = it_sec->_key;
		const StringOrderMap &sec_tree = it_sec->_value;
		if (!sec_tree.size())
			continue; // skip empty sections
		// write section name
		if (!sec_key.IsEmpty())
			s.Append(String::FromFormat("[%s]\n", sec_key.GetCStr()));
		// write all items
		for (StrStrOIter keyval = sec_tree.begin(); keyval != sec_tree.end(); ++keyval)
			s.Append(String::FromFormat("%s = %s\n", keyval->_key.GetCStr(), keyval->_value.GetCStr()));
	}
}

bool IniUtil::Merge(const String &file, const ConfigTree &tree) {
	// Read ini content
	IniFile ini;
	ReadIni(file, ini); // NOTE: missing file is a valid case

	// Remember the sections we find in file, if some sections are not found,
	// they will be appended to the end of file.
	std::map<String, bool> sections_found;
	for (ConfigNode it = tree.begin(); it != tree.end(); ++it)
		sections_found[it->_key] = false;

	// Merge existing sections
	for (SectionIterator sec = ini.Begin(); sec != ini.End(); ++sec) {
		if (!sec->GetItemCount())
			continue; // skip empty sections
		String secname = sec->GetName();
		ConfigNode tree_node = tree.find(secname);
		if (tree_node == tree.end())
			continue; // this section is not interesting for us

		// Remember the items we find in this section, if some items are not found,
		// they will be appended to the end of section.
		const StringOrderMap &subtree = tree_node->_value;
		std::map<String, bool> items_found;
		for (StrStrOIter keyval = subtree.begin(); keyval != subtree.end(); ++keyval)
			items_found[keyval->_key] = false;

		// Replace matching items
		for (ItemIterator item = sec->Begin(); item != sec->End(); ++item) {
			String key = item->GetKey();
			StrStrOIter keyval = subtree.find(key);
			if (keyval == subtree.end())
				continue; // this item is not interesting for us

			String old_value = item->GetValue();
			String new_value = keyval->_value;
			if (old_value != new_value)
				item->SetValue(new_value);
			items_found[key] = true;
		}

		// Append new items
		if (!sections_found[secname]) {
			for (std::map<String, bool>::const_iterator item_f = items_found.begin(); item_f != items_found.end(); ++item_f) {
				if (item_f->_value)
					continue; // item was already found
				StrStrOIter keyval = subtree.find(item_f->_key);
				ini.InsertItem(sec, sec->End(), keyval->_key, keyval->_value);
			}
			sections_found[secname] = true; // mark section as known
		}
	}

	// Add new sections
	for (std::map<String, bool>::const_iterator sec_f = sections_found.begin(); sec_f != sections_found.end(); ++sec_f) {
		if (sec_f->_value)
			continue;
		SectionIterator sec = ini.InsertSection(ini.End(), sec_f->_key);
		const StringOrderMap &subtree = tree.find(sec_f->_key)->_value;
		for (StrStrOIter keyval = subtree.begin(); keyval != subtree.end(); ++keyval)
			ini.InsertItem(sec, sec->End(), keyval->_key, keyval->_value);
	}

	// Write the resulting set of lines
	UStream fs(File::CreateFile(file));
	if (!fs.get())
		return false;
	ini.Write(fs.get());
	return true;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
