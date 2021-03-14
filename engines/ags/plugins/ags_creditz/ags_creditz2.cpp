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

#include "ags/plugins/ags_creditz/ags_creditz2.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

IntFunction AGSCreditz2::_playSound;

AGSCreditz2::AGSCreditz2() : AGSCreditz() {
	_version = VERSION_20;

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
	DLL_METHOD(AGS_EngineOnEvent);
}

const char *AGSCreditz2::AGS_GetPluginName() {
	return "AGSCreditz 2.0 (by Dima Software: AJA)";
}

void AGSCreditz2::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;
	_playSound = (IntFunction)_engine->GetScriptFunctionAddress("PlaySound");
	engine->RequestEventHook(AGSE_POSTSCREENDRAW);

	SCRIPT_METHOD(RunCreditSequence);
	SCRIPT_METHOD(SetCredit);
	SCRIPT_METHOD(GetCredit);
	SCRIPT_METHOD(CreditsSettings);
	SCRIPT_METHOD(SequenceSettings);
	SCRIPT_METHOD(IsSequenceFinished);
	SCRIPT_METHOD(PauseScrolling);
	SCRIPT_METHOD(SetCreditImage);
	SCRIPT_METHOD(ResetSequence);

	SCRIPT_METHOD(SetStaticCredit);
	SCRIPT_METHOD(SetStaticCreditTitle);
	SCRIPT_METHOD(SetStaticPause);
	SCRIPT_METHOD(RunStaticCreditSequence);
	SCRIPT_METHOD(IsStaticSequenceFinished);
	SCRIPT_METHOD(ShowStaticCredit);
	SCRIPT_METHOD(SetStaticImage);
	SCRIPT_METHOD(GetCurrentStaticCredit);
}

int64 AGSCreditz2::AGS_EngineOnEvent(int event, NumberPtr data) {
	if (event & AGSE_POSTSCREENDRAW)
		draw();

	return 0;
}

void AGSCreditz2::RunCreditSequence(ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	if (!_state->_creditsRunning) {
		_state->_seqSettings[sequence].finished = false;
		_state->_creditsRunning = true;
		_state->_creditSequence = sequence;

		_engine->GetScreenDimensions(&_state->_screenWidth, &_state->_screenHeight,
			&_state->_screenColorDepth);

		if (_state->_seqSettings[sequence].automatic) {
			calculateSequenceHeight(sequence);
			_state->_yPos = _state->_screenHeight + 1;
		} else {
			_state->_yPos = _state->_seqSettings[sequence].startpoint;
		}

		_state->_speedPoint = 0;
		_state->_timer = 0;
		draw();
	} else {
		_state->_paused = false;
		_state->_creditsRunning = false;
		_state->_creditSequence = -1;
		_state->_seqSettings[sequence].finished = true;
	}
}

void AGSCreditz2::SetCredit(ScriptMethodParams &params) {
	PARAMS7(int, sequence, int, line, string, credit, int, x_pos, int, font, int, color, int, gen_outline);

	assert(sequence >= 0 && sequence <= 10);
	if (line >= (int)_state->_credits[sequence].size())
		_state->_credits[sequence].resize(line + 1);

	Credit &c = _state->_credits[sequence][line];
	c._text = credit;
	c._fontSlot = font;
	c._colorHeight = color;
	c._x = x_pos;
	c._isSet = true;
	if (gen_outline > 0)
		c._outline = true;
}

void AGSCreditz2::GetCredit(ScriptMethodParams &params) {
	PARAMS2(int, sequence, int, ID);

	params._result = _state->_credits[sequence][ID]._text.c_str();
}

void AGSCreditz2::CreditsSettings(ScriptMethodParams &params) {
	PARAMS1(int, emptylineheight);

	if (emptylineheight >= 0)
		_state->_emptyLineHeight = emptylineheight;
}

void AGSCreditz2::SequenceSettings(ScriptMethodParams &params) {
	PARAMS6(int, sequence, int, startpoint, int, endPoint, int, speed, int, automatic, int, endwait);

	_state->_seqSettings[sequence].startpoint = startpoint;
	_state->_seqSettings[sequence].endpoint = endPoint;
	_state->_seqSettings[sequence].speed = speed;
	_state->_seqSettings[sequence].automatic = automatic;
	_state->_seqSettings[sequence].endwait = endwait;
}

void AGSCreditz2::IsSequenceFinished(ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	if (_state->_seqSettings[sequence].finished) {
		_state->_seqSettings[sequence].finished = false;
		params._result = 1;
	} else {
		params._result = 0;
	}
}

