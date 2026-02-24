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

#ifndef GUI_PRINTING_DIALOGS_H
#define GUI_PRINTING_DIALOGS_H

#include "gui/dialog.h"

namespace GUI {
class ButtonWidget;
class CheckboxWidget;
class PopUpWidget;
class GraphicsWidget;

class PrintingDialog : public Dialog {
public:
	enum {
		kCmdPrint = 'PRNT',
		kCmdSelectPrinterName = 'SLPN',
		kCmdSelectOrientation = 'SLOR',
		kCmdSaveAsImage = 'SVIM',
		kCmdFitToPage = 'FITP',
		kCmdCenter = 'CNTR',
	};

	PrintingDialog(const Graphics::ManagedSurface &surface, bool defaultFitToPage, bool defaultCenter, PageOrientation defaultOrientation);

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	void updatePreview();
	static Common::Rect computePlacementInContainer(int16 srcW, int16 srcH, int16 containerW, int16 containerH, bool fitToContainer, bool center);

	const Graphics::ManagedSurface &_surface;

	bool _initDone;
	GraphicsWidget *_preview;
	ButtonWidget *_printButton;
	CheckboxWidget *_saveAsImageCheckbox;
	CheckboxWidget *_fitToPageCheckbox;
	CheckboxWidget *_centerCheckbox;
	PopUpWidget *_printersListPopUp;

	PopUpWidget *_orientationPopUp;

	Common::HashMap<uint32, Common::String> _tagToPrinterName;
};

} // End of namespace GUI

#endif
