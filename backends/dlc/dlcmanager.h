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

#ifndef DLC_DLCMANAGER_H
#define DLC_DLCMANAGER_H

#include "common/str.h"
#include "common/singleton.h"
#include "backends/dlc/store.h"

namespace DLC {

enum Feature {
	kInProgress
};

class DLCManager : public Common::Singleton<DLCManager> {

	struct DLCDesc {
		Common::String name;
		Common::String description;
		uint32 size;
		uint32 game_id;
	};

	Store* _store;

public:
	DLCManager();
	virtual ~DLCManager() {}

	void init();

	DLCDesc getDLCList();

	void initDownload();

	void cancelDownload();

	bool getFeatureState(DLC::Feature f);
	void setFeatureState(DLC::Feature f, bool enable);

	void getPercentDownloaded();

	uint32 getInProgressGame();
};

#define DLCMan        DLC::DLCManager::instance()

} // End of namespace DLC


#endif
