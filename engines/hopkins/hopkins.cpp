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
#include "hopkins/saveload.h"
#include "hopkins/sound.h"
#include "hopkins/talk.h"

namespace Hopkins {

HopkinsEngine *g_vm;

HopkinsEngine::HopkinsEngine(OSystem *syst, const HopkinsGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _randomSource("Hopkins"), _animationManager() {
	g_vm = this;
	_debugger.setParent(this);
	_animationManager.setParent(this);
	_dialogsManager.setParent(this);
	_eventsManager.setParent(this);
	_fileManager.setParent(this);
	_fontManager.setParent(this);
	_globals.setParent(this);
	_graphicsManager.setParent(this);
	_linesManager.setParent(this);
	_menuManager.setParent(this);
	_objectsManager.setParent(this);
	_saveLoadManager.setParent(this);
	_scriptManager.setParent(this);
	_soundManager.setParent(this);
	_talkManager.setParent(this);
}

HopkinsEngine::~HopkinsEngine() {
}

Common::String HopkinsEngine::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool HopkinsEngine::canLoadGameStateCurrently() {
	return !_globals.SORTIE && !_globals.PLAN_FLAG;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool HopkinsEngine::canSaveGameStateCurrently() {
	return !_globals.SORTIE && !_globals.PLAN_FLAG;
}

/**
 * Load the savegame at the specified slot index
 */
Common::Error HopkinsEngine::loadGameState(int slot) {
	return _saveLoadManager.restore(slot);
}

/**
 * Save the game to the given slot index, and with the given name
 */
Common::Error HopkinsEngine::saveGameState(int slot, const Common::String &desc) {
	return _saveLoadManager.save(slot, desc);
}

Common::Error HopkinsEngine::run() {
	_saveLoadManager.initSaves();

	Common::StringMap iniParams;
	_fileManager.Chage_Inifile(iniParams);
	processIniParams(iniParams);

	_globals.setConfig();
	_fileManager.F_Censure();
	INIT_SYSTEM();

	_soundManager.WSOUND_INIT();

	bool retVal;
	if (getPlatform() == Common::kPlatformLinux) {
		if (getIsDemo())
			retVal = runLinuxDemo();
		else
			retVal = runLinuxFull();
	} else {
		warning("Unhandled version, switching to linux demo");
		retVal = runLinuxDemo();
	}

	if (!retVal)
		return Common::kNoError;

	// Stub event loop
	Common::Event evt;
	while (!g_system->getEventManager()->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(evt))
			g_system->updateScreen();
	}

	return Common::kNoError;
}

bool HopkinsEngine::runLinuxDemo() {
	_globals.CHARGE_OBJET();
	_objectsManager.CHANGE_OBJET(14);
	_objectsManager.AJOUTE_OBJET(14);

	_globals.HELICO = 0;
	_eventsManager.MOUSE_OFF();

	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();

	_graphicsManager.LOAD_IMAGE("LINUX");
	_graphicsManager.FADE_INW();
	_eventsManager.delay(1500);
	_graphicsManager.FADE_OUTW();

	if (!_globals.internet) {
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
	_fileManager.CONSTRUIT_SYSTEM("PERSO.SPR");
	_globals.PERSO = _fileManager.CHARGE_FICHIER(_globals.NFICHIER);
	_globals.PERSO_TYPE = 0;
	_globals.PLANX = _globals.PLANY = 0;
	memset(_globals.SAUVEGARDE, 0, 2000);
	_globals.SORTIE = 0;
	_globals.PASSWORD = true;

LABEL_12:
	if (_globals.SORTIE == 300)
LABEL_13:
		_globals.SORTIE = 0;

	if (!_globals.SORTIE) {
		_globals.SORTIE = _menuManager.MENU();
		if (_globals.SORTIE == -1) {
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
										return false;

									if (_globals.SORTIE == 300)
										goto LABEL_13;
									if (_globals.SORTIE == 18)
										PASS();
									if (_globals.SORTIE == 23)
										PASS();
									if (_globals.SORTIE == 22)
										PASS();
									if (_globals.SORTIE == 19)
										PASS();
									if (_globals.SORTIE == 20)
										PASS();
									if (_globals.SORTIE != 1)
										break;

									_globals.Max_Propre = 50;
									_globals.Max_Ligne_Long = 40;
									_globals.Max_Propre_Gen = 20;
									_globals.Max_Perso_Y = 435;
									_objectsManager.PERSONAGE2("IM01", "IM01", "ANIM01", "IM01", 1);
								}
                
								if (_globals.SORTIE != 3)
									break;
								
								if (!*((byte *)_globals.SAUVEGARDE + 170)) {
									_soundManager.WSOUND(3);
									if (_globals.FR == 1)
										_graphicsManager.LOAD_IMAGE("fondfr");
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
					
									if (!_globals.CENSURE)
										_animationManager.PLAY_ANM("BANQUE.ANM", 200, 28, 200);
									if (_globals.CENSURE == 1)
										_animationManager.PLAY_ANM("BANKUK.ANM", 200, 28, 200);
									_soundManager.SPECIAL_SOUND = 0;
									_soundManager.DEL_SAMPLE(1);
									_soundManager.DEL_SAMPLE(2);
									_soundManager.DEL_SAMPLE(3);
									_soundManager.DEL_SAMPLE(4);
									*((byte *)_globals.SAUVEGARDE + 170) = 1;
								}
                
								_globals.Max_Propre = 5;
								_globals.Max_Ligne_Long = 5;
								_globals.Max_Propre_Gen = 5;
								_globals.Max_Perso_Y = 450;
								_globals.NOSPRECRAN = true;
								_objectsManager.PERSONAGE2("IM03", "IM03", "ANIM03", "IM03", 2);
							}
              
							if (_globals.SORTIE != 4)
								break;
							_globals.DESACTIVE_INVENT = true;
							_objectsManager.PLAN_BETA();
							_globals.DESACTIVE_INVENT = false;
						}

						if (_globals.SORTIE != 5)
							break;
						_globals.Max_Propre = 5;
						_globals.Max_Ligne_Long = 5;
						_globals.Max_Propre_Gen = 5;
						_globals.Max_Perso_Y = 455;
						_globals.NOSPRECRAN = true;
						byte v1 = *((byte *)_globals.SAUVEGARDE + 80);
						if (v1) {
							if (v1 == 1)
								_objectsManager.PERSONAGE2("IM05", "IM05A", "ANIM05B", "IM05", 3);
						} else {
							_objectsManager.PERSONAGE2("IM05", "IM05", "ANIM05", "IM05", 3);
						}
            
						_globals.NOSPRECRAN = false;
					}
          
					if (_globals.SORTIE != 8)
						break;
					
					_globals.Max_Propre = 15;
					_globals.Max_Ligne_Long = 15;
					_globals.Max_Propre_Gen = 10;
					_globals.Max_Perso_Y = 450;
					_objectsManager.PERSONAGE2("IM08", "IM08", "ANIM08", "IM08", 2);
				}

