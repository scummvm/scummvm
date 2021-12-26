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

#include "ags/plugins/ags_creditz/ags_creditz.h"
#include "ags/lib/allegro/surface.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

void AGSCreditz::draw() {
	int endPoint;

	if (_creditsRunning) {
		_engine->PollSystem();

		if (!_staticCredits) {
			// Scrolling credits
			if (_seqSettings[_creditSequence].automatic == 1)
				endPoint = 0 - _calculatedSequenceHeight;
			else
				endPoint = _seqSettings[_creditSequence].endpoint;

			if (_yPos >= endPoint) {
				doCredits();
			} else {
				if (_seqSettings[_creditSequence].endwait > 0 && _timer <= _seqSettings[_creditSequence].endwait) {
					_paused = true;
					doCredits();
					_timer++;
					return;
				} else {
					_paused = false;
					_timer = 0;
					_creditsRunning = false;
					_seqSettings[_creditSequence].finished = true;
				}
			}

			_engine->MarkRegionDirty(0, 0, _screenWidth, _screenHeight);
		} else {
			// credits
			if (!_singleStatic.bool_) {
				if (_currentStatic < (int)_stCredits[_creditSequence].size()) {
					if (_stCredits[_creditSequence][_currentStatic].pause > 0) {
						// Pause
						if (_timer <= _stCredits[_creditSequence][_currentStatic].pause) {
							_timer++;
						} else {
							_timer = 0;
							_currentStatic++;
						}
					} else {
						if (_stCredits[_creditSequence][_currentStatic].image) {
							// Image
							if (_timer <= _stCredits[_creditSequence][_currentStatic].image_time) {
								drawCredit(_creditSequence, _currentStatic);
								_timer++;
							} else {
								_timer = 0;
								_currentStatic++;
								if (_stCredits[_creditSequence][_currentStatic].pause <= 0 &&
								        _currentStatic< (int)_stCredits[_creditSequence].size())
									drawCredit(_creditSequence, _currentStatic);
								else
									return;
							}
						} else {
							// Text
							if (_timer <= (_stSeqSettings[_creditSequence].speed *
							                       ((int)_stCredits[_creditSequence][_currentStatic].title.size() +
							                        (int)_stCredits[_creditSequence][_currentStatic].credit.size()))) {
								drawCredit(_creditSequence, _currentStatic);
								_timer++;
							} else {
								_timer = 0;
								_currentStatic++;
								if (_stCredits[_creditSequence][_currentStatic].pause <= 0 &&
								        (int)_currentStatic<= (int)_stCredits[_creditSequence].size())
									drawCredit(_creditSequence, _currentStatic);
								else
									return;
							}
						}
					}
				} else {
					_stSeqSettings[_creditSequence].finished = true;
					_creditsRunning = false;
					_creditSequence = -1;
					_timer = 0;
					_currentStatic= 1;
					return;
				}
			} else {
				// Single Static
				if (_timer <= _singleStatic.time) {
					if (_singleStatic.style == 0)
						drawCredit(_creditSequence, _singleStatic.id);
					else if (_singleStatic.style == 1)
						drawStEffects(_creditSequence, _singleStatic.id, _singleStatic.style);

					_timer++;
				} else {
					_timer = 0;
					_singleStatic.bool_ = false;
					_creditsRunning = false;
					_staticCredits = false;
					_stSeqSettings[_creditSequence].finished = true;
					_creditSequence = -1;
				}
			}
		}
	}
}

