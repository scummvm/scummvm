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


#include "harvester/npc/chessmaster_dialogue.h"

#include "common/events.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/framelimiter.h"
#include "graphics/screen.h"
#include "harvester/harvester.h"
#include "harvester/npc/dialogue_runtime.h"
#include "harvester/resources.h"
#include "video/flic_decoder.h"

namespace Harvester {

namespace {

static const char *const kChessmasterNpc = "CHESSMASTER";
static const char *const kChessSolvedFlag = "CHESS_SOLVED";
static const char *const kChessKingMonsterActionTag = "ADD_CHESS_KING_MONSTER";
static const int kChessmasterSlashDamageType = 2;
static const byte kTransparentPaletteIndex = 0;
static const uint32 kChessmasterSuccessHoldMs = 1000;

struct ChessmasterHotspot {
	int left;
	int top;
	int right;
	int bottom;
};

struct ChessmasterPuzzleStage {
	const char *choiceFlicPath;
	const char *moveFlicPath;
	ChessmasterHotspot successHotspot;
	const ChessmasterHotspot *failureHotspots;
	uint failureHotspotCount;
};

struct ChessmasterFlicState {
	Video::FlicDecoder decoder;
	const Graphics::Surface *frame = nullptr;
	uint32 nextFrameTick = 0;
};

static const ChessmasterHotspot kChessmasterStage1FailureHotspots[] = {
	{ 0xbc, 0xa0, 0xe8, 0xc3 },
	{ 0xfd, 0x8c, 0x130, 0xac },
	{ 0x166, 0x9a, 0x19c, 0xbc },
	{ 0x194, 0xbe, 0x1ce, 0xe5 },
	{ 0x17f, 0x106, 0x1c3, 0x13a },
	{ 0x12c, 0x126, 0x16d, 0x15f },
	{ 0xac, 0x112, 0xdf, 0x145 }
};

static const ChessmasterHotspot kChessmasterStage2FailureHotspots[] = {
	{ 0xab, 0x10f, 0xdf, 0x144 }
};

static const ChessmasterHotspot kChessmasterStage3FailureHotspots[] = {
	{ 0x113, 0xd0, 0x14b, 0xf8 },
	{ 0xeb, 0x11b, 0x127, 0x14f },
	{ 0x87, 0x9a, 0xb1, 0xbb }
};

static const ChessmasterPuzzleStage kChessmasterPuzzleStages[] = {
		{
		"GRAPHIC/FST/CHOOSE1.FLC",
		"GRAPHIC/FST/CHESMOV1.FLC",
		{ 0x8b, 0xe0, 0xc0, 0x10d },
		kChessmasterStage1FailureHotspots,
		ARRAYSIZE(kChessmasterStage1FailureHotspots)
	},
	{
		"GRAPHIC/FST/CHOOSE2.FLC",
		"GRAPHIC/FST/CHESMOV2.FLC",
		{ 0x2a, 0xfa, 0x5f, 0x12e },
		kChessmasterStage2FailureHotspots,
		ARRAYSIZE(kChessmasterStage2FailureHotspots)
	},
	{
		"GRAPHIC/FST/CHOOSE3.FLC",
		"GRAPHIC/FST/CHESMOV3.FLC",
		{ 0xee, 0xa7, 0x121, 0xcb },
		kChessmasterStage3FailureHotspots,
		ARRAYSIZE(kChessmasterStage3FailureHotspots)
	}
};

static bool isPrimaryMouseDown() {
	return g_system && g_system->getEventManager() &&
		((g_system->getEventManager()->getButtonState() & 1) != 0);
}

static bool pointHitsHotspot(const Common::Point &point, const ChessmasterHotspot &hotspot) {
	return point.x >= hotspot.left && point.x < hotspot.right &&
		point.y >= hotspot.top && point.y < hotspot.bottom;
}

static bool pointHitsAnyHotspot(const Common::Point &point, const ChessmasterHotspot *hotspots,
		uint hotspotCount) {
	for (uint i = 0; i < hotspotCount; ++i) {
		if (pointHitsHotspot(point, hotspots[i]))
			return true;
	}

	return false;
}

static void blitTransparentFrame(Graphics::Screen &screen, const Graphics::Surface &frame, int x, int y) {
	if (frame.w <= 0 || frame.h <= 0)
		return;

	int destX = x;
	int destY = y;
	int srcX = 0;
	int srcY = 0;
	int width = frame.w;
	int height = frame.h;

	if (destX < 0) {
		srcX = -destX;
		width += destX;
		destX = 0;
	}
	if (destY < 0) {
		srcY = -destY;
		height += destY;
		destY = 0;
	}
	if (destX >= screen.w || destY >= screen.h || width <= 0 || height <= 0)
		return;

	width = MIN<int>(width, screen.w - destX);
	height = MIN<int>(height, screen.h - destY);
	if (width <= 0 || height <= 0)
		return;

	const byte *src = (const byte *)frame.getBasePtr(srcX, srcY);
	byte *dst = (byte *)screen.getBasePtr(destX, destY);
	Graphics::keyBlit(dst, src, screen.pitch, frame.pitch, width, height,
		screen.format.bytesPerPixel, kTransparentPaletteIndex);
}

static Common::Error decodeInitialFlicFrame(DialogueRuntime &runtime, const char *path,
		ChessmasterFlicState &state) {
	ResourceManager *resources = runtime.engine().getResources();
	if (!resources || !state.decoder.loadStream(resources->openFile(path)))
		return Common::kReadingFailed;

	state.frame = state.decoder.decodeNextFrame();
	if (!state.frame)
		return Common::kReadingFailed;

	if (state.decoder.getCurFrameDelay() > 0 && g_system)
		state.nextFrameTick = g_system->getMillis() + (uint32)state.decoder.getCurFrameDelay();
	else
		state.nextFrameTick = 0;

	return Common::kNoError;
}

static Common::Error advanceFlicIfNeeded(ChessmasterFlicState &state, bool loop) {
	if (!g_system)
		return Common::kNoError;
	if (state.nextFrameTick != 0 && (int32)(state.nextFrameTick - g_system->getMillis()) > 0)
		return Common::kNoError;

	if (loop && state.decoder.endOfVideo())
		state.decoder.rewind();

	const Graphics::Surface *nextFrame = state.decoder.decodeNextFrame();
	if (!nextFrame) {
		if (!loop || !state.decoder.rewind())
			return Common::kReadingFailed;

		nextFrame = state.decoder.decodeNextFrame();
		if (!nextFrame)
			return Common::kReadingFailed;
	}

	state.frame = nextFrame;
	const int frameDelay = state.decoder.getCurFrameDelay();
	state.nextFrameTick = frameDelay > 0 ? g_system->getMillis() + (uint32)frameDelay : 0;
	return Common::kNoError;
}

static void drawFlicFrame(Graphics::Screen &screen, ChessmasterFlicState &state) {
	if (state.decoder.hasDirtyPalette())
		screen.setPalette(state.decoder.getPalette());
	if (state.frame)
		screen.blitFrom(*state.frame);
}

static Common::Error waitForPointerRelease() {
	if (!g_system)
		return Common::kNoError;

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!SHOULD_QUIT && isPrimaryMouseDown()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return Common::kNoError;
		}

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

static Common::Error runTimedDelay(uint32 durationMs) {
	if (!g_system || durationMs == 0)
		return Common::kNoError;

	const uint32 deadline = g_system->getMillis() + durationMs;
	Graphics::FrameLimiter limiter(g_system, 60);
	while (!SHOULD_QUIT && (int32)(deadline - g_system->getMillis()) > 0) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return Common::kNoError;
		}

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

static Common::Error playOneShotFlic(DialogueRuntime &runtime, const char *path) {
	Graphics::Screen *screen = runtime.engine().getScreen();
	if (!screen)
		return Common::kReadingFailed;

	ChessmasterFlicState video;
	Common::Error loadError = decodeInitialFlicFrame(runtime, path, video);
	if (loadError.getCode() != Common::kNoError)
		return loadError;

	Graphics::FrameLimiter limiter(g_system, 60);
	for (;;) {
		Common::Error advanceError = advanceFlicIfNeeded(video, false);
		if (advanceError.getCode() != Common::kNoError)
			return advanceError;

		drawFlicFrame(*screen, video);
		screen->makeAllDirty();
		screen->update();

		if (video.decoder.endOfVideo())
			return Common::kNoError;

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return Common::kNoError;
		}

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}
}

static Common::Error runChessmasterChoiceStage(DialogueRuntime &runtime,
		const ChessmasterPuzzleStage &stage, bool &selectedSuccessHotspot) {
	selectedSuccessHotspot = false;

	Graphics::Screen *screen = runtime.engine().getScreen();
	ResourceManager *resources = runtime.engine().getResources();
	if (!screen || !resources || !g_system || !g_system->getEventManager())
		return Common::kReadingFailed;

	ChessmasterFlicState background;
	Common::Error loadError = decodeInitialFlicFrame(runtime, stage.choiceFlicPath, background);
	if (loadError.getCode() != Common::kNoError)
		return loadError;

	ChessmasterFlicState pointer;
	loadError = decodeInitialFlicFrame(runtime, "GRAPHIC/ROOMANIM/HARVPNTR.FLC", pointer);
	if (loadError.getCode() != Common::kNoError)
		return loadError;

	ChessmasterFlicState operateCursor;
	loadError = decodeInitialFlicFrame(runtime, "GRAPHIC/ROOMANIM/OPERATE.FLC", operateCursor);
	if (loadError.getCode() != Common::kNoError)
		return loadError;

	loadError = waitForPointerRelease();
	if (loadError.getCode() != Common::kNoError)
		return loadError;

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!SHOULD_QUIT) {
		Common::Error advanceError = advanceFlicIfNeeded(background, true);
		if (advanceError.getCode() != Common::kNoError)
			return advanceError;
		advanceError = advanceFlicIfNeeded(pointer, true);
		if (advanceError.getCode() != Common::kNoError)
			return advanceError;
		advanceError = advanceFlicIfNeeded(operateCursor, true);
		if (advanceError.getCode() != Common::kNoError)
			return advanceError;

		const Common::Point mousePos = g_system->getEventManager()->getMousePos();
		const bool onInteractiveHotspot = pointHitsHotspot(mousePos, stage.successHotspot) ||
			pointHitsAnyHotspot(mousePos, stage.failureHotspots, stage.failureHotspotCount);

		drawFlicFrame(*screen, background);
		const ChessmasterFlicState &cursorState = onInteractiveHotspot ? operateCursor : pointer;
		if (cursorState.frame)
			blitTransparentFrame(*screen, *cursorState.frame, mousePos.x, mousePos.y);
		screen->makeAllDirty();
		screen->update();

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return Common::kNoError;
			if (event.type != Common::EVENT_LBUTTONDOWN)
				continue;

			if (pointHitsHotspot(event.mouse, stage.successHotspot)) {
				selectedSuccessHotspot = true;
				return Common::kNoError;
			}
			if (pointHitsAnyHotspot(event.mouse, stage.failureHotspots, stage.failureHotspotCount))
				return Common::kNoError;
		}

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

static Common::Error runChessmasterThreeMovePuzzle(DialogueRuntime &runtime, bool &solvedPuzzle) {
	solvedPuzzle = false;

	for (uint i = 0; i < ARRAYSIZE(kChessmasterPuzzleStages); ++i) {
		bool selectedSuccessHotspot = false;
		Common::Error stageError = runChessmasterChoiceStage(
			runtime, kChessmasterPuzzleStages[i], selectedSuccessHotspot);
		if (stageError.getCode() != Common::kNoError)
			return stageError;
		if (runtime.engine().shouldQuit())
			return Common::kNoError;
		if (!selectedSuccessHotspot)
			return Common::kNoError;

		stageError = playOneShotFlic(runtime, kChessmasterPuzzleStages[i].moveFlicPath);
		if (stageError.getCode() != Common::kNoError)
			return stageError;
		if (runtime.engine().shouldQuit())
			return Common::kNoError;
	}

	Common::Error delayError = runTimedDelay(kChessmasterSuccessHoldMs);
	if (delayError.getCode() != Common::kNoError)
		return delayError;

	solvedPuzzle = true;
	return Common::kNoError;
}

} // End of namespace

