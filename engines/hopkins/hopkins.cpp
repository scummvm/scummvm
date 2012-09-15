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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/files.h"
#include "hopkins/sound.h"

namespace Hopkins {

HopkinsEngine *g_vm;

HopkinsEngine::HopkinsEngine(OSystem *syst, const HopkinsGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _randomSource("Hopkins"), _animationManager() {
	g_vm = this;
	_animationManager.setParent(this);
	_soundManager.setParent(this);
}

HopkinsEngine::~HopkinsEngine() {
}

Common::Error HopkinsEngine::run() {
	FileManager::initSaves();

	Common::StringMap iniParams;
	FileManager::Chage_Inifile(iniParams);
	processIniParams(iniParams);

	GLOBALS.setConfig();
	FileManager::F_Censure();
	INIT_SYSTEM();

	_soundManager.WSOUND_INIT();

	GLOBALS.CHARGE_OBJET();
	ObjectManager::CHANGE_OBJET(14);
	ObjectManager::AJOUTE_OBJET(14);

	GLOBALS.HELICO = 0;
	_eventsManager.hideCursor();

	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();

	_graphicsManager.LOAD_IMAGE("LINUX");

	_graphicsManager.FADE_INW();
	_eventsManager.delay(1500);
	_graphicsManager.FADE_OUTW();

	if (!GLOBALS.internet) {
		_graphicsManager.FADE_LINUX = 2;
		_animationManager.PLAY_ANM("MP.ANM", 10, 16, 200);
	}

	_graphicsManager.LOAD_IMAGE("H2");
	_graphicsManager.FADE_INW();
	_eventsManager.delay(500);
	_graphicsManager.FADE_OUTW();

	if (!_eventsManager.ESC_KEY)
		INTRORUN();
  /*
  _globals.iRegul = 0;
  CONSTRUIT_SYSTEM("PERSO.SPR");
  PERSO = CHARGE_FICHIER(GLOBALS.NFICHIER);
  PERSO_TYPE = 0;
  PLANX = 0;
  PLANY = 0;
  memset(SAUVEGARDE, 0, 0x7CFu);
  SORTIE = 0;
  PASSWORD = 1;
LABEL_12:
  if ( SORTIE == 300 )
LABEL_13:
    SORTIE = 0;
  if ( !SORTIE )
  {
    SORTIE = MENU();
    if ( SORTIE == -1 )
    {
      PUBQUIT();
      PERSO = (void *)dos_free2(PERSO);
      REST_SYSTEM();
    }
  }
  while ( 1 )
  {
    while ( 1 )
    {
      while ( 1 )
      {
        while ( 1 )
        {
          while ( 1 )
          {
            while ( 1 )
            {
              while ( 1 )
              {
                while ( 1 )
                {
                  if ( SORTIE == 300 )
                    goto LABEL_13;
                  if ( SORTIE == 18 )
                    PASS();
                  if ( SORTIE == 23 )
                    PASS();
                  if ( SORTIE == 22 )
                    PASS();
                  if ( SORTIE == 19 )
                    PASS();
                  if ( SORTIE == 20 )
                    PASS();
                  if ( SORTIE != 1 )
                    break;
                  Max_Propre = 50;
                  Max_Ligne_Long = 40;
                  Max_Propre_Gen = 20;
                  Max_Perso_Y = 435;
                  PERSONAGE2((int)"IM01", (int)"IM01", (int)"ANIM01", (int)"IM01", 1);
                }
                if ( SORTIE != 3 )
                  break;
                if ( !*((_BYTE *)SAUVEGARDE + 170) )
                {
                  _soundManager.WSOUND(3);
                  if ( FR == 1 )
                    LOAD_IMAGE("fondfr");
                  if ( !FR )
                    LOAD_IMAGE("fondan");
                  if ( FR == 2 )
                    LOAD_IMAGE("fondes");
                  FADE_INW();
                  SDL_Delay(500);
                  FADE_OUTW();
                  _globals.iRegul = 1;
                  _soundManager.SPECIAL_SOUND = 2;
                  DD_Lock();
                  Cls_Video();
                  DD_Unlock();
                  Cls_Pal();
                  FADE_LINUX = 2;
                  if ( !CENSURE )
                    PLAY_ANM("BANQUE.ANM", 200, 28, 200);
                  if ( CENSURE == 1 )
                    PLAY_ANM("BANKUK.ANM", 200, 28, 200);
                  _soundManager.SPECIAL_SOUND = 0;
                  DEL_SAMPLE(1);
                  DEL_SAMPLE(2);
                  DEL_SAMPLE(3);
                  DEL_SAMPLE(4);
                  *((_BYTE *)SAUVEGARDE + 170) = 1;
                }
                Max_Propre = 5;
                Max_Ligne_Long = 5;
                Max_Propre_Gen = 5;
                Max_Perso_Y = 450;
                NOSPRECRAN = 1;
                PERSONAGE2((int)"IM03", (int)"IM03", (int)"ANIM03", (int)"IM03", 2);
              }
              if ( SORTIE != 4 )
                break;
              DESACTIVE_INVENT = 1;
              PLAN_BETA(a1);
              DESACTIVE_INVENT = 0;
            }
            if ( SORTIE != 5 )
              break;
            Max_Propre = 5;
            Max_Ligne_Long = 5;
            Max_Propre_Gen = 5;
            Max_Perso_Y = 455;
            NOSPRECRAN = 1;
            v1 = *((_BYTE *)SAUVEGARDE + 80);
            if ( v1 )
            {
              if ( v1 == 1 )
                PERSONAGE2((int)"IM05", (int)"IM05A", (int)"ANIM05B", (int)"IM05", 3);
            }
            else
            {
              PERSONAGE2((int)"IM05", (int)"IM05", (int)"ANIM05", (int)"IM05", 3);
            }
            NOSPRECRAN = 0;
          }
          if ( SORTIE != 8 )
            break;
          Max_Propre = 15;
          Max_Ligne_Long = 15;
          Max_Propre_Gen = 10;
          Max_Perso_Y = 450;
          PERSONAGE2((int)"IM08", (int)"IM08", (int)"ANIM08", (int)"IM08", 2);
        }
        if ( SORTIE != 6 )
          break;
        Max_Propre = 15;
        Max_Ligne_Long = 20;
        Max_Propre_Gen = 10;
        Max_Perso_Y = 460;
        PERSONAGE2((int)"IM06", (int)"IM06", (int)"ANIM06", (int)"IM06", 2);
      }
      if ( SORTIE != 7 )
        break;
      if ( *((_BYTE *)SAUVEGARDE + 220) )
        PERSONAGE((int)"BOMBEB", (int)"BOMBE", (int)"BOMBE", (int)"BOMBE", 2);
      else
        PERSONAGE((int)"BOMBEA", (int)"BOMBE", (int)"BOMBE", (int)"BOMBE", 2);
    }
    if ( SORTIE == 9 )
    {
      Max_Propre = 15;
      Max_Ligne_Long = 20;
      Max_Propre_Gen = 10;
      Max_Perso_Y = 440;
      if ( !*((_BYTE *)SAUVEGARDE + 225) )
        goto LABEL_109;
      PERSONAGE2((int)"IM09", (int)"IM09", (int)"ANIM09", (int)"IM09", 10);
    }
    else
    {
      if ( SORTIE == 10 )
      {
        NOSPRECRAN = 1;
        PERSONAGE((int)"IM10", (int)"IM10", (int)"ANIM10", (int)"IM10", 9);
        goto LABEL_124;
      }
      if ( SORTIE == 11 )
      {
        NOSPRECRAN = 1;
        Max_Propre = 15;
        Max_Ligne_Long = 20;
        Max_Propre_Gen = 10;
        Max_Perso_Y = 450;
        PERSONAGE2((int)"IM11", (int)"IM11", (int)"ANIM11", (int)"IM11", 2);
        goto LABEL_124;
      }
      switch ( SORTIE )
      {
        case 12:
          Max_Propre = 15;
          Max_Ligne_Long = 20;
          Max_Propre_Gen = 10;
          Max_Perso_Y = 450;
          if ( *((_BYTE *)SAUVEGARDE + 225) )
          {
            NOSPRECRAN = 1;
            PERSONAGE2((int)"IM12", (int)"IM12", (int)"ANIM12", (int)"IM12", 1);
          }
          else
          {
LABEL_109:
            BOOM(a1);
          }
          break;
        case 13:
          Max_Propre = 50;
          Max_Ligne_Long = 40;
          Max_Propre_Gen = 20;
          Max_Perso_Y = 440;
          PERSONAGE2((int)"IM13", (int)"IM13", (int)"ANIM13", (int)"IM13", 1);
          break;
        case 14:
          Max_Propre = 50;
          Max_Ligne_Long = 40;
          Max_Propre_Gen = 20;
          Max_Perso_Y = 440;
          PERSONAGE2((int)"IM14", (int)"IM14", (int)"ANIM14", (int)"IM14", 1);
          break;
        default:
          if ( SORTIE == 15 )
          {
            NOSPRECRAN = 1;
            PERSONAGE((int)"IM15", (int)"IM15", (int)"ANIM15", (int)"IM15", 29);
            goto LABEL_124;
          }
          if ( SORTIE == 16 )
          {
            Max_Propre = 5;
            Max_Ligne_Long = 5;
            Max_Propre_Gen = 5;
            Max_Perso_Y = 450;
            v2 = *((_BYTE *)SAUVEGARDE + 113);
            if ( v2 == 1 )
            {
              PERSONAGE2((int)"IM16", (int)"IM16A", (int)"ANIM16", (int)"IM16", 7);
            }
            else if ( !v2 )
            {
              PERSONAGE2((int)"IM16", (int)"IM16", (int)"ANIM16", (int)"IM16", 7);
            }
          }
          else
          {
            if ( SORTIE == 17 )
              PASS();
            if ( SORTIE == 24 )
              PASS();
            if ( SORTIE == 25 )
            {
              Max_Propre = 15;
              Max_Ligne_Long = 20;
              Max_Propre_Gen = 10;
              Max_Perso_Y = 445;
              PERSONAGE2((int)"IM25", (int)"IM25", (int)"ANIM25", (int)"IM25", 30);
            }
            else
            {
              if ( SORTIE == 33 )
              {
                NOSPRECRAN = 1;
                PERSONAGE((int)"IM33", (int)"IM33", (int)"ANIM33", (int)"IM33", 8);
                goto LABEL_124;
              }
              if ( SORTIE == 26 )
              {
                Max_Propre = 50;
                Max_Ligne_Long = 40;
                Max_Propre_Gen = 20;
                Max_Perso_Y = 435;
                PERSONAGE2((int)"IM26", (int)"IM26", (int)"ANIM26", (int)"IM26", 30);
              }
              else
              {
                if ( SORTIE == 27 )
                  PASS();
                if ( SORTIE == 28 )
                  PASS();
                if ( SORTIE == 29 )
                  PASS();
                if ( SORTIE == 30 )
                  PASS();
                if ( SORTIE == 31 )
                  PASS();
                if ( SORTIE == 35 )
                  ENDEMO();
                if ( SORTIE == 32 )
                  PASS();
                if ( SORTIE == 34 )
                  PASS();
                if ( (unsigned __int16)(SORTIE - 51) <= 0x26u )
                  PASS();
                if ( SORTIE == 111 )
                {
                  NOSPRECRAN = 1;
                  PERSONAGE((int)"IM111", (int)"IM111", (int)"ANIM111", (int)"IM111", 10);
                  goto LABEL_124;
                }
                if ( SORTIE == 112 )
                {
                  NOSPRECRAN = 1;
                  PERSONAGE((int)"IM112", (int)"IM112", (int)"ANIM112", (int)"IM112", 10);
LABEL_124:
                  NOSPRECRAN = 0;
                }
                else if ( SORTIE == 113 )
                {
                  SORTIE = 0;
                  OLD_ECRAN = ECRAN;
                  *((_BYTE *)SAUVEGARDE + 6) = ECRAN;
                  ECRAN = 113;
                  *((_BYTE *)SAUVEGARDE + 5) = 113;
                  COMPUT_HOPKINS(a1, 1);
                  DD_LOCK();
                  Cls_Video();
                  DD_UNLOCK();
                  DD_VBL();
                  memset(VESA_BUFFER, 0, 0x4B000u);
                  memset(VESA_SCREEN, 0, 0x4B000u);
                  Cls_Pal();
                  RESET_SEGMENT_VESA();
                }
                else
                {
                  if ( SORTIE == 114 )
                  {
                    SORTIE = 0;
                    OLD_ECRAN = ECRAN;
                    *((_BYTE *)SAUVEGARDE + 6) = ECRAN;
                    ECRAN = 114;
                    *((_BYTE *)SAUVEGARDE + 5) = 114;
                    COMPUT_HOPKINS(a1, 2);
                    goto LABEL_128;
                  }
                  if ( SORTIE == 115 )
                  {
                    SORTIE = 0;
                    OLD_ECRAN = ECRAN;
                    *((_BYTE *)SAUVEGARDE + 6) = ECRAN;
                    ECRAN = 115;
                    *((_BYTE *)SAUVEGARDE + 5) = 115;
                    COMPUT_HOPKINS(a1, 3);
LABEL_128:
                    DD_LOCK();
                    Cls_Video();
                    DD_UNLOCK();
                  }
                  else if ( (unsigned __int16)(SORTIE - 194) > 5u )
                  {
                    if ( SORTIE == 151 )
                    {
                      _soundManager.WSOUND(16);
                      _globals.iRegul = 1;
                      DD_Lock();
                      Cls_Video();
                      DD_Unlock();
                      Cls_Pal();
                      FADE_LINUX = 2;
                      PLAY_ANM("JOUR3A.anm", 12, 12, 2000);
                      _globals.iRegul = 0;
                      SORTIE = 300;
                    }
                    if ( SORTIE == 150 )
                    {
                      _soundManager.WSOUND(16);
                      _globals.iRegul = 1;
                      DD_Lock();
                      Cls_Video();
                      DD_Unlock();
                      Cls_Pal();
                      FADE_LINUX = 2;
                      PLAY_ANM("JOUR1A.anm", 12, 12, 2000);
                      _globals.iRegul = 0;
                      SORTIE = 300;
                    }
                    if ( SORTIE == 152 )
                    {
                      _soundManager.WSOUND(16);
                      _globals.iRegul = 1;
                      DD_Lock();
                      Cls_Video();
                      DD_Unlock();
                      Cls_Pal();
                      FADE_LINUX = 2;
                      PLAY_ANM("JOUR4A.anm", 12, 12, 2000);
                      _globals.iRegul = 0;
                      SORTIE = 300;
                    }
                    goto LABEL_12;
                  }
                }
              }
            }
          }
          break;
      }
    }
  }
*/
#if 0
	// Copy vesa surface to screen
	_graphicsManager.DD_Lock();

	const byte *srcP = (const byte *)_graphicsManager.VESA_SCREEN;
	uint16 *destP = (uint16 *)_graphicsManager.VideoPtr->pixels;
	for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); ++i, ++srcP, ++destP) {
		byte r = _graphicsManager.Palette[*srcP * 3];
		byte g = _graphicsManager.Palette[*srcP * 3 + 1];
		byte b = _graphicsManager.Palette[*srcP * 3 + 2];

		*destP = (b >> 2) | ((g >> 2) << 5) | ((r >> 2) << 10);
	}