void AGSCreditz2::PauseScrolling(ScriptMethodParams &params) {
	if (_state->_creditsRunning) {
		_state->_paused = !_state->_paused;
	}
}

void AGSCreditz2::SetCreditImage(ScriptMethodParams &params) {
	PARAMS5(int, sequence, int, line, int, xPos, int, slot, int, height);

	assert(sequence >= 0 && sequence <= 10);
	if (line >= (int)_state->_credits[sequence].size())
		_state->_credits[sequence].resize(line + 1);

	_state->_credits[sequence][line]._image = true;
	_state->_credits[sequence][line]._isSet = true;
	_state->_credits[sequence][line]._x = xPos;
	_state->_credits[sequence][line]._fontSlot = slot;
	_state->_credits[sequence][line]._colorHeight = height;
}

void AGSCreditz2::ResetSequence(ScriptMethodParams &params) {
	PARAMS1(int, seqtype);

	for (int i = 0; i < 10; ++i) {
		if (seqtype != 2)
			// Scrolling
			_state->_credits[i].clear();
		else
			// Static
			_state->_stCredits[i].clear();
	}
}

void AGSCreditz2::SetStaticCredit(ScriptMethodParams &params) {
	PARAMS8(int, sequence, int, id, string, credit, int, xPos, int, yPos,
		int, font, int, color, int, genOutline);

	assert(sequence >= 0 && sequence <= 10);
	if (id >= (int)_state->_stCredits[sequence].size())
		_state->_stCredits[sequence].resize(id + 1);

	_state->_stCredits[sequence][id].credit = credit;
	_state->_stCredits[sequence][id].x = xPos;
	_state->_stCredits[sequence][id].y = yPos;
	_state->_stCredits[sequence][id].font = font;
	_state->_stCredits[sequence][id].color = color;

	if (genOutline > 0)
		_state->_stCredits[sequence][id].outline = true;
}

void AGSCreditz2::SetStaticCreditTitle(ScriptMethodParams &params) {
	PARAMS8(int, sequence, int, id, string, title, int, xPos, int, yPos,
		int, font, int, color, int, genOutline);

	assert(sequence >= 0 && sequence < 10);
	if (id >= (int)_state->_stCredits[sequence].size())
		_state->_stCredits[sequence].resize(id + 1);

	_state->_stCredits[sequence][id].title = title;
	_state->_stCredits[sequence][id].title_x = xPos;
	_state->_stCredits[sequence][id].title_y = yPos;
	_state->_stCredits[sequence][id].title_font = font;
	_state->_stCredits[sequence][id].title_color = color;

	if (genOutline > 0)
		_state->_stCredits[sequence][id].title_outline = true;
}

void AGSCreditz2::SetStaticPause(ScriptMethodParams &params) {
	PARAMS3(int, sequence, int, id, int, length);

	assert(sequence >= 0 && sequence <= 10);
	if (id >= (int)_state->_stCredits[sequence].size())
		_state->_stCredits[sequence].resize(id + 1);

	_state->_stCredits[sequence][id].pause = length;
}

void AGSCreditz2::RunStaticCreditSequence(ScriptMethodParams &params) {
	PARAMS2(int, sequence, int, speed);

	if (!_state->_creditsRunning) {
		_state->_stSeqSettings[sequence].finished = false;
		_state->_stSeqSettings[sequence].speed = speed;
		_state->_creditSequence = sequence;
		_state->_staticCredits = true;
		_state->_creditsRunning = true;
		_state->_currentStatic = 1;
		_state->_timer = 0;
		draw();

	} else {
		_state->_staticCredits = false;
		_state->_creditSequence = -1;
		_state->_stSeqSettings[sequence].finished = false;
		_state->_creditsRunning = false;
		_state->_currentStatic = 0;
		_state->_timer = 0;
	}
}

void AGSCreditz2::IsStaticSequenceFinished(ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	int result = (_state->_stSeqSettings[sequence].finished) ? 1 : 0;
	_state->_stSeqSettings[sequence].finished = false;

	params._result = result;
}

void AGSCreditz2::ShowStaticCredit(ScriptMethodParams &params) {
	PARAMS6(int, sequence, int, id, int, time, int, style,
		int, styleSettings1, int, styleSettings2);

	_state->_creditSequence = sequence;
	_state->_creditsRunning = true;
	_state->_staticCredits = true;
	_state->_singleStatic.id = id;
	_state->_singleStatic.time = time;
	_state->_singleStatic.style = style;
	_state->_singleStatic.settings1 = styleSettings1;
	_state->_singleStatic.settings2 = styleSettings2;
	_state->_singleStatic.bool_ = true;
	_state->_stSeqSettings[sequence].finished = false;
	_state->_timer = 0;
	_state->_timer2 = 0;
	_state->_numChars = 0;
	draw();
}

