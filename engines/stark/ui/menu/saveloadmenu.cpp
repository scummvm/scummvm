/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/ui/menu/saveloadmenu.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/stateprovider.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/settings.h"
#include "engines/stark/services/gamechapter.h"
#include "engines/stark/services/gamemessage.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/gfx/surfacerenderer.h"

#include "engines/stark/stark.h"
#include "engines/stark/savemetadata.h"

#include "engines/engine.h"

#include "common/config-manager.h"
#include "common/savefile.h"

#include "gui/message.h"

namespace Stark {

const Color SaveDataWidget::_outlineColor = Color(0x1E, 0x1E, 0x96);
const Color SaveDataWidget::_textColor    = Color(0x5C, 0x48, 0x3D);

SaveLoadMenuScreen::SaveLoadMenuScreen(Gfx::Driver *gfx, Cursor *cursor, Screen::Name screenName) :
		StaticLocationScreen(gfx, cursor, "LoadSaveLocation", screenName),
		_page(0),
		_maxPage(10) {
}

SaveLoadMenuScreen::~SaveLoadMenuScreen() {
}

void SaveLoadMenuScreen::open() {
	StaticLocationScreen::open();

	_maxPage = computeMaxPage();

	_page = StarkSettings->getIntSetting(Settings::kSaveLoadPage);
	if (_page > _maxPage) {
		_page = _maxPage;
	}

	_widgets.push_back(new StaticLocationWidget(
			"loadsavebg",
			nullptr,
			nullptr));

	_widgets.push_back(new StaticLocationWidget(
			"back to index",
			CLICK_HANDLER(SaveLoadMenuScreen, backHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	_widgets.push_back(new StaticLocationWidget(
			"Cancel",
			CLICK_HANDLER(SaveLoadMenuScreen, backHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	_widgets.push_back(new StaticLocationWidget(
			"SaveText",
			nullptr,
			nullptr));

	_widgets.push_back(new StaticLocationWidget(
			"LoadText",
			nullptr,
			nullptr));

	_widgets.push_back(new StaticLocationWidget(
			"Back",
			CLICK_HANDLER(SaveLoadMenuScreen, prevPageHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);
	_widgets.back()->setTextColor(Color(0, 0, 0));
	_widgets.back()->setVisible(_page > 0);

	_widgets.push_back(new StaticLocationWidget(
			"Next",
			CLICK_HANDLER(SaveLoadMenuScreen, nextPageHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);
	_widgets.back()->setTextColor(Color(0, 0, 0));
	_widgets.back()->setVisible(_page < _maxPage);

	loadSaveData(_page);
}

void SaveLoadMenuScreen::close() {
	ConfMan.flushToDisk();
	StaticLocationScreen::close();
}

int SaveLoadMenuScreen::computeMaxPage() {
	const char *target = ConfMan.getActiveDomainName().c_str();

	int maxSlot = 0;
	Common::StringArray saves = StarkEngine::listSaveNames(target);
	for (Common::StringArray::const_iterator filename = saves.begin(); filename != saves.end(); filename++) {
		int slot = StarkEngine::getSaveNameSlot(target, *filename);

		if (slot > maxSlot) {
			maxSlot = slot;
		}
	}

	// Allow using one more page than the last page with saves
	int maxPage = CLIP((maxSlot / _slotPerPage) + 1, 0, 110);
	if (maxPage < 10) {
		maxPage = 10;
	}

	return maxPage;
}

void SaveLoadMenuScreen::backHandler() {
	StarkUserInterface->backPrevScreen();
}

void SaveLoadMenuScreen::checkError(Common::Error error) {
	if (error.getCode() != Common::kNoError) {
		GUI::MessageDialog dialog(error.getDesc());
		dialog.runModal();
	}
}

void SaveLoadMenuScreen::removeSaveDataWidgets() {
	assert(_widgets.size() == 7 + _slotPerPage);

	for (int i = 0; i < _slotPerPage; ++i) {
		delete _widgets.back();
		_widgets.pop_back();
	}
}

void SaveLoadMenuScreen::loadSaveData(int page) {
	for (int i = 0; i < _slotPerPage; ++i) {
		_widgets.push_back(new SaveDataWidget(i + page * _slotPerPage, _gfx, this));
	}
}

void SaveLoadMenuScreen::changePage(int page) {
	assert(page >= 0 && page <= _maxPage);

	removeSaveDataWidgets();
	loadSaveData(page);

	_widgets[kWidgetBack]->setVisible(page > 0);
	_widgets[kWidgetNext]->setVisible(page < _maxPage);

	StarkSettings->setIntSetting(Settings::kSaveLoadPage, page);
	_page = page;
}

void SaveMenuScreen::open() {
	SaveLoadMenuScreen::open();
	_widgets[kWidgetLoadText]->setVisible(false);
}

void SaveMenuScreen::onWidgetSelected(SaveDataWidget *widget) {
	if (widget->hasSave()) {
		_slotToSaveAfterConfirm = widget;

		Common::String format = StarkGameMessage->getTextByKey(GameMessage::kOverwriteSave);
		Common::String prompt = Common::String::format(format.c_str(), widget->getName().c_str());

		StarkUserInterface->confirm(prompt, this, &SaveMenuScreen::saveConfirmSlot);
	} else {
		saveGameToSlot(widget);
	}
}

void SaveMenuScreen::saveConfirmSlot() {
	assert(_slotToSaveAfterConfirm);

	saveGameToSlot(_slotToSaveAfterConfirm);
	_slotToSaveAfterConfirm = nullptr;
}

void SaveMenuScreen::saveGameToSlot(SaveDataWidget *widget) {
	checkError(g_engine->saveGameState(widget->getSlot(), StarkGameChapter->getCurrentChapterTitle()));

	// Freeze the screen for a while to let the user notice the change
	widget->loadSaveDataElements();
	render();
	StarkGfx->flipBuffer();
	g_system->delayMillis(100);
	render();
	StarkGfx->flipBuffer();

	StarkUserInterface->backPrevScreen();
}

void LoadMenuScreen::open() {
	SaveLoadMenuScreen::open();
	_widgets[kWidgetSaveText]->setVisible(false);
}

void LoadMenuScreen::onWidgetSelected(SaveDataWidget *widget) {
	if (!StarkGlobal->getCurrent()) {
		checkError(g_engine->loadGameState(widget->getSlot()));
	} else {
		_slotToLoadAfterConfirm = widget->getSlot();
		StarkUserInterface->confirm(GameMessage::kEndAndLoad, this, &LoadMenuScreen::loadConfirmSlot);
	}
}

void LoadMenuScreen::loadConfirmSlot() {
	assert(_slotToLoadAfterConfirm >= 0);

	checkError(g_engine->loadGameState(_slotToLoadAfterConfirm));
	_slotToLoadAfterConfirm = -1;
}

SaveDataWidget::SaveDataWidget(int slot, Gfx::Driver *gfx, SaveLoadMenuScreen *screen) :
		StaticLocationWidget(nullptr, nullptr, nullptr),
		_slot(slot),
		_screen(screen),
		_thumbWidth(kThumbnailWidth),
		_thumbHeight(kThumbnailHeight),
		_texture(gfx->createTexture()),
		_outline(gfx->createTexture()),
		_surfaceRenderer(gfx->createSurfaceRenderer()),
		_textDesc(gfx),
		_textTime(gfx),
		_isMouseHovered(false),
		_hasSave(false),
		_name() {
	// Load from the save data
	loadSaveDataElements();

	_textDesc.setColor(_textColor);
	_textDesc.setFont(FontProvider::kCustomFont, 3);

	_textTime.setColor(_textColor);
	_textTime.setFont(FontProvider::kCustomFont, 3);

	Graphics::PixelFormat pixelFormat = Gfx::Driver::getRGBAPixelFormat();
	uint32 outlineColor = pixelFormat.ARGBToColor(
			_outlineColor.a, _outlineColor.r, _outlineColor.g, _outlineColor.b
	);

	// Create the outline texture
	Graphics::Surface lineSurface;
	lineSurface.create(_thumbWidth, _thumbHeight, pixelFormat);
	lineSurface.drawThickLine(0, 0, _thumbWidth - 1, 0, 2, 2, outlineColor);
	lineSurface.drawThickLine(0, 0, 0, _thumbHeight - 1, 2, 2, outlineColor);
	lineSurface.drawThickLine(_thumbWidth - 2, 0, _thumbWidth - 2, _thumbHeight - 2, 2, 2, outlineColor);
	lineSurface.drawThickLine(0, _thumbHeight - 2, _thumbWidth - 2, _thumbHeight - 2, 2, 2, outlineColor);

	_outline->update(&lineSurface);
	lineSurface.free();

	// Set the position
	_thumbPos.x = 41 + (_slot % SaveLoadMenuScreen::_slotPerRow) * (_thumbWidth + 39);
	_thumbPos.y = 61 + (_slot % SaveLoadMenuScreen::_slotPerPage / SaveLoadMenuScreen::_slotPerColumn) * (_thumbHeight + 38);

	_textDescPos.x = _thumbPos.x;
	_textDescPos.y = _thumbPos.y + _thumbHeight + 2;

	_textTimePos.x = _thumbPos.x;
	_textTimePos.y = _textDescPos.y + 12;
}

SaveDataWidget::~SaveDataWidget() {
	delete _texture;
	delete _outline;
	delete _surfaceRenderer;
}

void SaveDataWidget::render() {
	_surfaceRenderer->render(_texture, _thumbPos);
	_textDesc.render(_textDescPos);
	_textTime.render(_textTimePos);
	if (_isMouseHovered) {
		_surfaceRenderer->render(_outline, _thumbPos);
	}
}

bool SaveDataWidget::isMouseInside(const Common::Point &mousePos) const {
	return mousePos.x >= _thumbPos.x && mousePos.x <= _thumbPos.x + _thumbWidth &&
		   mousePos.y >= _thumbPos.y && mousePos.y <= _thumbPos.y + _thumbHeight;
}

void SaveDataWidget::onClick() {
	StaticLocationWidget::onClick();
	_screen->onWidgetSelected(this);
}

void SaveDataWidget::onMouseMove(const Common::Point &mousePos) {
	StaticLocationWidget::onMouseMove(mousePos);
	_isMouseHovered = isMouseInside(mousePos);
}

void SaveDataWidget::onScreenChanged() {
	StaticLocationWidget::onScreenChanged();
	_textDesc.resetTexture();
	_textTime.resetTexture();
}

void SaveDataWidget::loadSaveDataElements() {
	Common::String filename = StarkEngine::formatSaveName(ConfMan.getActiveDomainName().c_str(), _slot);
	Common::InSaveFile *save = g_system->getSavefileManager()->openForLoading(filename);
	if (save) {
		_hasSave = true;

		SaveMetadata metadata;
		StateReadStream stream(save);
		Common::ErrorCode metadataErrorCode = metadata.read(&stream, filename);
		if (metadataErrorCode != Common::kNoError) {
			error("Unable to read save metadata with error code %d.", metadataErrorCode);
		}

		// Obtain the thumbnail
		if (metadata.version >= 9) {
			Graphics::Surface *thumb = metadata.readGameScreenThumbnail(&stream);
			_texture->update(thumb);
			_texture->setSamplingFilter(StarkSettings->getImageSamplingFilter());

			thumb->free();
			delete thumb;
		}

		// Obtain the text
		Common::String desc = metadata.description;
		Common::String time = Common::String::format("%02d:%02d:%02d %02d/%02d/%02d",
				metadata.saveHour, metadata.saveMinute, metadata.saveSecond,
				metadata.saveMonth, metadata.saveDay, metadata.saveYear % 100);

		_textDesc.setText(desc);
		_textTime.setText(time);
		_name = desc + " " + time;
	} else {
		_hasSave = false;
		setVisible(_screen->isSaveMenu());
	}
}

} // End of namespace Stark
