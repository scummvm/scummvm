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

#ifndef ULTIMA8_WORLD_TARGETRETICLEPROCESS_H
#define ULTIMA8_WORLD_TARGETRETICLEPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class Item;

/**
 * A process to update the targeting reticle location.  This is called the
 * DumbTimer process in the game.
 */
class TargetReticleProcess : public Process {
public:
	TargetReticleProcess();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;

	//!< Notify this process that the item we're targetting has moved
	void itemMoved(Item *item);

	//!< Avatar direction changed - force update of finding a new item next frame.
	void avatarMoved();

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	void setEnabled(bool val) {
		_reticleEnabled = val;
	}
	bool getEnabled() const {
		return _reticleEnabled;
	}

	void toggle();

	static TargetReticleProcess *get_instance() {
		return _instance;
	}

private:
	bool findTargetItem();
	void putTargetReticleOnItem(Item *);
	void clearSprite();

    bool _reticleEnabled;
	int32 _lastUpdate;
	uint16 _reticleSpriteProcess;
	Direction _lastTargetDir;
	uint16 _lastTargetItem;

	static TargetReticleProcess *_instance;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