void AGSCreditz2::SetStaticImage(ScriptMethodParams &params) {
	PARAMS6(int, sequence, int, id, int, slot, int, xPos, int, yPos, int, length);

	assert(sequence >= 0 && sequence < 10);
	if (id >= (int)_state->_stCredits[sequence].size())
		_state->_stCredits[sequence].resize(id + 1);

	_state->_stCredits[sequence][id].image = true;
	_state->_stCredits[sequence][id].image_slot = slot;
	_state->_stCredits[sequence][id].x = xPos;
	_state->_stCredits[sequence][id].y = yPos;
	_state->_stCredits[sequence][id].image_time = length;
}

void AGSCreditz2::GetCurrentStaticCredit(ScriptMethodParams &params) {
	int result = -1;
	if (_state->_creditsRunning && _state->_staticCredits)
		result = _state->_currentStatic;

	params._result = result;
}

void AGSCreditz2::calculateSequenceHeight(int sequence) {
	int32 height, creditHeight, dum;
	height = 0;

	for (uint currentCredit = 0; currentCredit < _state->_credits[sequence].size();
			++currentCredit) {

		if (_state->_credits[sequence][currentCredit]._isSet) {
			if (_state->_credits[sequence][currentCredit]._image) {
				if (_state->_credits[sequence][currentCredit]._colorHeight < 0)
					creditHeight = _engine->GetSpriteHeight(_state->_credits[sequence][currentCredit]._fontSlot);
				else
					creditHeight = _state->_credits[sequence][currentCredit]._colorHeight;
			} else {
				_engine->GetTextExtent(_state->_credits[sequence][currentCredit]._fontSlot,
					_state->_credits[sequence][currentCredit]._text.c_str(),
					&dum, &creditHeight);
			}

			height += creditHeight;
		} else {
			height += VGACheck(_state->_emptyLineHeight);
		}
	}

	_state->_calculatedSequenceHeight = height;
}

int AGSCreditz2::VGACheck(int value) {
	int32 screenX, dum;
	_engine->GetScreenDimensions(&screenX, &dum, &dum);

	if (screenX == 640)
		return value * 2;
	else
		return value;
}

void AGSCreditz2::draw() {
	int endPoint;

	if (_state->_creditsRunning) {
		_engine->PollSystem();

		if (!_state->_staticCredits) {
			// Scrolling credits
			drawMask(_state->_creditSequence, 0);

			if (_state->_seqSettings[_state->_creditSequence].automatic == 1)
				endPoint = 0 - _state->_calculatedSequenceHeight;
			else
				endPoint = _state->_seqSettings[_state->_creditSequence].endpoint;

			if (_state->_yPos >= endPoint) {
				doCredits();
			} else {
				if (_state->_seqSettings[_state->_creditSequence].endwait > 0 && _state->_timer <= _state->_seqSettings[_state->_creditSequence].endwait) {
					_state->_paused = true;
					doCredits();
					_state->_timer++;
					return;
				} else {
					_state->_paused = false;
					_state->_timer = 0;
					_state->_creditsRunning = false;
					_state->_seqSettings[_state->_creditSequence].finished = true;
				}
			}

			drawMask(_state->_creditSequence, 1);
		} else {
			// Static credits
			if (!_state->_singleStatic.bool_) {
				if (_state->_currentStatic < (int)_state->_stCredits[_state->_creditSequence].size()) {
					if (_state->_stCredits[_state->_creditSequence][_state->_currentStatic].pause > 0) {
						// Pause
						if (_state->_timer <= _state->_stCredits[_state->_creditSequence][_state->_currentStatic].pause) {
							_state->_timer++;
						} else {
							_state->_timer = 0;
							_state->_currentStatic++;
						}
					} else {
						if (_state->_stCredits[_state->_creditSequence][_state->_currentStatic].image) {
							// Image
							if (_state->_timer <= _state->_stCredits[_state->_creditSequence][_state->_currentStatic].image_time) {
								drawCredit(_state->_creditSequence, _state->_currentStatic);
								_state->_timer++;
							} else {
								_state->_timer = 0;
								_state->_currentStatic++;
								if (_state->_stCredits[_state->_creditSequence][_state->_currentStatic].pause <= 0 &&
									_state->_currentStatic < (int)_state->_stCredits[_state->_creditSequence].size())
									drawCredit(_state->_creditSequence, _state->_currentStatic);
								else
									return;
							}
						} else {
							// Text
							if (_state->_timer <= (_state->_stSeqSettings[_state->_creditSequence].speed *
								((int)_state->_stCredits[_state->_creditSequence][_state->_currentStatic].title.size() +
									(int)_state->_stCredits[_state->_creditSequence][_state->_currentStatic].credit.size()))) {
								drawCredit(_state->_creditSequence, _state->_currentStatic);
								_state->_timer++;
							} else {
								_state->_timer = 0;
								_state->_currentStatic++;
								if (_state->_stCredits[_state->_creditSequence][_state->_currentStatic].pause <= 0 &&
									(int)_state->_currentStatic <= (int)_state->_stCredits[_state->_creditSequence].size())
									drawCredit(_state->_creditSequence, _state->_currentStatic);
								else
									return;
							}
						}
					}
				} else {
					_state->_stSeqSettings[_state->_creditSequence].finished = true;
					_state->_creditsRunning = false;
					_state->_creditSequence = -1;
					_state->_timer = 0;
					_state->_currentStatic = 1;
					return;
				}
			} else {
				// Single Static
				if (_state->_timer <= _state->_singleStatic.time) {
					if (_state->_singleStatic.style == 0)
						drawCredit(_state->_creditSequence, _state->_singleStatic.id);
					else if (_state->_singleStatic.style == 1)
						drawStEffects(_state->_creditSequence, _state->_singleStatic.id, _state->_singleStatic.style);

					_state->_timer++;
				} else {
					_state->_timer = 0;
					_state->_singleStatic.bool_ = false;
					_state->_creditsRunning = false;
					_state->_staticCredits = false;
					_state->_stSeqSettings[_state->_creditSequence].finished = true;
					_state->_creditSequence = -1;
				}
			}
		}
	}
}

