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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/config.h"
#include "cge/sound.h"
#include "cge/vmenu.h"
#include "cge/text.h"
#include "cge/cge_main.h"

namespace CGE {

/*
   51=wska§ typ posiadanej karty d¦wi‘kowej
   52=wybierz numer portu dla karty d¦wi‘kowej
   53=wybierz numer przerwania dla karty d¦wi‘kowej
   54=wybierz numer kana’u DMA dla karty d¦wi‘kowej
   55=wybierz numer portu dla General MIDI
   55=konfiguracja karty d¦wi‘kowej
*/
#define STYPE_TEXT  51
#define SPORT_TEXT  52
#define SIRQ_TEXT   53
#define SDMA_TEXT   54
#define MPORT_TEXT  55
#define MENU_TEXT   56

#define NONE_TEXT   60
#define SB_TEXT     61
#define SBM_TEXT    62
#define GUS_TEXT    63
#define GUSM_TEXT   64
#define MIDI_TEXT   65
#define AUTO_TEXT   66

#define DETECT      0xFFFF


static int DevName[] = {
	NONE_TEXT, SB_TEXT, SBM_TEXT, GUS_TEXT, GUSM_TEXT,
	MIDI_TEXT, AUTO_TEXT
};

static Choice DevMenu[] = {
	{ NULL, &CGEEngine::NONE  },
	{ NULL, &CGEEngine::SB    },
	{ NULL, &CGEEngine::SBM   },
	{ NULL, &CGEEngine::GUS   },
	{ NULL, &CGEEngine::GUSM  },
	{ NULL, &CGEEngine::MIDI  },
	{ NULL, &CGEEngine::AUTO  },
	{ NULL, NULL              }
};


static Choice DigiPorts[] = {
	{ " 210h", &CGEEngine::setPortD },
	{ " 220h", &CGEEngine::setPortD },
	{ " 230h", &CGEEngine::setPortD },
	{ " 240h", &CGEEngine::setPortD },
	{ " 250h", &CGEEngine::setPortD },
	{ " 260h", &CGEEngine::setPortD },
	{ "AUTO ", &CGEEngine::setPortD },
	{ NULL,   NULL     }
};

static Choice MIDIPorts[] = {
	{ " 220h", &CGEEngine::setPortM },
	{ " 230h", &CGEEngine::setPortM },
	{ " 240h", &CGEEngine::setPortM },
	{ " 250h", &CGEEngine::setPortM },
	{ " 300h", &CGEEngine::setPortM },
	{ " 320h", &CGEEngine::setPortM },
	{ " 330h", &CGEEngine::setPortM },
	{ " 340h", &CGEEngine::setPortM },
	{ " 350h", &CGEEngine::setPortM },
	{ " 360h", &CGEEngine::setPortM },
	{ "AUTO ", &CGEEngine::setPortM },
	{ NULL,   NULL     }
};

static Choice BlsterIRQ[] = {
	{ "IRQ  2", &CGEEngine::setIRQ },
	{ "IRQ  5", &CGEEngine::setIRQ },
	{ "IRQ  7", &CGEEngine::setIRQ },
	{ "IRQ 10", &CGEEngine::setIRQ },
	{ "AUTO  ", &CGEEngine::setIRQ },
	{ NULL,   NULL     }
};

static Choice GravisIRQ[] = {
	{ "IRQ  2", &CGEEngine::setIRQ },
	{ "IRQ  5", &CGEEngine::setIRQ },
	{ "IRQ  7", &CGEEngine::setIRQ },
	{ "IRQ 11", &CGEEngine::setIRQ },
	{ "IRQ 12", &CGEEngine::setIRQ },
	{ "IRQ 15", &CGEEngine::setIRQ },
	{ "AUTO  ", &CGEEngine::setIRQ },
	{ NULL,   NULL     }
};

static Choice GravisDMA[] = {
	{ "DMA 1", &CGEEngine::setDMA },
	{ "DMA 3", &CGEEngine::setDMA },
	{ "DMA 5", &CGEEngine::setDMA },
	{ "DMA 6", &CGEEngine::setDMA },
	{ "DMA 7", &CGEEngine::setDMA },
	{ "AUTO ", &CGEEngine::setDMA },
	{ NULL,   NULL    }
};

static Choice BlsterDMA[] = {
	{ "DMA 0", &CGEEngine::setDMA },
	{ "DMA 1", &CGEEngine::setDMA },
	{ "DMA 3", &CGEEngine::setDMA },
	{ "AUTO ", &CGEEngine::setDMA },
	{ NULL,   NULL    }
};


void CGEEngine::selectSound() {
	int i;
	_sound.close();
	if (Vmenu::_addr)
		SNPOST_(SNKILL, -1, 0, Vmenu::_addr);
	inf(_text->getText(STYPE_TEXT));
	_talk->gotoxy(_talk->_x, FONT_HIG / 2);
	for (i = 0; i < (int)ArrayCount(DevName); i++)
		DevMenu[i]._text = _text->getText(DevName[i]);
	(new Vmenu(this, DevMenu, SCR_WID / 2, _talk->_y + _talk->_h + TEXT_VM + FONT_HIG))->setName(_text->getText(MENU_TEXT));
}


static void reset() {
	_sndDrvInfo._dBase = _sndDrvInfo._dIrq = _sndDrvInfo._dDma = _sndDrvInfo._mBase = DETECT;
}


static uint16 deco(const char *str, uint16(*dco)(const char *)) {
	while (*str && ! IsDigit(*str))
	++str;
	if (*str)
		return dco(str);
	else
		return DETECT;
}


static uint16 ddeco(const char *str) {
	return deco(str, atow);
}


static uint16 xdeco(const char *str) {
	return deco(str, xtow);
}


static Choice *_cho;
static int     _hlp;

void CGEEngine::snSelect() {
	inf(_text->getText(_hlp));
	_talk->gotoxy(_talk->_x, FONT_HIG / 2);
	(new Vmenu(this, _cho, SCR_WID / 2, _talk->_y + _talk->_h + TEXT_VM + FONT_HIG))->setName(_text->getText(MENU_TEXT));
}


static void select(Choice *cho, int hlp) {
	_cho = cho;
	_hlp = hlp;
	SNPOST2(SNEXEC, -1, 0, SNSELECT);
}


void CGEEngine::NONE() {
	_sndDrvInfo._dDev = DEV_QUIET;
	_sndDrvInfo._mDev = DEV_QUIET;
	_sound.open();
}


void CGEEngine::SB() {
	_sndDrvInfo._dDev = DEV_SB;
	_sndDrvInfo._mDev = DEV_SB;
	reset();
	select(DigiPorts, SPORT_TEXT);
}


void CGEEngine::SBM() {
	_sndDrvInfo._dDev = DEV_SB;
	_sndDrvInfo._mDev = DEV_GM;
	reset();
	select(DigiPorts, SPORT_TEXT);
}


void CGEEngine::GUS() {
	_sndDrvInfo._dDev = DEV_GUS;
	_sndDrvInfo._mDev = DEV_GUS;
	reset();
	select(DigiPorts, SPORT_TEXT);
}


void CGEEngine::GUSM() {
	_sndDrvInfo._dDev = DEV_GUS;
	_sndDrvInfo._mDev = DEV_GM;
	reset();
	select(DigiPorts, SPORT_TEXT);
}


void CGEEngine::MIDI() {
	_sndDrvInfo._dDev = DEV_QUIET;
	_sndDrvInfo._mDev = DEV_GM;
	_sndDrvInfo._mBase = DETECT;
	select(MIDIPorts, MPORT_TEXT);
}


void CGEEngine::AUTO() {
	_sndDrvInfo._dDev = DEV_AUTO;
	_sndDrvInfo._mDev = DEV_AUTO;
	reset();
	_sound.open();
}


void CGEEngine::setPortD() {
	_sndDrvInfo._dBase = xdeco(DigiPorts[Vmenu::_recent]._text);
	select((_sndDrvInfo._dDev == DEV_SB) ? BlsterIRQ : GravisIRQ, SIRQ_TEXT);
}


void CGEEngine::setPortM() {
	_sndDrvInfo._mBase = xdeco(MIDIPorts[Vmenu::_recent]._text);
	_sound.open();
}


void CGEEngine::setIRQ() {
	_sndDrvInfo._dIrq = ddeco(((_sndDrvInfo._dDev == DEV_SB) ? BlsterIRQ : GravisIRQ)[Vmenu::_recent]._text);
	select((_sndDrvInfo._dDev == DEV_SB) ? BlsterDMA : GravisDMA, SDMA_TEXT);
}


void CGEEngine::setDMA() {
	_sndDrvInfo._dDma = ddeco(((_sndDrvInfo._dDev == DEV_SB) ? BlsterDMA : GravisDMA)[Vmenu::_recent]._text);
	if (_sndDrvInfo._mDev != _sndDrvInfo._dDev)
		select(MIDIPorts, MPORT_TEXT);
	else
		_sound.open();
}

} // End of namespace CGE