int AGSCreditz::drawCredit(int sequence, int credit) {
	int font, color;
	int32 x_pos, leveys, korkeus = 0;
	int32 scrn_width, scrn_height, coldepth, y_posit;
	int slot, sprite_height, sprite_width, linecount, line, others;
	Common::String text;
	Common::String teksti;
	BITMAP *sprite;
	int result = 0;

	_engine->GetScreenDimensions(&scrn_width, &scrn_height, &coldepth);

	if (!_staticCredits) {
		// Scrolling Credits
		if ((_yPos + _sequenceHeight) > scrn_height)
			return 0;

		if (_credits[sequence][credit]._image) {
			slot = _credits[sequence][credit]._fontSlot;
			sprite_height = _credits[sequence][credit]._colorHeight;
			x_pos = _credits[sequence][credit]._x;

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
			_engine->BlitBitmap(x_pos, _yPos + _sequenceHeight, sprite, 1);

		} else {
			font = _credits[sequence][credit]._fontSlot;
			color = _credits[sequence][credit]._colorHeight;
			text = _credits[sequence][credit]._text;
			x_pos = _credits[sequence][credit]._x;

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
					if (_credits[sequence][credit]._outline) {
						// Outline
						_engine->DrawText(x_pos - 1, _yPos + _sequenceHeight, font, 16, text.c_str());
						_engine->DrawText(x_pos + 1, _yPos + _sequenceHeight, font, 16, text.c_str());
						_engine->DrawText(x_pos, _yPos + _sequenceHeight - 1, font, 16, text.c_str());
						_engine->DrawText(x_pos, _yPos + _sequenceHeight + 1, font, 16, text.c_str());
					}

					_engine->DrawText(x_pos, _yPos + _sequenceHeight, font, color, text.c_str());
				}
			}
		}

		result = korkeus;
	} else {

		if (_stCredits[sequence][credit].image) {
			x_pos = _stCredits[sequence][credit].x;
			y_posit = _stCredits[sequence][credit].y;
			font = _stCredits[sequence][credit].image_slot;

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
			font = _stCredits[sequence][credit].title_font;
			color = _stCredits[sequence][credit].title_color;
			text = _stCredits[sequence][credit].title;
			x_pos = _stCredits[sequence][credit].title_x;
			y_posit = _stCredits[sequence][credit].title_y;

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

				if (_stCredits[sequence][credit].title_outline) {
					_engine->DrawText(x_pos - 1, y_posit, font, 16, text.c_str());
					_engine->DrawText(x_pos + 1, y_posit, font, 16, text.c_str());
					_engine->DrawText(x_pos, y_posit - 1, font, 16, text.c_str());
					_engine->DrawText(x_pos, y_posit + 1, font, 16, text.c_str());
				}

				_engine->DrawText(x_pos, y_posit, font, color, text.c_str());
				_engine->MarkRegionDirty(x_pos - 15, y_posit - 15, x_pos + leveys + 15, y_posit + korkeus + 15);
			}

			// Credit
			font = _stCredits[sequence][credit].font;
			color = _stCredits[sequence][credit].color;
			text = _stCredits[sequence][credit].credit;
			x_pos = _stCredits[sequence][credit].x;
			y_posit = _stCredits[sequence][credit].y;

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

					if (_stCredits[sequence][credit].outline) {
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

						if (_stCredits[sequence][credit].outline) {
							_engine->DrawText(x_pos - 1, y_posit + others, font, 16, text.c_str());
							_engine->DrawText(x_pos + 1, y_posit + others, font, 16, text.c_str());
							_engine->DrawText(x_pos, y_posit + others - 1, font, 16, text.c_str());
							_engine->DrawText(x_pos, y_posit + others + 1, font, 16, text.c_str());
						}
						_engine->DrawText(x_pos, y_posit + others, font, color, text.c_str());
						_engine->MarkRegionDirty(x_pos, y_posit + others, x_pos + leveys, y_posit + others + korkeus + 15);
						others += korkeus;
						x_pos = _stCredits[sequence][credit].x;
						line++;
					}
				}
			}

			result = 0;
		}
	}

	return result;
}

void AGSCreditz::doCredits() {
	int current_line;
	int32 increment, dum;

	current_line = 1;
	_sequenceHeight = 0;

	while (current_line < (int)_credits[_creditSequence].size()) {
		if (_credits[_creditSequence][current_line]._isSet) {
			if (_credits[_creditSequence][current_line]._image) {
				increment = _engine->GetSpriteHeight(_credits[_creditSequence][current_line]._fontSlot);

				if ((_yPos + _sequenceHeight + increment) <= 0) {
					if (_credits[_creditSequence][current_line]._colorHeight >= 0)
						increment = VGACheck(_credits[_creditSequence][current_line]._colorHeight);
				}
			} else {
				_engine->GetTextExtent(_credits[_creditSequence][current_line]._fontSlot,
				                       _credits[_creditSequence][current_line]._text.c_str(), &dum, &increment);
			}

			if ((_yPos + _sequenceHeight + increment) > 0)
				increment = drawCredit(_creditSequence, current_line);

		} else {
			increment = VGACheck(_emptyLineHeight);
		}

		_sequenceHeight += increment;
		current_line++;
	}

	if (!_paused)
		speeder(_creditSequence);
}