void AGSCreditz2::makeMask(int sequence) {
	int32 wid, hei, cold;
	int32 x, y;
	uint8 *Bitmap, *Bitmap2, *Bitmap3;

	_engine->GetBitmapDimensions(_state->_maskScreen, &wid, &hei, &cold);
	Bitmap = _engine->GetRawBitmapSurface(_state->_maski);
	Bitmap2 = _engine->GetRawBitmapSurface(_state->_maskScreen);
	Bitmap3 = _engine->GetRawBitmapSurface(_state->_creditScreen);

	uint32 Pitch = _engine->GetBitmapPitch(_state->_maski);
	uint32 Pitch2 = _engine->GetBitmapPitch(_state->_maskScreen);
	uint32 Pitch3 = _engine->GetBitmapPitch(_state->_creditScreen);

	// Top Mask
	{
		x = y = 0;
		while (y <= _state->_seqSettings[sequence].topmask) {
			_engine->PollSystem();
			while (x < wid) {
				drawPixel(Bitmap, x, y, 15, Pitch, cold);
				x++;
			}

			x = 0;
			y++;
		}
	}

	// Credits Iterator
	x = 0;
	y = _state->_seqSettings[sequence].topmask + 1;
	while (y < _state->_seqSettings[sequence].bottommask) {
		_engine->PollSystem();
		while (x < wid) {
			drawPixel(Bitmap, x, y, getPixelColor(Bitmap3, x, y, Pitch3, cold), Pitch, cold);
			x++;
		}

		x = 0;
		y++;
	}

	// Bottom Mask
	{
		x = 0;
		y = _state->_seqSettings[sequence].bottommask;
		while (y < hei) {
			_engine->PollSystem();
			while (x < wid) {
				drawPixel(Bitmap, x, y, getPixelColor(Bitmap2, x, y, Pitch2, cold), Pitch, cold);
				x++;
			}

			x = 0;
			y++;
		}
	}

	_engine->MarkRegionDirty(0, 0, wid, hei);
}

void AGSCreditz2::drawMask(int sequence, int getput) {
	int32 wid, hei, col;

	if (getput == 0) {
		_engine->GetScreenDimensions(&wid, &hei, &col);
		_engine->FreeBitmap(_state->_maski);
		_state->_maski = _engine->CreateBlankBitmap(wid, hei, col);
		_engine->FreeBitmap(_state->_creditScreen);
		_state->_creditScreen = _engine->CreateBlankBitmap(wid, hei, col);
		_state->_maskScreen = _engine->GetBackgroundScene(_engine->GetCurrentBackground());

	} else if (getput == 1) {
		_engine->SetVirtualScreen(_state->_maski);
		makeMask(sequence);
		_engine->ReleaseBitmapSurface(_state->_maskScreen);
	}
}

