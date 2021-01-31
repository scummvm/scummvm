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

#include "ags/plugins/ags_creditz/ags_creditz.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

AGSCreditz::Version AGSCreditz::_version;
State *AGSCreditz::_state;
IAGSEngine *AGSCreditz::_engine;

AGSCreditz::AGSCreditz() {
	_state = new State();
	_engine = nullptr;

	DLL_METHOD(AGS_GetPluginName);
}

AGSCreditz::~AGSCreditz() {
	delete _state;
}


const char *AGSCreditz::AGS_GetPluginName() {
	if (_version == VERSION_11)
		return "AGSCreditz v1.1 by AJA";
	else
		return "AGSCreditz 2.0 (by Dima Software: AJA)";
}

void AGSCreditz::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;

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

int AGSCreditz::IsCreditScrollingFinished(const ScriptMethodParams &params) {
	return true;
}

void AGSCreditz::SetCreditImage(const ScriptMethodParams &params) {
	//PARAMS5(int, ID, int, Slot, int, center, int, xpos, int, pixtonext);
}

void AGSCreditz::PauseScroll(const ScriptMethodParams &params) {
	//PARAMS1(int, onoff);
}

void AGSCreditz::ScrollReset(const ScriptMethodParams &params) {
}

void AGSCreditz::SetEmptyLineHeight(const ScriptMethodParams &params) {
	//PARAMS1(int, Height);
}

int AGSCreditz::GetEmptyLineHeight(const ScriptMethodParams &params) {
	return 0;
}

void AGSCreditz::SetStaticCredit(const ScriptMethodParams &params) {
	//PARAMS8(int, ID, int, x, int, y, int, creditfont, int, creditcolour, int, centered, int, generateoutline, string, credit);

}

string AGSCreditz::GetStaticCredit(const ScriptMethodParams &params) {
	//PARAMS1(int, ID);
	return nullptr;
}

void AGSCreditz::StartEndStaticCredits(const ScriptMethodParams &params) {
	//PARAMS2(int, onoff, int, res);
}

int AGSCreditz::GetCurrentStaticCredit(const ScriptMethodParams &params) {
	return 0;
}

void AGSCreditz::SetDefaultStaticDelay(const ScriptMethodParams &params) {
	//PARAMS1(int, Cyclesperchar);
}

void AGSCreditz::SetStaticPause(const ScriptMethodParams &params) {
	//PARAMS2(int, ID, int, length);
}

void AGSCreditz::SetStaticCreditTitle(const ScriptMethodParams &params) {
	//PARAMS8(int, ID, int, x, int, y, int, titlefont, int, titlecolour, int, centered, int, generateoutline, string, title);
}

void AGSCreditz::ShowStaticCredit(const ScriptMethodParams &params) {
	//PARAMS6(int, ID, int, time, int, style, int, transtime, int, sound, int, resolution);
}

void AGSCreditz::StaticReset(const ScriptMethodParams &params) {
}

string AGSCreditz::GetStaticCreditTitle(const ScriptMethodParams &params) {
	//PARAMS1(int, ID);
	return nullptr;
}

void AGSCreditz::SetStaticCreditImage(const ScriptMethodParams &params) {
//int ID, int x, int y, int Slot, int Hcentered, int Vcentered, int time) {
}

int AGSCreditz::IsStaticCreditsFinished(const ScriptMethodParams &params) {
	return true;
}

/*------------------------------------------------------------------*/

const char *IMAGE_TEXT = "*i*m*a*g*e*";

AGSCreditz11::AGSCreditz11() {
	_version = VERSION_11;

	DLL_METHOD(AGS_EngineStartup);
}

void AGSCreditz11::AGS_EngineStartup(IAGSEngine *engine) {
	AGSCreditz::AGS_EngineStartup(engine);

	SCRIPT_METHOD(SetCredit);
	SCRIPT_METHOD(ScrollCredits);
	SCRIPT_METHOD(GetCredit);
}

void AGSCreditz11::SetCredit(const ScriptMethodParams &params) {
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
	c._color = colour;
}

void AGSCreditz11::ScrollCredits(const ScriptMethodParams &params) {
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

string AGSCreditz11::GetCredit(const ScriptMethodParams &params) {
	PARAMS1(int, ID);

	return (_state->_credits[0][ID]._text == IMAGE_TEXT) ?
		"image" : _state->_credits[0][ID]._text.c_str();
}

/*------------------------------------------------------------------*/

AGSCreditz20::AGSCreditz20() {
	_version = VERSION_20;

	DLL_METHOD(AGS_EngineStartup);
}

void AGSCreditz20::AGS_EngineStartup(IAGSEngine *engine) {
	AGSCreditz::AGS_EngineStartup(engine);

	SCRIPT_METHOD(SetCredit);
}

void AGSCreditz20::SetCredit(const ScriptMethodParams &params) {
	PARAMS7(int, sequence, int, line, string, credit, int, x_pos, int, font, int, color, int, gen_outline);

	assert(sequence >= 0 && sequence <= 10);
	if (line >= (int)_state->_credits[sequence].size())
		_state->_credits[sequence].resize(line + 1);

	Credit &c = _state->_credits[sequence][line];
	c._text = credit;
	c._fontSlot = font;
	c._color = color;
	c._x = x_pos;
	c._isSet = true;
	if (gen_outline > 0)
		c._outline = true;
}

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3
