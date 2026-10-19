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

#include "harvester/flow/flow.h"
#include "harvester/flow/flow_support.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/framelimiter.h"
#include "harvester/art.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/scene/room_presentation.h"
#include "harvester/scene/scene_entities.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const char *const kQuickTipsPath = "ADJHEAD.RCS";
static const char *const kMenuPath = "MENU.INI";
static const char *const kMenuSectionName = "menu";
static const char *const kBuiltInMenuItems[] = {
	"NEW GAME",
	"SAVE GAME",
	"LOAD GAME",
	"OPTIONS",
	"HELP",
	"QUIT GAME"
};
static const int kNativeIdentTextLineSpacing = 3;
static const float kPaletteBrightnessBlack = 0.0f;
static const float kPaletteBrightnessFull = 1.0f;
static const uint32 kDemoEndingScreenDurationMs = 10000;
static const int kDemoEndingScreenNumbers[] = { 1, 2, 3, 4, 6 };
static const char *const kDemoEndingMusicPath = "SOUND/MUSIC/ROCKFITE.CMP";
static const uint32 kDemoIntroductionDurationMs = 17000;
static const char *const kDemoIntroductionBitmapPath = "1:/GRAPHIC/OTHER/MAINMENU.BM";
static const char *const kDemoIntroductionPalettePath = "1:/GRAPHIC/PAL/MAINMENU.PAL";
static const char *const kDemoIntroductionMusicPath = "SOUND/MUSIC/LODGERB.CMP";
static const char *const kDemoIntroductionFstPath = "GRAPHIC/FST/PCFALL.FST";
static const char *const kDemoIntroductionText =
	"After solving the enigmas of the Harvest town, you are thrust into the realm of "
	"the Order of the Harvest Moon, a bizarre institution of secretive knowledge.  "
	"However, the only thing you really care about is escaping from this hellish reality...";

static Common::String trimAsciiLine(const Common::String &value) {
	uint start = 0;
	uint end = value.size();

	while (start < end && (value[start] == ' ' || value[start] == '\t'))
		++start;
	while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r'))
		--end;

	return value.substr(start, end - start);
}

static bool loadQuickTipsScene(HarvesterEngine &engine, SceneState &scene) {
	RoomSetupState state;
	// The original startup path enters the START room first and overlays quick tips on top of it.
	if (!engine.getScript()->resolveRoomSetupState("START", state, *engine.getResources()))
		return false;

	return SceneEntities::loadRoomSceneResources(state, *engine.getResources(), scene);
}

static void renderQuickTipsScreen(HarvesterEngine &engine, const SceneState &scene,
		const MenuTextConfig &config, const QuickTipsLayout &layout,
		const Common::String &tipText) {
	Graphics::Screen *screen = engine.getScreen();
	if (!screen)
		return;

	RoomPresentation::drawRoomScene(engine, *screen, scene, scene.targetPaletteBrightness);
	drawQuickTipsPanel(engine, config, layout, tipText);

	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

} // End of anonymous namespace

bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	bitmap = IndexedBitmap();
	return decodeBitmap(data, bitmap);
}

bool Flow::load() {
	(void)loadMenuTextConfig(_engine, _menuTextConfig);
	return loadQuickTips() && loadMenuItems();
}

bool Flow::loadQuickTips() {
	_quickTips.clear();

	Common::Array<byte> data;
	if (!_engine.getResources()->loadFile(kQuickTipsPath, data)) {
		warning("Harvester: unable to load quick tips '%s'", kQuickTipsPath);
		return true;
	}

	Common::String currentLine;
	for (uint i = 0; i < data.size(); ++i) {
		const char ch = (char)data[i];
		if (ch == '\r')
			continue;
		if (ch == '\n') {
			Common::String trimmed = trimAsciiLine(currentLine);
			if (!trimmed.empty())
				_quickTips.push_back(Common::move(trimmed));
			currentLine.clear();
			continue;
		}

		currentLine += ch;
	}

	Common::String trimmed = trimAsciiLine(currentLine);
	if (!trimmed.empty())
		_quickTips.push_back(Common::move(trimmed));

	debugC(1, kDebugGeneral, "Harvester: loaded %u quick tips from '%s'", (uint)_quickTips.size(), kQuickTipsPath);
	return true;
}

bool Flow::loadMenuItems() {
	_menuItems.clear();

	Common::Array<byte> data;
	if (!_engine.getResources()->loadFile(kMenuPath, data)) {
		for (const char *item : kBuiltInMenuItems)
			_menuItems.push_back(item);
		debugC(2, kDebugGeneral,
			"Harvester: using %u built-in startup menu items because '%s' is unavailable",
			(uint)_menuItems.size(), kMenuPath);
		return true;
	}

	Common::MemoryReadStream stream(data.data(), data.size());
	Common::INIFile menu;
	menu.setDefaultSectionName(kMenuSectionName);
	menu.requireKeyValueDelimiter();
	menu.suppressValuelessLineWarning();
	if (!menu.loadFromStream(stream)) {
		warning("Harvester: unable to parse startup menu '%s'", kMenuPath);
		return true;
	}

	for (uint i = 1; i <= 6; ++i) {
		Common::String key = Common::String::format("main_menu_%u", i);
		Common::String value;
		if (menu.getKey(key, kMenuSectionName, value) && !value.empty())
			_menuItems.push_back(Common::move(value));
	}

	debugC(1, kDebugGeneral, "Harvester: loaded %u startup menu items from '%s'", (uint)_menuItems.size(), kMenuPath);
	return true;
}