int AGSCreditz2::drawCredit(int sequence, int credit) {
	int font, color;
	int32 x_pos, leveys, korkeus = 0;
	int32 scrn_width, scrn_height, coldepth, y_posit;
	int slot, sprite_height, sprite_width, linecount, line, others;
	Common::String text;
	Common::String teksti;
	BITMAP *sprite;
	int result = 0;

	_engine->GetScreenDimensions(&scrn_width, &scrn_height, &coldepth);

	if (!_state->_staticCredits) {
		// Scrolling Credits
		if ((_state->_yPos + _state->_sequenceHeight) > scrn_height)
			return 0;

		if (_state->_credits[sequence][credit]._image) {
			slot = _state->_credits[sequence][credit]._fontSlot;
			sprite_height = _state->_credits[sequence][credit]._colorHeight;
			x_pos = _state->_credits[sequence][credit]._x;

			if (x_pos < 0) {
				sprite_width = _engine->GetSpriteWidth(slot);
				x_pos = (scrn_width - sprite_width) / 2;
			} else {
				x_pos = VGACheck(x_pos);
			}

			if (sprite_height < 0)
				sprite_height = _engine->GetSpriteHeight(slot);
			else
				sprite_height = VGACheck(sprite_height);

			korkeus = sprite_height;

			sprite = _engine->GetSpriteGraphic(slot);
			_engine->BlitBitmap(x_pos, _state->_yPos + _state->_sequenceHeight, sprite, 1);

		} else {
			font = _state->_credits[sequence][credit]._fontSlot;
			color = _state->_credits[sequence][credit]._colorHeight;
			text = _state->_credits[sequence][credit]._text;
			x_pos = _state->_credits[sequence][credit]._x;

			if (!text.empty()) {
				_engine->GetTextExtent(font, text.c_str(), &leveys, &korkeus);

				if (x_pos < 0) {
					x_pos = (scrn_width - leveys) / 2;
				} else {
					x_pos = VGACheck(x_pos);
				}

				if (text.contains('<')) {
					specialEffect(sequence, credit, text, font, color, x_pos);
				} else {
					if (_state->_credits[sequence][credit]._outline) {
						// Outline
						_engine->DrawText(x_pos - 1, _state->_yPos + _state->_sequenceHeight, font, 16, text.c_str());
						_engine->DrawText(x_pos + 1, _state->_yPos + _state->_sequenceHeight, font, 16, text.c_str());
						_engine->DrawText(x_pos, _state->_yPos + _state->_sequenceHeight - 1, font, 16, text.c_str());
						_engine->DrawText(x_pos, _state->_yPos + _state->_sequenceHeight + 1, font, 16, text.c_str());
					}

					_engine->DrawText(x_pos, _state->_yPos + _state->_sequenceHeight, font, color, text.c_str());
				}
			}
		}

		result = korkeus;
	} else {

		if (_state->_stCredits[sequence][credit].image) {
			x_pos = _state->_stCredits[sequence][credit].x;
			y_posit = _state->_stCredits[sequence][credit].y;
			font = _state->_stCredits[sequence][credit].image_slot;

			sprite = _engine->GetSpriteGraphic(font);
			_engine->GetBitmapDimensions(sprite, &leveys, &korkeus, &coldepth);

			if (x_pos < 0)
				x_pos = (scrn_width - leveys) / 2;
			else
				x_pos = VGACheck(x_pos);
			if (y_posit < 0)
				y_posit = (scrn_height - korkeus) / 2;
			else
				y_posit = VGACheck(y_posit);

			_engine->BlitBitmap(x_pos, y_posit, sprite, 1);
			result = 0;
		} else {
			// Title
			font = _state->_stCredits[sequence][credit].title_font;
			color = _state->_stCredits[sequence][credit].title_color;
			text = _state->_stCredits[sequence][credit].title;
			x_pos = _state->_stCredits[sequence][credit].title_x;
			y_posit = _state->_stCredits[sequence][credit].title_y;

			if (!text.empty()) {
				_engine->GetTextExtent(font, text.c_str(), &leveys, &korkeus);

				if (x_pos < 0)
					x_pos = (scrn_width - leveys) / 2;
				else
					x_pos = VGACheck(x_pos);
				if (y_posit < 0)
					y_posit = (scrn_height - korkeus) / 2;
				else
					y_posit = VGACheck(y_posit);

				if (_state->_stCredits[sequence][credit].title_outline) {
					_engine->DrawText(x_pos - 1, y_posit, font, 16, text.c_str());
					_engine->DrawText(x_pos + 1, y_posit, font, 16, text.c_str());
					_engine->DrawText(x_pos, y_posit - 1, font, 16, text.c_str());
					_engine->DrawText(x_pos, y_posit + 1, font, 16, text.c_str());
				}

				_engine->DrawText(x_pos, y_posit, font, color, text.c_str());
				_engine->MarkRegionDirty(x_pos - 15, y_posit - 15, x_pos + leveys + 15, y_posit + korkeus + 15);
			}

			// Credit
			font = _state->_stCredits[sequence][credit].font;
			color = _state->_stCredits[sequence][credit].color;
			text = _state->_stCredits[sequence][credit].credit;
			x_pos = _state->_stCredits[sequence][credit].x;
			y_posit = _state->_stCredits[sequence][credit].y;

			if (!text.empty()) {
				_engine->GetTextExtent(font, text.c_str(), &leveys, &korkeus);

				if (!text.contains("[[")) {
					if (x_pos < 0)
						x_pos = (scrn_width - leveys) / 2;
					else
						x_pos = VGACheck(x_pos);
					if (y_posit < 0)
						y_posit = (scrn_height - korkeus) / 2;
					else
						y_posit = VGACheck(y_posit);

					if (_state->_stCredits[sequence][credit].outline) {
						_engine->DrawText(x_pos - 1, y_posit, font, 16, text.c_str());
						_engine->DrawText(x_pos + 1, y_posit, font, 16, text.c_str());
						_engine->DrawText(x_pos, y_posit - 1, font, 16, text.c_str());
						_engine->DrawText(x_pos, y_posit + 1, font, 16, text.c_str());
					}

					_engine->DrawText(x_pos, y_posit, font, color, text.c_str());
					_engine->MarkRegionDirty(x_pos - 15, y_posit - 15, x_pos + leveys + 15, y_posit + korkeus + 15);
				} else {
					linecount = countLines(text);
					line = 1;
					teksti = text;
					others = 0;
					if (y_posit < 0)
						y_posit = (scrn_height - (korkeus * (linecount + 1))) / 2;
					else
						y_posit = VGACheck(y_posit);

					while (line <= linecount + 1) {
						text = extractParameter(teksti, "[[");
						_engine->GetTextExtent(font, text.c_str(), &leveys, &korkeus);

						if (x_pos < 0)
							x_pos = (scrn_width - leveys) / 2;
						else
							x_pos = VGACheck(x_pos);

						if (_state->_stCredits[sequence][credit].outline) {
							_engine->DrawText(x_pos - 1, y_posit + others, font, 16, text.c_str());
							_engine->DrawText(x_pos + 1, y_posit + others, font, 16, text.c_str());
							_engine->DrawText(x_pos, y_posit + others - 1, font, 16, text.c_str());
							_engine->DrawText(x_pos, y_posit + others + 1, font, 16, text.c_str());
						}
						_engine->DrawText(x_pos, y_posit + others, font, color, text.c_str());
						_engine->MarkRegionDirty(x_pos, y_posit + others, x_pos + leveys, y_posit + others + korkeus + 15);
						others += korkeus;
						x_pos = _state->_stCredits[sequence][credit].x;
						line++;
					}
				}
			}

			result = 0;
		}
	}

	return result;
}

