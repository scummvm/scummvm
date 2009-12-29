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

#ifndef MOHAWK_H
#define MOHAWK_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/system.h"
#include "common/rect.h"
#include "common/array.h"

#include "engines/engine.h"

#include "gui/dialog.h"

#include "mohawk/console.h"
#include "mohawk/dialogs.h"
#include "mohawk/file.h"
#include "mohawk/sound.h"
#include "mohawk/video/video.h"

namespace Mohawk {

enum MohawkGameType {
	GType_MYST,
	GType_MAKINGOF,
	GType_RIVEN,
	GType_ZOOMBINI,
	GType_CSWORLD,
	GType_CSAMTRAK,
	GType_MAGGIESS,
	GType_JAMESMATH,
	GType_TREEHOUSE,
	GType_1STDEGREE,
	GType_CSUSA,
	GType_OLDLIVINGBOOKS,
	GType_NEWLIVINGBOOKS
};

enum MohawkGameFeatures {
	GF_ME =      (1 << 0),	// Myst Masterpiece Edition
	GF_DVD =     (1 << 1),
	GF_10TH =    (1 << 2),	// 10th Anniversary
	GF_DEMO =    (1 << 3),
	GF_HASMIDI = (1 << 4),
	GF_HASBINK = (1 << 5)
};

struct MohawkGameDescription;
class Sound;
class PauseDialog;

class MohawkEngine : public ::Engine {
protected:
	virtual Common::Error run();

public:
	MohawkEngine(OSystem *syst, const MohawkGameDescription *gamedesc);
	virtual ~MohawkEngine();

	// Detection related functions
	const MohawkGameDescription *_gameDescription;
	const char* getGameId() const;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	uint8 getGameType();
	Common::Language getLanguage();
	
	bool hasFeature(EngineFeature f) const;

	Sound *_sound;
	VideoManager *_video;

	Common::SeekableReadStream *getRawData(uint32, uint16);
	bool hasResource(uint32 tag, uint16 id);

	void pauseGame();

protected:
	PauseDialog *_pauseDialog;
	void pauseEngineIntern(bool);

	// An array holding the main Mohawk archives require by the games
	Common::Array<MohawkFile*> _mhk;
};

} // End of namespace Mohawk

#endif
