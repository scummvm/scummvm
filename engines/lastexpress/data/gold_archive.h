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

#ifndef LASTEXPRESS_GOLD_ARCHIVE_H
#define LASTEXPRESS_GOLD_ARCHIVE_H

#include "lastexpress/data/archive.h"

#include "common/file.h"

namespace LastExpress {

class LastExpressEngine;
class ArchiveManager;

struct Seq;
struct HPF;

class GoldArchiveManager : public ArchiveManager {
public:
	GoldArchiveManager(LastExpressEngine *engine);
	~GoldArchiveManager();

	bool lockCD(int32 index) override;
	bool isCDAvailable(int cdNum, char *outPath, int pathSize) override;
	bool lockCache(char *filename) override;
	void initHPFS() override;
	void shutDownHPFS() override;

	HPF *openHPF(const char *filename) override;
	void readHPF(HPF *archive, void *dstBuf, uint32 size) override;

	int loadBG(const char *filename) override;

protected:
	Common::String _languagePrefix = "EN";
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_GOLD_ARCHIVE_H
