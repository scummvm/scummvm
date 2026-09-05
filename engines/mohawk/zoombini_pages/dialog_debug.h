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

#ifndef MOHAWK_ZOOMBINI_PAGES_DIALOG_DEBUG_H
#define MOHAWK_ZOOMBINI_PAGES_DIALOG_DEBUG_H

#include "mohawk/zoombini_debug.h"
#include "mohawk/zoombini_pages/dialog_base.h"

namespace Mohawk {

/**
 * ScummVM-specific diagnostic dialog for inspecting Zoombini resources.
 *
 * A @ref ZoombiniDebugCommand selects the display operation. The dialog
 * materializes the requested feature, draws its title and keyboard legend,
 * and routes operation-specific input to the corresponding renderer. Unlike
 * gameplay dialogs, it may replace its background with the completed debug
 * display so later dirty-rectangle restores preserve the inspected image.
 */
class ZoombiniDialogDebug : public ZoombiniDialog {
public:
	/** Create a diagnostic dialog for @p cmd. */
	ZoombiniDialogDebug(MohawkEngine_Zoombini *vm, const ZoombiniDebugCommand &cmd);
	/** Release the debug command and diagnostic features. */
	~ZoombiniDialogDebug() override;

	/** Select the neutral background used by the diagnostic display. */
	void setBackgroundBitmap() override;
	/** Load the feature required by the selected debug operation. */
	void loadFeatures() override;
	/** Commit or discard the diagnostic display before closing. */
	void close() override;

	/** Handle the diagnostic dialog's pointer activation. */
	virtual ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos);

protected:
	/** Debug operation requested by the console command. */
	ZoombiniDebugCommand _cmd;
	/** Whether the diagnostic display needs to be rendered again. */
	bool _updateScreen = true;

	/**
	 * Debug-display constants are owned by each dialog instance because their
	 * Common types require runtime construction and must not create global
	 * C++ constructors in ScummVM.
	 */
	/** Screen rectangle used by the diagnostic title. */
	const Common::Rect _titleRect = Common::Rect(0, 0, ZoombiniGraphics::kScreenWidth, 14);
	/** Keyboard legend shown in the upper-right corner. */
	const Common::U32String _escText = Common::U32String("[ESC] close");
	/** Print top left message */
	void drawTitleText(ZmbFeature *feature, const Common::U32String &titleText);
	/** Print top right message */
	void drawEscText(ZmbFeature *feature, const Common::U32String &keyLegendText = Common::U32String());
	/** Make the completed debug display the dialog background used by later dirty-rect restores. */
	void commitDisplayToBackground();

	// [*] draw cursor
	/** Render the cursor selected by the diagnostic command. */
	ZmbRenderResult drawCursor_render(ZmbFeature *feature);

	// [*] draw image
	/** Render the selected decoded image. */
	ZmbRenderResult drawImage_render(ZmbFeature *feature);

	// [*] draw shape
	/** Render the selected shape. */
	ZmbRenderResult drawShape_render(ZmbFeature *feature);
	/** Handle common keyboard navigation for diagnostic displays. */
	ZmbEventHandleResult common_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);

	// [*] draw shapes
	/** Render the current shape in the shape sequence. */
	ZmbRenderResult drawShapes_render(ZmbFeature *feature);
	/** Move through the shape sequence with keyboard input. */
	ZmbEventHandleResult drawShapes_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);
	/** Previously displayed shape indexes used for back navigation. */
	Common::Stack<uint16> _drawShapesPrevShapeIdxStack;
	/** Next shape index to display. */
	uint16 _drawShapesNextShapeIdx = 0;

	// [*] draw feature
	/** Render the current feature hotspot. */
	ZmbRenderResult drawFeature_render(ZmbFeature *feature);
	/** Move through feature hotspots with keyboard input. */
	ZmbEventHandleResult drawFeature_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);
	/** Previously displayed hotspot indexes used for back navigation. */
	Common::Stack<uint16> _drawFeaturePrevHsIdxStack;
	/** Next hotspot index to display. */
	uint16 _drawFeatureNextHsIdx = 1;
	/** Frame selected for the current feature diagnostic. */
	uint16 _drawFeatureFrame = 0;

	// [*] draw terrain
	/** Render the active page through its terrain mask. */
	ZmbRenderResult drawTerrain_render(ZmbFeature *feature);

	// [*] plot point
	/** Render a diagnostic point. */
	ZmbRenderResult plotPoint_render(ZmbFeature *feature);

	// [*] plot line
	/** Render a diagnostic line. */
	ZmbRenderResult plotLine_render(ZmbFeature *feature);

	// [*] plot rect
	/** Render a diagnostic rectangle. */
	ZmbRenderResult plotRect_render(ZmbFeature *feature);

	/** Pending multi-screen navigation operation. */
	enum MultiScreenOperation {
		kMultiScreenOpNone = 0,
		kMultiScreenOpInit,
		kMultiScreenOpPrev,
		kMultiScreenOpNext,
		kMultiScreenOpUp,
		kMultiScreenOpDown,
	};
	/** Operation applied to the next multi-screen diagnostic update. */
	MultiScreenOperation _multiScreenNextOp = kMultiScreenOpInit;
};

} // End of namespace Mohawk

#endif
