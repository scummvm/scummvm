/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is _globals.FRee software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the _globals.FRee Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the _globals.FRee Software
 * Foundation, Inc., 51 _globals.FRanklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "hopkins/talk.h"

namespace Hopkins {

HopkinsEngine *g_vm;

HopkinsEngine::HopkinsEngine(OSystem *syst, const HopkinsGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _randomSource("Hopkins"), _animationManager() {
	g_vm = this;
	_animationManager.setParent(this);
	_eventsManager.setParent(this);
	_fontManager.setParent(this);
	_globals.setParent(this);
	_graphicsManager.setParent(this);
	_menuManager.setParent(this);
	_objectsManager.setParent(this);
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
	_objectsManager.CHANGE_OBJET(14);
	_objectsManager.AJOUTE_OBJET(14);

	GLOBALS.HELICO = 0;
	_eventsManager.MOUSE_OFF();

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
  
	_globals.iRegul = 0;
	FileManager::CONSTRUIT_SYSTEM("PERSO.SPR");
	GLOBALS.PERSO = FileManager::CHARGE_FICHIER(GLOBALS.NFICHIER);
	GLOBALS.PERSO_TYPE = 0;
	GLOBALS.PLANX = GLOBALS.PLANY = 0;
	memset(GLOBALS.SAUVEGARDE, 0, 2000);
	GLOBALS.SORTIE = 0;
	GLOBALS.PASSWORD = 1;

LABEL_12:
	if (GLOBALS.SORTIE == 300)
LABEL_13:
		GLOBALS.SORTIE = 0;

	if (!GLOBALS.SORTIE) {
		GLOBALS.SORTIE = _menuManager.MENU();
		if (GLOBALS.SORTIE == -1) {
			if (!g_system->getEventManager()->shouldQuit())
				PUBQUIT();
			_globals.PERSO = _globals.dos_free2(_globals.PERSO);
			REST_SYSTEM();
		}
	}

	for (;;) {
		for (;;) {
			for (;;) {
				for (;;) {
					for (;;) {
						for (;;) {
							for (;;) {
								for (;;) {
									if (g_system->getEventManager()->shouldQuit())
										return Common::kNoError;

									if (GLOBALS.SORTIE == 300)
										goto LABEL_13;
									if (GLOBALS.SORTIE == 18)
										PASS();
									if (GLOBALS.SORTIE == 23)
										PASS();
									if (GLOBALS.SORTIE == 22)
										PASS();
									if (GLOBALS.SORTIE == 19)
										PASS();
									if (GLOBALS.SORTIE == 20)
										PASS();
									if (GLOBALS.SORTIE != 1)
										break;

									_globals.Max_Propre = 50;
									_globals.Max_Ligne_Long = 40;
									_globals.Max_Propre_Gen = 20;
									_globals.Max_Perso_Y = 435;
									PERSONAGE2("IM01", "IM01", "ANIM01", "IM01", 1);
								}
                
								if (GLOBALS.SORTIE != 3)
									break;
								
								if (!*((byte *)GLOBALS.SAUVEGARDE + 170)) {
									_soundManager.WSOUND(3);
									if (_globals.FR == 1)
										_graphicsManager.LOAD_IMAGE("fond_globals.FR");
									if (!_globals.FR)
										_graphicsManager.LOAD_IMAGE("fondan");
									if (_globals.FR == 2)
										_graphicsManager.LOAD_IMAGE("fondes");
									_graphicsManager.FADE_INW();
									_eventsManager.delay(500);
									_graphicsManager.FADE_OUTW();
									_globals.iRegul = 1;
									_soundManager.SPECIAL_SOUND = 2;

									_graphicsManager.DD_Lock();
									_graphicsManager.Cls_Video();
									_graphicsManager.DD_Unlock();
									_graphicsManager.Cls_Pal();
									_graphicsManager.FADE_LINUX = 2;
					
									if (!GLOBALS.CENSURE)
										_animationManager.PLAY_ANM("BANQUE.ANM", 200, 28, 200);
									if (GLOBALS.CENSURE == 1)
										_animationManager.PLAY_ANM("BANKUK.ANM", 200, 28, 200);
									_soundManager.SPECIAL_SOUND = 0;
									_soundManager.DEL_SAMPLE(1);
									_soundManager.DEL_SAMPLE(2);
									_soundManager.DEL_SAMPLE(3);
									_soundManager.DEL_SAMPLE(4);
									*((byte *)GLOBALS.SAUVEGARDE + 170) = 1;
								}
                
								_globals.Max_Propre = 5;
								_globals.Max_Ligne_Long = 5;
								_globals.Max_Propre_Gen = 5;
								_globals.Max_Perso_Y = 450;
								GLOBALS.NOSPRECRAN = 1;
								PERSONAGE2("IM03", "IM03", "ANIM03", "IM03", 2);
							}
              
							if (GLOBALS.SORTIE != 4)
								break;
							_globals.DESACTIVE_INVENT = true;
							_objectsManager.PLAN_BETA();
							_globals.DESACTIVE_INVENT = false;
						}

						if (GLOBALS.SORTIE != 5)
							break;
						_globals.Max_Propre = 5;
						_globals.Max_Ligne_Long = 5;
						_globals.Max_Propre_Gen = 5;
						_globals.Max_Perso_Y = 455;
						GLOBALS.NOSPRECRAN = 1;
						byte v1 = *((byte *)GLOBALS.SAUVEGARDE + 80);
						if (v1) {
							if (v1 == 1)
								PERSONAGE2("IM05", "IM05A", "ANIM05B", "IM05", 3);
						} else {
							PERSONAGE2("IM05", "IM05", "ANIM05", "IM05", 3);
						}
            
						GLOBALS.NOSPRECRAN = 0;
					}
          
					if (GLOBALS.SORTIE != 8)
						break;
					
					_globals.Max_Propre = 15;
					_globals.Max_Ligne_Long = 15;
					_globals.Max_Propre_Gen = 10;
					_globals.Max_Perso_Y = 450;
					PERSONAGE2("IM08", "IM08", "ANIM08", "IM08", 2);
				}

				if (GLOBALS.SORTIE != 6)
					break;
				_globals.Max_Propre = 15;
				_globals.Max_Ligne_Long = 20;
				_globals.Max_Propre_Gen = 10;
				_globals.Max_Perso_Y = 460;
				PERSONAGE2("IM06", "IM06", "ANIM06", "IM06", 2);
			}

			if (GLOBALS.SORTIE != 7)
				break;
			if (*((byte *)GLOBALS.SAUVEGARDE + 220))
				PERSONAGE("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2);
			else
				PERSONAGE("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2);
		}

		if (GLOBALS.SORTIE == 9) {
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 440;
			
			if (!*((byte *)GLOBALS.SAUVEGARDE + 225))
				goto LABEL_109;
			PERSONAGE2("IM09", "IM09", "ANIM09", "IM09", 10);
		} else {
			if (GLOBALS.SORTIE == 10) {
				GLOBALS.NOSPRECRAN = 1;
				PERSONAGE("IM10", "IM10", "ANIM10", "IM10", 9);
				goto LABEL_124;
			}
      
			if (GLOBALS.SORTIE == 11) {
				GLOBALS.NOSPRECRAN = 1;
				_globals.Max_Propre = 15;
				_globals.Max_Ligne_Long = 20;
				_globals.Max_Propre_Gen = 10;
				_globals.Max_Perso_Y = 450;
				PERSONAGE2("IM11", "IM11", "ANIM11", "IM11", 2);
				goto LABEL_124;
			}

			switch (GLOBALS.SORTIE) {
			case 12:
				_globals.Max_Propre = 15;
				_globals.Max_Ligne_Long = 20;
				_globals.Max_Propre_Gen = 10;
				_globals.Max_Perso_Y = 450;
				if (*((byte *)GLOBALS.SAUVEGARDE + 225)) {
					GLOBALS.NOSPRECRAN = 1;
					PERSONAGE2("IM12", "IM12", "ANIM12", "IM12", 1);
				} else {
LABEL_109:
					BOOM();
				}
				break;
			case 13:
				_globals.Max_Propre = 50;
				_globals.Max_Ligne_Long = 40;
				_globals.Max_Propre_Gen = 20;
				_globals.Max_Perso_Y = 440;
				PERSONAGE2("IM13", "IM13", "ANIM13", "IM13", 1);
				break;
			case 14:
				_globals.Max_Propre = 50;
				_globals.Max_Ligne_Long = 40;
				_globals.Max_Propre_Gen = 20;
				_globals.Max_Perso_Y = 440;
				PERSONAGE2("IM14", "IM14", "ANIM14", "IM14", 1);
				break;
			default:
				if (GLOBALS.SORTIE == 15) {
					GLOBALS.NOSPRECRAN = 1;
					PERSONAGE("IM15", "IM15", "ANIM15", "IM15", 29);
					goto LABEL_124;
				}
				if (GLOBALS.SORTIE == 16) {
					_globals.Max_Propre = 5;
					_globals.Max_Ligne_Long = 5;
					_globals.Max_Propre_Gen = 5;
					_globals.Max_Perso_Y = 450;

					byte v2 = *((byte *)GLOBALS.SAUVEGARDE + 113);
					if (v2 == 1) {
						PERSONAGE2("IM16", "IM16A", "ANIM16", "IM16", 7);
					} else if (!v2) {
						PERSONAGE2("IM16", "IM16", "ANIM16", "IM16", 7);
					}
				} else {
					if (GLOBALS.SORTIE == 17)
						PASS();
					if (GLOBALS.SORTIE == 24)
						PASS();
					if (GLOBALS.SORTIE == 25) {
						_globals.Max_Propre = 15;
						_globals.Max_Ligne_Long = 20;
						_globals.Max_Propre_Gen = 10;
						_globals.Max_Perso_Y = 445;
						PERSONAGE2("IM25", "IM25", "ANIM25", "IM25", 30);
					} else {
						if (GLOBALS.SORTIE == 33) {
							GLOBALS.NOSPRECRAN = 1;
							PERSONAGE("IM33", "IM33", "ANIM33", "IM33", 8);
							goto LABEL_124;
						}
              
						if (GLOBALS.SORTIE == 26) {
							_globals.Max_Propre = 50;
							_globals.Max_Ligne_Long = 40;
							_globals.Max_Propre_Gen = 20;
							_globals.Max_Perso_Y = 435;
							PERSONAGE2("IM26", "IM26", "ANIM26", "IM26", 30);
						} else {
							if (GLOBALS.SORTIE == 27)
								PASS();
							if (GLOBALS.SORTIE == 28)
								PASS();
							if (GLOBALS.SORTIE == 29)
								PASS();
							if (GLOBALS.SORTIE == 30)
								PASS();
							if (GLOBALS.SORTIE == 31)
								PASS();
							if (GLOBALS.SORTIE == 35)
								ENDEMO();
							if (GLOBALS.SORTIE == 32)
								PASS();
							if (GLOBALS.SORTIE == 34)
								PASS();
                
							if ((uint16)(GLOBALS.SORTIE - 51) <= 38)
								PASS();
							if (GLOBALS.SORTIE == 111) {
								GLOBALS.NOSPRECRAN = 1;
								PERSONAGE("IM111", "IM111", "ANIM111", "IM111", 10);
								goto LABEL_124;
							}
                
							if (GLOBALS.SORTIE == 112) {
								GLOBALS.NOSPRECRAN = 1;
								PERSONAGE("IM112", "IM112", "ANIM112", "IM112", 10);
LABEL_124:
								GLOBALS.NOSPRECRAN = 0;
							} else if (GLOBALS.SORTIE == 113) {
								GLOBALS.SORTIE = 0;
								_globals.OLD_ECRAN = _globals.ECRAN;
								*((byte *)GLOBALS.SAUVEGARDE + 6) = _globals.ECRAN;
								_globals.ECRAN = 113;
								*((byte *)GLOBALS.SAUVEGARDE + 5) = 113;
								COMPUT_HOPKINS(1);
                  
								_graphicsManager.DD_Lock();
								_graphicsManager.Cls_Video();
								_graphicsManager.DD_Unlock();
								_graphicsManager.DD_VBL();
								memset(_graphicsManager.VESA_BUFFER, 0, 0x4B000u);
								memset(_graphicsManager.VESA_SCREEN, 0, 0x4B000u);
								_graphicsManager.Cls_Pal();
								_graphicsManager.RESET_SEGMENT_VESA();
							} else {
								if (GLOBALS.SORTIE == 114) {
									GLOBALS.SORTIE = 0;
									_globals.OLD_ECRAN = _globals.ECRAN;
									*((byte *)GLOBALS.SAUVEGARDE + 6) = _globals.ECRAN;
									_globals.ECRAN = 114;
									*((byte *)GLOBALS.SAUVEGARDE + 5) = 114;
									COMPUT_HOPKINS(2);
									goto LABEL_128;
								}
								if (GLOBALS.SORTIE == 115) {
									GLOBALS.SORTIE = 0;
									_globals.OLD_ECRAN = _globals.ECRAN;
									*((byte *)GLOBALS.SAUVEGARDE + 6) = _globals.ECRAN;
									_globals.ECRAN = 115;
									*((byte *)GLOBALS.SAUVEGARDE + 5) = 115;
									COMPUT_HOPKINS(3);

LABEL_128:
									_graphicsManager.DD_Lock();
									_graphicsManager.Cls_Video();
									_graphicsManager.DD_Unlock();
								} else if ((uint16)(GLOBALS.SORTIE - 194) > 5) {
									if (GLOBALS.SORTIE == 151) {
										_soundManager.WSOUND(16);
										_globals.iRegul = 1;
                      
										_graphicsManager.DD_Lock();
										_graphicsManager.Cls_Video();
										_graphicsManager.DD_Unlock();
										_graphicsManager.Cls_Pal();
										_graphicsManager.FADE_LINUX = 2;
										_animationManager.PLAY_ANM("JOUR3A.anm", 12, 12, 2000);
										_globals.iRegul = 0;
										GLOBALS.SORTIE = 300;
									}
                    
									if (GLOBALS.SORTIE == 150) {
										_soundManager.WSOUND(16);
										_globals.iRegul = 1;
										
										_graphicsManager.DD_Lock();
										_graphicsManager.Cls_Video();
										_graphicsManager.DD_Unlock();
										_graphicsManager.Cls_Pal();
										_graphicsManager.FADE_LINUX = 2;
										_animationManager.PLAY_ANM("JOUR1A.anm", 12, 12, 2000);
										_globals.iRegul = 0;
										GLOBALS.SORTIE = 300;
									}
                    
									if (GLOBALS.SORTIE == 152) {
										_soundManager.WSOUND(16);
										_globals.iRegul = 1;
                      
										_graphicsManager.DD_Lock();
										_graphicsManager.Cls_Video();
										_graphicsManager.DD_Unlock();
										_graphicsManager.Cls_Pal();
										_graphicsManager.FADE_LINUX = 2;
										_animationManager.PLAY_ANM("JOUR4A.anm", 12, 12, 2000);
										_globals.iRegul = 0;
										GLOBALS.SORTIE = 300;
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

	// Stub event loop
	Common::Event evt;
	while (!g_system->getEventManager()->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(evt))
			g_system->updateScreen();
	}

	return Common::kNoError;
}

bool HopkinsEngine::shouldQuit() const {
	return g_system->getEventManager()->shouldQuit();
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
  
	switch (_globals.FR) {
	case 0:
		if (!_eventsManager.mouse_linux)
			FileManager::CONSTRUIT_SYSTEM("SOUAN.SPR");
		if (!_globals.FR && _eventsManager.mouse_linux)
			FileManager::CONSTRUIT_SYSTEM("LSOUAN.SPR");
		break;
	case 1:
		FileManager::CONSTRUIT_SYSTEM("LSOU_globals.FR.SPR");
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
	
	switch (_globals.FR) {
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
	byte paletteData[PALETTE_EXT_BLOCK_SIZE];
	byte paletteData2[PALETTE_EXT_BLOCK_SIZE];

	memset(&paletteData, 0, PALETTE_EXT_BLOCK_SIZE);
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
				_graphicsManager.DD_Lock();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_Unlock();
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

void HopkinsEngine::PASS() {
	if (_globals.FR == 1)
		_graphicsManager.LOAD_IMAGE("ndfr");
	else
		_graphicsManager.LOAD_IMAGE("nduk");
  
	_graphicsManager.FADE_INW();
	if (_soundManager.VOICEOFF)
	    _eventsManager.delay(500);
	else
		_soundManager.VOICE_MIX(628, 4);
	
	_graphicsManager.FADE_OUTW();
	_globals.SORTIE = 4;	
}

void HopkinsEngine::ENDEMO() {
	_soundManager.WSOUND(28);
	if (_globals.FR == 1)
		_graphicsManager.LOAD_IMAGE("endfr");
	else
	    _graphicsManager.LOAD_IMAGE("enduk");
  
	_graphicsManager.FADE_INW();
	_eventsManager.delay(1500);
	_graphicsManager.FADE_OUTW();
	_globals.SORTIE = 0;
}

void HopkinsEngine::BOOM() {
	_graphicsManager.nbrligne = 640;
	_graphicsManager.SCANLINE(640);
	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();
	_graphicsManager.Cls_Pal();
	
	_globals.iRegul = 1;
	_soundManager.SPECIAL_SOUND = 199;
	_graphicsManager.FADE_LINUX = 2;
	if (_globals.SVGA == 1)
		_animationManager.PLAY_ANM("BOMBE2.ANM", 50, 14, 500);
	if (_globals.SVGA == 2)
		_animationManager.PLAY_ANM("BOMBE2A.ANM", 50, 14, 500);
	
	_soundManager.SPECIAL_SOUND = 0;
	_graphicsManager.LOAD_IMAGE("IM15");
	_animationManager.CHARGE_ANIM("ANIM15");
	_graphicsManager.VISU_ALL();
	_animationManager.BOBANIM_OFF(7);
	_globals.BPP_NOAFF = 1;

	for (int idx = 0; idx < 5; ++idx) {
		_eventsManager.VBL();
	}
  
	_globals.BPP_NOAFF = 0;
	_graphicsManager.FADE_INW();
	_eventsManager.MOUSE_OFF();
	
	for (int idx = 0; idx < 20; ++idx) {
		_eventsManager.VBL();
	}
  
	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO2("vire.pe2");
	_globals.NOPARLE = false;
	_animationManager.BOBANIM_ON(7);

	for (int idx = 0; idx < 100; ++idx) {
		_eventsManager.VBL();
	}

	_graphicsManager.FADE_OUTW();
	_graphicsManager.FIN_VISU();
	_globals.iRegul = 0;
	_globals.SORTIE = 151;
}

void HopkinsEngine::PERSONAGE(const Common::String &s1, const Common::String &s2, const Common::String &s3,
							   const Common::String &s4, int v) {
	warning("TODO: PERSONAGE2");
}

void HopkinsEngine::PERSONAGE2(const Common::String &s1, const Common::String &s2, const Common::String &s3,
							   const Common::String &s4, int v) {
	warning("TODO: PERSONAGE2");
}

void HopkinsEngine::REST_SYSTEM() {
	quitGame();
	_eventsManager.CONTROLE_MES();
}

void HopkinsEngine::PUBQUIT() {
	_globals.PUBEXIT = 1;
	_graphicsManager.RESET_SEGMENT_VESA();
	_globals.FORET = 0;
	_eventsManager.CASSE = 0;
	_globals.DESACTIVE_INVENT = true;
	_globals.FLAG_VISIBLE = false;
	_graphicsManager.LOAD_IMAGE("BOX");
	_soundManager.WSOUND(28);
	_graphicsManager.FADE_INW();
	_eventsManager.MOUSE_ON();
	_eventsManager.CHANGE_MOUSE(0);
	_eventsManager.btsouris = 0;
	_eventsManager.souris_n = 0;
	_globals.netscape = true;

	bool mouseClicked = false;
	int xp, yp;
	do {
		xp = _eventsManager.XMOUSE();
		yp = _eventsManager.YMOUSE();
		_eventsManager.VBL();
		
		if (_eventsManager.BMOUSE() == 1)
			mouseClicked = true;
	} while (!mouseClicked && !g_system->getEventManager()->shouldQuit());

	// Original tried to open a web browser link here. Since ScummVM doesn't support
	// that, it's being skipped in favour of simply exitting

	_graphicsManager.FADE_OUTW();
}

void HopkinsEngine::COMPUT_HOPKINS(int a1) {
	warning("TODO: COMPUT_HOPKINS");
}

} // End of namespace Hopkins
