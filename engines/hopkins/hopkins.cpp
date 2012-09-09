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
		_gameDescription(gameDesc), _randomSource("Hopkins") {
	g_vm = this;
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
	Init_Interrupt();

	SoundManager::WSOUND_INIT();

	GLOBALS.CHARGE_OBJET();
	ObjectManager::CHANGE_OBJET(14);
	ObjectManager::AJOUTE_OBJET(14);

	GLOBALS.HELICO = 0;
	_mouse.hideCursor();

	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();

	_graphicsManager.LOAD_IMAGE("LINUX");
	/*
  FADE_INW();
  SDL_Delay(1500);
  FADE_OUTW();
  if ( !internet )
  {
    FADE_LINUX = 2;
    PLAY_ANM("MP.ANM", 10, 16, 200);
  }
  LOAD_IMAGE("H2");
  FADE_INW();
  SDL_Delay(500);
  FADE_OUTW();
  if ( !ESC_KEY )
    INTRORUN(a1);
  iRegul = 0;
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
                  WSOUND(3);
                  if ( FR == 1 )
                    LOAD_IMAGE("fondfr");
                  if ( !FR )
                    LOAD_IMAGE("fondan");
                  if ( FR == 2 )
                    LOAD_IMAGE("fondes");
                  FADE_INW();
                  SDL_Delay(500);
                  FADE_OUTW();
                  iRegul = 1;
                  SPECIAL_SOUND = 2;
                  DD_Lock();
                  Cls_Video();
                  DD_Unlock();
                  Cls_Pal();
                  FADE_LINUX = 2;
                  if ( !CENSURE )
                    PLAY_ANM("BANQUE.ANM", 200, 28, 200);
                  if ( CENSURE == 1 )
                    PLAY_ANM("BANKUK.ANM", 200, 28, 200);
                  SPECIAL_SOUND = 0;
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
                      WSOUND(16);
                      iRegul = 1;
                      DD_Lock();
                      Cls_Video();
                      DD_Unlock();
                      Cls_Pal();
                      FADE_LINUX = 2;
                      PLAY_ANM("JOUR3A.anm", 12, 12, 2000);
                      iRegul = 0;
                      SORTIE = 300;
                    }
                    if ( SORTIE == 150 )
                    {
                      WSOUND(16);
                      iRegul = 1;
                      DD_Lock();
                      Cls_Video();
                      DD_Unlock();
                      Cls_Pal();
                      FADE_LINUX = 2;
                      PLAY_ANM("JOUR1A.anm", 12, 12, 2000);
                      iRegul = 0;
                      SORTIE = 300;
                    }
                    if ( SORTIE == 152 )
                    {
                      WSOUND(16);
                      iRegul = 1;
                      DD_Lock();
                      Cls_Video();
                      DD_Unlock();
                      Cls_Pal();
                      FADE_LINUX = 2;
                      PLAY_ANM("JOUR4A.anm", 12, 12, 2000);
                      iRegul = 0;
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

	// Stub event loop
	Common::Event evt;
	while (!g_system->getEventManager()->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(evt))
			;
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
	
	_mouse.mouse_linux = true;
  
	switch (GLOBALS.FR) {
	case 0:
		if (!_mouse.mouse_linux)
			FileManager::CONSTRUIT_SYSTEM("SOUAN.SPR");
		if (!GLOBALS.FR && _mouse.mouse_linux)
			FileManager::CONSTRUIT_SYSTEM("LSOUAN.SPR");
		break;
	case 1:
		FileManager::CONSTRUIT_SYSTEM("LSOUFR.SPR");
		break;
	case 2:
		FileManager::CONSTRUIT_SYSTEM("SOUES.SPR");
		break;
	}
  
	if (_mouse.mouse_linux) {
		_mouse.souris_sizex = 52;
		_mouse.souris_sizey = 32;
	} else {
		_mouse.souris_sizex = 34;
		_mouse.souris_sizey = 20;
	}
	_mouse.pointeur_souris = FileManager::CHARGE_FICHIER(GLOBALS.NFICHIER);

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
	
	_mouse.INSTALL_SOURIS();
	_mouse.souris_on();
	_mouse.souris_flag = false;
	_mouse.souris_max();

	GLOBALS.HOPKINS_DATA();

	_mouse.ofset_souris_x = 0;
	_mouse.ofset_souris_y = 0;
	GLOBALS.lItCounter = 0;
	GLOBALS.lOldItCounter = 0;
}

void HopkinsEngine::Init_Interrupt() {
	// TODO: Determine whether the timer is needed
}

} // End of namespace Hopkins
