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
 */

#include "common/util.h"
#include "gui/widget.h"
#include "gui/dialog.h"
#include "gui/gui-manager.h"

#include "gui/Tooltip.h"
#include "gui/ThemeEval.h"

namespace GUI {


Tooltip::Tooltip() :
	Dialog(-1, -1, -1, -1), _maxWidth(-1), _parent(nullptr), _xdelta(0), _ydelta(0) {

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundTooltip;
}

void Tooltip::setup(Dialog *parent, Widget *widget, int x, int y) {
	assert(widget->hasTooltip());

	_parent = parent;

	_maxWidth = g_gui.xmlEval()->getVar("Globals.Tooltip.MaxWidth", 100);
	_xdelta = g_gui.xmlEval()->getVar("Globals.Tooltip.XDelta", 0);
	_ydelta = g_gui.xmlEval()->getVar("Globals.Tooltip.YDelta", 0);

	const Graphics::Font *tooltipFont = g_gui.theme()->getFont(ThemeEngine::kFontStyleTooltip);

	_wrappedLines.clear();
	_w = tooltipFont->wordWrapText(widget->getTooltip(), _maxWidth - 4, _wrappedLines) + 4;
	_h = (tooltipFont->getFontHeight() + 2) * _wrappedLines.size() + 4;

	_x = MIN<int16>(parent->_x + x + _xdelta, g_gui.getWidth() - _w - 3);
	_y = MIN<int16>(parent->_y + y + _ydelta, g_gui.getHeight() - _h - 3);

	if (g_gui.useRTL())
		_x = g_system->getOverlayWidth() - _w - _x + g_gui.getOverlayOffset();

#ifdef USE_TTS
	if (ConfMan.hasKey("tts_enabled", "scummvm") &&
			ConfMan.getBool("tts_enabled", "scummvm")) {
		Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
		if (ttsMan == nullptr)
			return;
		ttsMan->say(widget->getTooltip(), Common::TextToSpeechManager::QUEUE_NO_REPEAT);
	}
#endif
}

void Tooltip::drawDialog(DrawLayer layerToDraw) {
	int num = 0;
	int h = g_gui.theme()->getFontHeight(ThemeEngine::kFontStyleTooltip) + 2;

	Dialog::drawDialog(layerToDraw);

	int16 textX = g_gui.useRTL() ? _x - 3 : _x + 3; // including 2px padding and 1px original code shift
	int16 textY = _y + 3;

	Graphics::TextAlign textAlignment = g_gui.useRTL() ? Graphics::kTextAlignRight : Graphics::kTextAlignLeft;

	for (Common::U32StringArray::const_iterator i = _wrappedLines.begin(); i != _wrappedLines.end(); ++i, ++num) {
		g_gui.theme()->drawText(
			Common::Rect(textX, textY + num * h, textX + _w, textY + (num + 1) * h),
			*i,
			ThemeEngine::kStateEnabled,
			textAlignment,
			ThemeEngine::kTextInversionNone,
			0,
			false,
			ThemeEngine::kFontStyleTooltip,
			ThemeEngine::kFontColorNormal,
			false
		);
	}
}

}
