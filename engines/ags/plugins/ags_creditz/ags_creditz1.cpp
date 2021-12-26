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

#include "ags/plugins/ags_creditz/ags_creditz1.h"
#include "ags/lib/allegro/surface.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

const char *IMAGE_TEXT = "*i*m*a*g*e*";

AGSCreditz1::AGSCreditz1() : AGSCreditz() {
	_version = VERSION_11;
}

const char *AGSCreditz1::AGS_GetPluginName() {
	return "AGSCreditz v1.1 by AJA";
}

void AGSCreditz1::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);
	engine->RequestEventHook(AGSE_POSTSCREENDRAW);
	_engine->GetScreenDimensions(&_screenWidth,
	                             &_screenHeight, &_screenColorDepth);

	SCRIPT_METHOD(SetCredit, AGSCreditz1::SetCredit);
	SCRIPT_METHOD(ScrollCredits, AGSCreditz1::ScrollCredits);
	SCRIPT_METHOD(GetCredit, AGSCreditz1::GetCredit);
	SCRIPT_METHOD(IsCreditScrollingFinished, AGSCreditz1::IsCreditScrollingFinished);
	SCRIPT_METHOD(IsFinished, AGSCreditz1::IsCreditScrollingFinished);
	SCRIPT_METHOD(SetCreditImage, AGSCreditz1::SetCreditImage);
	SCRIPT_METHOD(PauseScroll, AGSCreditz1::PauseScroll);
	SCRIPT_METHOD(ScrollReset, AGSCreditz1::ScrollReset);
	SCRIPT_METHOD(SetEmptyLineHeight, AGSCreditz1::SetEmptyLineHeight);
	SCRIPT_METHOD(GetEmptyLineHeight, AGSCreditz1::GetEmptyLineHeight);
	SCRIPT_METHOD(SetStaticCredit, AGSCreditz1::SetStaticCredit);
	SCRIPT_METHOD(GetStaticCredit, AGSCreditz1::GetStaticCredit);
	SCRIPT_METHOD(StartEndStaticCredits, AGSCreditz1::StartEndStaticCredits);
	SCRIPT_METHOD(GetCurrentStaticCredit, AGSCreditz1::GetCurrentStaticCredit);
	SCRIPT_METHOD(SetDefaultStaticDelay, AGSCreditz1::SetDefaultStaticDelay);
	SCRIPT_METHOD(SetStaticPause, AGSCreditz1::SetStaticPause);
	SCRIPT_METHOD(SetStaticCreditTitle, AGSCreditz1::SetStaticCreditTitle);
	SCRIPT_METHOD(ShowStaticCredit, AGSCreditz1::ShowStaticCredit);
	SCRIPT_METHOD(StaticReset, AGSCreditz1::StaticReset);
	SCRIPT_METHOD(GetStaticCreditTitle, AGSCreditz1::GetStaticCreditTitle);
	SCRIPT_METHOD(SetStaticCreditImage, AGSCreditz1::SetStaticCreditImage);
	SCRIPT_METHOD(IsStaticCreditsFinished, AGSCreditz1::IsStaticCreditsFinished);
}

int64 AGSCreditz1::AGS_EngineOnEvent(int event, NumberPtr data) {
	if (event & AGSE_POSTSCREENDRAW)
		draw();

	return 0;
}

void AGSCreditz1::SetCredit(ScriptMethodParams &params) {
	PARAMS7(int, ID, string, credit, int, colour, int, font, bool, center, int, xpos, int, generateoutline);

	if (ID >= (int)_credits[0].size())
		_credits[0].resize(ID + 1);

	if (center) {
		int32 creditW, creditH;
		_engine->GetTextExtent(font, credit, &creditW, &creditH);
		xpos = (_screenWidth - creditW) / 2;
	}

	Credit &c = _credits[0][ID];
	c._text = credit;
	c._fontSlot = font;
	c._x = xpos;
	c._isSet = true;
	c._outline = generateoutline;
	c._colorHeight = colour;
}

void AGSCreditz1::SetCreditImage(ScriptMethodParams &params) {
	PARAMS5(int, ID, int, slot, bool, center, int, xpos, int, pixtonext);

	if (ID >= (int)_credits[0].size())
		_credits[0].resize(ID + 1);

	BITMAP *gfx = _engine->GetSpriteGraphic(slot);
	if (center)
		xpos = (_screenWidth - gfx->w) / 2;

	_credits[0][ID]._image = true;
	_credits[0][ID]._isSet = true;
	_credits[0][ID]._x = xpos;
	_credits[0][ID]._fontSlot = slot;

	if (pixtonext != -1) {
		_credits[0][ID]._colorHeight = pixtonext;
	} else {
		_credits[0][ID]._colorHeight = gfx->h;
	}
}

void AGSCreditz1::ScrollCredits(ScriptMethodParams &params) {
	PARAMS7(int, onoff, int, speed, int, fromY, int, toY, int, isautom, int, wait, int, resolution);

	if (onoff == 1) {
		_seqSettings[0].speed = speed;
		_seqSettings[0].endwait = wait;
		_seqSettings[0].startpoint = fromY;
		_seqSettings[0].endpoint = toY;
		_seqSettings[0].automatic = isautom;

		if (_screenWidth == 320) {
			_resolutionFlag = (resolution != 2) ? 1 : 0;
		} else if (_screenWidth == 640) {
			_resolutionFlag = (resolution != 1) ? 1 : 0;
		}

		startSequence(0);

	} else if (onoff == 0) {
		_creditsRunning = false;

	} else {
		_engine->AbortGame("ScrollCredits: OnOff value must be 1 or 0!");
	}
}