int AGSCreditz::countLines(const Common::String &text) {
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

Common::String AGSCreditz::extractParameter(Common::String &line, const Common::String &separator) {
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

void AGSCreditz::specialEffect(int sequence, int credit, const Common::String &text,
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

		if (_credits[sequence][credit]._outline) {
			_engine->DrawText(x_pos - 1, _yPos + _sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos + 1, _yPos + _sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos, _yPos + _sequenceHeight - 1, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos, _yPos + _sequenceHeight + 1, font, color, creditt[0].c_str());
		}

		_engine->DrawText(x_pos, _yPos + _sequenceHeight, font, color, creditt[0].c_str());

		if (_credits[sequence][credit]._outline) {
			_engine->DrawText(scrn_width - (x_pos + rightside_width) - 1, _yPos + _sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width) + 1, _yPos + _sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width), _yPos + _sequenceHeight - 1, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width), _yPos + _sequenceHeight + 1, font, color, creditt[2].c_str());
		}
		_engine->DrawText(scrn_width - (x_pos + rightside_width), _yPos + _sequenceHeight, font, color, creditt[2].c_str());
	} else if (text.contains("<.>")) {
		if (x_pos < 0)
			x_pos = 0;

		credit_text = text;
		creditt[0] = extractParameter(credit_text, "<");
		creditt[1] = extractParameter(credit_text, ">");
		creditt[2] = credit_text;

		_engine->GetTextExtent(font, creditt[2].c_str(), &rightside_width, &dum);
		_engine->GetTextExtent(font, creditt[0].c_str(), &leftside_width, &dum);

		if (_credits[sequence][credit]._outline) {
			_engine->DrawText(x_pos - 1, _yPos + _sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos + 1, _yPos + _sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos, _yPos + _sequenceHeight - 1, font, color, creditt[0].c_str());
			_engine->DrawText(x_pos, _yPos + _sequenceHeight + 1, font, color, creditt[0].c_str());
		}

		_engine->DrawText(x_pos, _yPos + _sequenceHeight, font, color, creditt[0].c_str());

		if (_credits[sequence][credit]._outline) {
			_engine->DrawText(scrn_width - (x_pos + rightside_width) - 1, _yPos + _sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width) + 1, _yPos + _sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width), _yPos + _sequenceHeight - 1, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width - (x_pos + rightside_width), _yPos + _sequenceHeight + 1, font, color, creditt[2].c_str());
		}
		_engine->DrawText(scrn_width - (x_pos + rightside_width), _yPos + _sequenceHeight, font, color, creditt[2].c_str());

		_engine->GetTextExtent(font, " .", &dotwidth, &dum);
		space = scrn_width - (x_pos + leftside_width + x_pos + rightside_width);
		dotcount = space / dotwidth;
		dotpos = 0;
		dots = "";
		while (dotpos < dotcount) {
			dots = dots + " .";
			dotpos++;
		}

		if (_credits[sequence][credit]._outline) {
			_engine->DrawText(x_pos + leftside_width - 1, _yPos + _sequenceHeight, font, color, dots.c_str());
			_engine->DrawText(x_pos + leftside_width + 1, _yPos + _sequenceHeight, font, color, dots.c_str());
			_engine->DrawText(x_pos + leftside_width, _yPos + _sequenceHeight - 1, font, color, dots.c_str());
			_engine->DrawText(x_pos + leftside_width, _yPos + _sequenceHeight + 1, font, color, dots.c_str());
		}
		_engine->DrawText(x_pos + leftside_width, _yPos + _sequenceHeight, font, color, dots.c_str());
	} else if (text.contains("<#>")) {
		if (x_pos < 0)
			x_pos = 0;

		credit_text = text;
		creditt[0] = extractParameter(credit_text, "<");
		creditt[1] = extractParameter(credit_text, ">");
		creditt[2] = credit_text;

		_engine->GetTextExtent(font, creditt[2].c_str(), &rightside_width, &dum);
		_engine->GetTextExtent(font, creditt[0].c_str(), &leftside_width, &dum);

		if (_credits[sequence][credit]._outline) {
			_engine->DrawText(scrn_width / 2 - x_pos / 2 - leftside_width - 1, _yPos + _sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(scrn_width / 2 - x_pos / 2 - leftside_width + 1, _yPos + _sequenceHeight, font, color, creditt[0].c_str());
			_engine->DrawText(scrn_width / 2 - x_pos / 2 - leftside_width, _yPos + _sequenceHeight - 1, font, color, creditt[0].c_str());
			_engine->DrawText(scrn_width / 2 - x_pos / 2 - leftside_width, _yPos + _sequenceHeight + 1, font, color, creditt[0].c_str());
		}
		_engine->DrawText(scrn_width / 2 - x_pos / 2 - leftside_width, _yPos + _sequenceHeight, font, color, creditt[0].c_str());

		if (_credits[sequence][credit]._outline) {
			_engine->DrawText(scrn_width / 2 + x_pos / 2 - 1, _yPos + _sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width / 2 + x_pos / 2 + 1, _yPos + _sequenceHeight, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width / 2 + x_pos / 2, _yPos + _sequenceHeight - 1, font, color, creditt[2].c_str());
			_engine->DrawText(scrn_width / 2 + x_pos / 2, _yPos + _sequenceHeight + 1, font, color, creditt[2].c_str());
		}
		_engine->DrawText(scrn_width / 2 + x_pos / 2, _yPos + _sequenceHeight, font, color, creditt[2].c_str());
	}
}