	_graphicsManager.DD_Unlock();
#endif

	// Stub event loop
	Common::Event evt;
	while (!g_system->getEventManager()->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(evt))
			g_system->updateScreen();
	}

	return Common::kNoError;
}

int HopkinsEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

void HopkinsEngine::processIniParams(Common::StringMap &iniParams) {
	GLOBALS.XFULLSCREEN = iniParams["FULLSCREEN"] == "YES";

	GLOBALS.XSETMODE = 1;
	if (iniParams.contains("SETMODE")) {
		int setMode = atoi(iniParams["SETMODE"].c_str());
		GLOBALS.XSETMODE = CLIP(setMode, 1, 5);
	}

	GLOBALS.XZOOM = 0;
	if (GLOBALS.XSETMODE == 5 && iniParams.contains("ZOOM")) {
		int zoom = atoi(iniParams["ZOOM"].c_str());
		GLOBALS.XZOOM = CLIP(zoom, 25, 100);
	}

	GLOBALS.XFORCE16 = iniParams["FORCE16BITS"] == "YES";
	GLOBALS.XFORCE8 = iniParams["FORCE8BITS"] == "YES";
	GLOBALS.CARD_SB = iniParams["SOUND"] == "YES";
}

void HopkinsEngine::INIT_SYSTEM() {
	// Set graphics mode
	_graphicsManager.SET_MODE(640, 480);
	
	_eventsManager.mouse_linux = true;
  
	switch (GLOBALS.FR) {
	case 0:
		if (!_eventsManager.mouse_linux)
			FileManager::CONSTRUIT_SYSTEM("SOUAN.SPR");
		if (!GLOBALS.FR && _eventsManager.mouse_linux)
			FileManager::CONSTRUIT_SYSTEM("LSOUAN.SPR");
		break;
	case 1:
		FileManager::CONSTRUIT_SYSTEM("LSOUFR.SPR");
		break;
	case 2:
		FileManager::CONSTRUIT_SYSTEM("SOUES.SPR");
		break;
	}
  
	if (_eventsManager.mouse_linux) {
		_eventsManager.souris_sizex = 52;
		_eventsManager.souris_sizey = 32;
	} else {
		_eventsManager.souris_sizex = 34;
		_eventsManager.souris_sizey = 20;
	}
	_eventsManager.pointeur_souris = FileManager::CHARGE_FICHIER(GLOBALS.NFICHIER);

	GLOBALS.clearAll();

	FileManager::CONSTRUIT_SYSTEM("FONTE3.SPR");
	GLOBALS.police = FileManager::CHARGE_FICHIER(GLOBALS.NFICHIER);
	GLOBALS.police_l = 12;
	GLOBALS.police_h = 21;
	FileManager::CONSTRUIT_SYSTEM("ICONE.SPR");
	GLOBALS.ICONE = FileManager::CHARGE_FICHIER(GLOBALS.NFICHIER);
	FileManager::CONSTRUIT_SYSTEM("TETE.SPR");
	GLOBALS.TETE = FileManager::CHARGE_FICHIER(GLOBALS.NFICHIER);
	
	switch (GLOBALS.FR) {
	case 0:
		FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "ZONEAN.TXT");
		GLOBALS.BUF_ZONE = FileManager::CHARGE_FICHIER(GLOBALS.NFICHIER);
		break;
	case 1:
		FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "ZONE01.TXT");
		GLOBALS.BUF_ZONE = FileManager::CHARGE_FICHIER(GLOBALS.NFICHIER);
		break;
	case 2:
		FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "ZONEES.TXT");
		GLOBALS.BUF_ZONE = FileManager::CHARGE_FICHIER(GLOBALS.NFICHIER);
		break;
	}

	GLOBALS.min_x = 0;
	GLOBALS.min_y = 20;
	GLOBALS.max_x = 1280;
	GLOBALS.max_y = 460;
	
	_eventsManager.INSTALL_SOURIS();
	_eventsManager.souris_on();
	_eventsManager.souris_flag = false;
	_eventsManager.souris_max();

	GLOBALS.HOPKINS_DATA();

	_eventsManager.ofset_souris_x = 0;
	_eventsManager.ofset_souris_y = 0;
	GLOBALS.lItCounter = 0;
	GLOBALS.lOldItCounter = 0;
}

