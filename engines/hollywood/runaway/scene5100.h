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

#ifndef HOLLYWOOD_RUNAWAY_SCENE5100_H
#define HOLLYWOOD_RUNAWAY_SCENE5100_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene5100 : public RunawayRoomScene {
public:
	Scene5100(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	const Common::U32String &subtitle() const override;

private:
	const char *actionCaptionVariant(byte id, bool look, byte item) const override;
	bool loadCustomResources() override;
	float actorHeight() const override;
	int sceneViewportYOffset() const override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void runCustomEntrySequence() override;
	Common::U32String hotspotName(byte id) const override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void walkHangar(Common::Point target, byte facing);
	uint studFrame() const;
	void startHangarTaunt();
	void handleAnimationFrameHook() override;
	void drawRoomComposite(Graphics::ManagedSurface &viewport, bool foreground);
	void advanceCustomGameplayLoop(uint32 delta) override;
	const RunawayHotspot *hotspot(byte id) const override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	// Stud's scripted gestures and flying cans are in front of Brian.
	bool roomAnimationInForeground() const override { return true; }
	void stopAmbientSpeech() override;

	RunawayAnimation _beerAnimation, _footrestAnimation, _canAnimation;
	RunawayAnimation _studAnimation, _drowsyStudAnimation, _lulaAnimation;
	Common::Array<RunawayHotspot> _hotspots;
	RunawaySpeech _hangarSpeech;
	byte _hangarShot = 0, _hangarShotCount = 0, _hangarShotFrame = 88;
	uint _hangarShotElapsed = 0;
	bool _hangarThrowing = false;
	bool _hangarIntroduction = false;
	byte _hangarLulaPose = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE5100_H
