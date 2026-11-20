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

#ifndef HOLLYWOOD_RUNAWAY_SCENE2020_H
#define HOLLYWOOD_RUNAWAY_SCENE2020_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene2020 : public RunawayRoomScene {
public:
	Scene2020(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	void start(uint32 now) override;
	bool busy() const override;
	void setCameraX(int x) override;

private:
	enum HospitalFilm { kGinaStoryFilm, kLedgeFilm, kDecoyFilm, kAmbushFilm, kEscapeFilm };

	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	byte defaultItemAction(byte item, byte id) const override;
	float actorHeight() const override;
	byte initialActorFacing() const override;
	void resumeAmbientAudio(bool entering) override;
	void runCustomEntrySequence() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void adaptGogFilm(const RunawaySequence::Command *commands);
	void startHospitalFilm(HospitalFilm film);
	void finishCustomFilm() override;
	void finishCustomAnimation(uint16 action) override;
	void handleAnimationFrameHook() override;
	Common::U32String hotspotName(byte id) const override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawRoomComposite(Graphics::ManagedSurface &viewport, bool foreground);

	RunawayAnimation _ginaAnimation, _chartAnimation, _chartOverlay;
	Common::Array<byte> _monitorPatches[2];
	Common::Array<RunawaySequence::Command> _gogFilmCommands;
	HospitalFilm _hospitalFilm = kGinaStoryFilm;
	byte _entryStep = 0, _ginaFrame = 0;
	int _ginaDirection = 1;
	uint32 _ginaTick = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE2020_H
