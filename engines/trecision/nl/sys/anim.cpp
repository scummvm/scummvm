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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/nl/ll/llinc.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "video/smk_decoder.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "trecision/trecision.h"

namespace Trecision {

class NightlongSmackerDecoder : public Video::SmackerDecoder {
public:
	bool loadStream(Common::SeekableReadStream *stream) override {
		if (Video::SmackerDecoder::loadStream(stream)) {
			// Map audio tracks to sound types
			for (uint32 i = 0; i < 8; i++) {
				Track *t = getTrack(i);
				if (t && t->getTrackType() == Track::kTrackTypeAudio) {
					AudioTrack *audio = (AudioTrack *)t;
					audio->setMute(false);
					audio->setSoundType(i == 7 ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType);
				}
			}
			return true;
		}
		return false;
	}

	void muteTrack(uint track, bool mute) {
		Track *t = getTrack(track);
		if (t && t->getTrackType() == Track::kTrackTypeAudio) {
			((AudioTrack *)t)->setMute(mute);
		}
	}

	void setMute(bool mute) {
		for (TrackList::iterator it = getTrackListBegin(); it != getTrackListEnd(); it++) {
			if ((*it)->getTrackType() == Track::kTrackTypeAudio)
				((AudioTrack *)*it)->setMute(mute);
		}
	}
};

SDText sdt, osdt;

// locals
#define SMACKNULL	0
#define SMACKOPEN	1
#define SMACKCLOSE  2
#define MAXSMACK	3

extern uint8 *SmackBuffer[MAXSMACK];

extern uint16 _smackPal[MAXSMACK][256];
extern uint8  _curSmackAction;
extern uint8  _curSmackBuffer;

extern uint16 _playingAnims[MAXSMACK];
extern uint16 _curAnimFrame[MAXSMACK];
extern uint16 _numPlayingAnims;
extern uint16 _newData[260];
extern uint32 _newData2[260];

NightlongSmackerDecoder *SmkAnims[MAXSMACK];
extern uint16 _animMaxX, _animMinX, _animMaxY, _animMinY;
extern Graphics::PixelFormat ScreenFormat;

// from regen.c
extern int16 limiti[20][4];
extern uint16 limitinum;
extern int Hlim;
extern const char *_sysSentence[];

/*-----------------18/01/97 21.05-------------------
					CallSmackOpen
--------------------------------------------------*/
void CallSmackOpen(Common::SeekableReadStream *stream) {
	SmkAnims[_curSmackBuffer] = new NightlongSmackerDecoder();

	if (!SmkAnims[_curSmackBuffer]->loadStream(stream)) {
		warning("Invalid SMK file");
		CallSmackClose();
	} else {
		SmkAnims[_curSmackBuffer]->start();
		CallSmackNextFrame();
	}
}

/*-----------------18/01/97 21.05-------------------
					CallSmackClose
--------------------------------------------------*/
void CallSmackClose() {
	if (SmkAnims[_curSmackBuffer] == NULL)
		return;

	delete SmkAnims[_curSmackBuffer];
	SmkAnims[_curSmackBuffer] = NULL;
}

/*-----------------18/01/97 21.05-------------------
				CallSmackNextFrame
--------------------------------------------------*/
void CallSmackNextFrame() {
	if (SmkAnims[_curSmackBuffer] == NULL)
		return;

	if (SmkAnims[_curSmackBuffer]->needsUpdate()) {
		const Graphics::Surface *surface = SmkAnims[_curSmackBuffer]->decodeNextFrame();
		if (surface != NULL)
			SmackBuffer[_curSmackBuffer] = (uint8 *)surface->getPixels();
	}
}

/*-----------------18/01/97 21.05-------------------
				CallSmackVolumePan
--------------------------------------------------*/
void CallSmackVolumePan(int buf, int track, int vol) {
	_curSmackBuffer = buf;

	if (SmkAnims[_curSmackBuffer] == NULL)
		return;

	SmkAnims[_curSmackBuffer]->muteTrack(track, vol == 0);
}

/* -----------------23/06/98 13.33-------------------
 * 					CallSmackGoto
 * --------------------------------------------------*/
void CallSmackGoto(int buf, int num) {
	_curSmackBuffer = buf;

	if (SmkAnims[_curSmackBuffer] == NULL)
		return;

	SmkAnims[_curSmackBuffer]->seekToFrame(num);
}

/* -----------------07/08/98 14.47-------------------
 * 				CallSmackSounOnOff
 * --------------------------------------------------*/
void CallSmackSoundOnOff(int pos, int on) {
	if (SmkAnims[pos] == NULL)
		return;
	
	SmkAnims[pos]->setMute(on == 0);
}

/* -----------------12/06/97 17.09-------------------
					PalTo16bit
 --------------------------------------------------*/
uint16 PalTo16bit(uint8 r, uint8 g, uint8 b) {
	return (uint16)ScreenFormat.RGBToColor(r, g, b);
}

/* -----------------09/09/97 18.02-------------------
				RefreshAllAnimations
 --------------------------------------------------*/
void RefreshAllAnimations() {
	soundtimefunct();

	for (int a = 0; a < MAXSMACK; a++) {
		if (_playingAnims[a]) {
			_curSmackBuffer = a;
			CallSmackNextFrame();
		}
	}
}

/*-----------------17/11/96 14.50-------------------
					RegenSmackAnim
--------------------------------------------------*/
void RegenSmackAnim(int num) {
	int pos = 0;

	if ((num == 0) || (num == 620))
		return;

	if (AnimTab[num]._flag & SMKANIM_ICON) {
		RegenSmackIcon(g_vm->_regenInvStartIcon, num);
		return;
	}

	while ((pos < MAXSMACK) && (_playingAnims[pos] != num))
		pos++;

	if (pos >= MAXSMACK) {
		if (AnimTab[num]._flag & SMKANIM_BKG)
			pos = 0;
		else if (AnimTab[num]._flag & SMKANIM_ICON)
			pos = 2;
		else
			pos = 1;
	}

	_curSmackBuffer = pos;

	if (SmkAnims[_curSmackBuffer] == NULL)
		return;

	_curAnimFrame[pos] ++;

	if (SmkAnims[pos]->hasDirtyPalette()) {
		for (int32 a = 0; a < 256; a++)
			_smackPal[pos][a] = PalTo16bit(SmkAnims[pos]->getPalette()[a * 3 + 0], SmkAnims[pos]->getPalette()[a * 3 + 1], SmkAnims[pos]->getPalette()[a * 3 + 2]);
	}

	//SmackDoFrame( SmkAnims[pos] );
	//SmackToBuffer( SmkAnims[pos], 0, 0,
	//			   SmkAnims[pos]->getWidth(), SmkAnims[pos]->getHeight(), SmackBuffer[pos], 0 );

	//while( SmackToBufferRect( SmkAnims[pos], SMACKSURFACEFAST ) )
	{
		int inters = 0;
		for (int32 a = 0; a < MAXCHILD; a++) {
			if (AnimTab[num]._flag & (SMKANIM_OFF1 << a)) {
				// se il rettangolo e' completamente dentro a limite lo leva
				if ((AnimTab[num]._lim[a][0] <= 0 + SmkAnims[pos]->getWidth()) &&
						(AnimTab[num]._lim[a][1] <= 0 + SmkAnims[pos]->getHeight()) &&
						(AnimTab[num]._lim[a][2] >= 0) &&
						(AnimTab[num]._lim[a][3] >= 0)) {
					inters ++;
				}
			}
		}

		if ((_curAnimFrame[pos] > 0) && (inters == 0)) {
			if (pos == 0) {
				for (int32 a = 0; a < SmkAnims[pos]->getHeight(); a++) {
					/*byte2wordn( Video2+0+(0+a+TOP)*MAXX,
						SmackBuffer[pos]+0+(0+a)*SmkAnims[pos]->getWidth(),
						_smackPal[pos],SmkAnims[pos]->getWidth());

					AddLine(0,0+SmkAnims[pos]->getWidth(),0+a+TOP);

					wordcopy( ImagePointer+0+(0+a)*MAXX,
							  Video2+0+(0+a+TOP)*MAXX,
							  SmkAnims[pos]->getWidth());*/
				}
			} else if (_curAnimFrame[pos] > 1) {
				_animMinX = (_animMinX > 0) ? 0 : _animMinX;
				_animMinY = (_animMinY > 0) ? 0 : _animMinY;

				_animMaxX = (_animMaxX < 0 + SmkAnims[pos]->getWidth()) ? 0 + SmkAnims[pos]->getWidth() : _animMaxX;
				_animMaxY = (_animMaxY < 0 + SmkAnims[pos]->getHeight()) ? 0 + SmkAnims[pos]->getHeight() : _animMaxY;
			}

		}
	}

	// se e' un background
	if (pos == 0) {
		for (int32 a = 0; a < MAXCHILD; a++) {
			if (!(AnimTab[num]._flag & (SMKANIM_OFF1 << a))  && (AnimTab[num]._lim[a][3] != 0)) {
				limiti[limitinum][0] = AnimTab[num]._lim[a][0];
				limiti[limitinum][1] = AnimTab[num]._lim[a][1] + TOP;
				limiti[limitinum][2] = AnimTab[num]._lim[a][2];
				limiti[limitinum][3] = AnimTab[num]._lim[a][3] + TOP;
				limitinum ++;
			}
		}
	}	// solo per l'omino
	else if (pos == 1) {
		if (_curAnimFrame[pos] == 1) {
			for (int32 b = 0; b < AREA; b++) {
				for (int32 a = 0; a < MAXX; a++) {
					if (SmackBuffer[pos][b * MAXX + a]) {
						_animMinX = (_animMinX > a) ? a : _animMinX;
						_animMinY = (_animMinY > b) ? b : _animMinY;

						_animMaxX = (_animMaxX < a) ? a : _animMaxX;
						_animMaxY = (_animMaxY < b) ? b : _animMaxY;
					}
				}

				_animMaxX = (_animMaxX + 1 > MAXX) ? MAXX : _animMaxX + 1;
				_animMaxY = (_animMaxY + 1 > AREA) ? AREA : _animMaxY + 1;
			}
		}

		for (int32 a = 0; a < (_animMaxY - _animMinY); a++) {
			byte2wordm(Video2 + _animMinX + (_animMinY + a + TOP)*MAXX,
					   SmackBuffer[pos] + _animMinX + (_animMinY + a)*SmkAnims[pos]->getWidth(),
					   _smackPal[pos], _animMaxX - _animMinX);

			AddLine(_animMinX, _animMaxX, _animMinY + a + TOP);
		}

		limiti[limitinum][0] = _animMinX;
		limiti[limitinum][1] = _animMinY + TOP;
		limiti[limitinum][2] = _animMaxX;
		limiti[limitinum][3] = _animMaxY + TOP;

		Hlim = limitinum;
		limitinum ++;
	}

	if (!(AnimTab[num]._flag & SMKANIM_LOOP) && !(AnimTab[num]._flag & SMKANIM_BKG)) {
		if (_curAnimFrame[pos] >= SmkAnims[pos]->getFrameCount()) {
			StopSmackAnim(num);
			SemPaintCharacter = 1;

			_animMaxX = 0;
			_animMinX = MAXX;
			_animMaxY = 0;
			_animMinY = MAXY;
		} else
			CallSmackNextFrame();

	} else
		CallSmackNextFrame();

	if ((SmkAnims[pos] != NULL) && (_curAnimFrame[pos] >= SmkAnims[pos]->getFrameCount())) {
		if ((AnimTab[num]._flag & SMKANIM_LOOP) || (AnimTab[num]._flag & SMKANIM_BKG))
			InitAtFrameHandler(num, 0);

		_curAnimFrame[pos] = 0;
	}
}

/*-----------------22/11/96 11.23-------------------
			Aggiorna Icona Smacker
--------------------------------------------------*/
void RegenSmackIcon(int StartIcon, int num) {
	int pos = MAXSMACK - 1;
	_curAnimFrame[pos] ++;

	_curSmackBuffer = pos;

	if (SmkAnims[_curSmackBuffer] == NULL)
		return;

	int stx = ICONMARGSX;
	int32 a;
	for (a = 0; a < ICONSHOWN; a++) {
		if (g_vm->_inventory[a + StartIcon] == (num - aiBANCONOTE + 1)) {
			stx = a * ICONDX + ICONMARGSX + CurScrollPageDx;
			break;
		}
	}
	if (a == ICONSHOWN)
		return;

	if (SmkAnims[pos]->hasDirtyPalette()) {
		for (a = 0; a < 256; a++)
			_smackPal[pos][a] = PalTo16bit(SmkAnims[pos]->getPalette()[a * 3 + 0], SmkAnims[pos]->getPalette()[a * 3 + 1], SmkAnims[pos]->getPalette()[a * 3 + 2]);
	}

	//SmackDoFrame( SmkAnims[pos] );
	//SmackToBuffer( SmkAnims[pos], 0, 0,
	//			   SmkAnims[pos]->getWidth(), SmkAnims[pos]->getHeight(), SmackBuffer[pos], 0 );

	//while ( SmackToBufferRect( SmkAnims[pos], SMACKSURFACESLOW ) )
	{
		for (a = 0; a < ICONDY - 0; a++) {
			byte2word(Video2 + 0 + stx + (0 + a + FIRSTLINE)*SCREENLEN,
					  SmackBuffer[pos] + 0 + (0 + a)*SmkAnims[pos]->getWidth(),
					  _smackPal[pos], SmkAnims[pos]->getWidth());

			AddLine(0 + stx, 0 + SmkAnims[pos]->getWidth() + stx,
					0 + a + FIRSTLINE);
		}
	}

//	for( a=0; a<ICONDY; a++ )
//		VCopy( stx+(FIRSTLINE+a)*SCREENLEN, Video2+stx+(FIRSTLINE+a)*SCREENLEN, SmkAnims[pos]->getWidth() );
//	UnlockVideo();

	CallSmackNextFrame();
}

int FullStart;
int FullEnd;

/*-----------------07/02/97 16.53-------------------
					PlayFullMotion
--------------------------------------------------*/
void PlayFullMotion(int start, int end) {
	extern unsigned short _curDialog;

	int pos = 1;
	_curSmackBuffer = pos;

	if (start < 1)
		start = 1;
	if (end < 1)
		end = 1;

	if (start > SmkAnims[pos]->getFrameCount()) {
		start = SmkAnims[pos]->getFrameCount() - 1;
	}
	if (end > SmkAnims[pos]->getFrameCount()) {
		end = SmkAnims[pos]->getFrameCount();
	}

//	Se sono sue scelte attaccate
	if (_curAnimFrame[pos] != (start - 1)) {
		for (int a = 0; a < MAXNEWSMKPAL; a++) {
			if (((_dialog[_curDialog]._newPal[a] > start) || !(_dialog[_curDialog]._newPal[a])) && (a)) {
				SmkAnims[pos]->seekToFrame(_dialog[_curDialog]._newPal[a - 1]);
				for (a = 0; a < 256; a++) {
					_smackPal[pos][a] = PalTo16bit(SmkAnims[pos]->getPalette()[a * 3 + 0], SmkAnims[pos]->getPalette()[a * 3 + 1], SmkAnims[pos]->getPalette()[a * 3 + 2]);

					_newData[a] = _smackPal[pos][a];
					_newData2[a] = (uint32)((uint32)_smackPal[pos][a] + (((uint32)_smackPal[pos][a]) << 16));
				}
				break;
			} else if ((_dialog[_curDialog]._newPal[a] == 0) || (_dialog[_curDialog]._newPal[a] == start))
				break;
		}

		if ((end - start) > 2) {
			if (start > 10)
				SmkAnims[pos]->seekToFrame(start - 10);
			else
				SmkAnims[pos]->seekToFrame(1);

			CallSmackSoundOnOff(pos, 1);

			/*while ( SmkAnims[pos]->getCurFrame() < start - 1 )
			{
				SmackDoFrame( SmkAnims[pos] );
				SmackNextFrame( SmkAnims[pos] );
				while (SmackWait( SmkAnims[pos] ));
			}*/
		} else
			SmkAnims[pos]->seekToFrame(start);

		_curAnimFrame[pos] = start - 1 ;
	} else if ((end - start) > 2)
		CallSmackSoundOnOff(pos, 1);

	FullStart = start;
	FullEnd = end;

	sdt.clear();
	osdt.clear();
}

/*-----------------07/02/97 17.31-------------------
					BattutaPrint
--------------------------------------------------*/
void BattutaPrint(int x, int y, int c, const char *txt) {
	osdt.set(sdt);

	sdt.x = x;
	sdt.y = y;
	sdt.tcol = c;
	sdt.sign = txt;
}

/* -----------------29/07/97 22.09-------------------
					PaintSmackBuffer
 --------------------------------------------------*/
void PaintSmackBuffer(int px, int py, int dx, int dy) {

	int pos = 1;
	for (int a = 0; a < dy; a++) {
		if (SmkAnims[pos]->getHeight() > MAXY / 2) {
			if (SmkAnims[pos]->getWidth() > MAXX / 2)
				byte2word(Video2 + (a + py + TOP)*MAXX + px,
						  SmackBuffer[pos] + (a + py)*SmkAnims[pos]->getWidth() + px, _newData, dx);
			else
				byte2long(Video2 + (a + py + TOP)*MAXX + px,
						  SmackBuffer[pos] + (a + py)*SmkAnims[pos]->getWidth() + px / 2, _newData2, dx / 2);
		} else {
			if (SmkAnims[pos]->getWidth() > MAXX / 2)
				byte2word(Video2 + (a + py + TOP)*MAXX + px,
						  SmackBuffer[pos] + ((a + py) / 2)*SmkAnims[pos]->getWidth() + px, _newData, dx);
			else
				byte2long(Video2 + (a + py + TOP)*MAXX + px,
						  SmackBuffer[pos] + ((a + py) / 2)*SmkAnims[pos]->getWidth() + px / 2, _newData2, dx / 2);
		}
	}
}

/*-----------------22/11/96 11.23-------------------
			Aggiorna FullMotion
--------------------------------------------------*/
void RegenFullMotion() {
	int32 yfact;

	int pos = 1;
	_curSmackBuffer = pos;

	if (((_curAnimFrame[pos] + 1) >= FullStart) &&
			((_curAnimFrame[pos] + 1) <= FullEnd)) {
		_curAnimFrame[pos] ++;

		if (SmkAnims[pos]->hasDirtyPalette()) {
			for (int32 a = 0; a < 256; a++) {
				_smackPal[pos][a] = PalTo16bit(SmkAnims[pos]->getPalette()[a * 3 + 0], SmkAnims[pos]->getPalette()[a * 3 + 1], SmkAnims[pos]->getPalette()[a * 3 + 2]);

				_newData[a] = _smackPal[pos][a];
				_newData2[a] = (uint32)((uint32)_smackPal[pos][a] + (((uint32)_smackPal[pos][a]) << 16));
			}
		}

		DialogHandler(_curAnimFrame[pos]);

		sdt.dx = TextLength(sdt.sign, 0);

		sdt.x = 20;
		sdt.y = 380;
		sdt.dx = MAXX - 40;
		sdt.dy = sdt.checkDText();
		sdt.l[0] = 0;
		sdt.l[1] = 0;
		sdt.l[2] = MAXX;
		sdt.l[3] = MAXY;
		sdt.scol = MASKCOL;

		// se c'era una scritta la cancella
		if (osdt.sign != nullptr) {
			if ((osdt.y < sdt.y) || (osdt.y + osdt.dy > sdt.y + sdt.dy) || (sdt.sign == nullptr)) {
				PaintSmackBuffer(0, osdt.y - TOP, MAXX, osdt.dy);
				ShowScreen(0, osdt.y, MAXX,  osdt.dy);
			}
			osdt.sign = nullptr;
		}
		// se c'e' una scritta
		if (sdt.sign != nullptr) {
			PaintSmackBuffer(0, sdt.y - TOP, MAXX, sdt.dy);
			// scrive stringa
			if (ConfMan.getBool("subtitles"))
				sdt.DText();
			// e la fa vedere
			//ShowScreen( 0, sdt._y, MAXX,  sdt._dy );
			osdt.sign = nullptr;
		}

		if (SmkAnims[pos]->getHeight() > MAXY / 2)
			yfact = 1;
		else
			yfact = 2;

		//while ( SmackToBufferRect( SmkAnims[pos], SMACKSURFACESLOW ) )
		{
			for (int32 a = 0; a < SmkAnims[pos]->getHeight(); a++) {
				// se non ho gia' copiato la scritta
				if ((sdt.sign == nullptr) ||
						((0 + a)*yfact < (sdt.y - TOP)) ||
						((0 + a)*yfact >= (sdt.y + sdt.dy - TOP))) {
					// sceglie se raddoppiare o no
					// in altezza
					if (SmkAnims[pos]->getHeight() > MAXY / 2) {
						// in larghezza
						if (SmkAnims[pos]->getWidth() > MAXX / 2) {
							BCopy(0 + (0 + a)*MAXX + ((MAXY - SmkAnims[pos]->getHeight()) / 2)*MAXX,
								  SmackBuffer[pos] + 0 + (0 + a)*SmkAnims[pos]->getWidth(),
								  SmkAnims[pos]->getWidth());
						} else {
							DCopy(0 * 2 + (0 + a)*MAXX + ((MAXY - SmkAnims[pos]->getHeight()) / 2)*MAXX,
								  SmackBuffer[pos] + 0 + (0 + a)*SmkAnims[pos]->getWidth(),
								  SmkAnims[pos]->getWidth());
						}
					} else {
						if (SmkAnims[pos]->getWidth() > MAXX / 2) {
							BCopy(0 + ((0 + a) * 2)*MAXX + ((MAXY - SmkAnims[pos]->getHeight() * 2) / 2)*MAXX,
								  SmackBuffer[pos] + 0 + (0 + a)*SmkAnims[pos]->getWidth(),
								  SmkAnims[pos]->getWidth());
							BCopy(0 + ((0 + a) * 2 + 1)*MAXX + ((MAXY - SmkAnims[pos]->getHeight() * 2) / 2)*MAXX,
								  SmackBuffer[pos] + 0 + (0 + a)*SmkAnims[pos]->getWidth(),
								  SmkAnims[pos]->getWidth());
						} else {
							DCopy(0 * 2 + ((0 + a) * 2)*MAXX + ((MAXY - SmkAnims[pos]->getHeight() * 2) / 2)*MAXX,
								  SmackBuffer[pos] + 0 + (0 + a)*SmkAnims[pos]->getWidth(),
								  SmkAnims[pos]->getWidth());
							DCopy(0 * 2 + ((0 + a) * 2 + 1)*MAXX + ((MAXY - SmkAnims[pos]->getHeight() * 2) / 2)*MAXX,
								  SmackBuffer[pos] + 0 + (0 + a)*SmkAnims[pos]->getWidth(),
								  SmkAnims[pos]->getWidth());
						}
					}
				}
			}
		}
		if (sdt.sign != nullptr)
			ShowScreen(0, sdt.y, MAXX,  sdt.dy);
		UnlockVideo();

		if (_curAnimFrame[pos] == FullEnd) {
			PaintSmackBuffer(0, 0, MAXX, AREA);
			doEvent(MC_DIALOG, ME_ENDCHOICE, MP_HIGH, _curAnimFrame[pos], 0, 0, 0);
			CallSmackSoundOnOff(pos, 0);
		} else {
			CallSmackNextFrame();

			if (_curAnimFrame[pos] >= SmkAnims[pos]->getFrameCount())
				StopFullMotion();
		}
	}
}

} // End of namespace Trecision
