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

static CHOICE DevMenu[] = {
	{ NULL, &CGEEngine::NONE  },
	{ NULL, &CGEEngine::SB    },
	{ NULL, &CGEEngine::SBM   },
	{ NULL, &CGEEngine::GUS   },
	{ NULL, &CGEEngine::GUSM  },
	{ NULL, &CGEEngine::MIDI  },
	{ NULL, &CGEEngine::AUTO  },
	{ NULL, NULL              }
};


static CHOICE DigiPorts[] = {
	{ " 210h", &CGEEngine::setPortD },
	{ " 220h", &CGEEngine::setPortD },
	{ " 230h", &CGEEngine::setPortD },
	{ " 240h", &CGEEngine::setPortD },
	{ " 250h", &CGEEngine::setPortD },
	{ " 260h", &CGEEngine::setPortD },
	{ "AUTO ", &CGEEngine::setPortD },
	{ NULL,   NULL     }
};

static CHOICE MIDIPorts[] = {
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

static CHOICE BlsterIRQ[] = {
	{ "IRQ  2", &CGEEngine::setIRQ },
	{ "IRQ  5", &CGEEngine::setIRQ },
	{ "IRQ  7", &CGEEngine::setIRQ },
	{ "IRQ 10", &CGEEngine::setIRQ },
	{ "AUTO  ", &CGEEngine::setIRQ },
	{ NULL,   NULL     }
};

static CHOICE GravisIRQ[] = {
	{ "IRQ  2", &CGEEngine::setIRQ },
	{ "IRQ  5", &CGEEngine::setIRQ },
	{ "IRQ  7", &CGEEngine::setIRQ },
	{ "IRQ 11", &CGEEngine::setIRQ },
	{ "IRQ 12", &CGEEngine::setIRQ },
	{ "IRQ 15", &CGEEngine::setIRQ },
	{ "AUTO  ", &CGEEngine::setIRQ },
	{ NULL,   NULL     }
};

static CHOICE GravisDMA[] = {
	{ "DMA 1", &CGEEngine::setDMA },
	{ "DMA 3", &CGEEngine::setDMA },
	{ "DMA 5", &CGEEngine::setDMA },
	{ "DMA 6", &CGEEngine::setDMA },
	{ "DMA 7", &CGEEngine::setDMA },
	{ "AUTO ", &CGEEngine::setDMA },
	{ NULL,   NULL    }
};

static CHOICE BlsterDMA[] = {
	{ "DMA 0", &CGEEngine::setDMA },
	{ "DMA 1", &CGEEngine::setDMA },
	{ "DMA 3", &CGEEngine::setDMA },
	{ "AUTO ", &CGEEngine::setDMA },
	{ NULL,   NULL    }
};


void CGEEngine::selectSound() {
	int i;
	_sound.Close();
	if (VMENU::Addr)
		SNPOST_(SNKILL, -1, 0, VMENU::Addr);
	inf(Text->getText(STYPE_TEXT));
	Talk->gotoxy(Talk->_x, FONT_HIG / 2);
	for (i = 0; i < (int)ArrayCount(DevName); i++)
		DevMenu[i].Text = Text->getText(DevName[i]);
	(new VMENU(this, DevMenu, SCR_WID / 2, Talk->_y + Talk->_h + TEXT_VM + FONT_HIG))->setName(Text->getText(MENU_TEXT));
}


static void Reset(void) {
	SNDDrvInfo.DBASE = SNDDrvInfo.DIRQ = SNDDrvInfo.DDMA = SNDDrvInfo.MBASE = DETECT;
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


static CHOICE *Cho;
static int     Hlp;

void CGEEngine::SNSelect() {
	inf(Text->getText(Hlp));
	Talk->gotoxy(Talk->_x, FONT_HIG / 2);
	(new VMENU(this, Cho, SCR_WID / 2, Talk->_y + Talk->_h + TEXT_VM + FONT_HIG))->setName(Text->getText(MENU_TEXT));
}


static void Select(CHOICE *cho, int hlp) {
	Cho = cho;
	Hlp = hlp;
	//TODO Change the SNPOST message send to a special way to send function pointer
	//SNPOST(SNEXEC, -1, 0, (void *)&SNSelect);
	warning("STUB: Select");
}


void CGEEngine::NONE() {
	SNDDrvInfo.DDEV = DEV_QUIET;
	SNDDrvInfo.MDEV = DEV_QUIET;
	_sound.Open();
}


void CGEEngine::SB() {
	SNDDrvInfo.DDEV = DEV_SB;
	SNDDrvInfo.MDEV = DEV_SB;
	Reset();
	Select(DigiPorts, SPORT_TEXT);
}


void CGEEngine::SBM() {
	SNDDrvInfo.DDEV = DEV_SB;
	SNDDrvInfo.MDEV = DEV_GM;
	Reset();
	Select(DigiPorts, SPORT_TEXT);
}


void CGEEngine::GUS() {
	SNDDrvInfo.DDEV = DEV_GUS;
	SNDDrvInfo.MDEV = DEV_GUS;
	Reset();
	Select(DigiPorts, SPORT_TEXT);
}


void CGEEngine::GUSM() {
	SNDDrvInfo.DDEV = DEV_GUS;
	SNDDrvInfo.MDEV = DEV_GM;
	Reset();
	Select(DigiPorts, SPORT_TEXT);
}


void CGEEngine::MIDI() {
	SNDDrvInfo.DDEV = DEV_QUIET;
	SNDDrvInfo.MDEV = DEV_GM;
	SNDDrvInfo.MBASE = DETECT;
	Select(MIDIPorts, MPORT_TEXT);
}


void CGEEngine::AUTO() {
	SNDDrvInfo.DDEV = DEV_AUTO;
	SNDDrvInfo.MDEV = DEV_AUTO;
	Reset();
	_sound.Open();
}


void CGEEngine::setPortD() {
	SNDDrvInfo.DBASE = xdeco(DigiPorts[VMENU::Recent].Text);
	Select((SNDDrvInfo.DDEV == DEV_SB) ? BlsterIRQ : GravisIRQ, SIRQ_TEXT);
}


void CGEEngine::setPortM() {
	SNDDrvInfo.MBASE = xdeco(MIDIPorts[VMENU::Recent].Text);
	_sound.Open();
}


void CGEEngine::setIRQ() {
	SNDDrvInfo.DIRQ = ddeco(((SNDDrvInfo.DDEV == DEV_SB) ? BlsterIRQ : GravisIRQ)[VMENU::Recent].Text);
	Select((SNDDrvInfo.DDEV == DEV_SB) ? BlsterDMA : GravisDMA, SDMA_TEXT);
}


void CGEEngine::setDMA() {
	SNDDrvInfo.DDMA = ddeco(((SNDDrvInfo.DDEV == DEV_SB) ? BlsterDMA : GravisDMA)[VMENU::Recent].Text);
	if (SNDDrvInfo.MDEV != SNDDrvInfo.DDEV)
		Select(MIDIPorts, MPORT_TEXT);
	else
		_sound.Open();
}

} // End of namespace CGE