				if (_globals.SORTIE != 6)
					break;
				_globals.Max_Propre = 15;
				_globals.Max_Ligne_Long = 20;
				_globals.Max_Propre_Gen = 10;
				_globals.Max_Perso_Y = 460;
				_objectsManager.PERSONAGE2("IM06", "IM06", "ANIM06", "IM06", 2);
			}

			if (_globals.SORTIE != 7)
				break;
			if (*((byte *)_globals.SAUVEGARDE + 220))
				_objectsManager.PERSONAGE("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2);
			else
				_objectsManager.PERSONAGE("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2);
		}

		if (_globals.SORTIE == 9) {
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 440;
			
			if (!*((byte *)_globals.SAUVEGARDE + 225))
				goto LABEL_109;
			_objectsManager.PERSONAGE2("IM09", "IM09", "ANIM09", "IM09", 10);
		} else {
			if (_globals.SORTIE == 10) {
				_globals.NOSPRECRAN = true;
				_objectsManager.PERSONAGE("IM10", "IM10", "ANIM10", "IM10", 9);
				goto LABEL_124;
			}
      
			if (_globals.SORTIE == 11) {
				_globals.NOSPRECRAN = true;
				_globals.Max_Propre = 15;
				_globals.Max_Ligne_Long = 20;
				_globals.Max_Propre_Gen = 10;
				_globals.Max_Perso_Y = 450;
				_objectsManager.PERSONAGE2("IM11", "IM11", "ANIM11", "IM11", 2);
				goto LABEL_124;
			}

			switch (_globals.SORTIE) {
			case 12:
				_globals.Max_Propre = 15;
				_globals.Max_Ligne_Long = 20;
				_globals.Max_Propre_Gen = 10;
				_globals.Max_Perso_Y = 450;
				if (*((byte *)_globals.SAUVEGARDE + 225)) {
					_globals.NOSPRECRAN = true;
					_objectsManager.PERSONAGE2("IM12", "IM12", "ANIM12", "IM12", 1);
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
				_objectsManager.PERSONAGE2("IM13", "IM13", "ANIM13", "IM13", 1);
				break;
			case 14:
				_globals.Max_Propre = 50;
				_globals.Max_Ligne_Long = 40;
				_globals.Max_Propre_Gen = 20;
				_globals.Max_Perso_Y = 440;
				_objectsManager.PERSONAGE2("IM14", "IM14", "ANIM14", "IM14", 1);
				break;
			default:
				if (_globals.SORTIE == 15) {
					_globals.NOSPRECRAN = true;
					_objectsManager.PERSONAGE("IM15", "IM15", "ANIM15", "IM15", 29);
					goto LABEL_124;
				}
				if (_globals.SORTIE == 16) {
					_globals.Max_Propre = 5;
					_globals.Max_Ligne_Long = 5;
					_globals.Max_Propre_Gen = 5;
					_globals.Max_Perso_Y = 450;

					byte v2 = *((byte *)_globals.SAUVEGARDE + 113);
					if (v2 == 1) {
						_objectsManager.PERSONAGE2("IM16", "IM16A", "ANIM16", "IM16", 7);
					} else if (!v2) {
						_objectsManager.PERSONAGE2("IM16", "IM16", "ANIM16", "IM16", 7);
					}
				} else {
					if (_globals.SORTIE == 17)
						PASS();
					if (_globals.SORTIE == 24)
						PASS();
					if (_globals.SORTIE == 25) {
						_globals.Max_Propre = 15;
						_globals.Max_Ligne_Long = 20;
						_globals.Max_Propre_Gen = 10;
						_globals.Max_Perso_Y = 445;
						_objectsManager.PERSONAGE2("IM25", "IM25", "ANIM25", "IM25", 30);
					} else {
						if (_globals.SORTIE == 33) {
							_globals.NOSPRECRAN = true;
							_objectsManager.PERSONAGE("IM33", "IM33", "ANIM33", "IM33", 8);
							goto LABEL_124;
						}
              
						if (_globals.SORTIE == 26) {
							_globals.Max_Propre = 50;
							_globals.Max_Ligne_Long = 40;
							_globals.Max_Propre_Gen = 20;
							_globals.Max_Perso_Y = 435;
							_objectsManager.PERSONAGE2("IM26", "IM26", "ANIM26", "IM26", 30);
						} else {
							if (_globals.SORTIE == 27)
								PASS();
							if (_globals.SORTIE == 28)
								PASS();
							if (_globals.SORTIE == 29)
								PASS();
							if (_globals.SORTIE == 30)
								PASS();
							if (_globals.SORTIE == 31)
								PASS();
							if (_globals.SORTIE == 35)
								ENDEMO();
							if (_globals.SORTIE == 32)
								PASS();
							if (_globals.SORTIE == 34)
								PASS();
                
							if ((uint16)(_globals.SORTIE - 51) <= 38)
								PASS();
							if (_globals.SORTIE == 111) {
								_globals.NOSPRECRAN = true;
								_objectsManager.PERSONAGE("IM111", "IM111", "ANIM111", "IM111", 10);
								goto LABEL_124;
							}
                
							if (_globals.SORTIE == 112) {
								_globals.NOSPRECRAN = true;
								_objectsManager.PERSONAGE("IM112", "IM112", "ANIM112", "IM112", 10);
LABEL_124:
								_globals.NOSPRECRAN = false;
							} else if (_globals.SORTIE == 113) {
								_globals.SORTIE = 0;
								_globals.OLD_ECRAN = _globals.ECRAN;
								*((byte *)_globals.SAUVEGARDE + 6) = _globals.ECRAN;
								_globals.ECRAN = 113;
								*((byte *)_globals.SAUVEGARDE + 5) = 113;
								_menuManager.COMPUT_HOPKINS(1);
                  
								_graphicsManager.DD_Lock();
								_graphicsManager.Cls_Video();
								_graphicsManager.DD_Unlock();
								_graphicsManager.DD_VBL();
								memset(_graphicsManager.VESA_BUFFER, 0, 0x4B000u);
								memset(_graphicsManager.VESA_SCREEN, 0, 0x4B000u);
								_graphicsManager.Cls_Pal();
								_graphicsManager.RESET_SEGMENT_VESA();
							} else {
								if (_globals.SORTIE == 114) {
									_globals.SORTIE = 0;
									_globals.OLD_ECRAN = _globals.ECRAN;
									*((byte *)_globals.SAUVEGARDE + 6) = _globals.ECRAN;
									_globals.ECRAN = 114;
									*((byte *)_globals.SAUVEGARDE + 5) = 114;
									_menuManager.COMPUT_HOPKINS(2);
									goto LABEL_128;
								}
								if (_globals.SORTIE == 115) {
									_globals.SORTIE = 0;
									_globals.OLD_ECRAN = _globals.ECRAN;
									*((byte *)_globals.SAUVEGARDE + 6) = _globals.ECRAN;
									_globals.ECRAN = 115;
									*((byte *)_globals.SAUVEGARDE + 5) = 115;
									_menuManager.COMPUT_HOPKINS(3);

LABEL_128:
									_graphicsManager.DD_Lock();
									_graphicsManager.Cls_Video();
									_graphicsManager.DD_Unlock();
								} else if ((uint16)(_globals.SORTIE - 194) > 5) {
									if (_globals.SORTIE == 151) {
										_soundManager.WSOUND(16);
										_globals.iRegul = 1;
                      
										_graphicsManager.DD_Lock();
										_graphicsManager.Cls_Video();
										_graphicsManager.DD_Unlock();
										_graphicsManager.Cls_Pal();
										_graphicsManager.FADE_LINUX = 2;
										_animationManager.PLAY_ANM("JOUR3A.anm", 12, 12, 2000);
										_globals.iRegul = 0;
										_globals.SORTIE = 300;
									}
                    
									if (_globals.SORTIE == 150) {
										_soundManager.WSOUND(16);
										_globals.iRegul = 1;
										
										_graphicsManager.DD_Lock();
										_graphicsManager.Cls_Video();
										_graphicsManager.DD_Unlock();
										_graphicsManager.Cls_Pal();
										_graphicsManager.FADE_LINUX = 2;
										_animationManager.PLAY_ANM("JOUR1A.anm", 12, 12, 2000);
										_globals.iRegul = 0;
										_globals.SORTIE = 300;
									}
                    
									if (_globals.SORTIE == 152) {
										_soundManager.WSOUND(16);
										_globals.iRegul = 1;
                      
										_graphicsManager.DD_Lock();
										_graphicsManager.Cls_Video();
										_graphicsManager.DD_Unlock();
										_graphicsManager.Cls_Pal();
										_graphicsManager.FADE_LINUX = 2;
										_animationManager.PLAY_ANM("JOUR4A.anm", 12, 12, 2000);
										_globals.iRegul = 0;
										_globals.SORTIE = 300;
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
	return true;
}

bool HopkinsEngine::runLinuxFull() {
	_soundManager.WSOUND(16);

	_globals.CHARGE_OBJET();
	_objectsManager.CHANGE_OBJET(14);
	_objectsManager.AJOUTE_OBJET(14);

	_globals.HELICO = 0;
	_eventsManager.MOUSE_OFF();

	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();

	_graphicsManager.LOAD_IMAGE("H2");
	_graphicsManager.FADE_INW();
	_eventsManager.delay(500);

	_globals.vitesse = 2;
	_globals.iRegul = 1;
	_graphicsManager.FADE_LINUX = 2;
	_graphicsManager.FADE_OUTW();

	if (!_eventsManager.ESC_KEY)
		INTRORUN();
  
	_globals.iRegul = 0;
	_fileManager.CONSTRUIT_SYSTEM("PERSO.SPR");
	_globals.PERSO = _fileManager.CHARGE_FICHIER(_globals.NFICHIER);
	_globals.PERSO_TYPE = 0;
	_globals.PLANX = _globals.PLANY = 0;
	memset(_globals.SAUVEGARDE, 0, 2000);
	_globals.SORTIE = 0;
	_globals.PASSWORD = false;

LABEL_10:
	if (_globals.SORTIE == 300)
LABEL_11:
		_globals.SORTIE = 0;
	if (!_globals.SORTIE) {
		_globals.SORTIE = _menuManager.MENU();
		if (_globals.SORTIE == -1) {
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
									for (;;) {
										for (;;) {
											for (;;) {
												for (;;) {
													for (;;) {
														if (_globals.SORTIE == 300)
															goto LABEL_11;
														if (_globals.SORTIE != 18)
															break;
														_globals.NOSPRECRAN = true;
														_globals.Max_Propre = 5;
														_globals.Max_Ligne_Long = 5;
														_globals.Max_Propre_Gen = 5;
														_globals.Max_Perso_Y = 450;
														_objectsManager.PERSONAGE2("IM18", "IM18", "ANIM18", "IM18", 29);
													}
													if (_globals.SORTIE != 23)
														break;
													_globals.Max_Propre = 15;
													_globals.Max_Ligne_Long = 20;
													_globals.Max_Propre_Gen = 10;
													_globals.Max_Perso_Y = 440;
													_objectsManager.PERSONAGE2("IM23", "IM23", "ANIM23", "IM23", 6);
												}
												if (_globals.SORTIE != 22)
													break;
												_globals.Max_Propre = 15;
												_globals.Max_Ligne_Long = 20;
												_globals.Max_Propre_Gen = 10;
												_globals.Max_Perso_Y = 445;
												_objectsManager.PERSONAGE2("IM22", "IM22", "ANIM22", "IM22", 6);
											}
											if (_globals.SORTIE != 19)
												break;
											_globals.Max_Propre = 50;
											_globals.Max_Ligne_Long = 40;
											_globals.Max_Propre_Gen = 20;
											_globals.Max_Perso_Y = 440;
											if (*((byte *)_globals.SAUVEGARDE + 123))
												_objectsManager.PERSONAGE2("IM19", "IM19A", "ANIM19", "IM19", 6);
											else
												_objectsManager.PERSONAGE2("IM19", "IM19", "ANIM19", "IM19", 6);
										}
										if (_globals.SORTIE != 20)
											break;
										_globals.Max_Propre = 8;
										_globals.Max_Ligne_Long = 10;
										_globals.Max_Propre_Gen = 8;
										_globals.Max_Perso_Y = 440;
										_objectsManager.PERSONAGE2("IM20", "IM20", "ANIM20", "IM20", 6);
										if (_globals.SORTIE == 17) {
											_globals.iRegul = 1;
											_graphicsManager.DD_Lock();
											_graphicsManager.Cls_Video();
											_graphicsManager.DD_Unlock();
											_graphicsManager.Cls_Pal();
											_soundManager.WSOUND_OFF();
											_soundManager.WSOUND(6);
											_graphicsManager.FADE_LINUX = 2;
											if (_globals.SVGA == 2)
												_animationManager.PLAY_ANM("PURG2A.ANM", 12, 18, 50);
											if (_globals.SVGA == 1)
												_animationManager.PLAY_ANM("PURG2.ANM", 12, 18, 50);
LABEL_231:
											_globals.iRegul = 0;
										}
									}
									if (_globals.SORTIE != 1)
										break;
									_globals.Max_Propre = 50;
									_globals.Max_Ligne_Long = 40;
									_globals.Max_Propre_Gen = 20;
									_globals.Max_Perso_Y = 435;
									_objectsManager.PERSONAGE2("IM01", "IM01", "ANIM01", "IM01", 1);
								}
								if (_globals.SORTIE != 3)
									break;
								if (!*((byte *)_globals.SAUVEGARDE + 170)) {
									_soundManager.WSOUND(3);
									if (_globals.FR == 1)
										_graphicsManager.LOAD_IMAGE("fondfr");
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
									if (!_globals.CENSURE)
										_animationManager.PLAY_ANM("BANQUE.ANM", 200, 28, 200);
									if (_globals.CENSURE)
										_animationManager.PLAY_ANM("BANKUK.ANM", 200, 28, 200);
									_soundManager.SPECIAL_SOUND = 0;
									_soundManager.DEL_SAMPLE(1);
									_soundManager.DEL_SAMPLE(2);
									_soundManager.DEL_SAMPLE(3);
									_soundManager.DEL_SAMPLE(4);
									*((byte *)_globals.SAUVEGARDE + 170) = 1;
								}
								_globals.Max_Propre = 5;
								_globals.Max_Ligne_Long = 5;
								_globals.Max_Propre_Gen = 5;
								_globals.Max_Perso_Y = 450;
								_globals.NOSPRECRAN = true;
								_objectsManager.PERSONAGE2("IM03", "IM03", "ANIM03", "IM03", 2);
							}
							if (_globals.SORTIE != 4)
								break;
							_globals.DESACTIVE_INVENT = true;
							_objectsManager.PLAN_BETA();
							_globals.DESACTIVE_INVENT = false;
						}
						if (_globals.SORTIE != 5)
							break;
						_globals.Max_Propre = 5;
						_globals.Max_Ligne_Long = 5;
						_globals.Max_Propre_Gen = 5;
						_globals.Max_Perso_Y = 455;
						_globals.NOSPRECRAN = true;
						byte v2 = *((byte *)_globals.SAUVEGARDE + 80);
						if (v2) {
							if (v2 == 1)
								_objectsManager.PERSONAGE2("IM05", "IM05A", "ANIM05B", "IM05", 3);
						} else {
							_objectsManager.PERSONAGE2("IM05", "IM05", "ANIM05", "IM05", 3);
						}
						_globals.NOSPRECRAN = false;
					}
					if (_globals.SORTIE != 8)
						break;
					_globals.Max_Propre = 15;
					_globals.Max_Ligne_Long = 15;
					_globals.Max_Propre_Gen = 10;
					_globals.Max_Perso_Y = 450;
					_objectsManager.PERSONAGE2("IM08", "IM08", "ANIM08", "IM08", 2);
				}
				if (_globals.SORTIE != 6)
					break;
				_globals.Max_Propre = 15;
				_globals.Max_Ligne_Long = 20;
				_globals.Max_Propre_Gen = 10;
				_globals.Max_Perso_Y = 460;
				_objectsManager.PERSONAGE2("IM06", "IM06", "ANIM06", "IM06", 2);
			}
			if (_globals.SORTIE != 7)
				break;
			if (*((byte *)_globals.SAUVEGARDE + 220))
				_objectsManager.PERSONAGE("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2);
			else
				_objectsManager.PERSONAGE("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2);
		}
		if (_globals.SORTIE == 9) {
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 440;
			if (!*((byte *)_globals.SAUVEGARDE + 225))
				goto LABEL_140;
			_objectsManager.PERSONAGE2("IM09", "IM09", "ANIM09", "IM09", 10);
		} else {
			if (_globals.SORTIE == 10) {
				_globals.NOSPRECRAN = true;
				_objectsManager.PERSONAGE("IM10", "IM10", "ANIM10", "IM10", 9);
				goto LABEL_239;
			}
			if (_globals.SORTIE == 11) {
				_globals.NOSPRECRAN = true;
				_globals.Max_Propre = 15;
				_globals.Max_Ligne_Long = 20;
				_globals.Max_Propre_Gen = 10;
				_globals.Max_Perso_Y = 450;
				_objectsManager.PERSONAGE2("IM11", "IM11", "ANIM11", "IM11", 2);
				goto LABEL_239;
			}
			switch (_globals.SORTIE) {
			case 12:
				_globals.Max_Propre = 15;
				_globals.Max_Ligne_Long = 20;
				_globals.Max_Propre_Gen = 10;
				_globals.Max_Perso_Y = 450;
				if (*((byte *)_globals.SAUVEGARDE + 225)) {
					_globals.NOSPRECRAN = true;
					_objectsManager.PERSONAGE2("IM12", "IM12", "ANIM12", "IM12", 1);
				} else {
LABEL_140:
					BOOM();
				}
				break;
			case 13:
				_globals.Max_Propre = 50;
				_globals.Max_Ligne_Long = 40;
				_globals.Max_Propre_Gen = 20;
				_globals.Max_Perso_Y = 440;
				_objectsManager.PERSONAGE2("IM13", "IM13", "ANIM13", "IM13", 1);
				break;
			case 14:
				_globals.Max_Propre = 50;
				_globals.Max_Ligne_Long = 40;
				_globals.Max_Propre_Gen = 20;
				_globals.Max_Perso_Y = 440;
				_objectsManager.PERSONAGE2("IM14", "IM14", "ANIM14", "IM14", 1);
				break;
			default:
				if (_globals.SORTIE == 15) {
					_globals.NOSPRECRAN = true;
					_objectsManager.PERSONAGE("IM15", "IM15", "ANIM15", "IM15", 29);
					goto LABEL_239;
				}
				switch (_globals.SORTIE) {
				case 16: {
					_globals.Max_Propre = 5;
					_globals.Max_Ligne_Long = 5;
					_globals.Max_Propre_Gen = 5;
					_globals.Max_Perso_Y = 450;
					byte v3 = *((byte *)_globals.SAUVEGARDE + 113);
					if (v3 == 1) {
						_objectsManager.PERSONAGE2("IM16", "IM16A", "ANIM16", "IM16", 7);
					} else if (!v3) {
						_objectsManager.PERSONAGE2("IM16", "IM16", "ANIM16", "IM16", 7);
					}
					break;
					}
				case 17: {
					_globals.Max_Propre = 50;
					_globals.Max_Ligne_Long = 40;
					_globals.Max_Propre_Gen = 20;
					_globals.Max_Perso_Y = 440;
					byte v4 = *((byte *)_globals.SAUVEGARDE + 117);
					if (v4 == 1) {
						_objectsManager.PERSONAGE2("IM17", "IM17A", "ANIM17", "IM17", 11);
					} else if (!v4) {
						_objectsManager.PERSONAGE2("IM17", "IM17", "ANIM17", "IM17", 11);
					}
					if (_globals.SORTIE == 18) {
						_globals.iRegul = 1;
						_graphicsManager.DD_Lock();
						_graphicsManager.Cls_Video();
						_graphicsManager.DD_Unlock();
						_graphicsManager.Cls_Pal();
						_soundManager.WSOUND_OFF();
						_soundManager.WSOUND(29);
						_graphicsManager.FADE_LINUX = 2;
						if (_globals.SVGA == 2)
							_animationManager.PLAY_ANM("PURG1A.ANM", 12, 18, 50);
						if (_globals.SVGA == 1)
							_animationManager.PLAY_ANM("PURG1.ANM", 12, 18, 50);
						goto LABEL_231;
					}
					break;
					}
				case 24: {
					_globals.Max_Propre = 5;
					_globals.Max_Ligne_Long = 5;
					_globals.Max_Propre_Gen = 5;
					_globals.Max_Perso_Y = 450;
					byte v5 = *((byte *)_globals.SAUVEGARDE + 181);
					if (v5) {
						if (v5 == 1)
							_objectsManager.PERSONAGE2("IM24", "IM24a", "ANIM24", "IM24", 1);
					} else {
						_objectsManager.PERSONAGE2("IM24", "IM24", "ANIM24", "IM24", 1);
					}
					break;
					}
				case 25:
					_globals.Max_Propre = 15;
					_globals.Max_Ligne_Long = 20;
					_globals.Max_Propre_Gen = 10;
					_globals.Max_Perso_Y = 445;
					_objectsManager.PERSONAGE2("IM25", "IM25", "ANIM25", "IM25", 30);
					break;
				default:
					if (_globals.SORTIE == 33) {
						_globals.NOSPRECRAN = true;
						_objectsManager.PERSONAGE("IM33", "IM33", "ANIM33", "IM33", 8);
						goto LABEL_239;
					}
					switch (_globals.SORTIE) {
					case 26:
						_globals.Max_Propre = 50;
						_globals.Max_Ligne_Long = 40;
						_globals.Max_Propre_Gen = 20;
						_globals.Max_Perso_Y = 435;
						_objectsManager.PERSONAGE2("IM26", "IM26", "ANIM26", "IM26", 30);
						break;
					case 27: {
						_globals.Max_Propre = 10;
						_globals.Max_Ligne_Long = 15;
						_globals.Max_Propre_Gen = 20;
						_globals.Max_Perso_Y = 440;
						byte v6 = *((byte *)_globals.SAUVEGARDE + 177);
						if (v6 == 1) {
							_objectsManager.PERSONAGE2("IM27", "IM27A", "ANIM27", "IM27", 27);
						} else if (!v6) {
							_objectsManager.PERSONAGE2("IM27", "IM27", "ANIM27", "IM27", 27);
						}
						break;
						}
					case 28:
						_globals.Max_Propre = 5;
						_globals.Max_Ligne_Long = 5;
						_globals.Max_Propre_Gen = 5;
						_globals.Max_Perso_Y = 450;
						_globals.NOSPRECRAN = true;
						if (*((byte *)_globals.SAUVEGARDE + 166) != 1 || *((byte *)_globals.SAUVEGARDE + 167) != 1)
							_objectsManager.PERSONAGE2("IM28", "IM28", "ANIM28", "IM28", 1);
						else
							_objectsManager.PERSONAGE2("IM28a", "IM28", "ANIM28", "IM28", 1);
						break;
					case 29:
						_globals.Max_Propre = 60;
						_globals.Max_Ligne_Long = 50;
						_globals.Max_Propre_Gen = 50;
						_globals.Max_Perso_Y = 445;
						_objectsManager.PERSONAGE2("IM29", "IM29", "ANIM29", "IM29", 1);
						break;
					default:
						if (_globals.SORTIE == 30) {
							_globals.Max_Propre = 10;
							_globals.Max_Ligne_Long = 15;
							_globals.Max_Propre_Gen = 20;
							_globals.Max_Perso_Y = 440;
							_globals.NOSPRECRAN = true;
							_objectsManager.PERSONAGE2("IM30", "IM30", "ANIM30", "IM30", 24);
							goto LABEL_239;
						}
						if (_globals.SORTIE == 31) {
							_objectsManager.PERSONAGE("IM31", "IM31", "ANIM31", "IM31", 10);
						} else if ((unsigned __int16)(_globals.SORTIE - 35) <= 6u) {
							_globals.fmusic = 13;
							_globals.Max_Propre = 50;
							_globals.Max_Ligne_Long = 40;
							_globals.Max_Propre_Gen = 20;
							_globals.Max_Perso_Y = 435;
							_globals.DESACTIVE_INVENT = false;
							_globals.FORET = 1;
							_globals.NOSPRECRAN = true;
							Common::String im = "IM" + _globals.SORTIE;
							_soundManager.WSOUND(13);
							if (_globals.FORETSPR == g_PTRNUL) {
								_fileManager.CONSTRUIT_SYSTEM("HOPDEG.SPR");
								_globals.FORETSPR = _objectsManager.CHARGE_SPRITE(_globals.NFICHIER);
								_soundManager.CHARGE_SAMPLE(1, "SOUND41.WAV");
							}
							_objectsManager.PERSONAGE2(im, im, "BANDIT", im, 13);
							_globals.NOSPRECRAN = false;
							if (_globals.SORTIE - 35 > 14) {
								_globals.dos_free2(_globals.FORETSPR);
								_globals.FORETSPR = g_PTRNUL;
								_globals.FORET = 0;
								_soundManager.DEL_SAMPLE(1);
							}
						} else if (_globals.SORTIE == 32) {
							_globals.Max_Propre = 15;
							_globals.Max_Ligne_Long = 20;
							_globals.Max_Propre_Gen = 10;
							_globals.Max_Perso_Y = 445;
							_objectsManager.PERSONAGE2("IM32", "IM32", "ANIM32", "IM32", 2);
						} else {
							if (_globals.SORTIE == 34) {
								_globals.NOSPRECRAN = true;
								_objectsManager.PERSONAGE("IM34", "IM34", "ANIM34", "IM34", 2);
								goto LABEL_239;
							}
							switch (_globals.SORTIE) {
							case 51:
								_globals.Max_Propre = 20;
								_globals.Max_Ligne_Long = 10;
								_globals.Max_Propre_Gen = 10;
								_globals.Max_Perso_Y = 440;
								_objectsManager.PERSONAGE2("IM51", "IM51", "ANIM51", "IM51", 14);
								break;
							case 52:
								_globals.Max_Propre = 15;
								_globals.Max_Ligne_Long = 15;
								_globals.Max_Propre_Gen = 10;
								_globals.Max_Perso_Y = 445;
								_objectsManager.PERSONAGE2("IM52", "IM52", "ANIM52", "IM52", 14);
								break;
							case 54:
								_globals.Max_Propre = 40;
								_globals.Max_Ligne_Long = 30;
								_globals.Max_Propre_Gen = 20;
								_globals.Max_Perso_Y = 440;
								_objectsManager.PERSONAGE2("IM54", "IM54", "ANIM54", "IM54", 14);
								break;
							case 55:
								_globals.Max_Propre = 40;
								_globals.Max_Ligne_Long = 30;
								_globals.Max_Propre_Gen = 20;
								_globals.Max_Perso_Y = 460;
								_globals.NOSPRECRAN = true;
								_objectsManager.PERSONAGE2("IM55", "IM55", "ANIM55", "IM55", 14);
								break;
							case 56:
								_globals.Max_Propre = 40;
								_globals.Max_Ligne_Long = 30;
								_globals.Max_Propre_Gen = 20;
								_globals.Max_Perso_Y = 440;
								_globals.NOSPRECRAN = true;
								_objectsManager.PERSONAGE2("IM56", "IM56", "ANIM56", "IM56", 14);
								break;
							case 58:
								_globals.Max_Propre = 40;
								_globals.Max_Ligne_Long = 30;
								_globals.Max_Propre_Gen = 20;
								_globals.Max_Perso_Y = 440;
								_globals.NOSPRECRAN = true;
								_objectsManager.PERSONAGE2("IM58", "IM58", "ANIM58", "IM58", 14);
								break;
							case 57:
								_globals.Max_Propre = 40;
								_globals.Max_Ligne_Long = 30;
								_globals.Max_Propre_Gen = 20;
								_globals.Max_Perso_Y = 440;
								_objectsManager.PERSONAGE2("IM57", "IM57", "ANIM57", "IM57", 14);
								break;
							default:
								if (_globals.SORTIE == 59) {
									_globals.Max_Propre = 5;
									_globals.Max_Ligne_Long = 5;
									_globals.Max_Propre_Gen = 5;
									_globals.Max_Perso_Y = 445;
									_globals.NOSPRECRAN = true;
									_objectsManager.PERSONAGE2("IM59", "IM59", "ANIM59", "IM59", 21);
									goto LABEL_239;
								}
								if (_globals.SORTIE == 60) {
									_globals.Max_Propre = 40;
									_globals.Max_Ligne_Long = 30;
									_globals.Max_Propre_Gen = 20;
									_globals.Max_Perso_Y = 440;
									_globals.NOSPRECRAN = true;
									_objectsManager.PERSONAGE2("IM60", "IM60", "ANIM60", "IM60", 21);
									goto LABEL_239;
								}
								if (_globals.SORTIE == 61) {
									if (*((byte *)_globals.SAUVEGARDE + 311) == 1 && !*((byte *)_globals.SAUVEGARDE + 312))
										INCENDIE();
									_globals.NOSPRECRAN = true;
									_objectsManager.PERSONAGE("IM61", "IM61", "ANIM61", "IM61", 21);
								} else {
									if (_globals.SORTIE == 63) {
										_globals.Max_Propre = 40;
										_globals.Max_Ligne_Long = 30;
										_globals.Max_Propre_Gen = 20;
										_globals.Max_Perso_Y = 435;
										_globals.NOSPRECRAN = true;
										_objectsManager.PERSONAGE2("IM63", "IM63", "ANIM63", "IM63", 21);
										goto LABEL_239;
									}
									if (_globals.SORTIE == 64) {
										_globals.Max_Propre = 40;
										_globals.Max_Ligne_Long = 30;
										_globals.Max_Propre_Gen = 20;
										_globals.Max_Perso_Y = 435;
										_objectsManager.PERSONAGE2("IM64", "IM64", "ANIM64", "IM64", 21);
									} else {
										if (_globals.SORTIE == 65) {
											_globals.Max_Propre = 40;
											_globals.Max_Ligne_Long = 30;
											_globals.Max_Propre_Gen = 20;
											_globals.Max_Perso_Y = 435;
											_globals.NOSPRECRAN = true;
											_objectsManager.PERSONAGE2("IM65", "IM65", "ANIM65", "IM65", 21);
											goto LABEL_239;
										}
										if (_globals.SORTIE == 66) {
											_globals.Max_Propre = 5;
											_globals.Max_Ligne_Long = 5;
											_globals.Max_Propre_Gen = 5;
											_globals.Max_Perso_Y = 445;
											_globals.NOSPRECRAN = true;
											_objectsManager.PERSONAGE2("IM66", "IM66", "ANIM66", "IM66", 21);
											goto LABEL_239;
										}
										if (_globals.SORTIE == 69) {
											_globals.Max_Propre = 5;
											_globals.Max_Ligne_Long = 5;
											_globals.Max_Propre_Gen = 5;
											_globals.Max_Perso_Y = 445;
											_globals.NOSPRECRAN = true;
											_objectsManager.PERSONAGE2("IM69", "IM69", "ANIM69", "IM69", 21);
											goto LABEL_239;
										}
										if (_globals.SORTIE == 62) {
											_globals.Max_Propre = 8;
											_globals.Max_Ligne_Long = 8;
											_globals.Max_Propre_Gen = 20;
											_globals.Max_Perso_Y = 435;
											_globals.NOSPRECRAN = true;
											_objectsManager.PERSONAGE2("IM62", "IM62", NULL, "IM62", 21);
											goto LABEL_239;
										}
										if (_globals.SORTIE == 68) {
											_globals.Max_Propre = 8;
											_globals.Max_Ligne_Long = 8;
											_globals.Max_Propre_Gen = 20;
											_globals.Max_Perso_Y = 435;
											_objectsManager.PERSONAGE2("IM68", "IM68", "ANIM68", "IM68", 21);
										} else {
											if (_globals.SORTIE == 67) {
												_globals.Max_Propre = 8;
												_globals.Max_Ligne_Long = 8;
												_globals.Max_Propre_Gen = 20;
												_globals.Max_Perso_Y = 435;
												_globals.NOSPRECRAN = true;
												_objectsManager.PERSONAGE2("IM67", "IM67", NULL, "IM67", 21);
												goto LABEL_239;
											}
											if (_globals.SORTIE == 70) {
												_globals.Max_Propre = 8;
												_globals.Max_Ligne_Long = 8;
												_globals.Max_Propre_Gen = 20;
												_globals.Max_Perso_Y = 435;
												_globals.NOSPRECRAN = true;
												_objectsManager.PERSONAGE2("IM70", "IM70", NULL, "IM70", 21);
												goto LABEL_239;
											}
											if (_globals.SORTIE == 71) {
												_globals.Max_Propre = 5;
												_globals.Max_Ligne_Long = 5;
												_globals.Max_Propre_Gen = 5;
												_globals.Max_Perso_Y = 445;
												_globals.NOSPRECRAN = true;
												_objectsManager.PERSONAGE2("IM71", "IM71", "ANIM71", "IM71", 21);
												goto LABEL_239;
											}
											switch (_globals.SORTIE) {
											case 73: {
												_globals.Max_Propre = 15;
												_globals.Max_Ligne_Long = 15;
												_globals.Max_Propre_Gen = 10;
												_globals.Max_Perso_Y = 445;
												byte v7 = *((byte *)_globals.SAUVEGARDE + 318);
												if (v7 == 1) {
													_objectsManager.PERSONAGE2("IM73", "IM73A", "ANIM73", "IM73", 21);
												} else if (!v7) {
													_objectsManager.PERSONAGE2("IM73", "IM73", "ANIM73", "IM73", 21);
												}
												break;
												}
											case 75:
												BASE();
												break;
											case 93:
												_globals.Max_Propre = 5;
												_globals.Max_Ligne_Long = 5;
												_globals.Max_Propre_Gen = 5;
												_globals.Max_Perso_Y = 445;
												if (*((byte *)_globals.SAUVEGARDE + 330))
													_objectsManager.PERSONAGE2("IM93", "IM93c", "ANIM93", "IM93", 29);
												else
													_objectsManager.PERSONAGE2("IM93", "IM93", "ANIM93", "IM93", 29);
												break;
											case 94:
												_globals.Max_Propre = 5;
												_globals.Max_Ligne_Long = 5;
												_globals.Max_Propre_Gen = 5;
												_globals.Max_Perso_Y = 440;
												_objectsManager.PERSONAGE2("IM94", "IM94", "ANIM94", "IM94", 19);
												break;
											case 95:
												_globals.Max_Propre = 5;
												_globals.Max_Ligne_Long = 5;
												_globals.Max_Propre_Gen = 5;
												_globals.Max_Perso_Y = 435;
												_globals.NOSPRECRAN = true;
												_objectsManager.PERSONAGE2("IM95", "IM95", "ANIM95", "IM95", 19);
												break;
											case 97:
												_globals.Max_Propre = 5;
												_globals.Max_Ligne_Long = 5;
												_globals.Max_Propre_Gen = 5;
												_globals.Max_Perso_Y = 435;
												_globals.NOSPRECRAN = true;
												_objectsManager.PERSONAGE2("IM97", "IM97", "ANIM97", "IM97", 19);
												if (_globals.SORTIE == 18) {
													_globals.iRegul = 1;
													_soundManager.WSOUND_OFF();
													_graphicsManager.DD_Lock();
													_graphicsManager.Cls_Video();
													_graphicsManager.DD_Unlock();
													_graphicsManager.Cls_Pal();
													_soundManager.WSOUND(6);
													if (_globals.SVGA == 2)
														_animationManager.PLAY_ANM("PURG1A.ANM", 12, 18, 50);
													if (_globals.SVGA == 1)
														_animationManager.PLAY_ANM("PURG1.ANM", 12, 18, 50);
													_graphicsManager.FADE_OUTS();
													goto LABEL_231;
												}
												break;
											case 98:
												_globals.Max_Propre = 5;
												_globals.Max_Ligne_Long = 5;
												_globals.Max_Propre_Gen = 5;
												_globals.Max_Perso_Y = 435;
												_objectsManager.PERSONAGE2("IM98", "IM98", "ANIM98", "IM98", 19);
												break;
											case 99:
												_globals.Max_Propre = 5;
												_globals.Max_Ligne_Long = 5;
												_globals.Max_Propre_Gen = 5;
												_globals.Max_Perso_Y = 435;
												_objectsManager.PERSONAGE2("IM99", "IM99", "ANIM99", "IM99", 19);
												break;
											case 96:
												_globals.Max_Propre = 5;
												_globals.Max_Ligne_Long = 5;
												_globals.Max_Propre_Gen = 5;
												_globals.Max_Perso_Y = 435;
												_globals.NOSPRECRAN = true;
												_objectsManager.PERSONAGE2("IM96", "IM96", "ANIM96", "IM96", 19);
												break;
											case 77:
												OCEAN(77, "OCEAN01", "OCEAN1", 3, 0, 84, 0, 0, 25);
												break;
											case 78:
												OCEAN(78, "OCEAN02", "OCEAN1", 1, 0, 91, 84, 0, 25);
												break;
											case 79:
												OCEAN(79, "OCEAN03", "OCEAN1", 7, 87, 0, 0, 83, 25);
												break;
											case 80:
												OCEAN(80, "OCEAN04", "OCEAN1", 1, 86, 88, 0, 81, 25);
												break;
											case 81:
												OCEAN(81, "OCEAN05", "OCEAN1", 1, 91, 82, 80, 85, 25);
												break;
											case 82:
												OCEAN(82, "OCEAN06", "OCEAN1", 7, 81, 0, 88, 0, 25);
												break;
											case 83:
												OCEAN(83, "OCEAN07", "OCEAN1", 1, 89, 0, 79, 88, 25);
												break;
											case 84:
												OCEAN(84, "OCEAN08", "OCEAN1", 1, 77, 0, 0, 78, 25);
												break;
											case 85:
												OCEAN(85, "OCEAN09", "OCEAN1", 1, 0, 0, 81, 0, 25);
												break;
											case 86:
												OCEAN(86, "OCEAN10", "OCEAN1", 1, 0, 80, 0, 91, 25);
												break;
											case 87:
												OCEAN(87, "OCEAN11", "OCEAN1", 3, 0, 79, 90, 0, 25);
												break;
											case 88:
												OCEAN(88, "OCEAN12", "OCEAN1", 1, 80, 0, 83, 82, 25);
												break;
											case 89:
												OCEAN(89, "OCEAN13", "OCEAN1", 3, 0, 83, 0, 0, 25);
												break;
											case 91:
												OCEAN(91, "OCEAN15", "OCEAN1", 3, 78, 81, 86, 0, 25);
												break;
											case 90:
												BASED();
												break;
											default:
												if (_globals.SORTIE == 111) {
													_globals.NOSPRECRAN = true;
													_objectsManager.PERSONAGE("IM111", "IM111", "ANIM111", "IM111", 10);
													goto LABEL_239;
												}
												if (_globals.SORTIE == 112) {
													_globals.NOSPRECRAN = true;
													_objectsManager.PERSONAGE("IM112", "IM112", "ANIM112", "IM112", 10);
LABEL_239:
													_globals.NOSPRECRAN = false;
												} else if (_globals.SORTIE == 113) {
													_globals.SORTIE = 0;
													_globals.OLD_ECRAN = _globals.ECRAN;
													*((byte *)_globals.SAUVEGARDE + 6) = _globals.ECRAN;
													_globals.ECRAN = 113;
													*((byte *)_globals.SAUVEGARDE + 5) = 113;
													_menuManager.COMPUT_HOPKINS(1);
													_graphicsManager.DD_Lock();
													_graphicsManager.Cls_Video();
													_graphicsManager.DD_Unlock();
													_graphicsManager.DD_VBL();
													memset(_graphicsManager.VESA_BUFFER, 0, 0x4B000u);
													memset(_graphicsManager.VESA_SCREEN, 0, 0x4B000u);
													_graphicsManager.Cls_Pal();
													_graphicsManager.RESET_SEGMENT_VESA();
												} else {
													if (_globals.SORTIE == 114) {
														_globals.SORTIE = 0;
														_globals.OLD_ECRAN = _globals.ECRAN;
														*((byte *)_globals.SAUVEGARDE + 6) = _globals.ECRAN;
														_globals.ECRAN = 114;
														*((byte *)_globals.SAUVEGARDE + 5) = 114;
														_menuManager.COMPUT_HOPKINS(2);
														goto LABEL_243;
													}
													switch (_globals.SORTIE) {
													case 115:
														_globals.SORTIE = 0;
														_globals.OLD_ECRAN = _globals.ECRAN;
														*((byte *)_globals.SAUVEGARDE + 6) = _globals.ECRAN;
														_globals.ECRAN = 115;
														*((byte *)_globals.SAUVEGARDE + 5) = 115;
														_menuManager.COMPUT_HOPKINS(3);
LABEL_243:
														_graphicsManager.DD_Lock();
														_graphicsManager.Cls_Video();
														_graphicsManager.DD_Unlock();
														break;
													case 100:
														JOUE_FIN();
														break;
													case 50:
														AVION();
														_globals.SORTIE = 51;
														break;
													default:
														if ((unsigned __int16)(_globals.SORTIE - 194) > 5u) {
															if (_globals.SORTIE == 151) {
																_soundManager.WSOUND(16);
																_globals.iRegul = 1;
																_graphicsManager.DD_Lock();
																_graphicsManager.Cls_Video();
																_graphicsManager.DD_Unlock();
																_graphicsManager.Cls_Pal();
																_graphicsManager.FADE_LINUX = 2;
																_animationManager.PLAY_ANM("JOUR3A.anm", 12, 12, 2000);
																_globals.iRegul = 0;
																_globals.SORTIE = 300;
															}
															if (_globals.SORTIE == 150) {
																_soundManager.WSOUND(16);
																_globals.iRegul = 1;
																_graphicsManager.DD_Lock();
																_graphicsManager.Cls_Video();
																_graphicsManager.DD_Unlock();
																_graphicsManager.Cls_Pal();
																_graphicsManager.FADE_LINUX = 2;
																_animationManager.PLAY_ANM("JOUR1A.anm", 12, 12, 2000);
																_globals.iRegul = 0;
																_globals.SORTIE = 300;
															}
															if (_globals.SORTIE == 152) {
																_soundManager.WSOUND(16);
																_globals.iRegul = 1;
																_graphicsManager.DD_Lock();
																_graphicsManager.Cls_Video();
																_graphicsManager.DD_Unlock();
																_graphicsManager.Cls_Pal();
																_graphicsManager.FADE_LINUX = 2;
																_animationManager.PLAY_ANM("JOUR4A.anm", 12, 12, 2000);
																_globals.iRegul = 0;
																_globals.SORTIE = 300;
															}
															goto LABEL_10;
														}
														_globals.PERSO = _globals.dos_free2(_globals.PERSO);
														_globals.iRegul = 1;
														_soundManager.WSOUND(23);
														_globals.SORTIE = PWBASE();
														_soundManager.WSOUND_OFF();
														_fileManager.CONSTRUIT_SYSTEM("PERSO.SPR");
														_globals.PERSO = _fileManager.CHARGE_FICHIER(_globals.NFICHIER);
														_globals.PERSO_TYPE = 0;
														_globals.iRegul = 0;
														_graphicsManager.nbrligne = 640;
														break;
													}
												}
												break;
											}
										}
									}
								}
								break;
							}
						}
						break;
					}
					break;
				}
				break;
			}
		}
	}
	return true;
}

bool HopkinsEngine::shouldQuit() const {
	return g_system->getEventManager()->shouldQuit();
}

int HopkinsEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

void HopkinsEngine::processIniParams(Common::StringMap &iniParams) {
	_globals.XFULLSCREEN = iniParams["FULLSCREEN"] == "YES";

	_globals.XSETMODE = 1;
	if (iniParams.contains("SETMODE")) {
		int setMode = atoi(iniParams["SETMODE"].c_str());
		_globals.XSETMODE = CLIP(setMode, 1, 5);
	}

	_globals.XZOOM = 0;
	if (_globals.XSETMODE == 5 && iniParams.contains("ZOOM")) {
		int zoom = atoi(iniParams["ZOOM"].c_str());
		_globals.XZOOM = CLIP(zoom, 25, 100);
	}

	_globals.XFORCE16 = iniParams["FORCE16BITS"] == "YES";
	_globals.XFORCE8 = iniParams["FORCE8BITS"] == "YES";
	_globals.CARD_SB = iniParams["SOUND"] == "YES";
}

void HopkinsEngine::INIT_SYSTEM() {
	// Set graphics mode
	_graphicsManager.SET_MODE(640, 480);

	// Synchronise the sound settings from ScummVM
	_soundManager.syncSoundSettings();

	if (getPlatform() == Common::kPlatformLinux)
		_eventsManager.mouse_linux = true;
	else
		_eventsManager.mouse_linux = false;

	switch (_globals.FR) {
	case 0:
		if (!_eventsManager.mouse_linux)
			_fileManager.CONSTRUIT_SYSTEM("SOUAN.SPR");
		else 
			_fileManager.CONSTRUIT_SYSTEM("LSOUAN.SPR");
		break;
	case 1:
		if (!_eventsManager.mouse_linux)
			_fileManager.CONSTRUIT_SYSTEM("SOUFR.SPR");
		else
			_fileManager.CONSTRUIT_SYSTEM("LSOUFR.SPR");
		break;
	case 2:
		_fileManager.CONSTRUIT_SYSTEM("SOUES.SPR");
		break;
	}
  
	if (_eventsManager.mouse_linux) {
		_eventsManager.souris_sizex = 52;
		_eventsManager.souris_sizey = 32;
	} else {
		_eventsManager.souris_sizex = 34;
		_eventsManager.souris_sizey = 20;
	}
	_eventsManager.pointeur_souris = _fileManager.CHARGE_FICHIER(_globals.NFICHIER);

	_globals.clearAll();

	_fileManager.CONSTRUIT_SYSTEM("FONTE3.SPR");
	_globals.police = _fileManager.CHARGE_FICHIER(_globals.NFICHIER);
	_globals.police_l = 12;
	_globals.police_h = 21;
	_fileManager.CONSTRUIT_SYSTEM("ICONE.SPR");
	_globals.ICONE = _fileManager.CHARGE_FICHIER(_globals.NFICHIER);
	_fileManager.CONSTRUIT_SYSTEM("TETE.SPR");
	_globals.TETE = _fileManager.CHARGE_FICHIER(_globals.NFICHIER);
	
	switch (_globals.FR) {
	case 0:
		_fileManager.CONSTRUIT_FICHIER(_globals.HOPLINK, "ZONEAN.TXT");
		_globals.BUF_ZONE = _fileManager.CHARGE_FICHIER(_globals.NFICHIER);
		break;
	case 1:
		_fileManager.CONSTRUIT_FICHIER(_globals.HOPLINK, "ZONE01.TXT");
		_globals.BUF_ZONE = _fileManager.CHARGE_FICHIER(_globals.NFICHIER);
		break;
	case 2:
		_fileManager.CONSTRUIT_FICHIER(_globals.HOPLINK, "ZONEES.TXT");
		_globals.BUF_ZONE = _fileManager.CHARGE_FICHIER(_globals.NFICHIER);
		break;
	}

	_eventsManager.INSTALL_SOURIS();
	_eventsManager.souris_on();
	_eventsManager.souris_flag = false;
	_eventsManager.souris_max();

	_globals.HOPKINS_DATA();

	_eventsManager.ofset_souris_x = 0;
	_eventsManager.ofset_souris_y = 0;
	_globals.lItCounter = 0;
	_globals.lOldItCounter = 0;
}

void HopkinsEngine::INTRORUN() {
	signed int v2;
	signed int v3;
	uint16 v4;
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
	int v21;
	char v22; 
	char v23; 
	byte paletteData[PALETTE_EXT_BLOCK_SIZE];
	byte paletteData2[PALETTE_EXT_BLOCK_SIZE];

	memset(&paletteData, 0, PALETTE_EXT_BLOCK_SIZE);
	_eventsManager.VBL();
	_eventsManager.souris_flag = false;
	_globals.iRegul = 1;
	_eventsManager.VBL();
	_soundManager.WSOUND(16);
	_animationManager.CLS_ANM = true;
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
				_globals.BPP_NOAFF = true;
				v2 = 0;
				do {
					_eventsManager.VBL();
					++v2;
				} while (v2 <= 4);

				_globals.BPP_NOAFF = false;
				_globals.iRegul = 1;
				_graphicsManager.FADE_INW();
				if (_graphicsManager.DOUBLE_ECRAN == true) {
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
					} while (!shouldQuit() && v3 != 1 && _graphicsManager.SCROLL != SCREEN_WIDTH);
          
					_eventsManager.VBL();
					_graphicsManager.no_scroll = 0;

					if (shouldQuit())
						return;
				}
        
				_soundManager.VOICE_MIX(4, 3);
				_graphicsManager.FADE_OUTW();
				_graphicsManager.no_scroll = 0;
				_graphicsManager.LOAD_IMAGE("intro2");
				_graphicsManager.SCROLL_ECRAN(0);
				_animationManager.CHARGE_ANIM("INTRO2");
				_graphicsManager.VISU_ALL();
				_soundManager.WSOUND(23);
				_objectsManager.BOBANIM_OFF(3);
				_objectsManager.BOBANIM_OFF(5);
				_graphicsManager.ofscroll = 0;
				_graphicsManager.SETCOLOR3(252, 100, 100, 100);
				_graphicsManager.SETCOLOR3(253, 100, 100, 100);
				_graphicsManager.SETCOLOR3(251, 100, 100, 100);
				_graphicsManager.SETCOLOR3(254, 0, 0, 0);
				_globals.BPP_NOAFF = true;
				v5 = 0;
        
				do {
					_eventsManager.VBL();
					++v5;
				} while (v5 <= 4);
        
				_globals.BPP_NOAFF = false;
				_globals.iRegul = 1;
				_graphicsManager.FADE_INW();
				for (i = 0; i < 200 / _globals.vitesse; ++i)
					_eventsManager.VBL();
        
				_objectsManager.BOBANIM_ON(3);
				_soundManager.VOICE_MIX(5, 3);
				_objectsManager.BOBANIM_OFF(3);
				_eventsManager.VBL();
				memcpy(&paletteData2, _graphicsManager.Palette, 796);
				v21 = *(uint16 *)&_graphicsManager.Palette[796];
				v22 = _graphicsManager.Palette[798];
				v7 = (int)&v23;
				_graphicsManager.setpal_vga256_linux(paletteData, _graphicsManager.VESA_BUFFER);
				_graphicsManager.FIN_VISU();

				if (shouldQuit())
					return;

				_soundManager.SPECIAL_SOUND = 5;
				_graphicsManager.FADE_LINUX = 2;
				_animationManager.PLAY_ANM("ELEC.ANM", 10, 26, 200);
				if (shouldQuit())
					return;

				_soundManager.SPECIAL_SOUND = 0;
        
				if (!_eventsManager.ESC_KEY) {
					_graphicsManager.LOAD_IMAGE("intro2");
					_graphicsManager.SCROLL_ECRAN(0);
					_animationManager.CHARGE_ANIM("INTRO2");
					_graphicsManager.VISU_ALL();
					_soundManager.WSOUND(23);
					_objectsManager.BOBANIM_OFF(3);
					_objectsManager.BOBANIM_OFF(5);
					_objectsManager.BOBANIM_OFF(1);
					_graphicsManager.ofscroll = 0;
					_graphicsManager.SETCOLOR3(252, 100, 100, 100);
					_graphicsManager.SETCOLOR3(253, 100, 100, 100);
					_graphicsManager.SETCOLOR3(251, 100, 100, 100);
					_graphicsManager.SETCOLOR3(254, 0, 0, 0);
					_globals.BPP_NOAFF = true;
					v8 = 0;
          
					do {
						_eventsManager.VBL();
						++v8;
					} while (v8 <= 3);
          
					_globals.BPP_NOAFF = false;
					_globals.iRegul = 1;
					_graphicsManager.setpal_vga256_linux(paletteData2, _graphicsManager.VESA_BUFFER);
					v9 = 0;
          
					while (!shouldQuit() && !_eventsManager.ESC_KEY) {
						if (v9 == 12) {
							_objectsManager.BOBANIM_ON(3);
							_eventsManager.VBL();
							_soundManager.VOICE_MIX(6, 3);
							_eventsManager.VBL();
							_objectsManager.BOBANIM_OFF(3);
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
              
							_objectsManager.BOBANIM_ON(3);
							_soundManager.VOICE_MIX(7, 3);
							_objectsManager.BOBANIM_OFF(3);
							
							for (k = 1; k < 60 / _globals.vitesse; ++k)
								_eventsManager.VBL();
							_objectsManager.BOBANIM_ON(5);
							for (l = 0; l < 20 / _globals.vitesse; ++l)
								_eventsManager.VBL();

							Common::copy(&paletteData2[0], &paletteData2[PALETTE_BLOCK_SIZE], &_graphicsManager.Palette[0]);
							_graphicsManager.setpal_vga256_linux(_graphicsManager.Palette, _graphicsManager.VESA_BUFFER);
              
							for (m = 0; m < 50 / _globals.vitesse; ++m) {
								if (m == 30 / _globals.vitesse) {
									_objectsManager.BOBANIM_ON(3);
									_soundManager.VOICE_MIX(8, 3);
									_objectsManager.BOBANIM_OFF(3);
								}
                
								_eventsManager.VBL();
							}

							_graphicsManager.FADE_OUTW();
							_graphicsManager.FIN_VISU();
							_animationManager.CLS_ANM = true;
							_soundManager.WSOUND(3);
							_soundManager.SPECIAL_SOUND = 1;
							_animationManager.PLAY_ANM("INTRO1.anm", 10, 24, 18);
							if (shouldQuit())
								return;

							_soundManager.SPECIAL_SOUND = 0;

							if (!_eventsManager.ESC_KEY) {
								_animationManager.PLAY_ANM("INTRO2.anm", 10, 24, 18);
								if (shouldQuit())
									return;
                
								if (!_eventsManager.ESC_KEY) {
									_animationManager.PLAY_ANM("INTRO3.anm", 10, 24, 200);
									if (shouldQuit())
										return;

									if (!_eventsManager.ESC_KEY) {
										_animationManager.CLS_ANM = false;
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
	_objectsManager.BOBANIM_OFF(7);
	_globals.BPP_NOAFF = true;

	for (int idx = 0; idx < 5; ++idx) {
		_eventsManager.VBL();
	}
  
	_globals.BPP_NOAFF = false;
	_graphicsManager.FADE_INW();
	_eventsManager.MOUSE_OFF();
	
	for (int idx = 0; idx < 20; ++idx) {
		_eventsManager.VBL();
	}
  
	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO2("vire.pe2");
	_globals.NOPARLE = false;
	_objectsManager.BOBANIM_ON(7);

	for (int idx = 0; idx < 100; ++idx) {
		_eventsManager.VBL();
	}

	_graphicsManager.FADE_OUTW();
	_graphicsManager.FIN_VISU();
	_globals.iRegul = 0;
	_globals.SORTIE = 151;
}

void HopkinsEngine::REST_SYSTEM() {
	quitGame();
	_eventsManager.CONTROLE_MES();
}

void HopkinsEngine::PUBQUIT() {
	_globals.PUBEXIT = true;
	_graphicsManager.RESET_SEGMENT_VESA();
	_globals.FORET = 0;
	_eventsManager.CASSE = false;
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

void HopkinsEngine::INCENDIE() {
	warning("INCENDIE()");

	_globals.DESACTIVE_INVENT = true;
	_globals.iRegul = 1;
	_graphicsManager.LOAD_IMAGE("IM71");
	_animationManager.CHARGE_ANIM("ANIM71");
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_graphicsManager.VISU_ALL();
	_globals.BPP_NOAFF = true;

	int cpt = 0;
	do {
		_eventsManager.VBL();
		++cpt;
	} while (cpt <= 4);

	_globals.BPP_NOAFF = false;
	_graphicsManager.FADE_INW();
	_globals.iRegul = 1;

	cpt = 0;
	do {
		_eventsManager.VBL();
		++cpt;
	} while (cpt <= 249);

	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO("SVGARD1.pe2");
	_globals.NOPARLE = false;

	cpt = 0;
	do {
		_eventsManager.VBL();
		++cpt;
	} while (cpt <= 49);

	_graphicsManager.FADE_OUTW();
	_graphicsManager.FIN_VISU();
	*((byte *)_globals.SAUVEGARDE + 312) = 1;
	_globals.DESACTIVE_INVENT = false;
}

void HopkinsEngine::BASE() {
	warning("BASE()");

	_globals.iRegul = 1;
	_graphicsManager.nbrligne = 640;
	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();
	_graphicsManager.Cls_Pal();
	_animationManager.CLS_ANM = true;
	_soundManager.WSOUND(25);
	if (_globals.SVGA == 1) {
		_animationManager.PLAY_ANM("base00.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("base05.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("base10.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("base20.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("base30.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("base40.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("base50.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("OC00.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("OC05.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("OC10.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("OC20.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY) {
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.PLAY_ANM("OC30.anm", 10, 18, 18);
		}
	}
	if (_globals.SVGA == 2) {
		_animationManager.PLAY_ANM("base00a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("base05a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("base10a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("base20a.anm", 10, 18, 18);
		// CHECKME: The original code was doing the opposite test, which looks like a bug.
		if (!_eventsManager.ESC_KEY) 
			_animationManager.PLAY_ANM("base30a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("base40a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("base50a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("OC00a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("OC05a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("OC10a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("OC20a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY) {
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.PLAY_ANM("OC30a.anm", 10, 18, 18);
		}
	}

	_eventsManager.ESC_KEY = false;
	_animationManager.CLS_ANM = false;
	_globals.SORTIE = 85;
}

void HopkinsEngine::BASED() {
	warning("BASED()");

	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();
	_graphicsManager.Cls_Pal();
	_animationManager.NO_SEQ = false;
	_soundManager.WSOUND(26);
	_globals.iRegul = 1;
	_globals.DESACTIVE_INVENT = true;
	_animationManager.NO_COUL = true;
	_graphicsManager.FADE_LINUX = 2;
	_animationManager.PLAY_SEQ("abase.seq", 50, 15, 50);
	_animationManager.NO_COUL = false;
	_graphicsManager.LOAD_IMAGE("IM92");
	_animationManager.CHARGE_ANIM("ANIM92");
	_graphicsManager.VISU_ALL();
	_objectsManager.INILINK("IM92");
	_globals.BPP_NOAFF = true;

	int cpt = 0;
	do {
		_eventsManager.VBL();
		++cpt;
	} while (cpt <= 4);

	_globals.BPP_NOAFF = false;
	_graphicsManager.FADE_INW();
	_globals.CACHE_ON();

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(8) != 22);

	_graphicsManager.FADE_OUTW();
	_graphicsManager.FIN_VISU();
	_globals.RESET_CACHE();
	_globals.DESACTIVE_INVENT = false;
	_globals.SORTIE = 93;
	_globals.iRegul = 0;
}

void HopkinsEngine::JOUE_FIN() {
	warning("JOUE_FIN()");

	_globals.PERSO = _globals.dos_free2(_globals.PERSO);
	_dialogsManager.VIRE_INVENT = true;
	_globals.DESACTIVE_INVENT = true;
	_graphicsManager.ofscroll = 0;
	_globals.PLAN_FLAG = false;
	_globals.iRegul = 1;
	_soundManager.WSOUND(26);
	_globals.chemin = (int16 *)g_PTRNUL;
	_globals.NOMARCHE = true;
	_globals.SORTIE = 0;
	_globals.AFFLI = false;
	_globals.AFFIVBL = false;
	_soundManager.CHARGE_SAMPLE(1, "SOUND90.WAV");
	_graphicsManager.LOAD_IMAGE("IM100");
	_animationManager.CHARGE_ANIM("ANIM100");
	_graphicsManager.VISU_ALL();
	_eventsManager.MOUSE_ON();
	_objectsManager.BOBANIM_OFF(7);
	_objectsManager.BOBANIM_OFF(8);
	_objectsManager.BOBANIM_OFF(9);
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_eventsManager.CHANGE_MOUSE(0);
	_globals.BPP_NOAFF = true;

	int cpt = 0;
	do {
		_eventsManager.VBL();
		++cpt;
	} while (cpt <= 4);

	_globals.BPP_NOAFF = false;
	_graphicsManager.FADE_INW();
	_globals.iRegul = 1;

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(6) != 54);

	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO("GM4.PE2");
	_globals.DESACTIVE_INVENT = true;
	_objectsManager.BOBANIM_OFF(6);
	_objectsManager.BOBANIM_OFF(10);
	_objectsManager.BOBANIM_ON(9);
	_objectsManager.BOBANIM_ON(7);

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(7) != 54);

	_soundManager.PLAY_SAMPLE2(1);

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(7) != 65);

	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO("DUELB4.PE2");
	_eventsManager.MOUSE_OFF();
	_globals.DESACTIVE_INVENT = true;

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(7) != 72);

	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO("DUELH1.PE2");

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(7) != 81);

	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO("DUELB5.PE2");

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(7) != 120);

	_objectsManager.BOBANIM_OFF(7);
	if (*((byte *)_globals.SAUVEGARDE + 135) == 1) {
		_soundManager.SPECIAL_SOUND = 200;
		_soundManager.VBL_MERDE = true;
		_graphicsManager.FADE_LINUX = 2;
		_animationManager.PLAY_ANM("BERM.ANM", 100, 24, 300);
		_graphicsManager.FIN_VISU();
		_soundManager.DEL_SAMPLE(1);
		_graphicsManager.LOAD_IMAGE("PLAN3");
		_graphicsManager.FADE_INW();
		_globals.lItCounter = 0;
		if (!_eventsManager.ESC_KEY) {
			do
				_eventsManager.CONTROLE_MES();
			while (_globals.lItCounter < 2000 / _globals.vitesse && !_eventsManager.ESC_KEY);
		}
		_eventsManager.ESC_KEY = false;
		_graphicsManager.FADE_OUTW();
		_globals.iRegul = 1;
		_soundManager.SPECIAL_SOUND = 0;
		_graphicsManager.FADE_LINUX = 2;
		_animationManager.PLAY_ANM("JOUR2A.anm", 12, 12, 1000);
		_soundManager.WSOUND(11);
		_graphicsManager.DD_Lock();
		_graphicsManager.Cls_Video();
		_graphicsManager.DD_Unlock();
		_graphicsManager.Cls_Pal();
		_animationManager.PLAY_ANM("FF1a.anm", 18, 18, 9);
		_animationManager.PLAY_ANM("FF1a.anm", 9, 18, 9);
		_animationManager.PLAY_ANM("FF1a.anm", 9, 18, 18);
		_animationManager.PLAY_ANM("FF1a.anm", 9, 18, 9);
		_animationManager.PLAY_ANM("FF2a.anm", 24, 24, 100);
		Credits();
		_globals.iRegul = 0;
		_globals.SORTIE = 300;
		_dialogsManager.VIRE_INVENT = false;
		_globals.DESACTIVE_INVENT = false;
	} else {
		_soundManager.SPECIAL_SOUND = 200;
		_soundManager.VBL_MERDE = true;
		_animationManager.PLAY_ANM2("BERM.ANM", 100, 24, 300);
		_objectsManager.BOBANIM_OFF(7);
		_objectsManager.BOBANIM_ON(8);
		_globals.NOPARLE = true;
		_talkManager.PARLER_PERSO("GM5.PE2");
		_globals.DESACTIVE_INVENT = true;

		do
			_eventsManager.VBL();
		while (_objectsManager.BOBPOSI(8) != 5);

		_soundManager.PLAY_SOUND2("SOUND41.WAV");

		do
			_eventsManager.VBL();
		while (_objectsManager.BOBPOSI(8) != 21);

		_graphicsManager.FADE_OUTW();
		_graphicsManager.FIN_VISU();
		_soundManager.DEL_SAMPLE(1);
		_soundManager.WSOUND(16);
		_globals.iRegul = 1;
		_soundManager.SPECIAL_SOUND = 0;
		_dialogsManager.VIRE_INVENT = false;
		_globals.DESACTIVE_INVENT = false;
		_animationManager.PLAY_ANM("JOUR4A.anm", 12, 12, 1000);
		_globals.iRegul = 0;
		_globals.SORTIE = 300;
	}
	_fileManager.CONSTRUIT_SYSTEM("PERSO.SPR");
	_globals.PERSO = _fileManager.CHARGE_FICHIER(_globals.NFICHIER);
	_globals.PERSO_TYPE = 0;
	_globals.iRegul = 0;
}

void HopkinsEngine::AVION() {
	warning("AVION()");

	_soundManager.WSOUND(28);
	_globals.iRegul = 1;
	_globals.nbrligne = 640;
	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();
	_graphicsManager.Cls_Pal();

	_animationManager.CLS_ANM = false;
	if (_globals.SVGA == 1) {
		_animationManager.PLAY_ANM("aerop00.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop10.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop20.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop30.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop40.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop50.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop60.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop70.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans00.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans10.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans15.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans20.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans30.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans40.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY) {
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.PLAY_ANM("PARA00.anm", 9, 9, 9);
		}
	} else if (_globals.SVGA == 2) {
		_animationManager.PLAY_ANM("aerop00a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("serop10a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop20a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop30a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop40a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop50a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop60a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("aerop70a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans00a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans10a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans15a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans20a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans30a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.PLAY_ANM("trans40a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY) {
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.PLAY_ANM("PARA00a.anm", 9, 9, 9);
		}
	}

	_eventsManager.ESC_KEY = 0;
	_animationManager.CLS_ANM = 0;
}

int HopkinsEngine::PWBASE() {
	warning("PWBASE()");

	_globals.DESACTIVE_INVENT = true;
	_graphicsManager.LOAD_IMAGE("PBASE");
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_eventsManager.CHANGE_MOUSE(0);
	_graphicsManager.FADE_INW();
	bool loopCond = false;
	int zone;
	do {
		int mouseButton = _eventsManager.BMOUSE();
		int posX = _eventsManager.XMOUSE();
		int posY = _eventsManager.YMOUSE();
		zone = 0;
		if ((posX - 181 <= 16) && (posY - 66 <= 22) &&
		    (posX - 181 >= 0) && (posY - 66 >= 0))
			zone = 1;
		if ((posX - 353 <= 22) && (posY - 116 <= 19) &&
		    (posX - 353 >= 0) && (posY - 116 >= 0))
			zone = 2;
		if ((posX - 483 <= 20) && (posY - 250 <= 25) &&
		    (posX - 483 >= 0) && (posY - 250 >= 0))
			zone = 3;
		if ((posX - 471 <= 27) && (posY - 326 <= 20) &&
		    (posX - 471 >= 0) && (posY - 326 >= 0))
			zone = 4;
		if ((posX - 162 <= 21) && (posY - 365 <= 23) &&
		    (posX - 162 >= 0) && (posY - 365 >= 0))
			zone = 5;
		if ((posX - 106 <= 20) && (posY - 267 <= 26) &&
		    (posX - 106 >= 0) && (posY - 267 >= 0))
			zone = 6;
		if (zone) {
			_eventsManager.CHANGE_MOUSE(4);
			_globals.couleur_40 += 25;
			if (_globals.couleur_40 > 100)
				_globals.couleur_40 = 0;
			_graphicsManager.SETCOLOR4(251, _globals.couleur_40, _globals.couleur_40, _globals.couleur_40);
		} else {
			_eventsManager.CHANGE_MOUSE(0);
			_graphicsManager.SETCOLOR4(251, 100, 100, 100);
		}
		_eventsManager.VBL();
		if ((mouseButton == 1) && zone)
			loopCond = true;
	} while (!loopCond);

	_globals.DESACTIVE_INVENT = false;
	_graphicsManager.FADE_OUTW();

	int result;
	switch (zone) {
	case 1:
		result = 94;
		break;
	case 2:
		result = 95;
		break;
	case 3:
		result = 96;
		break;
	case 4:
		result = 97;
		break;
	case 5:
		result = 98;
		break;
	case 6:
		result = 99;
		break;
	default:
		result = 0;
		break;
	}
	return result;
}

void HopkinsEngine::Charge_Credits() {
	warning("STUB - Charge_Credits()");
}

void HopkinsEngine::CREDIT_AFFICHE(int startPosY, byte *buffer, char colour) {
	warning("CREDIT_AFFICHE");

	byte *v3 = buffer;
	byte *v4 = buffer;
	int strWidth = 0;
	byte curChar;
	while (1) {
		curChar = *v4++;
		if (!curChar)
			break;
		if (curChar > 31)
			strWidth += _objectsManager.Get_Largeur(_globals.police, curChar - 32);
	}
	int startPosX = 320 - strWidth / 2;
	int endPosX = strWidth + startPosX;
	int endPosY = startPosY + 12;
	if ((_globals.Credit_bx == -1) && (_globals.Credit_bx1 == -1) && (_globals.Credit_by == -1) && (_globals.Credit_by1 == -1)) {
		_globals.Credit_bx = startPosX;
		_globals.Credit_bx1 = endPosX;
		_globals.Credit_by = startPosY;
		_globals.Credit_by1 = endPosY;
	}
	if (startPosX < _globals.Credit_bx)
		_globals.Credit_bx = startPosX;
	if (endPosX > _globals.Credit_bx1)
		_globals.Credit_bx1 = endPosX;
	if (_globals.Credit_by > startPosY)
		_globals.Credit_by = startPosY;
	if (endPosY > _globals.Credit_by1)
		_globals.Credit_by1 = endPosY;

	while (1) {
		curChar = *v3++;
		if (!curChar)
			break;
		if (curChar > 31) {
			_graphicsManager.Affiche_Fonte(_graphicsManager.VESA_BUFFER, _globals.police, startPosX, startPosY, curChar - 32, colour);
			startPosX += _objectsManager.Get_Largeur(_globals.police, curChar - 32);
		}
	}
}

void HopkinsEngine::Credits() {
	warning("Credits()");

	Charge_Credits();
	_globals.Credit_y = 436;
	_graphicsManager.LOAD_IMAGE("GENERIC");
	_graphicsManager.FADE_INW();
	_soundManager.WSOUND(28);
	_eventsManager.souris_flag = false;
	_globals.iRegul = 3;
	_globals.Credit_bx = _globals.Credit_bx1 = _globals.Credit_by = _globals.Credit_by1 = -1;
	int soundId = 28;
	do {
		for (int i = 0; i < _globals.Credit_lignes; ++i) {
			if (_globals.Credit[60 * i] == 1) {
				int nextY = _globals.Credit_y + i * _globals.Credit_step;
//				*(_DWORD *)&Credit[60 * i + 4] = nextY;
				_globals.Credit[60 * i + 4] = nextY & 0xFF;
				_globals.Credit[60 * i + 5] = (nextY >> 8) & 0xFF;
				_globals.Credit[60 * i + 6] = (nextY >> 16) & 0xFF;
				_globals.Credit[60 * i + 7] = (nextY >> 24) & 0xFF;

				if ((nextY - 21  >= 0) && (nextY - 21 <= 418)) {
					char tmpVal = _globals.Credit[60 * i + 1];
					int a1 = 0;
					if (tmpVal == 49)
						a1 = 163;
					if (tmpVal == 50)
						a1 = 161;
					if (tmpVal == 51)
						a1 = 162;
//					if (*(_WORD *)&Credit[60 * i + 8] != -1)
					if ((_globals.Credit[60 * i + 8] != 0xFF) && _globals.Credit[60 * i + 9] != 0xFF)
						CREDIT_AFFICHE(nextY, _globals.Credit + (60 * i + 8), a1);
				}
			}
		}
		--_globals.Credit_y;
		if (_globals.Credit_bx != -1 || _globals.Credit_bx1 != -1 || _globals.Credit_by != -1 || _globals.Credit_by1 != -1) {
			_eventsManager.VBL();
			_graphicsManager.SCOPY(_graphicsManager.VESA_SCREEN, 60, 50, 520, 380, _graphicsManager.VESA_BUFFER, 60, 50);
		} else {
			_eventsManager.VBL();
		}
//		if (*(_DWORD *)&Credit[20 * (3 * Credit_lignes - 3) + 4] <= 39) {
		if ( _globals.Credit[20 * (3 * _globals.Credit_lignes - 3) + 4] +
			(_globals.Credit[20 * (3 * _globals.Credit_lignes - 3) + 4] << 8) +
			(_globals.Credit[20 * (3 * _globals.Credit_lignes - 3) + 4] << 16) +
			(_globals.Credit[20 * (3 * _globals.Credit_lignes - 3) + 4] << 24) <= 39) {
			_globals.Credit_y = 440;
			++soundId;
			if (soundId > 31)
				soundId = 28;
			_soundManager.WSOUND(soundId);
		}
		_globals.Credit_bx = -1;
		_globals.Credit_bx1 = -1;
		_globals.Credit_by = -1;
		_globals.Credit_by1 = -1;
	} while (_eventsManager.BMOUSE() != 1);
	_graphicsManager.FADE_OUTW();
	_globals.iRegul = 1;
	_eventsManager.souris_flag = true;
}

void HopkinsEngine::BTOCEAN() {
	warning("BTOCEAN()");

	_fontManager.TEXTE_OFF(9);
	if (_eventsManager.btsouris == 16) {
		_eventsManager.XMOUSE();
		if (_objectsManager.NUMZONE > 0) {
			int oldPosX = _eventsManager.XMOUSE();
			int oldPosY = _eventsManager.YMOUSE();
			bool displAnim = false;
			if (_objectsManager.NUMZONE == 1) {
				if (_globals.OCEAN_SENS == 3)
					_objectsManager.SPACTION(_globals.PERSO, "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,-1,", 0, 0, 6, 0);
				if (_globals.OCEAN_SENS == 1)
					_objectsManager.SPACTION(_globals.PERSO, "27,26,25,24,23,22,21,20,19,18,-1,", 0, 0, 6, 0);
				if (_globals.OCEAN_SENS == 5)
					_objectsManager.SPACTION(_globals.PERSO, "9,10,11,12,13,14,15,16,17,18,-1,", 0, 0, 6, 0);
				_globals.OCEAN_SENS = 7;
				_globals.SORTIE = 1;
				int oldX = _objectsManager.XSPR(0);
				while (1) {
					if (_globals.vitesse == 1)
						oldX -= 2;
					if (_globals.vitesse == 2)
						oldX -= 4;
					if (_globals.vitesse == 3)
						oldX -= 6;
					_objectsManager.SETXSPR(0, oldX);
					OCEAN_HOME();
					_eventsManager.VBL();
					if (_eventsManager.BMOUSE() == 1) {
						if (oldPosX == _eventsManager.XMOUSE()) {
							if (_eventsManager.YMOUSE() == oldPosY)
								break;
						}
					}
					if (oldX <= -100)
						goto LABEL_22;
				}
				displAnim = true;
			}
LABEL_22:
			if (_objectsManager.NUMZONE == 2) {
				if (_globals.OCEAN_SENS == 7)
					_objectsManager.SPACTION(_globals.PERSO, "18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,-1,", 0, 0, 6, 0);
				if (_globals.OCEAN_SENS == 1)
					_objectsManager.SPACTION(_globals.PERSO, "27,28,29,30,31,32,33,34,35,36,-1,", 0, 0, 6, 0);
				if (_globals.OCEAN_SENS == 5)
					_objectsManager.SPACTION(_globals.PERSO, "9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 6, 0);
				_globals.OCEAN_SENS = 3;
				_globals.SORTIE = 2;
				int oldX = _objectsManager.XSPR(0);
				while (1) {
					if (_globals.vitesse == 1)
						oldX += 2;
					if (_globals.vitesse == 2)
						oldX += 4;
					if (_globals.vitesse == 3)
						oldX += 6;
					_objectsManager.SETXSPR(0, oldX);
					OCEAN_HOME();
					_eventsManager.VBL();
					if (_eventsManager.BMOUSE() == 1) {
						if (oldPosX == _eventsManager.XMOUSE()) {
							if (_eventsManager.YMOUSE() == oldPosY)
								break;
						}
					}
					if (oldX > 499)
						goto LABEL_41;
				}
				displAnim = true;
			}
LABEL_41:
			if (_objectsManager.NUMZONE == 3) {
				if (_globals.OCEAN_SENS == 3) {
					int oldX = _objectsManager.XSPR(0);
					do {
						if (_globals.vitesse == 1)
							oldX += 2;
						if (_globals.vitesse == 2)
							oldX += 4;
						if (_globals.vitesse == 3)
							oldX += 6;
						_objectsManager.SETXSPR(0, oldX);
						OCEAN_HOME();
						_eventsManager.VBL();
						if (_eventsManager.BMOUSE() == 1) {
							if (oldPosX == _eventsManager.XMOUSE()) {
								if (_eventsManager.YMOUSE() == oldPosY) {
									displAnim = true;
									goto LABEL_57;
								}
							}
						}
					} while (oldX <= 235);
					if (!displAnim)
						_objectsManager.SPACTION(_globals.PERSO, "36,35,34,33,32,31,30,29,28,27,-1,", 0, 0, 6, 0);
				}
LABEL_57:
				if (_globals.OCEAN_SENS == 7) {
					int oldX = _objectsManager.XSPR(0);
					do {
						if (_globals.vitesse == 1)
							oldX -= 2;
						if (_globals.vitesse == 2)
							oldX -= 4;
						if (_globals.vitesse == 3)
							oldX -= 6;
						_objectsManager.SETXSPR(0, oldX);
						OCEAN_HOME();
						_eventsManager.VBL();
						if (_eventsManager.BMOUSE() == 1) {
							if (oldPosX == _eventsManager.XMOUSE()) {
								if (_eventsManager.YMOUSE() == oldPosY) {
									displAnim = true;
									goto LABEL_72;
								}
							}
						}
					} while (oldX > 236);
					if (!displAnim)
						_objectsManager.SPACTION(_globals.PERSO, "18,19,20,21,22,23,24,25,26,27,-1,", 0, 0, 6, 0);
				}
LABEL_72:
				if (_globals.OCEAN_SENS == 5)
					_objectsManager.SPACTION(_globals.PERSO, "9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,-1,", 0, 0, 6, 0);
				_globals.OCEAN_SENS = 1;
				_globals.SORTIE = 3;
			}
			if (_objectsManager.NUMZONE == 4) {
				if (_globals.OCEAN_SENS == 3) {
					int oldX = _objectsManager.XSPR(0);
					do {
						if (_globals.vitesse == 1)
							oldX += 2;
						if (_globals.vitesse == 2)
							oldX += 4;
						if (_globals.vitesse == 3)
							oldX += 6;
						_objectsManager.SETXSPR(0, oldX);
						OCEAN_HOME();
						_eventsManager.VBL();
						if (_eventsManager.BMOUSE() == 1) {
							if (oldPosX == _eventsManager.XMOUSE()) {
								if (_eventsManager.YMOUSE() == oldPosY) {
									displAnim = true;
									goto LABEL_91;
								}
							}
						}
					} while (oldX <= 235);
					if (!displAnim)
						_objectsManager.SPACTION(_globals.PERSO, "0,1,2,3,4,5,6,7,8,9,-1,", 0, 0, 6, 0);
				}
LABEL_91:
				if (_globals.OCEAN_SENS == 7) {
					int oldX = _objectsManager.XSPR(0);
					while (1) {
						if (_globals.vitesse == 1)
							oldX -= 2;
						if (_globals.vitesse == 2)
							oldX -= 4;
						if (_globals.vitesse == 3)
							oldX -= 6;
						_objectsManager.SETXSPR(0, oldX);
						OCEAN_HOME();
						_eventsManager.VBL();
						if (_eventsManager.BMOUSE() == 1) {
							if (oldPosX == _eventsManager.XMOUSE()) {
								if (_eventsManager.YMOUSE() == oldPosY)
									break;
							}
						}
						if (oldX <= 236) {
							if (!displAnim)
								_objectsManager.SPACTION(_globals.PERSO, "18,17,16,15,14,13,12,11,10,9,-1,", 0, 0, 6, 0);
							break;
						}
					}
				}
				if (_globals.OCEAN_SENS == 1)
					_objectsManager.SPACTION(_globals.PERSO, "27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,-1,", 0, 0, 6, 0);
				_globals.OCEAN_SENS = 5;
				_globals.SORTIE = 4;
			}
		}
	}
}

void HopkinsEngine::OCEAN_HOME() {
	warning("OCEAN_HOME()");

	if (_globals.OCEAN_SENS == 3)
		_objectsManager.SETANISPR(0, 0);
	if (_globals.OCEAN_SENS == 7)
		_objectsManager.SETANISPR(0, 18);
	if (_globals.OCEAN_SENS == 1)
		_objectsManager.SETANISPR(0, 27);
	if (_globals.OCEAN_SENS == 5)
		_objectsManager.SETANISPR(0, 9);
}

void HopkinsEngine::OCEAN(int16 a1, Common::String a2, Common::String a3, int16 a4, int16 a5, int16 a6, int16 a7, int16 a8, int16 a9) {
	warning("OCEAN()");

	_globals.PLAN_FLAG = false;
	_graphicsManager.NOFADE = false;
	_globals.NOMARCHE = false;
	_globals.SORTIE = 0;
	_globals.AFFLI = false;
	_globals.AFFIVBL = true;
	_globals.DESACTIVE_INVENT = true;
	_soundManager.WSOUND(a9);
	_fileManager.CONSTRUIT_SYSTEM("VAISSEAU.SPR");
	_fileManager.CHARGE_FICHIER2(_globals.NFICHIER, _globals.PERSO);
	if (a2.size())
		_graphicsManager.LOAD_IMAGE(a2);

	if ((a1 != 77) && (a1 != 84) && (a1 != 91))
		_objectsManager.INILINK("ocean");
	else if (a1 == 77)
		_objectsManager.INILINK("IM77");
	else if (a1 == 84)
		_objectsManager.INILINK("IM84");
	else if (a1 == 91)
		_objectsManager.INILINK("IM91");

	if (!a5)
		_objectsManager.ZONE_OFF(1);
	if (!a6)
		_objectsManager.ZONE_OFF(2);
	if (!a7)
		_objectsManager.ZONE_OFF(3);
	if (!a8)
		_objectsManager.ZONE_OFF(4);
	if (!_globals.OCEAN_SENS)
		_globals.OCEAN_SENS = a4;
	if (_globals.OCEAN_SENS == 5) {
		_objectsManager.PERX = 236;
		_objectsManager.PERI = 9;
	}
	if (_globals.OCEAN_SENS == 1) {
		_objectsManager.PERX = 236;
		_objectsManager.PERI = 27;
	}
	if (_globals.OCEAN_SENS == 7) {
		_objectsManager.PERX = 415;
		_objectsManager.PERI = 18;
	}
	if (_globals.OCEAN_SENS == 3) {
		_objectsManager.PERX = -20;
		_objectsManager.PERI = 0;
	}
	_objectsManager.SPRITE(_globals.PERSO, _objectsManager.PERX, 110, 0, _objectsManager.PERI, 0, 0, 0, 0);
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_objectsManager.SPRITE_ON(0);
	_globals.chemin = (int16 *)g_PTRNUL;
	_eventsManager.MOUSE_ON();
	_eventsManager.CHANGE_MOUSE(4);

	int cpt = 0;
	do {
		_eventsManager.VBL();
		++cpt;
	} while (cpt <= 4);

	if (!_graphicsManager.NOFADE)
		_graphicsManager.FADE_INW();
	_graphicsManager.NOFADE = false;
	_globals.iRegul = 1;

	bool loopCond = false;
	do {
		int mouseButton = _eventsManager.BMOUSE();
		if (mouseButton && mouseButton == 1)
			BTOCEAN();
		_objectsManager.VERIFZONE();
		OCEAN_HOME();
		_eventsManager.VBL();
		if (_globals.SORTIE)
			loopCond = true;
	} while (!loopCond);

	if (_globals.SORTIE == 1)
		_globals.SORTIE = a5;
	if (_globals.SORTIE == 2)
		_globals.SORTIE = a6;
	if (_globals.SORTIE == 3)
		_globals.SORTIE = a7;
	if (_globals.SORTIE == 4)
		_globals.SORTIE = a8;
	_graphicsManager.FADE_OUTW();
	_objectsManager.SPRITE_OFF(0);
	_globals.AFFLI = false;
	_objectsManager.CLEAR_ECRAN();
	_fileManager.CONSTRUIT_SYSTEM("PERSO.SPR");
	_fileManager.CHARGE_FICHIER2(_globals.NFICHIER, _globals.PERSO);
	_globals.PERSO_TYPE = 0;
}

void HopkinsEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_soundManager.syncSoundSettings();
}

} // End of namespace Hopkins