void AGSCreditz2::doCredits() {
	int current_line;
	int32 increment, dum;

	current_line = 1;
	_state->_sequenceHeight = 0;

	while (current_line < (int)_state->_credits[_state->_creditSequence].size()) {
		if (_state->_credits[_state->_creditSequence][current_line]._isSet) {
			if (_state->_credits[_state->_creditSequence][current_line]._image) {
				increment = _engine->GetSpriteHeight(_state->_credits[_state->_creditSequence][current_line]._fontSlot);

				if ((_state->_yPos + _state->_sequenceHeight + increment) <= 0) {
					if (_state->_credits[_state->_creditSequence][current_line]._colorHeight >= 0)
						increment = VGACheck(_state->_credits[_state->_creditSequence][current_line]._colorHeight);
				}
			} else {
				_engine->GetTextExtent(_state->_credits[_state->_creditSequence][current_line]._fontSlot,
					_state->_credits[_state->_creditSequence][current_line]._text.c_str(), &dum, &increment);
			}

			if ((_state->_yPos + _state->_sequenceHeight + increment) > 0)
				increment = drawCredit(_state->_creditSequence, current_line);

		} else {
			increment = VGACheck(_state->_emptyLineHeight);
		}

		_state->_sequenceHeight += increment;
		current_line++;
	}

	if (!_state->_paused)
		speeder(_state->_creditSequence);
}

