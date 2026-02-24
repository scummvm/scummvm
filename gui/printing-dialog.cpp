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

#include "graphics/palette.h"

#include "gui/printing-dialog.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"

namespace GUI {

void GuiManager::printImage(const Graphics::ManagedSurface &surf, bool defaultFitToPage, bool defaultCenter, PageOrientation defaultOrientation) {
	GUI::PrintingDialog dialog(surf, defaultFitToPage, defaultCenter, defaultOrientation);
	dialog.runModal();
}

void GuiManager::printImage(const Graphics::ManagedSurface &surf) {
	PageOrientation defaultOrientation = (surf.w > surf.h) ? kPageOrientationLandscape : kPageOrientationPortrait;
	GUI::PrintingDialog dialog(surf, true, true, defaultOrientation);
	dialog.runModal();
}

PrintingDialog::PrintingDialog(const Graphics::ManagedSurface &surface, bool defaultFitToPage, bool defaultCenter, PageOrientation defaultOrientation)
	: Dialog("PrintingDialog"), _surface(surface) {

	_initDone = false;

	_preview = new GraphicsWidget(this, "PrintingDialog.Preview");
	_printButton = new ButtonWidget(this, "PrintingDialog.Print", _("Print"), Common::U32String(), kCmdPrint);

	_saveAsImageCheckbox = new CheckboxWidget(this, "PrintingDialog.SaveAsImage", _("Save as image"), Common::U32String(), kCmdSaveAsImage);
	_saveAsImageCheckbox->setState(false);
	_fitToPageCheckbox = new CheckboxWidget(this, "PrintingDialog.FitToPage", _("Fit to page"), Common::U32String(), kCmdFitToPage);
	_fitToPageCheckbox->setState(defaultFitToPage);
	_centerCheckbox = new CheckboxWidget(this, "PrintingDialog.Center", _("Center"), Common::U32String(), kCmdCenter);
	_centerCheckbox->setState(defaultCenter);

	new StaticTextWidget(this, "PrintingDialog.PrintersListDesc", _("Printer:"));
	_printersListPopUp = new PopUpWidget(this, "PrintingDialog.PrintersList", Common::U32String(), kCmdSelectPrinterName);

	Common::PrintingManager *printMan = g_system->getPrintingManager();
	Common::StringArray printerNames = printMan->listPrinterNames();
	Common::String defaultPrinterName = printMan->getDefaultPrinterName();

	if (printerNames.empty()) {
		// Only the "Save as image" option will be usable, check it in advance
		_saveAsImageCheckbox->setState(true);
	}

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

	_orientationPopUp->setSelectedTag(defaultOrientation);
	printMan->_orientation = defaultOrientation;

	new ButtonWidget(this, "PrintingDialog.Cancel", _("Cancel"), Common::U32String(), kCloseCmd);

	_initDone = true;
	updatePreview();
}

Common::Rect PrintingDialog::computePlacementInContainer(int16 srcW, int16 srcH, int16 containerW, int16 containerH, bool fitToContainer, bool center) {
	if (!srcW || !srcH)
		return Common::Rect();

	int16 destW = srcW;
	int16 destH = srcH;

	if (fitToContainer) {
		double scaleW = (double)containerW / srcW;
		double scaleH = (double)containerH / srcH;

		if (scaleW <= scaleH) {
			destW = containerW;
			destH = (int16)(srcH * scaleW);
		} else {
			destW = (int16)(srcW * scaleH);
			destH = containerH;
		}
	}

	int16 destX = 0;
	int16 destY = 0;

	if (center) {
		destX = (containerW - destW) / 2;
		destY = (containerH - destH) / 2;
	}

	return Common::Rect(destX, destY, destX + destW, destY + destH);
}

void PrintingDialog::updatePreview() {
	if (!_initDone)
		return;

	if (_surface.w <= 0 || _surface.h <= 0) {
		error("Invalid surface dimensions for printing preview");
		return;
	}

	int16 previewX = 0, previewY = 0, previewW = 0, previewH = 0;
	if (!g_gui.xmlEval()->getWidgetData("PrintingDialog.Preview", previewX, previewY, previewW, previewH)) {
		error("Failed to get widget data for PrintingDialog.Preview");
		return;
	}

	if (_saveAsImageCheckbox->getState()) {
		// In this mode, there is no "page", just fit the image exactly in the preview area
		Common::Rect destRect = computePlacementInContainer(_surface.w, _surface.h, previewW, previewH, true, false);

		Graphics::ManagedSurface fittedImage(destRect.width(), destRect.height(), g_gui.theme()->getPixelFormat());
		fittedImage.blitFrom(_surface, _surface.getBounds(), destRect);

		int16 deltaX = (previewW - destRect.width()) / 2;
		int16 deltaY = (previewH - destRect.height()) / 2;

		_preview->resize(previewX + deltaX, destRect.top + deltaY, destRect.width(), destRect.height(), false);
		_preview->setGfx(&fittedImage, false);
		g_gui.scheduleTopDialogRedraw();
		return;
	}

	Common::PrintingManager *printMan = g_system->getPrintingManager();
	Common::Rect paperDim = printMan->getPaperDimensions();
	Common::Rect printDim = printMan->getPrintableArea();
	Common::Point printOffset = printMan->getPrintableAreaOffset();

	if (paperDim.isEmpty()) {
		// Defaults to A4 size at 600 DPI, just to display something in the preview.
		paperDim = Common::Rect(4960, 7016);
		if (printMan->_orientation == kPageOrientationLandscape)
			SWAP(paperDim.right, paperDim.bottom);
	}

	if (printDim.isEmpty()) {
		printDim = Common::Rect(paperDim.width() - 2 * printOffset.x, paperDim.height() - 2 *  printOffset.y);
	}

	bool fitToPage = _fitToPageCheckbox->getState();
	bool center = _centerCheckbox->getState();

	// Draw a white rectangle representing the page
	Common::Rect pageInPreviewDims = computePlacementInContainer(paperDim.width(), paperDim.height(), previewW, previewH, true, true);
	Graphics::ManagedSurface page(pageInPreviewDims.width(), pageInPreviewDims.height(), g_gui.theme()->getPixelFormat());
	page.fillRect(page.getBounds(), page.format.RGBToColor(255, 255, 255));

	// Blit the image to the page surface, taking into account the printable area offset and the "fit to page" and "center" options.
	double pageToPreviewScale = (double)pageInPreviewDims.width() / paperDim.width();

	Common::Rect imageOnPageDims = computePlacementInContainer(_surface.w, _surface.h, printDim.width(), printDim.height(), fitToPage, center);
	imageOnPageDims.translate(printOffset.x, printOffset.y);

	Common::Rect imageInPagePreview;
	imageInPagePreview.left   = (int16)round(imageOnPageDims.left   * pageToPreviewScale);
	imageInPagePreview.top    = (int16)round(imageOnPageDims.top    * pageToPreviewScale);
	imageInPagePreview.right  = (int16)round(imageOnPageDims.right  * pageToPreviewScale);
	imageInPagePreview.bottom = (int16)round(imageOnPageDims.bottom * pageToPreviewScale);

	if (!imageInPagePreview.isEmpty())
		page.blitFrom(_surface, _surface.getBounds(), imageInPagePreview);

	// Update the preview widget
	int16 deltaX = (previewW - pageInPreviewDims.width()) / 2;
	int16 deltaY = (previewH - pageInPreviewDims.height()) / 2;

	_preview->resize(previewX + deltaX, previewY + deltaY, pageInPreviewDims.width(), pageInPreviewDims.height(), false);
	_preview->setGfx(&page, false);
	g_gui.scheduleTopDialogRedraw();
}

void PrintingDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCmdPrint: {
		Common::PrintingManager *printMan = g_system->getPrintingManager();
		if (_saveAsImageCheckbox->getState()) {
			printMan->saveAsImage(_surface);
		}

		bool fitToPage = _fitToPageCheckbox->getState();
		bool center = _centerCheckbox->getState();

		Common::Rect printArea = printMan->getPrintableArea();
		Common::Rect destRect = computePlacementInContainer(_surface.w, _surface.h,
		                                                    printArea.width(), printArea.height(),
		                                                    fitToPage, center);

		printMan->doPrint(_surface, destRect);
		close();
		break;
	}
	case kCmdSelectPrinterName:
		g_system->getPrintingManager()->setPrinterName(_tagToPrinterName[data]);
		updatePreview();
		break;
	case kCmdSelectOrientation:
		g_system->getPrintingManager()->_orientation = (PageOrientation)data;
		updatePreview();
		break;

	case kCmdSaveAsImage:
	case kCmdFitToPage:
	case kCmdCenter:
		updatePreview();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
