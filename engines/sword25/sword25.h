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

#ifndef SWORD25_H
#define SWORD25_H

#include "common/scummsys.h"
#include "common/str-array.h"
#include "common/util.h"
#include "engines/engine.h"

#include "sword25/kernel/log.h"

namespace Sword25 {

enum {
	kFileTypeHash = 0
};

enum {
	kDebugScript = 1 << 0
};

#define MESSAGE_BASIC 1
#define MESSAGE_INTERMEDIATE 2
#define MESSAGE_DETAILED 3

struct Sword25GameDescription;

class Sword25Engine : public Engine {
private:
	Common::Error AppStart(const Common::StringArray &CommandParameters);
	bool AppMain();
	bool AppEnd();

	bool LoadPackages();
protected:
	virtual Common::Error run();
	void shutdown();
	
public:
	Sword25Engine(OSystem *syst, const Sword25GameDescription *gameDesc);
	virtual ~Sword25Engine();

	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	const Sword25GameDescription *_gameDescription;
};

} // End of namespace Sword25

#endif
