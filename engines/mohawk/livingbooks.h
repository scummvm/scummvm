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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef MOHAWK_LIVINGBOOKS_H
#define MOHAWK_LIVINGBOOKS_H

#include "mohawk/mohawk.h"
#include "mohawk/console.h"
#include "mohawk/graphics.h"

#include "common/config-file.h"

namespace Mohawk {

enum {
	kIntroPage = 0
};

class LBGraphics;

class MohawkEngine_LivingBooks : public MohawkEngine {
protected:
	Common::Error run();

public:
	MohawkEngine_LivingBooks(OSystem *syst, const MohawkGameDescription *gamedesc);
	virtual ~MohawkEngine_LivingBooks();

	LBGraphics *_gfx;
	bool _needsUpdate;

	Common::SeekableSubReadStreamEndian *wrapStreamEndian(uint32 tag, uint16 id);
	GUI::Debugger *getDebugger() { return _console; }

private:
	LivingBooksConsole *_console;
	Common::ConfigFile _bookInfoFile;

	uint16 _curPage;
	Common::String getBookInfoFileName();
	void loadBookInfo(Common::String filename);
	void loadIntro();

	uint16 getResourceVersion();
	void loadSHP(uint16 resourceId);
	void loadANI(uint16 resourceId);

	uint16 _screenWidth;
	uint16 _screenHeight;
	uint16 _numLanguages;
	uint16 _numPages;
	Common::String _title;
	Common::String _copyright;

	// String Manipulation Functions
	Common::String removeQuotesFromString(Common::String string);
	Common::String convertMacFileName(Common::String string);
	Common::String convertWinFileName(Common::String string);
	
	// Configuration File Functions
	Common::String getStringFromConfig(Common::String section, Common::String key);
	int getIntFromConfig(Common::String section, Common::String key);
	Common::String getFileNameFromConfig(Common::String section, Common::String key);
	
	// Platform/Version functions
	bool isBigEndian() { return getGameType() == GType_NEWLIVINGBOOKS || getPlatform() == Common::kPlatformMacintosh; }
	MohawkFile *createMohawkFile() { return (getGameType() == GType_NEWLIVINGBOOKS) ? new MohawkFile() : new OldMohawkFile(); }
};

} // End of namespace Mohawk

#endif