void HopkinsEngine::INTRORUN() {
	signed int v2;
	signed int v3;
	unsigned __int16 v4;
	signed int v5;
	int i; 
	int v7; 
	signed int v8;
	signed int v9;
	signed int v11;
	int v12;
	signed int v13;
	signed int v14;
	int j;
	int k;
	int l;
	int m; 
	__int16 v21;
	char v22; 
	char v23; 
	byte paletteData[800];
	byte paletteData2[800];

	memset(&paletteData, 0, 800);
	_eventsManager.VBL();
	_eventsManager.souris_flag = 0;
	_globals.iRegul = 1;
	_eventsManager.VBL();
	_soundManager.WSOUND(16);
	_animationManager.CLS_ANM = 1;
	_animationManager.PLAY_ANM("J1.anm", 12, 12, 50);
	if (!_eventsManager.ESC_KEY) {
		_soundManager.VOICE_MIX(1, 3);
		_animationManager.PLAY_ANM("J2.anm", 12, 12, 50);

		if (!_eventsManager.ESC_KEY) {
			_soundManager.VOICE_MIX(2, 3);
			_animationManager.PLAY_ANM("J3.anm", 12, 12, 50);

			if (!_eventsManager.ESC_KEY) {
				_soundManager.VOICE_MIX(3, 3);
				_graphicsManager.DD_LOCK();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_UNLOCK();
				_graphicsManager.Cls_Pal();
				_graphicsManager.DD_VBL();
				_soundManager.WSOUND(11);
				_graphicsManager.LOAD_IMAGE("intro1");
				_graphicsManager.SCROLL_ECRAN(0);
				_graphicsManager.ofscroll = 0;
				_graphicsManager.SETCOLOR3(252, 100, 100, 100);
				_graphicsManager.SETCOLOR3(253, 100, 100, 100);
				_graphicsManager.SETCOLOR3(251, 100, 100, 100);
				_graphicsManager.SETCOLOR3(254, 0, 0, 0);
				_globals.BPP_NOAFF = 1;
				v2 = 0;
				do {
					_eventsManager.VBL();
					++v2;
				} while (v2 <= 4);

				_globals.BPP_NOAFF = 0;
				_globals.iRegul = 1;
				_graphicsManager.FADE_INW();
				if (_graphicsManager.DOUBLE_ECRAN == 1) {
					_graphicsManager.no_scroll = 2;
					v3 = 0;
					_graphicsManager.SCROLL = 0;
          
					do {
						_graphicsManager.SCROLL += 2;
						if (_graphicsManager.SCROLL > (SCREEN_WIDTH - 2)) {
							_graphicsManager.SCROLL = SCREEN_WIDTH;
							v3 = 1;
						}
            
						if (_eventsManager.XMOUSE() < _graphicsManager.SCROLL + 10) {
							v4 = _eventsManager.YMOUSE();
							_eventsManager.souris_xy(_eventsManager.souris_x + 4, v4);
						}
						_eventsManager.VBL();
					} while (v3 != 1 && _graphicsManager.SCROLL != SCREEN_WIDTH);
          
					_eventsManager.VBL();
					_graphicsManager.no_scroll = 0;
				}
        
				_soundManager.VOICE_MIX(4, 3);
				_graphicsManager.FADE_OUTW();
				_graphicsManager.no_scroll = 0;
				_graphicsManager.LOAD_IMAGE("intro2");
				_graphicsManager.SCROLL_ECRAN(0);
				_animationManager.CHARGE_ANIM("INTRO2");
				_graphicsManager.VISU_ALL();
				_soundManager.WSOUND(23);
				_animationManager.BOBANIM_OFF(3);
				_animationManager.BOBANIM_OFF(5);
				_graphicsManager.ofscroll = 0;
				_graphicsManager.SETCOLOR3(252, 100, 100, 100);
				_graphicsManager.SETCOLOR3(253, 100, 100, 100);
				_graphicsManager.SETCOLOR3(251, 100, 100, 100);
				_graphicsManager.SETCOLOR3(254, 0, 0, 0);
				_globals.BPP_NOAFF = 1;
				v5 = 0;
        
				do {
					_eventsManager.VBL();
					++v5;
				} while (v5 <= 4);
        
				_globals.BPP_NOAFF = 0;
				_globals.iRegul = 1;
				_graphicsManager.FADE_INW();
				for (i = 0; i < 200 / _globals.vitesse; ++i)
					_eventsManager.VBL();
        
				_animationManager.BOBANIM_ON(3);
				_soundManager.VOICE_MIX(5, 3);
				_animationManager.BOBANIM_OFF(3);
				_eventsManager.VBL();
				memcpy(&paletteData2, _graphicsManager.Palette, 796);
				v21 = *(uint16 *)&_graphicsManager.Palette[796];
				v22 = _graphicsManager.Palette[798];
				v7 = (int)&v23;
				_graphicsManager.setpal_vga256_linux(paletteData, _graphicsManager.VESA_BUFFER);
				_graphicsManager.FIN_VISU();
				_soundManager.SPECIAL_SOUND = 5;
				_graphicsManager.FADE_LINUX = 2;
				_animationManager.PLAY_ANM("ELEC.ANM", 10, 26, 200);
				_soundManager.SPECIAL_SOUND = 0;
        
				if (!_eventsManager.ESC_KEY) {
					_graphicsManager.LOAD_IMAGE("intro2");
					_graphicsManager.SCROLL_ECRAN(0);
					_animationManager.CHARGE_ANIM("INTRO2");
					_graphicsManager.VISU_ALL();
					_soundManager.WSOUND(23);
					_animationManager.BOBANIM_OFF(3);
					_animationManager.BOBANIM_OFF(5);
					_animationManager.BOBANIM_OFF(1);
					_graphicsManager.ofscroll = 0;
					_graphicsManager.SETCOLOR3(252, 100, 100, 100);
					_graphicsManager.SETCOLOR3(253, 100, 100, 100);
					_graphicsManager.SETCOLOR3(251, 100, 100, 100);
					_graphicsManager.SETCOLOR3(254, 0, 0, 0);
					_globals.BPP_NOAFF = 1;
					v8 = 0;
          
					do {
						_eventsManager.VBL();
						++v8;
					} while (v8 <= 3);
          
					_globals.BPP_NOAFF = 0;
					_globals.iRegul = 1;
					_graphicsManager.setpal_vga256_linux(paletteData2, _graphicsManager.VESA_BUFFER);
					v9 = 0;
          
					while (!_eventsManager.ESC_KEY) {
						if (v9 == 12) {
							_animationManager.BOBANIM_ON(3);
							_eventsManager.VBL();
							_soundManager.VOICE_MIX(6, 3);
							_eventsManager.VBL();
							_animationManager.BOBANIM_OFF(3);
						}
            
						Common::copy(&paletteData2[0], &paletteData2[PALETTE_BLOCK_SIZE], &_graphicsManager.Palette[0]);
						
						v11 = 1;
						v12 = 4 * v9;
						do {
							if (_graphicsManager.Palette[v11] > v12)
								_graphicsManager.Palette[v11] -= v12;
							++v11;
						} while ( v11 <= PALETTE_BLOCK_SIZE);
            
						_graphicsManager.setpal_vga256_linux(_graphicsManager.Palette, _graphicsManager.VESA_BUFFER);
						v13 = 1;
            
						if (2 * v9 > 1) {
							v7 = 2 * v9;
              
							do {
								_eventsManager.VBL();
								++v13;
							} while (v13 < v7);
						} 
						
						_graphicsManager.setpal_vga256_linux(paletteData2, _graphicsManager.VESA_BUFFER);
						v14 = 1;
						if (20 - v9 > 1) {
							v7 = 20 - v9;
              
							do {
								_eventsManager.VBL();
								++v14;
							} while (v14 < v7);
						}
            
						v9 += 2;
						if (v9 > 15) {
							_graphicsManager.setpal_vga256_linux(paletteData, _graphicsManager.VESA_BUFFER);
							for (j = 1; j < 100 / _globals.vitesse; ++j)
								_eventsManager.VBL();
              
							_animationManager.BOBANIM_ON(3);
							_soundManager.VOICE_MIX(7, 3);
							_animationManager.BOBANIM_OFF(3);
							
							for (k = 1; k < 60 / _globals.vitesse; ++k)
								_eventsManager.VBL();
							_animationManager.BOBANIM_ON(5);
							for (l = 0; l < 20 / _globals.vitesse; ++l)
								_eventsManager.VBL();

							Common::copy(&paletteData2[0], &paletteData2[PALETTE_BLOCK_SIZE], &_graphicsManager.Palette[0]);
							_graphicsManager.setpal_vga256_linux(_graphicsManager.Palette, _graphicsManager.VESA_BUFFER);
              
							for (m = 0; m < 50 / _globals.vitesse; ++m) {
								if (m == 30 / _globals.vitesse) {
									_animationManager.BOBANIM_ON(3);
									_soundManager.VOICE_MIX(8, 3);
									_animationManager.BOBANIM_OFF(3);
								}
                
								_eventsManager.VBL();
							}

							_graphicsManager.FADE_OUTW();
							_graphicsManager.FIN_VISU();
							_animationManager.CLS_ANM = 1;
							_soundManager.WSOUND(3);
							_soundManager.SPECIAL_SOUND = 1;
							_animationManager.PLAY_ANM("INTRO1.anm", 10, 24, 18);
							_soundManager.SPECIAL_SOUND = 0;

							if (!_eventsManager.ESC_KEY) {
								_animationManager.PLAY_ANM("INTRO2.anm", 10, 24, 18);
                
								if (!_eventsManager.ESC_KEY) {
									_animationManager.PLAY_ANM("INTRO3.anm", 10, 24, 200);
									if (!_eventsManager.ESC_KEY) {
										_animationManager.CLS_ANM = 0;
										_graphicsManager.FADE_LINUX = 2;
										_animationManager.PLAY_ANM("J4.anm", 12, 12, 1000);
									}
								}
							}
							break;
						}
					}
				}
			}
		}
	}
  
	_eventsManager.ESC_KEY = false;
}

} // End of namespace Hopkins
