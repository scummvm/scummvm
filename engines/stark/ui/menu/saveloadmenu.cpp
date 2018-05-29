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

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/gfx/surfacerenderer.h"

#include "engines/stark/stark.h"
#include "engines/stark/savemetadata.h"

#include "common/config-manager.h"
#include "common/savefile.h"

#include "gui/message.h"

namespace Stark {

SaveLoadMenuScreen::SaveLoadMenuScreen(Gfx::Driver *gfx, Cursor *cursor, Screen::Name screenName) :
		StaticLocationScreen(gfx, cursor, "LoadSaveLocation", screenName) {
}

SaveLoadMenuScreen::~SaveLoadMenuScreen() {
}

void SaveLoadMenuScreen::open() {
	StaticLocationScreen::open();

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
			nullptr,
			nullptr));
	_widgets.back()->setupSounds(0, 1);
	_widgets.back()->setTextColor(_textColorBlack);
	
	_widgets.push_back(new StaticLocationWidget(
			"Next",
			nullptr,
			nullptr));
	_widgets.back()->setupSounds(0, 1);
	_widgets.back()->setTextColor(_textColorBlack);
	
	_widgets.push_back(new SaveDataWidget(
			0, _gfx, this));
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

void SaveMenuScreen::open() {
	SaveLoadMenuScreen::open();
	_widgets[kWidgetLoadText]->setVisible(false);
}

void LoadMenuScreen::open() {
	SaveLoadMenuScreen::open();
	_widgets[kWidgetSaveText]->setVisible(false);
}

SaveDataWidget::SaveDataWidget(int slot, Gfx::Driver *gfx, SaveLoadMenuScreen *screen) :
		StaticLocationWidget(nullptr, nullptr, nullptr),
		_slot(slot),
		_screen(screen) {
	_texture = gfx->createTexture();
	_surfaceRenderer = gfx->createSurfaceRenderer();

	Common::String filename = StarkEngine::formatSaveName(ConfMan.getActiveDomainName().c_str(), _slot);
	Common::InSaveFile *save = g_system->getSavefileManager()->openForLoading(filename);

	if (save) {
		// Obtain the thumbnail
		SaveMetadata metadata;
		StateReadStream stream(save);
		Common::ErrorCode metadataErrorCode = metadata.read(&stream, filename);
		if (metadataErrorCode != Common::kNoError) {
			error("Unable to read save metadata with error code %d.", metadataErrorCode);
		}

		Graphics::Surface *thumb = metadata.readGameScreenThumbnail(&stream);
		_texture->update(thumb);
	}
}

SaveDataWidget::~SaveDataWidget() {
	delete _texture;
	delete _surfaceRenderer;
}

void SaveDataWidget::render() {
	_surfaceRenderer->render(_texture, Common::Point(200, 200));
}

bool SaveDataWidget::isMouseInside(const Common::Point &mousePos) const {
	return mousePos.x >= 200 && mousePos.x <= 200 + _texture->width() &&
		   mousePos.y >= 200 && mousePos.y <= 200 + _texture->height();
}

void SaveDataWidget::onClick() {
	StaticLocationWidget::onClick();
	_screen->clickHandler(_slot);
}

void SaveDataWidget::onMouseMove(const Common::Point &mousePos) {
	StaticLocationWidget::onMouseMove(mousePos);
}

} // End of namespace Stark
