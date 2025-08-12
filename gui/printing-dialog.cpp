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

#if defined(USE_PRINTING)

#include "common/translation.h"
#include "common/system.h"

#include "backends/printing/printman.h"

#include "gui/printing-dialog.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

namespace GUI {

PrintingDialog::PrintingDialog(const Graphics::ManagedSurface &surface)
	: Dialog("PrintingDialog"), _surface(surface) {
	_printButton = new GUI::ButtonWidget(this, "PrintingDialog.Print", _("Print"), Common::U32String(), kCmdPrint);
	_saveAsImageCheckbox = new GUI::CheckboxWidget(this, "PrintingDialog.SaveAsImage", _("Save as image"));

	_printersListPopUp = new GUI::PopUpWidget(this, "PrintingDialog.PrintersList", Common::U32String(), kCmdSelectPrinterName);
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
}

void PrintingDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCmdPrint: {
		Common::PrintingManager *printMan = g_system->getPrintingManager();
		if (_saveAsImageCheckbox->getState()) {
			printMan->saveAsImage(_surface);
		}
		printMan->doPrint(_surface);
		close();
		break;
	}
	case kCmdSelectPrinterName:
		g_system->getPrintingManager()->setPrinterName(_tagToPrinterName[data]);
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI

#endif
