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

#include "common/file.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_textdb.h"

namespace QDEngine {

qdTextDB *g_textDB = nullptr;

qdTextDB::qdTextDB() {
}

qdTextDB::~qdTextDB() {
	clear();

	if (g_textDB)
		delete g_textDB;

	g_textDB = nullptr;
}

qdTextDB &qdTextDB::instance() {
	if (!g_textDB)
		g_textDB = new qdTextDB;

	return *g_textDB;
}

const char *qdTextDB::getText(const char *text_id) const {
	if (_texts.contains(text_id))
		return _texts[text_id]._text.c_str();

	static const char *const str = "";
	return str;
}

const char *qdTextDB::getSound(const char *text_id) const {
	if (_texts.contains(text_id))
		return _texts[text_id]._sound.c_str();

	static const char *const str = "";
	return str;
}

const char *qdTextDB::getComment(const char *text_id) const {
	if (_texts.contains(text_id))
		return _texts[text_id]._comment.c_str();

	static const char *const str = "";
	return str;
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

		_texts.setVal(idStr, qdText(txtStr.c_str(), sndStr.c_str()));
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

			if (_texts.contains(idStr))
				_texts[idStr]._comment = txtStr;
		}
	}

	return true;
}

} // namespace QDEngine
