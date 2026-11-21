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

#ifndef HOLLYWOOD_RUNAWAY_PRESENTATION_SCENE_H
#define HOLLYWOOD_RUNAWAY_PRESENTATION_SCENE_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {

// Numbered full-screen scenes share a display and have no room interactions.
class RunawayPresentationScene : public RunawayScene {
public:
	bool busy() const override { return true; }
	bool playingVideo() const override { return true; }
	bool speaking() const override { return !subtitle().empty(); }
	Graphics::ManagedSurface &screen() override { return _screen; }
	int roomWidth() const override { return kRunawayScreenWidth; }
	byte hotspotAt(Common::Point mouse) const override { return 0; }
	Common::U32String actionCaption(byte hotspot, RunawayVerb verb, byte item) const override { return Common::U32String(); }
	const Common::U32String &committedCaption() const override { return _empty; }
	void click(Common::Point mouse, RunawayVerb verb, byte item) override {}
	void sayInventory(const RunawayInventory::Reply &reply) override {}
	void setCameraX(int x) override {}
	void setOptions(bool antialias, bool fastScrolling) override {}

protected:
	Graphics::ManagedSurface _screen;

private:
	Common::U32String _empty;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_PRESENTATION_SCENE_H