void AGSCreditz1::GetCredit(ScriptMethodParams &params) {
	PARAMS1(int, ID);

	params._result = (_credits[0][ID]._text == IMAGE_TEXT) ?
	                 "image" : _credits[0][ID]._text.c_str();
}

void AGSCreditz1::IsCreditScrollingFinished(ScriptMethodParams &params) {
	params._result = _seqSettings[0].finished;
}

void AGSCreditz1::PauseScroll(ScriptMethodParams &params) {
	PARAMS1(bool, onoff);
	_paused = onoff;
}

void AGSCreditz1::ScrollReset(ScriptMethodParams &params) {
	_credits[0].clear();
	_creditsRunning = false;
}

void AGSCreditz1::SetEmptyLineHeight(ScriptMethodParams &params) {
	PARAMS1(int, emptylineheight);
	_emptyLineHeight = emptylineheight;
}

void AGSCreditz1::GetEmptyLineHeight(ScriptMethodParams &params) {
	params._result = _emptyLineHeight;
}

void AGSCreditz1::SetStaticCredit(ScriptMethodParams &params) {
	PARAMS8(int, ID, int, x, int, y, int, font, int, creditcolour, \
	        bool, center, int, generateoutline, string, credit);

	if (ID >= (int)_credits[0].size())
		_credits[0].resize(ID + 1);

	if (center) {
		int32 creditW, creditH;
		_engine->GetTextExtent(font, credit, &creditW, &creditH);
		x = (_screenWidth - creditW) / 2;
	}

	StCredit &c = _stCredits[0][ID];
	c.credit = credit;
	c.font = font;
	c.color = creditcolour;
	c.x = x;
	c.y = y;
	c.outline = generateoutline;
}

void AGSCreditz1::GetStaticCredit(ScriptMethodParams &params) {
	PARAMS1(int, ID);
	StCredit &c = _stCredits[0][ID];
	params._result = c.credit.c_str();
}

void AGSCreditz1::StartEndStaticCredits(ScriptMethodParams &params) {
	PARAMS2(bool, onoff, int, res);

	if (!onoff) {
		_staticCredits = false;

	} else if (res != 1 && res != 2) {
		_engine->AbortGame("StartEndStaticCredits: Wrong resolution");

	} else {
		_currentStatic = 0;
		_engine->GetScreenDimensions(&_screenWidth,
		                             &_screenHeight, &_screenColorDepth);

		_staticScreenWidth = (res == 1) ? 320 : 640;
		_staticWidthMatches = _screenWidth == _staticScreenWidth;
	}
}

void AGSCreditz1::GetCurrentStaticCredit(ScriptMethodParams &params) {
	params._result = _currentStatic;
}

void AGSCreditz1::SetDefaultStaticDelay(ScriptMethodParams &params) {
	PARAMS1(int, Cyclesperchar);
	_stSeqSettings[0].speed = Cyclesperchar;
}

void AGSCreditz1::SetStaticPause(ScriptMethodParams &params) {
	PARAMS2(int, ID, int, length);
	_stCredits[0][ID].pause = length;
}

void AGSCreditz1::SetStaticCreditTitle(ScriptMethodParams &params) {
	PARAMS8(int, ID, int, x, int, y, int, titlefont, int, titlecolour, \
	        int, centered, int, generateoutline, string, title);

	StCredit &c = _stCredits[0][ID];
	c.title_x = x;
	c.title_y = y;
	c.title_font = titlefont;
	c.title_color = titlecolour;
	c.title_centered = centered;
	c.title_outline = generateoutline;
	c.title = title;
}

void AGSCreditz1::ShowStaticCredit(ScriptMethodParams &params) {
	PARAMS6(int, ID, int, time, int, style, int, transtime, \
	        int, sound, int, res);
	const StCredit &c = _stCredits[0][ID];

	if (!_staticCredits) {
		if (c.credit.empty() && c.title.empty()) {
			_engine->AbortGame("ShowStaticCredit: Credit not set!");
		} else if (res == 1 || (res == 2 && c.credit != "P=A=U=S=E")) {
			if (style == 1) {
				// TODO: style 1 setup
				warning("TODO: Use %d %d %d", transtime, time, sound);
			}

			_engine->GetScreenDimensions(&_screenWidth,
			                             &_screenHeight, &_screenColorDepth);

			_staticScreenWidth = (res == 1) ? 320 : 640;
			_staticWidthMatches = _screenWidth == _staticScreenWidth;
			_currentStatic = ID;

			// TODO: Final setup
		}
	}

	params._result = 0;
}

void AGSCreditz1::StaticReset(ScriptMethodParams &params) {
	_stCredits[0].clear();
}

void AGSCreditz1::GetStaticCreditTitle(ScriptMethodParams &params) {
	PARAMS1(int, ID);

	const StCredit &c = _stCredits[0][ID];
	params._result = c.title.c_str();
}

void AGSCreditz1::SetStaticCreditImage(ScriptMethodParams &params) {
	PARAMS7(int, ID, int, x, int, y, int, slot, int, Hcentered, \
	        bool, Vcentered, int, time);

	if (Hcentered) {
		BITMAP *gfx = _engine->GetSpriteGraphic(slot);
		if (Hcentered)
			x = (_screenWidth - gfx->w) / 2;
	}

	StCredit &c = _stCredits[0][ID];
	c.credit = "I=M=A=G=E";
	c.x = x;
	c.y = y;
	c.font = slot;
	// TODO: Below seems *weird*
	c.outline = Vcentered;
	c.color = time;
}

void AGSCreditz1::IsStaticCreditsFinished(ScriptMethodParams &params) {
	params._result = _stSeqSettings[0].finished;
}

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3