int AGSCreditz2::countLines(const Common::String &text) {
	int lines;
	Common::String teksti;

	teksti = text;
	lines = 0;
	while (teksti.contains("[[")) {
		extractParameter(teksti, "[[");
		++lines;
	}

	return lines;
}

Common::String AGSCreditz2::extractParameter(Common::String &line, const Common::String &separator) {
	int p;
	Common::String result;

	p = line.find(separator, 0);
	if (p != -1) {
		if (p > 0) {
			result = line.substr(0, p);
			result.trim();
		}

		line = line.substr(p + separator.size());
	} else if (!line.empty()) {
		line.trim();
		result.clear();
	}

	return result;
}

void AGSCreditz2::specialEffect(int sequence, int credit, const Common::String &text,
	int font, int color, int32 x_pos) {
	Common::String creditt[3];
	Common::String credit_text, dots;
	int32 scrn_width, dum, rightside_width, leftside_width, dotwidth;
	int space, dotcount, dotpos;

	_engine->GetScreenDimensions(&scrn_width, &dum, &dum);

	if (text.contains("<>")) {
		if (x_pos < 0)
			x_pos = 0;

		credit_text = text;
		creditt[0] = extractParameter(credit_text, "<");
		creditt[1] = extractParameter(credit_text, ">");
		creditt[2] = credit_text;

		_engine->GetTextExtent(font, creditt[2].c_str(), &rightside_width, &dum);

		if (_state->_credits[sequence][credit]._outline) {
			_engine->DrawText(x_pos - 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos + 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos, _state->_yPos + _state->_sequenceHeight - 1, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos, _state->_yPos + _state->_sequenceHeight + 1, font, color, creditt[0].c_str());
		}

		_engine->DrawText(x_pos, _state->_yPos + _state->_sequenceHeight, font, color, creditt[0].c_str());

		if (_state->_credits[sequence][credit]._outline) {
			_engine->DrawText(scrn_width - (x_pos + rightside_width) - 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width) + 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width), _state->_yPos + _state->_sequenceHeight - 1, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width), _state->_yPos + _state->_sequenceHeight + 1, font, color, creditt[2].c_str());
		}
		_engine->DrawText(scrn_width - (x_pos + rightside_width), _state->_yPos + _state->_sequenceHeight, font, color, creditt[2].c_str());
	} else if (text.contains("<.>")) {
		if (x_pos < 0)
			x_pos = 0;

		credit_text = text;
		creditt[0] = extractParameter(credit_text, "<");
		creditt[1] = extractParameter(credit_text, ">");
		creditt[2] = credit_text;

		_engine->GetTextExtent(font, creditt[2].c_str(), &rightside_width, &dum);
		_engine->GetTextExtent(font, creditt[0].c_str(), &leftside_width, &dum);

		if (_state->_credits[sequence][credit]._outline) {
			_engine->DrawText(x_pos - 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos + 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos, _state->_yPos + _state->_sequenceHeight - 1, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos, _state->_yPos + _state->_sequenceHeight + 1, font, color, creditt[0].c_str());
		}

		_engine->DrawText(x_pos, _state->_yPos + _state->_sequenceHeight, font, color, creditt[0].c_str());

		if (_state->_credits[sequence][credit]._outline) {
			_engine->DrawText(scrn_width - (x_pos + rightside_width) - 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width) + 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width), _state->_yPos + _state->_sequenceHeight - 1, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width), _state->_yPos + _state->_sequenceHeight + 1, font, color, creditt[2].c_str());
		}
		_engine->DrawText(scrn_width - (x_pos + rightside_width), _state->_yPos + _state->_sequenceHeight, font, color, creditt[2].c_str());

		_engine->GetTextExtent(font, " .", &dotwidth, &dum);
		space = scrn_width - (x_pos + leftside_width + x_pos + rightside_width);
		dotcount = space / dotwidth;
		dotpos = 0;
		dots = "";
		while (dotpos < dotcount) {
			dots = dots + " .";
			dotpos++;
		}

		if (_state->_credits[sequence][credit]._outline) {
			_engine->DrawText(x_pos + leftside_width - 1, _state->_yPos + _state->_sequenceHeight, font, color, dots.c_str());
			_engine->DrawText(x_pos + leftside_width + 1, _state->_yPos + _state->_sequenceHeight, font, color, dots.c_str());
			_engine->DrawText(x_pos + leftside_width, _state->_yPos + _state->_sequenceHeight - 1, font, color, dots.c_str());
			_engine->DrawText(x_pos + leftside_width, _state->_yPos + _state->_sequenceHeight + 1, font, color, dots.c_str());
		}
		_engine->DrawText(x_pos + leftside_width, _state->_yPos + _state->_sequenceHeight, font, color, dots.c_str());
	} else if (text.contains("<#>")) {
		if (x_pos < 0)
			x_pos = 0;

		credit_text = text;
		creditt[0] = extractParameter(credit_text, "<");
		creditt[1] = extractParameter(credit_text, ">");
		creditt[2] = credit_text;

		_engine->GetTextExtent(font, creditt[2].c_str(), &rightside_width, &dum);
		_engine->GetTextExtent(font, creditt[0].c_str(), &leftside_width, &dum);

		if (_state->_credits[sequence][credit]._outline) {
			_engine->DrawText(scrn_width / 2 - x_pos / 2 - leftside_width - 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(scrn_width / 2 - x_pos / 2 - leftside_width + 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(scrn_width / 2 - x_pos / 2 - leftside_width, _state->_yPos + _state->_sequenceHeight - 1, font, color, creditt[0].c_str());
			_engine->DrawText(scrn_width / 2 - x_pos / 2 - leftside_width, _state->_yPos + _state->_sequenceHeight + 1, font, color, creditt[0].c_str());
		}
		_engine->DrawText(scrn_width / 2 - x_pos / 2 - leftside_width, _state->_yPos + _state->_sequenceHeight, font, color, creditt[0].c_str());

		if (_state->_credits[sequence][credit]._outline) {
			_engine->DrawText(scrn_width / 2 + x_pos / 2 - 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width / 2 + x_pos / 2 + 1, _state->_yPos + _state->_sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width / 2 + x_pos / 2, _state->_yPos + _state->_sequenceHeight - 1, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width / 2 + x_pos / 2, _state->_yPos + _state->_sequenceHeight + 1, font, color, creditt[2].c_str());
		}
		_engine->DrawText(scrn_width / 2 + x_pos / 2, _state->_yPos + _state->_sequenceHeight, font, color, creditt[2].c_str());
	}
}

void AGSCreditz2::drawStEffects(int sequence, int id, int style) {
	Common::String teksti;
	Common::String teksti2;
	int thischar;
	Common::String text;
	int font, color, set1, set2;
	int32 x_pos, y_posit, scrn_width, leveys, scrn_height, korkeus, coldepth;

	teksti = _state->_stCredits[sequence][id].credit;
	font = _state->_stCredits[sequence][id].font;
	color = _state->_stCredits[sequence][id].color;
	x_pos = _state->_stCredits[sequence][id].x;
	y_posit = _state->_stCredits[sequence][id].y;
	set1 = _state->_singleStatic.settings1;
	set2 = _state->_singleStatic.settings2;

	_engine->GetScreenDimensions(&scrn_width, &scrn_height, &coldepth);
	_engine->GetTextExtent(font, teksti.c_str(), &leveys, &korkeus);

	if (style == 1) {
		if (set2 >= 0 && _state->_numChars < (int)teksti.size() && _state->_timer2 == 0) {
			(*_playSound)(set2);
		}

		if (_state->_timer2 <= set1) {
			thischar = 0;
			if (thischar <= _state->_numChars && _state->_numChars <= (int)teksti.size()) {
				for (thischar = 0; thischar < _state->_numChars; ++thischar)
					teksti2 = teksti2 + teksti[thischar];
				text = teksti2;
			} else {
				text = teksti;
			}

			if (x_pos < 0)
				x_pos = (scrn_width - leveys) / 2;
			else
				x_pos = VGACheck(x_pos);
			if (y_posit < 0)
				y_posit = (scrn_height - korkeus) / 2;
			else
				y_posit = VGACheck(y_posit);

			_engine->DrawText(x_pos, y_posit, font, color, text.c_str());
			_state->_timer2++;
			x_pos = _state->_stCredits[sequence][id].x;
			y_posit = _state->_stCredits[sequence][id].y;
		} else {
			_state->_numChars++;
			thischar = 0;
			_state->_timer2 = 0;
			drawStEffects(sequence, id, style);
		}
	}
}

void AGSCreditz2::speeder(int sequence) {
	int speed = _state->_seqSettings[sequence].speed;

	if (_state->_speedPoint == speed) {
		_state->_yPos -= VGACheck(1);
		_state->_speedPoint = 0;
	} else {
		_state->_speedPoint++;
	}
}

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3
