/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/grim/update/lang_filter.h"

#include "common/file.h"
#include "common/archive.h"
#include "common/str.h"

namespace Grim {

const char *LangFilter::kLanguages1[7] = { "@@_", "US_", "FR_", "GE_", "IT_", "PT_", "SP_" };
const char *LangFilter::kLanguages2[7] = { "Common/", "Eng/", "Fra/", "Deu/", "Ita/", "Brz/", "Esp/" };

LangFilter::LangFilter(Common::Archive *arc, Common::Language lang) : _arc(arc) {
	if (!_arc)
		return;

	switch (lang) {
		case Common::EN_ANY:
		case Common::EN_GRB:
		case Common::EN_USA:
			_lang = kEnglish;
			break;
		case Common::FR_FRA:
			_lang = kFrench;
			break;
		case Common::DE_DEU:
			_lang = kGerman;
			break;
		case Common::IT_ITA:
			_lang = kItalian;
			break;
		case Common::PT_BRA:
			_lang = kPortuguese;
			break;
		case Common::ES_ESP:
			_lang = kSpanish;
			break;
		default:
			_lang = kCommon;
			break;
	}
}

LangFilter::~LangFilter() {
	delete _arc;
}

bool LangFilter::hasFile(const Common::String &name) const {
	if (!_arc)
		return false;

	return (_arc->hasFile(kLanguages1[_lang] + name)) ||
			(_arc->hasFile(kLanguages1[kCommon] + name)) ||
			(_arc->hasFile(kLanguages2[_lang] + name)) ||
			(_arc->hasFile(kLanguages2[kCommon] + name));
}

int LangFilter::listMembers(Common::ArchiveMemberList &list) const {
	if (!_arc)
		return false;

	Common::ArchiveMemberList orgList;
	Common::String orgName, name;

	_arc->listMembers(orgList);

	int num = 0;
	//Search only files with the right language and create a list with their basenames
	for (Common::ArchiveMemberList::const_iterator it = orgList.begin(); it != orgList.end(); ++it) {
		orgName = (*it)->getName();
		if (orgName.hasPrefix(kLanguages1[_lang]) || orgName.hasPrefix(kLanguages1[kCommon]))
			name = Common::String(orgName.c_str() + 3);
		else if (orgName.hasPrefix(kLanguages2[_lang]) || orgName.hasPrefix(kLanguages2[kCommon])) {
			int i = 0;
			while (orgName[i++] != '/') {;}
			name = Common::String(orgName.c_str() + i);

			//If the file is a subfolder, reject it
			if (name.contains('/'))
				continue;
		} else
			continue;
		name.toLowercase();
		list.push_back(getMember(name));
		++num;
	}

	return num;
}

const Common::ArchiveMemberPtr LangFilter::getMember(const Common::String &name) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *LangFilter::createReadStreamForMember(const Common::String &name) const {
	if (!_arc)
		return nullptr;

	//Search the right file
	Common::String fullName;
	Common::List<Common::String> namesToTry;
	namesToTry.push_front(kLanguages1[_lang] + name);
	namesToTry.push_front(kLanguages1[kCommon] + name);
	namesToTry.push_front(kLanguages2[_lang] + name);
	namesToTry.push_front(kLanguages2[kCommon] + name);
	for (Common::List<Common::String>::const_iterator it = namesToTry.begin(); it != namesToTry.end(); ++it)
		if (_arc->hasFile(*it)) {
			fullName = *it;
			break;
		}

	if (fullName.empty())
		return nullptr;

	return _arc->createReadStreamForMember(fullName);
}

} // End of namespace Grim
