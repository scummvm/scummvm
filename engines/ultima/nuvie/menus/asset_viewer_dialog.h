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

#ifndef NUVIE_MENUS_ASSET_VIEWER_DIALOG_H
#define NUVIE_MENUS_ASSET_VIEWER_DIALOG_H

#include "ultima/nuvie/gui/gui_dialog.h"
#include "ultima/nuvie/core/nuvie_defs.h"

namespace Ultima {
namespace Nuvie {

#define GAMEMENUDIALOG_CB_DELETE 3

class GUI;
class GUI_CallBack;
class GUI_Button;
class GUI_Text;
class U6Shape;

class AssetViewerDialog : public GUI_Dialog {
	enum ViewMode {
		TileViewMode,
		ScreenViewMode,
	};

protected:
	CallBack *callback_object;
	GUI_Text *_titleTxt, *_infoTxt;
	int _curIdx, _maxIdx;
	int _shapeIdx;
	ViewMode _viewMode;
	U6Shape *_curShape;
	Common::Path _screenFile;
public:
	AssetViewerDialog(CallBack *callback);
	~AssetViewerDialog() override;

	void Display(bool full_redraw) override;
	GUI_status close_dialog();
	GUI_status KeyDown(const Common::KeyState &key) override;
	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;

private:
	void updateInfoTxt();
	void updateShape();
	bool init();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