Common::Error Flow::runQuickTips() {
	if (!_engine.getScript()->isQuickTipsEnabled() || _quickTips.empty())
		return Common::kNoError;

	Common::Error transitionError = beginRoomSetupTransition();
	if (transitionError.getCode() != Common::kNoError)
		return transitionError;

	SceneState scene;
	if (!loadQuickTipsScene(_engine, scene))
		return Common::kReadingFailed;

	Graphics::Screen *screen = _engine.getScreen();
	if (screen) {
		if (!populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
			return Common::kReadingFailed;

		transitionError = waitForRoomSetupTransitionHold();
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;

		logScenePaletteSummary("quick tips scene palette", scene, kPaletteBrightnessBlack);
		RoomPresentation::drawRoomScene(_engine, *screen, scene, kPaletteBrightnessBlack);
		screen->makeAllDirty();
		screen->update();

		logScenePaletteSummary("quick tips fade target", scene, scene.targetPaletteBrightness);
		transitionError = fadeInRoomScene(scene.palette, scene.targetPaletteBrightness);
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;
	}

	return runQuickTipsOverlay(scene);
}

Common::Error Flow::runQuickTipsOverlay(const SceneState &scene) {
	resetCursorAnimationSequence();

	QuickTipsLayout quickTipsLayout;
	if (!resolveQuickTipsLayout(_engine, _menuTextConfig, quickTipsLayout))
		return Common::kReadingFailed;
	const Common::String &toggleLabel = _engine.getScript()->isQuickTipsEnabled()
		? _menuTextConfig.quickTipsOnLabel : _menuTextConfig.quickTipsOffLabel;
	debugC(1, kDebugGeneral,
		"Harvester: quick tips panel='%s' header='%s' exit='%s' next='%s' toggle='%s'",
		_menuTextConfig.hasQuickTipsHeader() ? "TEXTBOX6.BM" : "TIPS.BM",
		_menuTextConfig.quickTipsHeader.c_str(), _menuTextConfig.quickTipsExitLabel.c_str(),
		_menuTextConfig.quickTipsNextLabel.c_str(), toggleLabel.c_str());

	uint tipIndex = _engine.getRandomNumber(_quickTips.size() - 1);
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderQuickTipsScreen(_engine, scene, _menuTextConfig, quickTipsLayout, _quickTips[tipIndex]);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				needsRedraw = true;
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (quickTipsLayout.exitRect.contains(_mousePos))
					return Common::kNoError;

				if (quickTipsLayout.nextRect.contains(_mousePos)) {
					tipIndex = (tipIndex + 1) % _quickTips.size();
					needsRedraw = true;
				} else if (quickTipsLayout.toggleRect.contains(_mousePos)) {
					_engine.getScript()->setQuickTipsEnabled(!_engine.getScript()->isQuickTipsEnabled());
					if (!resolveQuickTipsLayout(_engine, _menuTextConfig, quickTipsLayout))
						return Common::kReadingFailed;
					needsRedraw = true;
				}
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;
				break;
			default:
				break;
			}
		}

		if (tickRuntimeEntities())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error Flow::runDemoEnding() {
	Graphics::Screen *screen = _engine.getScreen();
	ResourceManager *resources = _engine.getResources();
	EntityManager *entityManager = _engine.getRuntimeEntities();
	if (!screen || !resources)
		return Common::kReadingFailed;

	if (entityManager)
		entityManager->hideCursor();
	_engine.stopSound();
	if (!_engine.playMusic(kDemoEndingMusicPath))
		return Common::kReadingFailed;

	for (uint i = 0; i < ARRAYSIZE(kDemoEndingScreenNumbers) && !_engine.shouldQuit(); ++i) {
		const int screenNumber = kDemoEndingScreenNumbers[i];
		const Common::String bitmapPath = Common::String::format(
			"1:/GRAPHIC/OTHER/SCREEN%d.BM", screenNumber);
		const Common::String palettePath = Common::String::format(
			"1:/GRAPHIC/PAL/SCREEN%d.PAL", screenNumber);
		IndexedBitmap bitmap;
		byte palette[256 * 3];
		if (!loadBitmapResource(*resources, bitmapPath, bitmap) ||
				!Art::loadPaletteResource(*resources, palettePath, palette)) {
			warning("Harvester: unable to load DOS demo ending screen %d", screenNumber);
			return Common::kReadingFailed;
		}

		setScaledPalette(*screen, palette, kPaletteBrightnessBlack);
		screen->fillRect(Common::Rect(0, 0, screen->w, screen->h), 0);
		RoomPresentation::blitBitmap(*screen, bitmap, 0, 0);
		screen->makeAllDirty();
		screen->update();

		Common::Error fadeError = fadePalette(
			palette, kPaletteBrightnessBlack, kPaletteBrightnessFull);
		if (fadeError.getCode() != Common::kNoError)
			return fadeError;

		const uint32 deadline = g_system->getMillis() + kDemoEndingScreenDurationMs;
		bool skipScreen = false;
		while (!skipScreen && !_engine.shouldQuit() &&
				(int32)(deadline - g_system->getMillis()) > 0) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error eventError = Common::kNoError;
				if (handleSystemEvent(event, eventError))
					return eventError;
				if (event.type == Common::EVENT_LBUTTONDOWN ||
						(event.type == Common::EVENT_KEYDOWN &&
						(event.kbd.keycode == Common::KEYCODE_SPACE ||
						event.kbd.keycode == Common::KEYCODE_ESCAPE ||
						event.kbd.keycode == Common::KEYCODE_RETURN))) {
					skipScreen = true;
				}
			}
			g_system->delayMillis(1);
		}

		if (i + 1 == ARRAYSIZE(kDemoEndingScreenNumbers))
			_engine.stopMusic();
		fadeError = fadePalette(
			palette, kPaletteBrightnessFull, kPaletteBrightnessBlack);
		if (fadeError.getCode() != Common::kNoError)
			return fadeError;
	}

	_engine.quitGame();
	return Common::kNoError;
}