bool ChessmasterDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kChessmasterNpc);
}

Common::Error ChessmasterDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	Common::Error lineError = runtime.playDialogueLine(0x4115, kChessmasterNpc);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(0x22, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	bool solvedPuzzle = false;
	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x4122, kChessmasterNpc, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runChessmasterThreeMovePuzzle(runtime, solvedPuzzle);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (runtime.engine().shouldQuit())
			return Common::kNoError;
	}

	if (responseIndex != 1 || !solvedPuzzle) {
		lineError = runtime.playDialogueLineWithVariant(0x4126, kChessmasterNpc, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		InteractionResult interaction;
		if (runtime.executeActionTag(kChessKingMonsterActionTag, interaction)) {
			interaction.cutscenePath = "GRAPHIC/FST/KINGGROW.FST";
			runtime.applyImmediateDialogueInteractionEffects(interaction);
			runtime.queueDialogueInteractionIfNeeded(interaction);
			return Common::kNoError;
		}

		return runtime.playDialogueFst("GRAPHIC/FST/KINGGROW.FST");
	}

	(void)runtime.startupScript().setRuntimeFlagValue(kChessSolvedFlag, true);
	lineError = runtime.playDialogueFst("GRAPHIC/FST/C125.FST");
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	InteractionResult interaction;
	// C125.FST already shows the chessmaster death; do not queue the room death bank again.
	if (runtime.startupScript().finalizeRuntimeNpcDeathOrMonsterfy(
			kChessmasterNpc, kChessmasterSlashDamageType)) {
		interaction.mutatedRuntimeState = true;
		interaction.visualRuntimeStateChanged = true;
	}
	runtime.queueDialogueInteractionIfNeeded(interaction);
	return Common::kNoError;
}

} // End of namespace Harvester
