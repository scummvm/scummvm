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

#include "common/printman.h"
#include "common/translation.h"
#include "common/system.h"

#include "gui/printing-dialog.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"

namespace GUI {

void GuiManager::printImage(const Graphics::ManagedSurface &surf) {
	GUI::PrintingDialog dialog(surf);
	dialog.runModal();
}

PrintingDialog::PrintingDialog(const Graphics::ManagedSurface &surface)
	: Dialog("PrintingDialog"), _surface(surface) {
	_preview = new GraphicsWidget(this, "PrintingDialog.Preview");

	int16 x, y, w, h;
	if (!g_gui.xmlEval()->getWidgetData("PrintingDialog.Preview", x, y, w, h))
		error("Failed to get widget data for PrintingDialog.Preview");

	// Scale the surface to fit the preview area, if needed
	float scaleX = (float)w / surface.w;
	float scaleY = (float)h / surface.h;
	float scale = MIN(scaleX, scaleY);

	// Draw page and center the image on it
	Graphics::ManagedSurface render(w, h, g_gui.theme()->getPixelFormat());
	render.fillRect(Common::Rect(0, 0, w, h), render.format.RGBToColor(255, 255, 255));
	render.blitFrom(_surface, _surface.getBounds(), Common::Rect((w - _surface.w * scale) / 2, (h - _surface.h * scale) / 2,
		(w + _surface.w * scale) / 2, (h + _surface.h * scale) / 2));

	_preview->setGfx(&render, false);

	_printButton = new ButtonWidget(this, "PrintingDialog.Print", _("Print"), Common::U32String(), kCmdPrint);
	_saveAsImageCheckbox = new CheckboxWidget(this, "PrintingDialog.SaveAsImage", _("Save as image"));

	new StaticTextWidget(this, "PrintingDialog.PrintersListDesc", _("Printer:"));
	_printersListPopUp = new PopUpWidget(this, "PrintingDialog.PrintersList", Common::U32String(), kCmdSelectPrinterName);
	Common::PrintingManager *printMan = g_system->getPrintingManager();
	Common::StringArray printerNames = printMan->listPrinterNames();

	Common::String defaultPrinterName = printMan->getDefaultPrinterName();
	uint32 defaultPrinterId = 0;
	uint32 tag = 0;

	for (auto &name : printerNames) {
		_printersListPopUp->appendEntry(name, tag);
		_tagToPrinterName[tag] = name;

		if (name == defaultPrinterName)
			defaultPrinterId = tag;

		tag++;
	}

	_printersListPopUp->setSelectedTag(defaultPrinterId);
	g_system->getPrintingManager()->setPrinterName(_tagToPrinterName[defaultPrinterId]);

	// Page settings
	new StaticTextWidget(this, "PrintingDialog.OrientationDesc", _("Orientation:"));
	_orientationPopUp = new PopUpWidget(this, "PrintingDialog.Orientation", Common::U32String(), kCmdSelectOrientation);
	_orientationPopUp->appendEntry("Portrait", kPageOrientationPortrait);
	_orientationPopUp->appendEntry("Landscape", kPageOrientationLandscape);
	// Pre-select orientation
	if (surface.w > surface.h) {
		_orientationPopUp->setSelectedTag(kPageOrientationLandscape);
		printMan->_orientation = kPageOrientationLandscape;
	}

	new ButtonWidget(this, "PrintingDialog.Cancel", _("Cancel"), Common::U32String(), kCloseCmd);
}

void PrintingDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCmdPrint: {
		Common::PrintingManager *printMan = g_system->getPrintingManager();
		if (_saveAsImageCheckbox->getState()) {
			printMan->saveAsImage(_surface);
		}
		printMan->doPrint(_surface, _surface.getBounds());
		close();
		break;
	}
	case kCmdSelectPrinterName:
		g_system->getPrintingManager()->setPrinterName(_tagToPrinterName[data]);
		break;
	case kCmdSelectOrientation:
		g_system->getPrintingManager()->_orientation = (PageOrientation)data;
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
