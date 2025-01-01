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

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/file.h"
#include "common/util.h"
#include "common/events.h"

#include "graphics/scaler.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/characters_shadow.h"
#include "tetraedge/game/in_game_scene.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_resource_manager.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_font2.h"
#include "tetraedge/te/te_font3.h"
#include "tetraedge/te/te_input_mgr.h"
#include "tetraedge/te/te_sound_manager.h"

//#define TETRAEDGE_DUMP_LAYOUTS

namespace Tetraedge {

bool Application::_dontUpdateWhenApplicationPaused = false;

Application::Application() : _finishedGame(false), _finishedFremium(false),
_captureFade(false), _difficulty(1), _created(false), _tutoActivated(false),
_drawShadows(true), _compassLook(false), _inverseLook(false),
_permanentHelp(true), _musicOn(true) {
	//
	// TODO: Game defaults _ratioStretched to false, but then
	// the horizontally scrolling scenes don't scroll properly.
	// For now just default to true.
	//
	_ratioStretched = true;

	TeCore *core = g_engine->getCore();
	core->_coreNotReady = true;
	const char *platform = "";
	switch (g_engine->getGamePlatform()) {
	case Common::Platform::kPlatformAndroid:
		platform = "Android";
		core->fileFlagSystemSetFlag("pad", "padDisabled");
		break;
	case Common::Platform::kPlatformMacintosh:
		platform = "MacOSX";
		break;
	case Common::Platform::kPlatformIOS:
		platform = "iPhone";
		break;
	case Common::Platform::kPlatformNintendoSwitch:
		platform = "NX";
		core->fileFlagSystemSetFlag("pad", "padDisabled");
		break;
	case Common::Platform::kPlatformPS3:
		platform = "PS3";
		break;
	default:
		error("Unsupported platform");
	}
	core->fileFlagSystemSetFlag("platform", platform);
	//
	// WORKAROUND: Syberia 2 A5_ValDomaine/54000/Logic54000.lua
	// checks a typo of this flag..
	//
	core->fileFlagSystemSetFlag("plateform", platform);

	core->fileFlagSystemSetFlag("part", "Full");
	if (g_engine->isGameDemo())
		core->fileFlagSystemSetFlag("distributor", "Freemium");
	else
		core->fileFlagSystemSetFlag("distributor", "DefaultDistributor");

	TeLuaGUI tempGui;
	tempGui.load("texts/Part.lua");
	_applicationTitle = tempGui.value("applicationTitle").toString();
	_versionString = tempGui.value("versionString").toString();
	_firstWarpPath = tempGui.value("firstWarpPath").toString();
	_firstZone = tempGui.value("firstZone").toString();
	_firstScene = tempGui.value("firstScene").toString();

	TeSoundManager *soundmgr = g_engine->getSoundManager();
	soundmgr->setChannelVolume("sfx", 0.7f);
	soundmgr->setChannelVolume("music", 0.7f);
	soundmgr->setChannelVolume("dialog", 0.7f);
	soundmgr->setChannelVolume("video", 0.7f);
	// TODO: Configure freemium things here?

	// Note: original has an app run timer, but it's never used?

	_defaultCursor = g_engine->gameIsAmerzone() ? "2D/arrow6.png" : "pictures/cursor.png";

	loadOptions("options.xml");
}

Application::~Application() {
	destroy();
}

void Application::create() {
	// TODO: Move mainWindowCamera to mainWindow?

	const int winWidth = g_engine->getDefaultScreenWidth();
	const int winHeight = g_engine->getDefaultScreenHeight();

	// See TeMainWindowBase::initCamera
	_mainWindowCamera.reset(new TeCamera());
	_mainWindowCamera->setName("_mainWinCam");
	_mainWindowCamera->setProjMatrixType(4);
	_mainWindowCamera->viewport(0, 0, winWidth, winHeight);
	_mainWindowCamera->orthogonalParams(winWidth * -0.5f, winWidth * 0.5f, winHeight * 0.5f, winHeight * -0.5f);
	_mainWindowCamera->setOrthoPlanes(-2048.0f, 2048.0f);

	_mainWindow.setSize(TeVector3f32(winWidth, winHeight, 0.0));
	_mainWindow.setSizeType(TeILayout::ABSOLUTE);
	_mainWindow.setPositionType(TeILayout::ABSOLUTE);
	_mainWindow.setPosition(TeVector3f32(0.0f, 0.0f, 0.0f));
	_mainWindow.setName("TeEngine Application");

	TeResourceManager *resmgr = g_engine->getResourceManager();
	TeCore *core = g_engine->getCore();
	// Cache some fonts
	if (g_engine->gameIsAmerzone()) {
		resmgr->getResource<TeFont2>(core->findFile("Common/Fonts/Arial_r_10.tef"));
		resmgr->getResource<TeFont2>(core->findFile("Common/Fonts/Arial_r_12.tef"));
		resmgr->getResource<TeFont2>(core->findFile("Common/Fonts/Arial_r_16.tef"));
		resmgr->getResource<TeFont2>(core->findFile("Common/Fonts/Colaborate-Regular_r_16.tef"));
		resmgr->getResource<TeFont2>(core->findFile("Common/Fonts/Colaborate-Regular_r_24.tef"));
		resmgr->getResource<TeFont2>(core->findFile("Common/Fonts/Credits.tef"));
		resmgr->getResource<TeFont2>(core->findFile("Common/Fonts/FontLoadingMenu.tef"));
	} else {
		_fontComic = resmgr->getResource<TeFont3>(core->findFile("Common/Fonts/ComicRelief.ttf"));
		_fontArgh = resmgr->getResource<TeFont3>(core->findFile("Common/Fonts/Argh.ttf"));
		_fontArial = resmgr->getResource<TeFont3>(core->findFile("Common/Fonts/arial.ttf"));
		_fontChaucer = resmgr->getResource<TeFont3>(core->findFile("Common/Fonts/CHAUCER.TTF"));
		_fontColaborate = resmgr->getResource<TeFont3>(core->findFile("Common/Fonts/Colaborate-Regular.otf"));
		_fontProDisplay = resmgr->getResource<TeFont3>(core->findFile("Common/Fonts/ProDisplay.ttf"));
	}

	// The app prebuilds some fonts.. cover letters, numbers, a few accented chars, and punctuation.
	// Skip that here.
	/*
	TeTextBase2 textBase;
	textBase.setText("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/,*?;.:/!\xA7&\xE9\"'(-\xE8_\xE7\xE0)=");
	textBase.setFont(0, _fontComic);
	textBase.setRect(TeVector2s32(1, 1));
	textBase.setFontSize(12);
	textBase.build();
	textBase.setFontSize(14);
	textBase.build();
	textBase.setFontSize(16);
	textBase.build();
	textBase.setFontSize(18);
	textBase.build();
	textBase.setFontSize(30);
	textBase.build();
	textBase.setFont(0, _fontColaborate);
	textBase.setFontSize(18);
	textBase.build();
	textBase.setFont(0, _fontProDisplay);
	textBase.setFontSize(24);
	textBase.build();
	 */

	static const char allLangs[][3] = {"en", "fr", "de", "es", "it", "ru", "he"};
	const Common::Path textsPath("texts");

	// Try alternate langs..
	int i = 0;
	TetraedgeFSNode textFileNode;
	while (i < ARRAYSIZE(allLangs)) {
		textFileNode = core->findFile(textsPath.join(core->language() + ".xml"));
		if (textFileNode.exists())
			break;
		core->language(allLangs[i]);
		i++;
	}
	if (i == ARRAYSIZE(allLangs)) {
		error("Couldn't find texts/[lang].xml for any language.");
	}

	_loc.load(textFileNode);
	core->addLoc(&_loc);

	if (!g_engine->gameIsAmerzone()) {
		const Common::Path helpMenuPath("menus/help/help_");
		Common::Path helpMenuFilePath;
		Common::String lang(core->language());
		i = 0;
		while (i < ARRAYSIZE(allLangs)) {
			helpMenuFilePath = helpMenuPath.append(lang + ".xml");
			if (Common::File::exists(helpMenuFilePath))
				break;
			lang = allLangs[i];
			i++;
		}
		if (i == ARRAYSIZE(allLangs)) {
			error("Couldn't find menus/help/help_[lang].xml for any language.");
		}

		_helpGui.load(helpMenuFilePath);
	}

	// TODO: set TeCore field 0x74 and 0x78 to true here? Do they do anything?");

	// Game calls these here but does nothing with result?
	//TeGetDeviceDPI();
	//TeGetDeviceReferenceDPI();

	_backLayout.setName("layoutBack");
	_backLayout.setSizeType(TeLayout::CoordinatesType::RELATIVE_TO_PARENT);
	_backLayout.setSize(TeVector3f32(1.0f, 1.0f, 0.0f));
	_mainWindow.addChild(&_backLayout);

	_frontOrientationLayout.setName("orientationLayoutFront");
	_frontOrientationLayout.setSizeType(TeLayout::CoordinatesType::RELATIVE_TO_PARENT);
	_frontOrientationLayout.setSize(TeVector3f32(1.0f, 1.0f, 0.0f));
	_mainWindow.addChild(&_frontOrientationLayout);

	_frontLayout.setName("layoutFront");
	_frontLayout.setSizeType(TeLayout::CoordinatesType::RELATIVE_TO_PARENT);
	_frontLayout.setSize(TeVector3f32(1.0f, 1.0f, 0.0f));
	_frontOrientationLayout.addChild(&_frontLayout);

	_visFade.init();

	_frontOrientationLayout.addChild(&_visFade._fadeCaptureSprite);
	_frontOrientationLayout.addChild(&_visFade._blackFadeSprite);
	_frontOrientationLayout.addChild(&_visFade._buttonLayout);

	_frontLayout.addChild(&_appSpriteLayout);
	_appSpriteLayout.setSizeType(TeLayout::CoordinatesType::RELATIVE_TO_PARENT);
	_appSpriteLayout.setSize(TeVector3f32(1.0f, 1.0f, 1.0f));
	_appSpriteLayout.setVisible(false);

	// Note: The games do some loading of a "version.ver" file here to add a
	// watermark to the backLayout, but that file doesn't exist in any of the
	// GOG games so it was probably only used during development.
	if (Common::File::exists("version.ver")) {
		warning("Skipping doing anything with version.ver file");
	}

	_mouseCursorLayout.setName("mouseCursor");

	// Not needed in scummvm:
	g_system->showMouse(false);
	//mainWindow->setNativeCursorVisible(false);

	_mouseCursorLayout.load(_defaultCursor);
	_mouseCursorLayout.setAnchor(TeVector3f32(0.3f, 0.1f, 0.0f));
	_frontOrientationLayout.addChild(&_mouseCursorLayout);

	_lockCursorButton.setName("lockCursorButton");
	_lockCursorButton.setSizeType(TeLayout::CoordinatesType::RELATIVE_TO_PARENT);
	_lockCursorButton.setSize(TeVector3f32(2.0f, 0.095f, 0.0f));
	_lockCursorButton.setPositionType(TeLayout::CoordinatesType::RELATIVE_TO_PARENT);
	_lockCursorButton.setPosition(TeVector3f32(0.95f, 0.95f, 0.0f));
	_lockCursorButton.setVisible(false);
	_frontOrientationLayout.addChild(&_lockCursorButton);

	_lockCursorFromActionButton.setName("lockCursorFromActionButton");
	_lockCursorFromActionButton.setSizeType(TeLayout::CoordinatesType::RELATIVE_TO_PARENT);
	_lockCursorFromActionButton.setSize(TeVector3f32(2.0f, 2.0f, 0.0f));
	_lockCursorFromActionButton.setVisible(false);
	_frontOrientationLayout.addChild(&_lockCursorFromActionButton);

	_autoSaveIcon1.setName("autosaveIcon");
	_autoSaveIcon1.setAnchor(TeVector3f32(0.5f, 0.5f, 0.0f));
	_autoSaveIcon1.setPosition(TeVector3f32(0.2f, 0.9f, 0.0f));
	_autoSaveIcon1.setSize(TeVector3f32(128.0f, 64.0f, 0.0f));
	_autoSaveIcon1.load("menus/inGame/autosave_icon.png");
	_autoSaveIcon1.setVisible(false);
	_frontOrientationLayout.addChild(&_autoSaveIcon1);

	_autoSaveIconAnim1._runTimer.pausable(false);
	_autoSaveIconAnim1.pause();
	_autoSaveIconAnim1._startVal = TeColor(255, 255, 255, 0);
	_autoSaveIconAnim1._endVal = TeColor(255, 255, 255, 255);
	_autoSaveIconAnim1._repeatCount = -1;
	Common::Array<float> curve;
	curve.push_back(0.0f);
	curve.push_back(1.0f);
	curve.push_back(1.0f);
	curve.push_back(0.0f);
	_autoSaveIconAnim1.setCurve(curve);
	_autoSaveIconAnim1._duration = 4000.0f;
	_autoSaveIconAnim1._callbackObj = &_autoSaveIcon1;
	_autoSaveIconAnim1._callbackMethod = &Te3DObject2::setColor;

	_autoSaveIcon2.setName("autosaveIcon");
	_autoSaveIcon2.setAnchor(TeVector3f32(0.5f, 0.5f, 0.0f));
	_autoSaveIcon2.setPosition(TeVector3f32(0.2f, 0.7f, 0.0f));
	_autoSaveIcon2.setSize(TeVector3f32(64.0f, 86.0f, 0.0f));
	_autoSaveIcon2.load("menus/inGame/NoCel.png");
	_autoSaveIcon2.setVisible(false);
	_frontOrientationLayout.addChild(&_autoSaveIcon2);

	_autoSaveIconAnim2._runTimer.pausable(false);
	_autoSaveIconAnim2.pause();
	_autoSaveIconAnim2._startVal = TeColor(255, 255, 255, 0);
	_autoSaveIconAnim2._endVal = TeColor(255, 255, 255, 255);
	_autoSaveIconAnim2._repeatCount = 1;
	_autoSaveIconAnim2.setCurve(curve);
	_autoSaveIconAnim2._duration = 4000.0f;
	_autoSaveIconAnim2._callbackObj = &_autoSaveIcon2;
	_autoSaveIconAnim2._callbackMethod = &Te3DObject2::setColor;

	_visFade.blackFadeCurveAnim().onFinished().add(this, &Application::onBlackFadeAnimationFinished);

	g_engine->getInputMgr()->_mouseMoveSignal.add(this, &Application::onMousePositionChanged);

	onMainWindowSizeChanged();
	_splashScreens.enter();

	_drawShadows = !(g_engine->gameIsAmerzone() || ConfMan.getBool("disable_shadows"));

	// Note: this is not in the original, but seems like a good place to do it..
	g_engine->getGame()->loadUnlockedArtwork();

	_created = true;
}

void Application::destroy() {
	Character::animCacheFreeAll();

	_globalBonusMenu.unload();
	_bonusMenu.unload();
	_mainMenu.unload();
	_credits.leave();
	_ownerErrorMenu.unload();
	_splashScreens.unload();
}

void Application::startGame(bool newGame, int difficulty) {
	_appSpriteLayout.setVisible(false);
	_appSpriteLayout.pause();
	_appSpriteLayout.unload();
	if (newGame)
		_difficulty = difficulty;
	g_engine->getGame()->enter();
}

void Application::resume() {
	// Probably not needed.
	error("Implement Application::resume");
}

bool Application::run() {
	if (_created) {
		TeTimer::updateAll();
		if (!_dontUpdateWhenApplicationPaused) {
			// Note: we run the inputmgr separately.. probably no need for this.
			//_inputmgr->update();
			TeAnimation::updateAll();
			//TeVideo::updateAll();
		}
		_captureFade = false;

		TeRenderer *renderer = g_engine->getRenderer();
		Game *game = g_engine->getGame();

		renderer->reset();
		game->update();
		game->scene().updateScroll();
		g_engine->getSoundManager()->update();
		performRender();
		if (game->_returnToMainMenu) {
			game->leave(true);
			if (!game->luaShowOwnerError()) {
				_mainMenu.enter();
			} else {
				_ownerErrorMenu.enter();
			}
			game->_returnToMainMenu = false;
		}
		if (_finishedGame) {
			game->leave(false);
			_mainMenu.enter();
			if (Common::File::exists("finalURL.lua") || Common::File::exists("finalURL.data")) {
				TeLuaGUI finalGui;
				finalGui.load("finalURL.lua");
				/*TeVariant finalVal =*/ finalGui.value("finalURL");
				// Not clear if this variant is ever used in original.
				debug("TODO: use final URL??");
				finalGui.unload();
			}
			_finishedGame = false;
		}
		TeObject::deleteNow();
	}
	return true;
}

void Application::suspend() {
	// Probably not needed.
	error("Implement Application::suspend");
}

void Application::showNoCellIcon(bool show) {
	if (!show) {
		_autoSaveIconAnim2._repeatCount = 1;
	} else {
		_autoSaveIcon2.setVisible(true);
		_autoSaveIcon2.setColor(TeColor(255, 255, 255, 255));
		_autoSaveIconAnim2._repeatCount = -1;
		_autoSaveIconAnim2.play();
	}
}

void Application::showLoadingIcon(bool show) {
	if (!show) {
		_autoSaveIconAnim1._repeatCount = 1;
	} else {
		_autoSaveIcon1.setVisible(true);
		_autoSaveIcon1.setColor(TeColor(255, 255, 255, 255));
		_autoSaveIconAnim1._repeatCount = -1;
		_autoSaveIconAnim1.play();
	}
}

void Application::saveCorrupted(const Common::String &fname) {
	// Probably not needed.
	error("Implement Application::saveCorrupted");
}

void Application::drawBack() {
	_mainWindowCamera->apply();
	_backLayout.draw();
	TeCamera::restore();
	g_engine->getRenderer()->loadIdentityMatrix();
}

void Application::drawFront() {
	_mainWindowCamera->apply();
	_frontOrientationLayout.draw();
	TeCamera::restore();
	g_engine->getRenderer()->loadIdentityMatrix();
}

#ifdef TETRAEDGE_DUMP_LAYOUTS
static int renderCount = 0;
static void dumpLayout(TeLayout *layout, Common::String indent = "++") {
	assert(layout);
	if (!layout->worldVisible())
		return;
	debug("%s '%s'%s  pos:%s  worldScale:%s  userSize:%s  size:%s  col:%s", indent.c_str(), layout->name().c_str(), (layout->worldVisible() ? "" : " (invis)"),
			layout->position().dump().c_str(), layout->worldScale().dump().c_str(),
			layout->userSize().dump().c_str(), layout->size().dump().c_str(),
			layout->color().dump().c_str());
	for (auto & child: layout->childList()) {
		TeLayout *childLayout = dynamic_cast<TeLayout *>(child);
		if (childLayout)
			dumpLayout(childLayout, indent + "++");
	}
}
#endif

void Application::performRender() {
	Game *game = g_engine->getGame();
	TeRenderer *renderer = g_engine->getRenderer();

	if (_drawShadows && game->running() && game->scene()._character
			&& game->scene().shadowLightNo() != -1
			&& game->scene().charactersShadow() != nullptr) {
		renderer->shadowMode(TeRenderer::ShadowModeCreating);
		game->scene().charactersShadow()->createTexture(&game->scene());
		renderer->shadowMode(TeRenderer::ShadowModeNone);
	}

	drawBack();

	renderer->renderTransparentMeshes();
	renderer->clearBuffer(TeRenderer::DepthBuffer);
	if (game->running()) {
		if (_drawShadows && game->scene()._character
			&& game->scene().shadowLightNo() != -1
			&& game->scene().charactersShadow() != nullptr) {
			TeIntrusivePtr<TeCamera> currentCamera = game->scene().currentCamera();
			if (currentCamera) {
				currentCamera->apply();
				renderer->shadowMode(TeRenderer::ShadowModeDrawing);
				game->scene().charactersShadow()->draw(&game->scene());
				renderer->shadowMode(TeRenderer::ShadowModeNone);
			}
		}
		game->draw();
	}

	renderer->renderTransparentMeshes();
	renderer->clearBuffer(TeRenderer::DepthBuffer);
	drawFront();
	renderer->renderTransparentMeshes();
	game->scene().drawPath();
	renderer->updateScreen();

#ifdef TETRAEDGE_DUMP_LAYOUTS
	renderCount++;
	if (renderCount % 100 == 0) {
		debug("\n--------------------\nFrame %d back layout: ", renderCount);
		dumpLayout(&_backLayout);
		debug("\n--------------------\nFrame %d front orientation layout: ", renderCount);
		dumpLayout(&_frontOrientationLayout);
	}
#endif

}

//void Application::preloadTextrue(); does nothing..

void Application::fade() {
	_visFade.animateFade();
}

void Application::blackFade() {
	_visFade.animateBlackFade();
}

void Application::captureFade() {
	if (_captureFade)
		return;
	_captureFade = true;
	performRender();
	_visFade.captureFrame();
}

void Application::getSavegameThumbnail(Graphics::Surface &thumb) {
	captureFade();
	Graphics::Surface screen;
	_visFade.texture()->writeTo(screen);
	screen.flipVertical(Common::Rect(screen.w, screen.h));
	Common::ScopedPtr<Graphics::Surface> scaledScreen(screen.scale(kThumbnailWidth, kThumbnailHeight2));
	thumb.copyFrom(*scaledScreen);
	screen.free();
	scaledScreen->free();
}

bool Application::isFading() {
	return _visFade.blackFading() || _visFade.fading();
}

bool Application::onBlackFadeAnimationFinished() {
	_visFade._blackFadeSprite.setVisible(false);
	_visFade._buttonLayout.setVisible(false);
	return false;
}

bool Application::onMainWindowSizeChanged() {
	// This sets HD or SD "definition" in the core depending on device DPI.
	// For now just default to SD.
	debug("mainWindowSizeChanged: defaulting to SD.");
	g_engine->getCore()->fileFlagSystemSetFlag("definition", "SD");
	return false;
}

bool Application::onMousePositionChanged(const Common::Point &p) {
	const TeVector3f32 mainWinSize = _mainWindow.size();
	const TeVector3f32 newCursorPos(p.x / mainWinSize.x(), p.y / mainWinSize.y(), 0.0);
	_mouseCursorLayout.setPosition(newCursorPos);
	return false;
}

bool Application::isLockCursor() {
	return _lockCursorButton.visible() || _lockCursorFromActionButton.visible();
}

bool Application::isLockPad() {
	Game *game = g_engine->getGame();
	bool result = isLockCursor() || game->dialog2().isDialogPlaying() || game->isMoviePlaying()
		|| game->question2().gui().layoutChecked("background")->visible()
		|| game->isDocumentOpened();
	return result;
}

void Application::lockCursor(bool lock) {
	_lockCursorButton.setVisible(lock);
}

void Application::lockCursorFromAction(bool lock) {
	_lockCursorFromActionButton.setVisible(lock);
	g_engine->getGame()->showMarkers(lock);
}

void Application::loadOptions(const Common::String &fname) {
	// Probably not needed.  We sync confman in addArtworkUnlocked.
	debug("Application::loadOptions %s", fname.c_str());
}

void Application::saveOptions(const Common::String &fname) {
	// Probably not needed.  We sync confman in addArtworkUnlocked.
	debug("Application::saveOptions %s", fname.c_str());
}

Common::String Application::getHelpText(const Common::String &key) {
	return _helpGui.value(key);
}

const char *Application::inAppUnlockFullVersionID() {
	// Probably not needed.
	error("Implement Application::inAppUnlockFullVersionID");
}

} // end namespace Tetraedge