void AGSCreditz::drawStEffects(int sequence, int id, int style) {
	Common::String teksti;
	Common::String teksti2;
	int thischar;
	Common::String text;
	int font, color, set1, set2;
	int32 x_pos, y_posit, scrn_width, leveys, scrn_height, korkeus, coldepth;

	teksti = _stCredits[sequence][id].credit;
	font = _stCredits[sequence][id].font;
	color = _stCredits[sequence][id].color;
	x_pos = _stCredits[sequence][id].x;
	y_posit = _stCredits[sequence][id].y;
	set1 = _singleStatic.settings1;
	set2 = _singleStatic.settings2;

	_engine->GetScreenDimensions(&scrn_width, &scrn_height, &coldepth);
	_engine->GetTextExtent(font, teksti.c_str(), &leveys, &korkeus);

	if (style == 1) {
		if (set2 >= 0 && _numChars < (int)teksti.size() && _timer2 == 0) {
			_playSound(set2);
		}

		if (_timer2 <= set1) {
			thischar = 0;
			if (thischar <= _numChars && _numChars <= (int)teksti.size()) {
				for (thischar = 0; thischar < _numChars; ++thischar)
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
			_timer2++;
			x_pos = _stCredits[sequence][id].x;
			y_posit = _stCredits[sequence][id].y;
		} else {
			_numChars++;
			thischar = 0;
			_timer2 = 0;
			drawStEffects(sequence, id, style);
		}
	}
}

void AGSCreditz::speeder(int sequence) {
	int speed = _seqSettings[sequence].speed;

	if (_speedPoint == speed) {
		_yPos -= VGACheck(1);
		_speedPoint = 0;
	} else {
		_speedPoint++;
	}
}


void AGSCreditz::calculateSequenceHeight(int sequence) {
	int32 height, creditHeight, dum;
	height = 0;

	for (uint currentCredit = 0; currentCredit < _credits[sequence].size();
	        ++currentCredit) {

		if (_credits[sequence][currentCredit]._isSet) {
			if (_credits[sequence][currentCredit]._image) {
				if (_credits[sequence][currentCredit]._colorHeight < 0)
					creditHeight = _engine->GetSpriteHeight(_credits[sequence][currentCredit]._fontSlot);
				else
					creditHeight = _credits[sequence][currentCredit]._colorHeight;
			} else {
				_engine->GetTextExtent(_credits[sequence][currentCredit]._fontSlot,
				                       _credits[sequence][currentCredit]._text.c_str(),
				                       &dum, &creditHeight);
			}

			height += creditHeight;
		} else {
			height += VGACheck(_emptyLineHeight);
		}
	}

	_calculatedSequenceHeight = height;
}

int AGSCreditz::VGACheck(int value) {
	int32 screenX, dum;
	_engine->GetScreenDimensions(&screenX, &dum, &dum);

	if (screenX == 640)
		return value * 2;
	else
		return value;
}

void AGSCreditz::startSequence(int sequence) {
	if (!_creditsRunning) {
		_seqSettings[sequence].finished = false;
		_creditsRunning = true;
		_creditSequence = sequence;

		_engine->GetScreenDimensions(&_screenWidth, &_screenHeight,
		                             &_screenColorDepth);

		if (_seqSettings[sequence].automatic) {
			calculateSequenceHeight(sequence);
			_yPos = _screenHeight + 1;
		} else {
			_yPos = _seqSettings[sequence].startpoint;
		}

		_speedPoint = 0;
		_timer = 0;
		draw();
	} else {
		_paused = false;
		_creditsRunning = false;
		_creditSequence = -1;
		_seqSettings[sequence].finished = true;
	}
}

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3
