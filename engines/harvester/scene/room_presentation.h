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

#ifndef HARVESTER_ROOM_PRESENTATION_H
#define HARVESTER_ROOM_PRESENTATION_H

#include "common/error.h"
#include "graphics/font.h"
#include "harvester/script/script.h"

namespace Graphics {
class Screen;
}

namespace Harvester {

class Art;
class Flow;
class HarvesterEngine;
struct IndexedBitmap;
struct ResolvedText;
struct SceneState;

struct RoomViewResources {
	const Art *art;
	const Graphics::Font *promptFont;
	const Graphics::Font *inspectFont;
	const Graphics::Font *inventoryTooltipFont;
	bool useNativePromptFont;
	bool useNativeInspectFont;
};

struct RoomPresentationState {
	ResolvedText inspectText;
	Common::String currentInteractionPromptText;
	bool showingInspectText = false;
	bool inspectCanDismiss = false;
	ResolvedText combatLoadoutStatusText;
	uint32 combatLoadoutStatusHideTick = 0;
	bool closeInventoryAfterCombatLoadoutStatus = false;
};

class RoomPresentation {
public:
	RoomPresentation(HarvesterEngine &engine, SceneState &scene,
		const Art *art, const Graphics::Font *inspectFont, bool useNativeInspectFont);

	static void drawRoomScene(HarvesterEngine &engine, Graphics::Screen &screen,
		const SceneState &scene, float brightness);

	static void blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y);
	static void drawShadowedString(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color, Graphics::TextAlign align = Graphics::kTextAlignLeft);
	static void drawWrappedText(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color, int lineSpacing, bool useCftCharacterWrapping = false);
	static const IndexedBitmap *resolveInspectTextboxBitmap(const Art &art, const ResolvedText &text);
	static void drawRoomInspectText(Graphics::Screen &screen, const Art &art, const Graphics::Font &font,
		const ResolvedText &inspectText, bool useNativeFont);

	bool captureDialogueBackdrop(IndexedBitmap &dialogueBackdrop);
	Common::Error applyLightingCommand(StartupLightingCommand lightingCommand);
	Common::Error runModalShowText(const ResolvedText &modalText, Flow &flow);

private:
	HarvesterEngine &_engine;
	SceneState &_scene;
	const Art *_art;
	const Graphics::Font *_inspectFont;
	bool _useNativeInspectFont;
};

} // End of namespace Harvester

#endif // HARVESTER_ROOM_PRESENTATION_H
