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

#ifndef HODJNPODJ_CRYPT_REC_H
#define HODJNPODJ_CRYPT_REC_H

#include "common/file.h"
//#include <lzexpand.h>

namespace Bagel {
namespace HodjNPodj {
namespace Crypt {

#define CRYPT_TXT_FILE      "Crypt.lz"
#define CRYPT_RECS          (g_engine->isDemo() ? 8 : 200)

#define MAX_GRAM_LEN        512
#define MAX_SOURCE_LEN      128
#define RECORD_LEN          392


class CCryptRecord {
private:
	int     m_nID = 0;
	char    m_lpszGram[MAX_GRAM_LEN] = { 0 };
	char    m_lpszSource[MAX_SOURCE_LEN] = { 0 };

public:
	bool GetRecord(int nID);

	int GetID() const {
		return m_nID;
	};
	const char *GetGram() const {
		return m_lpszGram;
	};
	const char *GetSource() const {
		return m_lpszSource;
	};
};

} // namespace Crypt
} // namespace HodjNPodj
} // namespace Bagel

#endif
