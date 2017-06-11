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

#ifndef KINGDOM_KINGDOM_H
#define KINGDOM_KINGDOM_H

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/error.h"
#include "engines/engine.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"
#include "common/events.h"
#include "common/file.h"
#include "common/queue.h"
#include "common/system.h"
#include "common/timer.h"
#include "graphics/palette.h"
#include "graphics/screen.h"
#include "graphics/surface.h"
#include "image/bmp.h"

#include "kingdom/console.h"

namespace Kingdom {
	enum KingdomDebugChannels {
		kDebugGeneral = 1 << 0
	};

	class KingdomGame : public Engine {
	public:
		KingdomGame(OSystem *syst, const ADGameDescription *gameDesc);
		~KingdomGame();

		virtual Common::Error run();

		// Detection related functions
		const ADGameDescription *_gameDescription;
		const char *getGameId() const;
		Common::Platform getPlatform() const;

	private:
		Console *_console;

		bool _LoopFlag;
		int _GameMode;
		int _StatPlay;

		void drawScreen();
		void SetupPics();
		void InitTools();
		void TitlePage();
		void InitPlay();
		void InitHelp();
		void FadeToBlack2();
		void GameHelp();
		void GPLogic1();
		void GPLogic2();
		void GPLogic3();
		void GPLogic4();
	};
} // End of namespace Kingdom

#endif
