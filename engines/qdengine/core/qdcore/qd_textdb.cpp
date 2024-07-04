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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/file.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/qd_textdb.h"


namespace QDEngine {

//#include <tchar.h>

char const DELIMETER = '.';

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdTextDB::qdTextDB() {
}

qdTextDB::~qdTextDB() {
	clear();
}

qdTextDB &qdTextDB::instance() {
	static qdTextDB db;
	return db;
}

const char *qdTextDB::getText(const char *text_id) const {
	qdTextMap::const_iterator it = texts_.find(text_id);
	if (it != texts_.end())
		return it->second.text_.c_str();

	static const char *const str = "";
	return str;
}

const char *qdTextDB::getSound(const char *text_id) const {
	qdTextMap::const_iterator it = texts_.find(text_id);
	if (it != texts_.end())
		return it->second.sound_.c_str();

	static const char *const str = "";
	return str;
}

const char *qdTextDB::getComment(const char *text_id) const {
#ifndef _FINAL_VERSION_
	qdTextMap::const_iterator it = texts_.find(text_id);
	if (it != texts_.end())
		return it->second.comment_.c_str();
#endif

	static const char *const str = "";
	return str;
}

bool qdTextDB::load(const char *file_name, const char *comments_file_name, bool clear_old_texts) {
	XStream fh(0);
	if (!fh.open(file_name, XS_IN))
		return false;

	return load(fh, comments_file_name, clear_old_texts);
}

bool qdTextDB::load(XStream &fh, const char *comments_file_name, bool clear_old_texts) {
	warning("STUB: qdTextDB::load(XStream fh: %s)", comments_file_name);
	return true;
}

bool qdTextDB::load(Common::SeekableReadStream *fh, const char *commentsFileName, bool clearOldTexts) {
	if (clearOldTexts) {
		clear();
	}

	int32  textCount = fh->readUint32LE();

	for (int i = 0; i < textCount; i++) {
		int32 idLength = fh->readSint32LE();
		Common::String idStr = fh->readString(0, idLength);

		int32 txtlength = fh->readSint32LE();
		Common::String txtStr = fh->readString(0, txtlength);

		int32 sndLength = fh->readSint32LE();
		Common::String sndStr = fh->readString(0, sndLength);

		texts_.insert(qdTextMap::value_type(idStr.c_str(), qdText(txtStr.c_str(), sndStr.c_str())));

	}

	if (commentsFileName) {
		Common::File fh1;
		if (!fh1.open(commentsFileName)) {
			return true;
		}

		textCount = fh1.readUint32LE();
		for (int i = 0; i < textCount; i++) {

			int32 idLength = fh->readSint32LE();
			Common::String idStr = fh->readString(0, idLength);

			int32 txtlength = fh->readSint32LE();
			Common::String txtStr = fh->readString(0, txtlength);

			int32 sndLength = fh->readSint32LE();
			Common::String sndStr = fh->readString(0, sndLength);

			qdTextMap::iterator it = texts_.find(idStr.c_str());
			if (it != texts_.end())
				it->second.comment_ = txtStr.c_str();
		}
	}
}

bool qdTextDB::load(XZipStream &fh, const char *comments_file_name, bool clear_old_texts) {
	warning("STUB: qdTextDB::load(XZipStream fh: %s)", comments_file_name);
	return true;
}

void qdTextDB::getIdList(const char *mask, IdList &idList) const {
	idList.clear();
//	int const maskLen = _tcslen(mask);
	int const maskLen = strlen(mask);
	qdTextMap::const_iterator i;
	for (auto &i : texts_) {
		if (!i.first.find(mask)) {
			std::string str = i.first;
			str.erase(0, maskLen + 1);
			if (!str.empty()) {
				int pos = str.find(DELIMETER);
				if (pos != std::string::npos)
					str.erase(pos, str.size());
				if (std::find(idList.begin(), idList.end(), str) == idList.end())
					idList.push_back(str);
			}
		}
	}

	idList.sort();
}

bool qdTextDB::getIdList(IdList &idList) const {
	warning("STUB: qdTextDB::getIdList");
#if 0
	try {
		std::transform(texts_.begin(),
		               texts_.end(),
		               std::back_inserter(idList),
		               std::select1st<qdTextMap::value_type>());

	} catch (std::bad_alloc &) {
		return false;
	}
#endif
	return true;
}

bool qdTextDB::getRootIdList(IdList &idList) const {
	qdTextMap::const_iterator i = texts_.begin(), e = texts_.end();
	std::string copy;
	for (; i != e; ++i) {
		std::string const &str = i->first;
		std::size_t pos = str.find(DELIMETER);
		if (pos == std::string::npos)
			copy.assign(str);
		else
			copy.assign(str, 0, pos);

		if (std::find(idList.begin(), idList.end(), copy) == idList.end())
			idList.push_back(copy);
	}
	return true;
}
} // namespace QDEngine
