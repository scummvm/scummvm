/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 * $URL$
 * $Id$
 *
 */

#include "common/util.h"
#include "common/system.h"

#include "graphics/surface.h"
#include "graphics/colormasks.h"
#include "graphics/imageman.h"
#include "graphics/cursorman.h"
#include "graphics/VectorRenderer.h"

#include "gui/ThemeEngine.h"
#include "gui/ThemeData.h"

namespace GUI {

/**********************************************************
 *	Data definitions for theme engine elements
 *********************************************************/
const ThemeEngine::DrawDataInfo ThemeEngine::kDrawDataDefaults[] = {
	{kDDMainDialogBackground, 		"mainmenu_bg", 		true, 	kDDNone},
	{kDDSpecialColorBackground, 	"special_bg", 		true, 	kDDNone},
	{kDDPlainColorBackground, 		"plain_bg", 		true, 	kDDNone},
	{kDDDefaultBackground, 			"default_bg", 		true, 	kDDNone},
	{kDDTextSelectionBackground,	"text_selection", 	false, 	kDDNone},

	{kDDWidgetBackgroundDefault,	"widget_default", 	true, 	kDDNone},
	{kDDWidgetBackgroundSmall, 		"widget_small", 	true, 	kDDNone},
	{kDDWidgetBackgroundEditText, 	"widget_textedit", 	true, 	kDDNone},
	{kDDWidgetBackgroundSlider, 	"widget_slider", 	true, 	kDDNone},

	{kDDButtonIdle, 				"button_idle", 		true, 	kDDWidgetBackgroundSlider},
	{kDDButtonHover, 				"button_hover", 	false, 	kDDButtonIdle},
	{kDDButtonDisabled, 			"button_disabled", 	true, 	kDDNone},

	{kDDSliderFull, 				"slider_full", 		false, 	kDDNone},
	{kDDSliderHover, 				"slider_hover", 	false, 	kDDNone},
	{kDDSliderDisabled, 			"slider_disabled", 	true, 	kDDNone},

	{kDDCheckboxDefault, 			"checkbox_default", 		true, 	kDDNone},
	{kDDCheckboxDisabled, 			"checkbox_disabled",		true, 	kDDNone},
	{kDDCheckboxSelected, 			"checkbox_selected",		false, 	kDDCheckboxDefault},

	{kDDTabActive, 					"tab_active", 				false, 	kDDTabInactive},
	{kDDTabInactive, 				"tab_inactive", 			true, 	kDDNone},
	{kDDTabBackground, 				"tab_background", 			true, 	kDDNone},

	{kDDScrollbarBase, 				"scrollbar_base", 			true, 	kDDNone},

	{kDDScrollbarButtonIdle, 		"scrollbar_button_idle", 	true, 	kDDNone},
	{kDDScrollbarButtonHover, 		"scrollbar_button_hover", 	false, 	kDDScrollbarButtonIdle},

	{kDDScrollbarHandleIdle, 		"scrollbar_handle_idle", 	false, 	kDDNone},
	{kDDScrollbarHandleHover, 		"scrollbar_handle_hover", 	false, 	kDDScrollbarBase},

	{kDDPopUpIdle, 					"popup_idle", 	true, 	kDDNone},
	{kDDPopUpHover, 				"popup_hover", 	false, 	kDDPopUpIdle},

	{kDDCaret, 						"caret", 		false, 	kDDNone},
	{kDDSeparator, 					"separator", 	true, 	kDDNone},
};

const ThemeEngine::TextDataInfo ThemeEngine::kTextDataDefaults[] = {
	{kTextDataDefault, 		"text_default"},
	{kTextDataHover, 		"text_hover"},
	{kTextDataDisabled, 	"text_disabled"},
	{kTextDataInverted, 	"text_inverted"},
	{kTextDataButton, 		"text_button"},
	{kTextDataButtonHover, 	"text_button_hover"},
	{kTextDataNormalFont, 	"text_normal"}
};


/**********************************************************
 *	ThemeItem functions for drawing queues.
 *********************************************************/
void ThemeItemDrawData::drawSelf(bool draw, bool restore) {

	Common::Rect extendedRect = _area;
	extendedRect.grow(_engine->kDirtyRectangleThreshold + _data->_backgroundOffset);

	if (restore)
		_engine->restoreBackground(extendedRect);

	if (draw) {
		Common::List<Graphics::DrawStep>::const_iterator step;
		for (step = _data->_steps.begin(); step != _data->_steps.end(); ++step)
			_engine->renderer()->drawStep(_area, *step, _dynamicData);
	}

	_engine->addDirtyRect(extendedRect);
}

void ThemeItemTextData::drawSelf(bool draw, bool restore) {
	if (_restoreBg || restore)
		_engine->restoreBackground(_area);

	if (draw) {
		_engine->renderer()->setFgColor(_data->_color.r, _data->_color.g, _data->_color.b);
		_engine->renderer()->drawString(_data->_fontPtr, _text, _area, _alignH, _alignV, _deltax, _ellipsis);
	}

	_engine->addDirtyRect(_area);
}

void ThemeItemBitmap::drawSelf(bool draw, bool restore) {
	if (restore)
		_engine->restoreBackground(_area);

	if (draw) {
		if (_alpha)
			_engine->renderer()->blitAlphaBitmap(_bitmap, _area);
		else
			_engine->renderer()->blitSubSurface(_bitmap, _area);
	}

	_engine->addDirtyRect(_area);
}

}
