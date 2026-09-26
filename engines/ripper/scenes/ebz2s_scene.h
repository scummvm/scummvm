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
 */

#ifndef RIPPER_SCENES_EBZ2S_SCENE_H
#define RIPPER_SCENES_EBZ2S_SCENE_H

#include "ripper/scenes/scene.h"

namespace Ripper {

class Ebz2sScene : public Scene {
public:
	explicit Ebz2sScene(RipperEngine *engine);

	Result run(uint sceneArgument) override;

private:
	bool runInventory(int grantFlag);
	void resetSelection();
};

} // End of namespace Ripper

#endif // RIPPER_SCENES_EBZ2S_SCENE_H
