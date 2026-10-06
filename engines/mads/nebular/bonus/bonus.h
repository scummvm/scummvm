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

#ifndef MADS_NEBULAR_BONUS_H
#define MADS_NEBULAR_BONUS_H

#include "graphics/surface.h"
#include "mads/nebular/nebular.h"

namespace MADS {
namespace RexNebular {

class BonusEngine final : public RexNebularEngine {
public:
	BonusEngine(OSystem *syst, const MADSGameDescription *gameDesc);
	~BonusEngine() override;

	Common::Error run() override;

protected:
	bool hasFeature(EngineFeature feature) const override;
	Common::Point screenToGame(const Common::Point &point) const override;
	Common::Point gameToScreen(const Common::Point &point) const override;
	void presentScreen(int shakeOffset) override;

private:
	Graphics::Surface _presentation;
};

} // namespace RexNebular
} // namespace MADS

#endif // MADS_NEBULAR_BONUS_H
