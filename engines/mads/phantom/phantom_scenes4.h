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

#ifndef MADS_PHANTOM_SCENES4_H
#define MADS_PHANTOM_SCENES4_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/phantom/phantom_scenes.h"

namespace MADS {

namespace Phantom {

class Scene4xx : public PhantomScene {
protected:
	/**
	 * Plays an appropriate sound when entering a scene
	 */
	void sceneEntrySound();

	/**
	 *Sets the AA file to use for the scene
	 */
	void setAAName();

	/**
	 * Updates the prefix used for getting player sprites for the scene
	 */
	void setPlayerSpritesPrefix();
public:
	Scene4xx(MADSEngine *vm) : PhantomScene(vm) {}
};

class Scene401 : public Scene4xx {
private:
	bool _anim0ActvFl;
	bool _frameInRoomFl;
	bool _takingFrameInRoomFl;

	int _redFrameHotspotId;
	int _greenFrameHostpotId;
	int _blueFrameHotspotId;
	int _yellowFrameHotspotId;

public:
	Scene401(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene403 : public Scene4xx {
private:
	bool _frameInRoomFl;
	bool _takingFrameInRoomFl;

	int _redFrameHotspotId;
	int _greenFrameHostpotId;
	int _blueFrameHotspotId;
	int _yellowFrameHotspotId;

public:
	Scene403(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene404 : public Scene4xx {
private:
	bool _frameInRoomFl;
	bool _takingFrameInRoomFl;
	bool _anim0ActvFl;

	int _redFrameHotspotId;
	int _greenFrameHostpotId;
	int _blueFrameHotspotId;
	int _yellowFrameHotspotId;

public:
	Scene404(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene406 : public Scene4xx {
private:
	bool _frameInRoomFl;
	bool _takingFrameInRoomFl;

	int _redFrameHotspotId;
	int _greenFrameHostpotId;
	int _blueFrameHotspotId;
	int _yellowFrameHotspotId;

public:
	Scene406(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene407 : public Scene4xx {
private:
	bool _frameInRoomFl;
	bool _takingFrameInRoomFl;

	int _redFrameHotspotId;
	int _greenFrameHotspotId;
	int _blueFrameHotspotId;
	int _yellowFrameHotspotId;

public:
	Scene407(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene408 : public Scene4xx {
private:
	bool _frameInRoomFl;
	bool _takingFrameInRoomFl;

	int _redFrameHotspotId;
	int _greenFrameHotspotId;
	int _blueFrameHotspotId;
	int _yellowFrameHotspotId;

public:
	Scene408(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene409 : public Scene4xx {
private:
	bool _frameInRoomFl;
	bool _takingFrameInRoomFl;

	int _redFrameHotspotId;
	int _greenFrameHotspotId;
	int _blueFrameHotspotId;
	int _yellowFrameHotspotId;

public:
	Scene409(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene410 : public Scene4xx {
private:
	int _skullSequence[26];
	void getLeverInfo(Common::Point *pos, int *type, int lever_number, int *noun);

public:
	Scene410(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene453 : public Scene4xx {
private:
	bool _frameInRoomFl;
	bool _takingFrameInRoomFl;

	int _redFrameHotspotId;
	int _greenFrameHotspotId;
	int _blueFrameHotspotId;
	int _yellowFrameHotspotId;

public:
	Scene453(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene456 : public Scene4xx {
private:
	bool _frameInRoomFl;
	bool _takingFrameInRoomFl;

	int _redFrameHotspotId;
	int _greenFrameHotspotId;
	int _blueFrameHotspotId;
	int _yellowFrameHotspotId;

public:
	Scene456(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};
} // End of namespace Phantom
} // End of namespace MADS

#endif /* MADS_PHANTOM_SCENES4_H */
