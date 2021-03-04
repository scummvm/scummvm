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

#include "ags/plugins/ags_creditz/ags_creditz1.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

const char *IMAGE_TEXT = "*i*m*a*g*e*";

AGSCreditz1::AGSCreditz1() : AGSCreditz() {
	_version = VERSION_11;

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSCreditz1::AGS_GetPluginName() {
	return "AGSCreditz v1.1 by AJA";
}

void AGSCreditz1::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;

	SCRIPT_METHOD(SetCredit);
	SCRIPT_METHOD(ScrollCredits);
	SCRIPT_METHOD(GetCredit);
	SCRIPT_METHOD(IsCreditScrollingFinished);
	SCRIPT_METHOD(SetCreditImage);
	SCRIPT_METHOD(PauseScroll);
	SCRIPT_METHOD(ScrollReset);
	SCRIPT_METHOD(SetEmptyLineHeight);
	SCRIPT_METHOD(GetEmptyLineHeight);
	SCRIPT_METHOD(SetStaticCredit);
	SCRIPT_METHOD(GetStaticCredit);
	SCRIPT_METHOD(StartEndStaticCredits);
	SCRIPT_METHOD(GetCurrentStaticCredit);
	SCRIPT_METHOD(SetDefaultStaticDelay);
	SCRIPT_METHOD(SetStaticPause);
	SCRIPT_METHOD(SetStaticCreditTitle);
	SCRIPT_METHOD(ShowStaticCredit);
	SCRIPT_METHOD(StaticReset);
	SCRIPT_METHOD(GetStaticCreditTitle);
	SCRIPT_METHOD(SetStaticCreditImage);
	SCRIPT_METHOD(IsStaticCreditsFinished);
}

void AGSCreditz1::SetCredit(ScriptMethodParams &params) {
	PARAMS7(int, ID, string, credit, int, colour, int, font, int, center, int, xpos, int, generateoutline);

	if (ID >= (int)_state->_credits[0].size())
		_state->_credits[0].resize(ID + 1);

	Credit &c = _state->_credits[0][ID];
	c._text = credit;
	c._fontSlot = font;
	c._center = center;
	c._x = xpos;
	c._isSet = true;
	c._outline = generateoutline;
	c._colorHeight = colour;
}

void AGSCreditz1::ScrollCredits(ScriptMethodParams &params) {
	PARAMS7(int, onoff, int, speed, int, fromY, int, toY, int, isautom, int, wait, int, resolution);

	if (onoff == 1) {
		_state->_creditsRunning = true;
		_state->_seqSettings[0].speed = speed;
		_state->_seqSettings[0].endwait = wait;
		_state->_seqSettings[0].startpoint = fromY;
		_state->_seqSettings[0].endpoint = toY;
		_state->_seqSettings[0].automatic = isautom;

		_engine->GetScreenDimensions(&_state->_screenWidth,
			&_state->_screenHeight, &_state->_screenColorDepth);
		if (_state->_screenWidth == 320) {
			_state->_resolutionFlag = (resolution != 2) ? 1 : 0;
		} else if (_state->_screenWidth == 640) {
			_state->_resolutionFlag = (resolution != 1) ? 1 : 0;
		}

	} else if (onoff == 0) {
		_state->_creditsRunning = false;

	} else {
		_engine->AbortGame("ScrollCredits: OnOff value must be 1 or 0!");
	}
}

void AGSCreditz1::GetCredit(ScriptMethodParams &params) {
	PARAMS1(int, ID);

	params._result = (_state->_credits[0][ID]._text == IMAGE_TEXT) ?
		"image" : _state->_credits[0][ID]._text.c_str();
}

void AGSCreditz1::IsCreditScrollingFinished(ScriptMethodParams &params) {
	params._result = true;
}

void AGSCreditz1::SetCreditImage(ScriptMethodParams &params) {
	//PARAMS5(int, ID, int, Slot, int, center, int, xpos, int, pixtonext);
}

void AGSCreditz1::PauseScroll(ScriptMethodParams &params) {
	//PARAMS1(int, onoff);
}

void AGSCreditz1::ScrollReset(ScriptMethodParams &params) {
}

void AGSCreditz1::SetEmptyLineHeight(ScriptMethodParams &params) {
	//PARAMS1(int, Height);
}

void AGSCreditz1::GetEmptyLineHeight(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSCreditz1::SetStaticCredit(ScriptMethodParams &params) {
	//PARAMS8(int, ID, int, x, int, y, int, creditfont, int, creditcolour, int, centered, int, generateoutline, string, credit);

}

void AGSCreditz1::GetStaticCredit(ScriptMethodParams &params) {
	//PARAMS1(int, ID);
	params._result = NumberPtr();
}

void AGSCreditz1::StartEndStaticCredits(ScriptMethodParams &params) {
	//PARAMS2(int, onoff, int, res);
}

void AGSCreditz1::GetCurrentStaticCredit(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSCreditz1::SetDefaultStaticDelay(ScriptMethodParams &params) {
	//PARAMS1(int, Cyclesperchar);
}

void AGSCreditz1::SetStaticPause(ScriptMethodParams &params) {
	//PARAMS2(int, ID, int, length);
}

void AGSCreditz1::SetStaticCreditTitle(ScriptMethodParams &params) {
	//PARAMS8(int, ID, int, x, int, y, int, titlefont, int, titlecolour, int, centered, int, generateoutline, string, title);
}

void AGSCreditz1::ShowStaticCredit(ScriptMethodParams &params) {
	//PARAMS6(int, ID, int, time, int, style, int, transtime, int, sound, int, resolution);
}

void AGSCreditz1::StaticReset(ScriptMethodParams &params) {
}

void AGSCreditz1::GetStaticCreditTitle(ScriptMethodParams &params) {
	//PARAMS1(int, ID);
	params._result = NumberPtr();
}

void AGSCreditz1::SetStaticCreditImage(ScriptMethodParams &params) {
//int ID, int x, int y, int Slot, int Hcentered, int Vcentered, int time) {
}

void AGSCreditz1::IsStaticCreditsFinished(ScriptMethodParams &params) {
	params._result = true;
}

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3