Common::Error Flow::runDemoIntroduction() {
	Graphics::Screen *screen = _engine.getScreen();
	ResourceManager *resources = _engine.getResources();
	EntityManager *entityManager = _engine.getRuntimeEntities();
	const Art *art = _engine.getArt();
	const Text *text = _engine.getText();
	const CftFontResource *fontResource = text ? text->findFont("TEXTFONT") : nullptr;
	if (!screen || !resources || !art || !fontResource)
		return Common::kReadingFailed;

	IndexedBitmap bitmap;
	byte palette[256 * 3];
	if (!loadBitmapResource(*resources, kDemoIntroductionBitmapPath, bitmap) ||
			!Art::loadPaletteResource(*resources, kDemoIntroductionPalettePath, palette)) {
		return Common::kReadingFailed;
	}

	HarvesterCftFont font(*fontResource);
	if (!font.isValid())
		return Common::kReadingFailed;
	debugC(1, kDebugGeneral,
		"Harvester: showing DOS demo introduction for %u ms",
		kDemoIntroductionDurationMs);

	if (entityManager)
		entityManager->hideCursor();
	setScaledPalette(*screen, palette, kPaletteBrightnessBlack);
	screen->fillRect(Common::Rect(0, 0, screen->w, screen->h), 0);
	RoomPresentation::blitBitmap(*screen, bitmap, 0, 0);
	blitTransparentBitmap(*screen, art->getLogoBitmap(), 160, 0);
	RoomPresentation::drawWrappedText(*screen, font, kDemoIntroductionText, 90, 200, 420, 0xce,
		kNativeIdentTextLineSpacing);
	screen->makeAllDirty();
	screen->update();

	if (!_engine.playMusic(kDemoIntroductionMusicPath))
		return Common::kReadingFailed;
	Common::Error fadeError = fadePalette(
		palette, kPaletteBrightnessBlack, kPaletteBrightnessFull);
	if (fadeError.getCode() != Common::kNoError)
		return fadeError;

	const uint32 deadline = g_system->getMillis() + kDemoIntroductionDurationMs;
	bool skipIntroduction = false;
	while (!skipIntroduction && !_engine.shouldQuit() &&
			(int32)(deadline - g_system->getMillis()) > 0) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error eventError = Common::kNoError;
			if (handleSystemEvent(event, eventError))
				return eventError;
			if (event.type == Common::EVENT_LBUTTONDOWN ||
					event.type == Common::EVENT_RBUTTONDOWN ||
					(event.type == Common::EVENT_KEYDOWN &&
					(event.kbd.keycode == Common::KEYCODE_SPACE ||
					event.kbd.keycode == Common::KEYCODE_ESCAPE ||
					event.kbd.keycode == Common::KEYCODE_RETURN))) {
				skipIntroduction = true;
			}
		}
		g_system->delayMillis(1);
	}

	fadeError = fadePalette(palette, kPaletteBrightnessFull, kPaletteBrightnessBlack);
	if (fadeError.getCode() != Common::kNoError)
		return fadeError;
	_engine.stopMusic();

	FstPlayer fstPlayer(_engine);
	if (!fstPlayer.play(kDemoIntroductionFstPath))
		return Common::kReadingFailed;
	return Common::kNoError;
}

} // End of namespace Harvester
