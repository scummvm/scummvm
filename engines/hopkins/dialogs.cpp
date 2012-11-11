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
#include "common/events.h"
#include "common/file.h"
#include "common/util.h"
#include "hopkins/dialogs.h"
#include "hopkins/events.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/sound.h"

namespace Hopkins {

DialogsManager::DialogsManager() {
	DESACTIVE_INVENT = false;
	INVENTFLAG = false;
	AFFINVEN = false;
	VIRE_INVENT = false;
	inventairex = inventairey = 0;
	inventairel = inventaireh = 0;
	Winventaire = NULL;
	inventaire2 = g_PTRNUL;
}

DialogsManager::~DialogsManager() {
	_vm->_globals.dos_free2(Winventaire);
}

void DialogsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void DialogsManager::showOptionsDialog() {
	bool doneFlag;

	doneFlag = false;
	_vm->_eventsManager.CHANGE_MOUSE(0);
	_vm->_eventsManager.VBL();
	if (_vm->_globals.FR == 1)
		_vm->_fileManager.CONSTRUIT_SYSTEM("OPTIFR.SPR");
	if (!_vm->_globals.FR)
		_vm->_fileManager.CONSTRUIT_SYSTEM("OPTIAN.SPR");
	if (_vm->_globals.FR == 2)
		_vm->_fileManager.CONSTRUIT_SYSTEM("OPTIES.SPR");
  
	_vm->_globals.OPTION_SPR = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	_vm->_globals.OPTION_FLAG = true;

	do {
		if (_vm->_eventsManager.BMOUSE()) {
			Common::Point mousePos(_vm->_eventsManager.XMOUSE(), _vm->_eventsManager.YMOUSE());
			mousePos.x = _vm->_eventsManager.XMOUSE();
			mousePos.y = _vm->_eventsManager.YMOUSE();
	      
			if (!_vm->_soundManager.MUSICOFF) {
				if (mousePos.x >= _vm->_graphicsManager.ofscroll + 300 && mousePos.y > 113 && mousePos.x <= _vm->_graphicsManager.ofscroll + 327 && mousePos.y <= 138) {
					// Change the music volume
					++_vm->_soundManager.MUSICVOL;
					_vm->_soundManager.OLD_MUSICVOL = _vm->_soundManager.MUSICVOL;
	          
					if (_vm->_soundManager.MUSICVOL <= 12)
						_vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						_vm->_soundManager.MUSICVOL = 12;
					_vm->_soundManager.MODSetMusicVolume(_vm->_soundManager.MUSICVOL);

					_vm->_soundManager.updateScummVMSoundSettings();
				}
	        
				if (!_vm->_soundManager.MUSICOFF && mousePos.x >= _vm->_graphicsManager.ofscroll + 331 && mousePos.y > 113 && mousePos.x <= _vm->_graphicsManager.ofscroll + 358 && mousePos.y <= 138) {
					--_vm->_soundManager.MUSICVOL;
					if (_vm->_soundManager.MUSICVOL >= 0)
						_vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						_vm->_soundManager.MUSICVOL = 0;
				
					_vm->_soundManager.OLD_MUSICVOL = _vm->_soundManager.MUSICVOL;
					_vm->_soundManager.MODSetMusicVolume(_vm->_soundManager.MUSICVOL);

					_vm->_soundManager.updateScummVMSoundSettings();
				}
			}
			if (!_vm->_soundManager.SOUNDOFF) {
				if (mousePos.x >= _vm->_graphicsManager.ofscroll + 300 && mousePos.y > 140 && mousePos.x <= _vm->_graphicsManager.ofscroll + 327 && mousePos.y <= 165) {
					++_vm->_soundManager.SOUNDVOL;
					if (_vm->_soundManager.SOUNDVOL <= 16)
						_vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						_vm->_soundManager.SOUNDVOL = 16;
					_vm->_soundManager.OLD_SOUNDVOL = _vm->_soundManager.SOUNDVOL;
					_vm->_soundManager.MODSetSampleVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}
	        
				if (!_vm->_soundManager.SOUNDOFF && mousePos.x >= _vm->_graphicsManager.ofscroll + 331 && mousePos.y > 140 && mousePos.x <= _vm->_graphicsManager.ofscroll + 358 && mousePos.y <= 165) {
					--_vm->_soundManager.SOUNDVOL;
					if (_vm->_soundManager.SOUNDVOL >= 0)
						_vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						_vm->_soundManager.SOUNDVOL = 0;
					_vm->_soundManager.OLD_SOUNDVOL = _vm->_soundManager.SOUNDVOL;
					_vm->_soundManager.MODSetSampleVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}
			}
	      
			if (!_vm->_soundManager.VOICEOFF) {
				if (mousePos.x >= _vm->_graphicsManager.ofscroll + 300 && mousePos.y > 167 && mousePos.x <= _vm->_graphicsManager.ofscroll + 327 && mousePos.y <= 192) {
					++_vm->_soundManager.VOICEVOL;
	          
					if (_vm->_soundManager.VOICEVOL <= 16)
						_vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						_vm->_soundManager.VOICEVOL = 16;
					_vm->_soundManager.OLD_VOICEVOL = _vm->_soundManager.VOICEVOL;
					_vm->_soundManager.MODSetVoiceVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}
	        
				if (!_vm->_soundManager.VOICEOFF && mousePos.x >= _vm->_graphicsManager.ofscroll + 331 && mousePos.y > 167 && mousePos.x <= _vm->_graphicsManager.ofscroll + 358 && mousePos.y <= 192) {
					--_vm->_soundManager.VOICEVOL;
					if (_vm->_soundManager.VOICEVOL >= 0)
						_vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						_vm->_soundManager.VOICEVOL = 0;
					_vm->_soundManager.OLD_VOICEVOL = _vm->_soundManager.VOICEVOL;
					_vm->_soundManager.MODSetVoiceVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}
			}
	      
			if (mousePos.x >= _vm->_graphicsManager.ofscroll + 431) {
				if (mousePos.y > 194 && mousePos.x <= _vm->_graphicsManager.ofscroll + 489 && mousePos.y <= 219)
					_vm->_soundManager.TEXTOFF = _vm->_soundManager.TEXTOFF != 1;

				if (mousePos.x >= _vm->_graphicsManager.ofscroll + 431) {
					if (mousePos.y > 167 && mousePos.x <= _vm->_graphicsManager.ofscroll + 489 && mousePos.y <= 192) {
						_vm->_soundManager.VOICEOFF = _vm->_soundManager.VOICEOFF != 1;

						_vm->_soundManager.updateScummVMSoundSettings();
					}
					if (mousePos.x >= _vm->_graphicsManager.ofscroll + 431) {
						if (mousePos.y > 113 && mousePos.x <= _vm->_graphicsManager.ofscroll + 489 && mousePos.y <= 138) {
							if (_vm->_soundManager.MUSICOFF == 1) {
								_vm->_soundManager.MUSICOFF = 0;
								_vm->_soundManager.MODSetMusicVolume(_vm->_soundManager.MUSICVOL);
							} else {
								_vm->_soundManager.MUSICOFF = 1;
								_vm->_soundManager.MODSetMusicVolume(0);
							}

							_vm->_soundManager.updateScummVMSoundSettings();
						}
	        
						if (mousePos.x >= _vm->_graphicsManager.ofscroll + 431 && mousePos.y > 140 && mousePos.x <= _vm->_graphicsManager.ofscroll + 489 && mousePos.y <= 165) {
							_vm->_soundManager.SOUNDOFF = _vm->_soundManager.SOUNDOFF != 1;

							_vm->_soundManager.updateScummVMSoundSettings();
						}
					}
				}
			}
	      
			if (mousePos.x >= _vm->_graphicsManager.ofscroll + 175 && mousePos.y > 285 && mousePos.x <= _vm->_graphicsManager.ofscroll + 281 && mousePos.y <= 310) {
				_vm->_globals.SORTIE = 300;
				doneFlag = true;
			}
			if (mousePos.x >= _vm->_graphicsManager.ofscroll + 355 && mousePos.y > 285 && mousePos.x <= _vm->_graphicsManager.ofscroll + 490 && mousePos.y <= 310)
				doneFlag = true;
			if (mousePos.x >= _vm->_graphicsManager.ofscroll + 300 && mousePos.y > 194 && mousePos.x <= _vm->_graphicsManager.ofscroll + 358 && mousePos.y <= 219) {
				switch (_vm->_graphicsManager.SPEED_SCROLL) {
				case 1:
					_vm->_graphicsManager.SPEED_SCROLL = 2;
					break;
				case 2:
					_vm->_graphicsManager.SPEED_SCROLL = 4;
					break;
				case 4:
					_vm->_graphicsManager.SPEED_SCROLL = 8;
					break;
				case 8:
					_vm->_graphicsManager.SPEED_SCROLL = 16;
					break;
				case 16:
					_vm->_graphicsManager.SPEED_SCROLL = 32;
					break;
				case 32:
					_vm->_graphicsManager.SPEED_SCROLL = 48;
					break;
				case 48:
					_vm->_graphicsManager.SPEED_SCROLL = 64;
					break;
				case 64:
					_vm->_graphicsManager.SPEED_SCROLL = 128;
					break;
				case 128:
					_vm->_graphicsManager.SPEED_SCROLL = 160;
					break;
				case 160:
					_vm->_graphicsManager.SPEED_SCROLL = 320;
					break;
				case 320:
					_vm->_graphicsManager.SPEED_SCROLL = 1;
					break;
				}
			}

			if (mousePos.x >= _vm->_graphicsManager.ofscroll + 348 && mousePos.y > 248 && mousePos.x <= _vm->_graphicsManager.ofscroll + 394 && mousePos.y <= 273)
				_vm->_globals.vitesse = 2;
			if (mousePos.x >= _vm->_graphicsManager.ofscroll + 300 && mousePos.y > 221 && mousePos.x <= _vm->_graphicsManager.ofscroll + 358 && mousePos.y <= 246)
				_vm->_globals.SVGA = 2;
			if (mousePos.x < _vm->_graphicsManager.ofscroll + 165 || mousePos.x > _vm->_graphicsManager.ofscroll + 496 || (uint)(mousePos.y - 107) > 0xD3u)
				doneFlag = true;
		}

		if (!_vm->_graphicsManager.MANU_SCROLL)
			_vm->_globals.opt_scrtype = 2;
		if (_vm->_graphicsManager.MANU_SCROLL == 1)
			_vm->_globals.opt_scrtype = 1;
		if (_vm->_globals.vitesse == 1)
			_vm->_globals.opt_vitesse = 6;
		if (_vm->_globals.vitesse == 2)
			_vm->_globals.opt_vitesse = 5;
		if (_vm->_globals.vitesse == 3)
			_vm->_globals.opt_vitesse = 4;

		_vm->_globals.opt_txt = !_vm->_soundManager.TEXTOFF ? 7 : 8;
		_vm->_globals.opt_voice = !_vm->_soundManager.VOICEOFF ? 7 : 8;
		_vm->_globals.opt_sound = !_vm->_soundManager.SOUNDOFF ? 7 : 8;
		_vm->_globals.opt_music = !_vm->_soundManager.MUSICOFF ? 7 : 8;
			
		if (_vm->_globals.SVGA == 1)
			_vm->_globals.opt_anm = 10;
		if (_vm->_globals.SVGA == 2)
			_vm->_globals.opt_anm = 9;
		if (_vm->_globals.SVGA == 3)
			_vm->_globals.opt_anm = 11;
		if (_vm->_graphicsManager.SPEED_SCROLL == 1)
			_vm->_globals.opt_scrspeed = 12;
		if (_vm->_graphicsManager.SPEED_SCROLL == 2)
			_vm->_globals.opt_scrspeed = 13;
		if (_vm->_graphicsManager.SPEED_SCROLL == 4)
			_vm->_globals.opt_scrspeed = 14;
		if (_vm->_graphicsManager.SPEED_SCROLL == 8)
			_vm->_globals.opt_scrspeed = 15;
		if (_vm->_graphicsManager.SPEED_SCROLL == 16)
			_vm->_globals.opt_scrspeed = 16;
		if (_vm->_graphicsManager.SPEED_SCROLL == 32)
			_vm->_globals.opt_scrspeed = 17;
		if (_vm->_graphicsManager.SPEED_SCROLL == 48)
			_vm->_globals.opt_scrspeed = 18;
		if (_vm->_graphicsManager.SPEED_SCROLL == 64)
			_vm->_globals.opt_scrspeed = 19;
		if (_vm->_graphicsManager.SPEED_SCROLL == 128)
			_vm->_globals.opt_scrspeed = 20;
		if (_vm->_graphicsManager.SPEED_SCROLL == 160)
			_vm->_globals.opt_scrspeed = 21;
		if (_vm->_graphicsManager.SPEED_SCROLL == 320)
			_vm->_globals.opt_scrspeed = 22;
		if (_vm->_graphicsManager.SPEED_SCROLL == 640)
			_vm->_globals.opt_scrspeed = 23;

		_vm->_eventsManager.VBL();
	} while (!doneFlag);

	_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, _vm->_graphicsManager.ofscroll + 164, 
		107, 335, 215, _vm->_graphicsManager.VESA_BUFFER, _vm->_graphicsManager.ofscroll + 164, 107);
	_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_graphicsManager.ofscroll + 164, 107, 
		_vm->_graphicsManager.ofscroll + 498, 320);

	_vm->_globals.OPTION_SPR = _vm->_globals.dos_free2(_vm->_globals.OPTION_SPR);
	_vm->_globals.OPTION_FLAG = false;
}

void DialogsManager::showInventory() {
	int v1; 
	size_t filesize; 
	int v4;
	int v5; 
	int v6; 
	byte *v7; 
	int v8; 
	int v9; 
	int v10; 
	int v11; 
	int v12; 
	int v13; 
	int v14; 
	int v15; 
	int v16;
	int v17;
	int v18;
	int v19;
	int v20; 
	Common::File f;

	v13 = 0;
	if (!VIRE_INVENT && !AFFINVEN && !_vm->_globals.DESACTIVE_INVENT) {
		_vm->_graphicsManager.no_scroll = 1;
		_vm->_objectsManager.FLAG_VISIBLE_EFFACE = 4;
		_vm->_objectsManager.FLAG_VISIBLE = false;
		v1 = 0;
		do {
			INVENT_ANIM();
			_vm->_eventsManager.XMOUSE();
			_vm->_eventsManager.YMOUSE();
			_vm->_eventsManager.VBL();
			++v1;
		} while (v1 <= 1);
		_vm->_dialogsManager.Winventaire = g_PTRNUL;

LABEL_7:
		_vm->_eventsManager.souris_bb = 0;
		_vm->_eventsManager.souris_b = 0;
		_vm->_globals.DESACTIVE_INVENT = true;
		_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);

		switch (_vm->_globals.FR) {
			case 0:
				_vm->_fileManager.CONSTRUIT_SYSTEM("INVENTAN.SPR");
				break;
			case 1:
				_vm->_fileManager.CONSTRUIT_SYSTEM("INVENTFR.SPR");
				break;
			case 2:
				_vm->_fileManager.CONSTRUIT_SYSTEM("INVENTES.SPR");
				break;
		}

		if (!f.open(_vm->_globals.NFICHIER))
			error("Error opening file - %s", _vm->_globals.NFICHIER.c_str());

		filesize = f.size();
		_vm->_dialogsManager.Winventaire = _vm->_globals.dos_malloc2(filesize);
		_vm->_fileManager.bload_it(f, _vm->_dialogsManager.Winventaire, filesize);
		f.close();

		_vm->_fileManager.CONSTRUIT_SYSTEM("INVENT2.SPR");
		inventaire2 = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
		v19 = _vm->_graphicsManager.ofscroll + 152;
		v18 = _vm->_objectsManager.Get_Largeur(_vm->_dialogsManager.Winventaire, 0);
		v17 = _vm->_objectsManager.Get_Hauteur(_vm->_dialogsManager.Winventaire, 0);
		inventairex = v19;
		inventairey = 114;
		inventairel = v18;
		inventaireh = v17;
		_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_dialogsManager.Winventaire, 
			v19 + 300, 414, 0, 0, 0, 0);
		v15 = 0;
		v4 = 0;
		v14 = 1;
		do {
			v16 = 0;
			v5 = 1;
			do {
				++v4;
				v6 = _vm->_globals.INVENTAIRE[v4];
				if (v6 && v4 <= 29) {
					v7 = _vm->_objectsManager.CAPTURE_OBJET(v6, 0);
					_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, v7, v19 + v16 + 6, 
						v15 + 120, _vm->_globals.OBJL, _vm->_globals.OBJH);
					_vm->_globals.dos_free2(v7);
				}
				v16 += 54;
				++v5;
			} while (v5 <= 6);
			v15 += 38;
			++v14;
		} while (v14 <= 5);
		_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_dialogsManager.Winventaire, inventairex, inventairey, inventairel, inventaireh);
		_vm->_eventsManager.souris_bb = 0;
		v20 = 0;

		// Main loop to select an inventory item
		while (!_vm->shouldQuit()) {
			// Turn on drawing the inventory dialog in the event manager
			AFFINVEN = true;

			v8 = _vm->_eventsManager.XMOUSE();
			v9 = _vm->_eventsManager.YMOUSE();
			v12 = _vm->_eventsManager.BMOUSE();
			v10 = v13;
			v11 = _vm->_linesManager.ZONE_OBJET(v8, v9);
			v13 = v11;
			if (v11 != v10)
				_vm->_objectsManager.PARAMCADRE(v11);
			if (_vm->_eventsManager.btsouris != 16) {
				if ((uint16)(_vm->_eventsManager.btsouris - 1) > 1u) {
					if (_vm->_eventsManager.btsouris != 3) {
						if (v12 == 2) {
							_vm->_objectsManager.OBJETPLUS(v13);
							if (_vm->_eventsManager.btsouris != 23)
								_vm->_eventsManager.CHANGE_MOUSE(_vm->_eventsManager.btsouris);
						}
					}
				}
			}
			if (v12 == 1) {
				if (_vm->_eventsManager.btsouris == 1 || _vm->_eventsManager.btsouris == 16 || !_vm->_eventsManager.btsouris || (uint16)(_vm->_eventsManager.btsouris - 2) <= 1u)
					break;
				v9 = v13;
				_vm->_objectsManager.VALID_OBJET(_vm->_globals.INVENTAIRE[v13]);
				if (_vm->_eventsManager.btsouris == 8)
					v20 = 1;
				if (v20 != 1) {
					_vm->_scriptManager.TRAVAILOBJET = 1;
					_vm->_globals.SAUVEGARDE->data[svField3] = _vm->_globals.OBJET_EN_COURS;
					_vm->_globals.SAUVEGARDE->data[svField8] = _vm->_globals.INVENTAIRE[v13];
					_vm->_globals.SAUVEGARDE->data[svField9] = _vm->_eventsManager.btsouris;
					_vm->_objectsManager.OPTI_OBJET();
					_vm->_scriptManager.TRAVAILOBJET = 0;

					if (_vm->_soundManager.VOICEOFF == 1) {
						do
							_vm->_eventsManager.VBL();
						while (!_vm->_globals.SORTIE && _vm->_eventsManager.BMOUSE() != 1);
						_vm->_fontManager.TEXTE_OFF(9);
					}
					if (_vm->_globals.SORTIE) {
						if (_vm->_globals.SORTIE == 2)
							v20 = 1;
						_vm->_globals.SORTIE = 0;
						if (v20 != 1) {
							inventaire2 = _vm->_globals.dos_free2(inventaire2);
							if (g_PTRNUL != _vm->_dialogsManager.Winventaire)
								_vm->_dialogsManager.Winventaire = _vm->_globals.dos_free2(_vm->_dialogsManager.Winventaire);
							goto LABEL_7;
						}
					} else if (v20 != 1) {
						AFFINVEN = true;
					}
				}
			}
			if (VIRE_INVENT == true)
				v20 = 1;
			if (v20 == 1)
				break;
			_vm->_eventsManager.VBL();
			if ((uint16)(_vm->_globals.ECRAN - 35) <= 5u)
				_vm->_objectsManager.SPECIAL_JEU();
		}
		_vm->_fontManager.TEXTE_OFF(9);
		if (AFFINVEN) {
			AFFINVEN = false;
			v9 = 114;
			_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, v19, 114, v18, v17, _vm->_graphicsManager.VESA_BUFFER, v19, 114);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(v19, 114, v19 + v18, v18 + 114);
			_vm->_objectsManager.BOBTOUS = true;
		}
		if (_vm->_dialogsManager.Winventaire != g_PTRNUL)
			_vm->_dialogsManager.Winventaire = _vm->_globals.dos_free2(_vm->_dialogsManager.Winventaire);
		inventaire2 = _vm->_globals.dos_free2(inventaire2);

		if (_vm->_eventsManager.btsouris == 1)
			showOptionsDialog();
		if (_vm->_eventsManager.btsouris == 3)
			_vm->_dialogsManager.CHARGE_PARTIE();
		if (_vm->_eventsManager.btsouris == 2)
			_vm->_dialogsManager.SAUVE_PARTIE();

		_vm->_eventsManager.btsouris = 4;
		_vm->_eventsManager.CHANGE_MOUSE(4);
		_vm->_objectsManager.old_cady = 0;
		_vm->_objectsManager.cady = 0;
		_vm->_objectsManager.old_cadx = 0;
		_vm->_objectsManager.cadx = 0;
		_vm->_globals.DESACTIVE_INVENT = false;
		_vm->_graphicsManager.no_scroll = 0;
	}
}

void DialogsManager::INVENT_ANIM() {
	int v0; 
	int v1;

	if (!DESACTIVE_INVENT) {
		if (_vm->_objectsManager.FLAG_VISIBLE_EFFACE && !_vm->_objectsManager.FLAG_VISIBLE) {
			_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, _vm->_objectsManager.I_old_x, 27, 48, 38, 
				_vm->_graphicsManager.VESA_BUFFER, _vm->_objectsManager.I_old_x, 27);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_objectsManager.I_old_x, 27, _vm->_objectsManager.I_old_x + 48, 65);
			--_vm->_objectsManager.FLAG_VISIBLE_EFFACE;
		}
    
		if (_vm->_objectsManager.FLAG_VISIBLE) {
			if (_vm->_objectsManager.I_old_x <= 1)
				_vm->_objectsManager.I_old_x = 2;
			_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, _vm->_objectsManager.I_old_x, 27, 48, 38, 
				_vm->_graphicsManager.VESA_BUFFER, _vm->_objectsManager.I_old_x, 27);
      
			_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_objectsManager.I_old_x, 27, _vm->_objectsManager.I_old_x + 48, 65);
			v0 = _vm->_graphicsManager.ofscroll + 2;
			v1 = _vm->_graphicsManager.ofscroll + 2;
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.ICONE, v1 + 300, 327, 0);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(v1, 27, v1 + 45, 62);
			_vm->_objectsManager.I_old_x = v0;
		}
    
		if (_vm->_globals.SAUVEGARDE->data[svField357] == 1) {
			if (_vm->_globals.SAUVEGARDE->data[svField353] == 1)
				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
			if (_vm->_globals.SAUVEGARDE->data[svField355] == 1)
				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 866, 325, 1, 0, 0, 0);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(532, 25, 560, 60);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(566, 25, 594, 60);
		}
		if (_vm->_globals.SAUVEGARDE->data[svField356] == 1) {
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(532, 25, 560, 60);
		}

		if (_vm->_globals.SAUVEGARDE->data[svField354] == 1) {
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(532, 25, 560, 60);
		}
	}
}

void DialogsManager::TestForDialogOpening() {
	if (_vm->_globals.PLAN_FLAG)
		_vm->_eventsManager.GAME_KEY = KEY_NONE;
	
	if (_vm->_eventsManager.GAME_KEY != KEY_NONE) {
		if (!INVENTFLAG) {
			DIALOG_KEY key = _vm->_eventsManager.GAME_KEY;
			_vm->_eventsManager.GAME_KEY = KEY_NONE;
			INVENTFLAG = true;

			switch (key) {
			case KEY_INVENTORY:
				_vm->_dialogsManager.showInventory();
				break;
			case KEY_OPTIONS:
				_vm->_dialogsManager.showOptionsDialog();
				break;
			case KEY_LOAD:
				_vm->_dialogsManager.CHARGE_PARTIE();
				break;
			case KEY_SAVE:
				_vm->_dialogsManager.SAUVE_PARTIE();
				break;
			default:
				break;
			}

			INVENTFLAG = false;
			_vm->_eventsManager.GAME_KEY = KEY_NONE;
		}
	}
}

// Load Game
void DialogsManager::CHARGE_PARTIE() {
	int slotNumber; 

	_vm->_eventsManager.VBL();
	LOAD_SAUVE(2);
	do {
		do {
			slotNumber = CHERCHE_PARTIE();
			_vm->_eventsManager.VBL();
		} while (_vm->_eventsManager.BMOUSE() != 1);
	} while (!slotNumber);
	_vm->_objectsManager.SL_FLAG = false;
	_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x + 183, 60, 274, 353, _vm->_graphicsManager.VESA_BUFFER, _vm->_eventsManager.start_x + 183, 60);
	_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_eventsManager.start_x + 183, 60, 457, 413);
	_vm->_objectsManager.BOBTOUS = true;
	_vm->_objectsManager.SL_SPR = _vm->_globals.dos_free2(_vm->_objectsManager.SL_SPR);
	_vm->_objectsManager.SL_SPR2 = _vm->_globals.dos_free2(_vm->_objectsManager.SL_SPR2);
	_vm->_objectsManager.SL_X = 0;
	_vm->_objectsManager.SL_Y = 0;

	if (slotNumber != 7) {
		_vm->_saveLoadManager.restore(slotNumber);
	}
	
	_vm->_objectsManager.CHANGE_OBJET(14);
}

// Save Game
void DialogsManager::SAUVE_PARTIE() {
	int slotNumber; 
	Common::String saveName; 

	_vm->_eventsManager.VBL();

	LOAD_SAUVE(1);
	do {
		do {
			slotNumber = CHERCHE_PARTIE();
			_vm->_eventsManager.VBL();
		} while (!_vm->shouldQuit() && _vm->_eventsManager.BMOUSE() != 1);
	} while (!_vm->shouldQuit() && !slotNumber);

	_vm->_objectsManager.SL_FLAG = false;
	_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x + 183, 60, 274, 353, _vm->_graphicsManager.VESA_BUFFER, _vm->_eventsManager.start_x + 183, 60);
	_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_eventsManager.start_x + 183, 60, _vm->_eventsManager.start_x + 457, 413);
	_vm->_objectsManager.BOBTOUS = true;
	_vm->_objectsManager.SL_SPR = _vm->_globals.dos_free2(_vm->_objectsManager.SL_SPR);
	_vm->_objectsManager.SL_SPR2 = _vm->_globals.dos_free2(_vm->_objectsManager.SL_SPR2);
	_vm->_objectsManager.SL_X = 0;
	_vm->_objectsManager.SL_Y = 0;

	if (slotNumber != 7) {
		// Since the original GUI doesn't support save names, use a default name
		saveName = Common::String::format("Save #%d", slotNumber);

		// Save the game
		_vm->_saveLoadManager.save(slotNumber, saveName);
	}
}


// Load Save
void DialogsManager::LOAD_SAUVE(int a1) {
	int slotNumber; 
	hopkinsSavegameHeader header;
	byte *thumb;

	switch (_vm->_globals.FR) {
	case 0:
		_vm->_fileManager.CONSTRUIT_SYSTEM("SAVEAN.SPR");
		break;
	case 1:
		_vm->_fileManager.CONSTRUIT_SYSTEM("SAVEFR.SPR");
		break;
	case 2:
		_vm->_fileManager.CONSTRUIT_SYSTEM("SAVEES.SPR");
		break;
	}

	_vm->_objectsManager.SL_SPR = _vm->_objectsManager.CHARGE_SPRITE(_vm->_globals.NFICHIER);
	_vm->_fileManager.CONSTRUIT_SYSTEM("SAVE2.SPR");
	_vm->_objectsManager.SL_SPR2 = _vm->_objectsManager.CHARGE_SPRITE(_vm->_globals.NFICHIER);
	_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_objectsManager.SL_SPR, _vm->_eventsManager.start_x + 483, 360, 0);

	if (_vm->_globals.FR) {
		if (a1 == 1)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_objectsManager.SL_SPR, _vm->_eventsManager.start_x + 525, 375, 1);
		if (a1 == 2)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_objectsManager.SL_SPR, _vm->_eventsManager.start_x + 515, 375, 2);
	} else {
		if (a1 == 1)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_objectsManager.SL_SPR, _vm->_eventsManager.start_x + 535, 372, 1);
		if (a1 == 2)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_objectsManager.SL_SPR, _vm->_eventsManager.start_x + 539, 372, 2);
	}

	for (slotNumber = 1; slotNumber <= 6; ++slotNumber) {
		if (_vm->_saveLoadManager.readSavegameHeader(slotNumber, header)) {
			thumb = (byte *)header.thumbnail->pixels;

			switch (slotNumber) {
			case 1:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, thumb, _vm->_eventsManager.start_x + 190, 112, 0x80u, 87);
				break;
			case 2:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, thumb, _vm->_eventsManager.start_x + 323, 112, 0x80u, 87);
				break;
			case 3:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, thumb, _vm->_eventsManager.start_x + 190, 203, 0x80u, 87);
				break;
			case 4:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, thumb, _vm->_eventsManager.start_x + 323, 203, 0x80u, 87);
				break;
			case 5:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, thumb, _vm->_eventsManager.start_x + 190, 294, 0x80u, 87);
				break;
			case 6:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, thumb, _vm->_eventsManager.start_x + 323, 294, 0x80u, 87);
				break;
			}
			
			delete header.thumbnail;
		}
	}

	_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_objectsManager.SL_SPR, _vm->_eventsManager.start_x + 183, 60, 0x112u, 353);
	_vm->_objectsManager.SL_FLAG = true;
	_vm->_objectsManager.SL_MODE = a1;
	_vm->_objectsManager.SL_X = 0;
	_vm->_objectsManager.SL_Y = 0;
}

// Search Game
int DialogsManager::CHERCHE_PARTIE() {
	int slotNumber; 
	int xp; 
	int yp; 

	slotNumber = 0;
	xp = _vm->_eventsManager.XMOUSE();
	yp = _vm->_eventsManager.YMOUSE();

	_vm->_graphicsManager.ofscroll = _vm->_eventsManager.start_x;
	if ((uint16)(yp - 112) <= 0x56u) {
		if (xp > _vm->_eventsManager.start_x + 189 && xp < _vm->_eventsManager.start_x + 318)
			slotNumber = 1;
		if ((uint16)(yp - 112) <= 0x56u && xp > _vm->_graphicsManager.ofscroll + 322 && xp < _vm->_graphicsManager.ofscroll + 452)
			slotNumber = 2;
	}
	if ((uint16)(yp - 203) <= 0x56u) {
		if (xp > _vm->_graphicsManager.ofscroll + 189 && xp < _vm->_graphicsManager.ofscroll + 318)
			slotNumber = 3;
		if ((uint16)(yp - 203) <= 0x56u && xp > _vm->_graphicsManager.ofscroll + 322 && xp < _vm->_graphicsManager.ofscroll + 452)
			slotNumber = 4;
	}
	if ((uint16)(yp - 294) <= 0x56u) {
		if (xp > _vm->_graphicsManager.ofscroll + 189 && xp < _vm->_graphicsManager.ofscroll + 318)
			slotNumber = 5;
		if ((uint16)(yp - 294) <= 0x56u && xp > _vm->_graphicsManager.ofscroll + 322 && xp < _vm->_graphicsManager.ofscroll + 452)
			slotNumber = 6;
	}
	if ((uint16)(yp - 388) <= 0x10u && xp > _vm->_graphicsManager.ofscroll + 273 && xp < _vm->_graphicsManager.ofscroll + 355)
		slotNumber = 7;
	if (slotNumber == 1) {
		_vm->_objectsManager.SL_X = 189;
		_vm->_objectsManager.SL_Y = 111;
	}
	if (slotNumber == 2) {
		_vm->_objectsManager.SL_X = 322;
		_vm->_objectsManager.SL_Y = 111;
	}
	if (slotNumber == 3) {
		_vm->_objectsManager.SL_X = 189;
		_vm->_objectsManager.SL_Y = 202;
	}
	if (slotNumber == 4) {
		_vm->_objectsManager.SL_X = 322;
		_vm->_objectsManager.SL_Y = 202;
	}
	if (slotNumber == 5) {
		_vm->_objectsManager.SL_X = 189;
		_vm->_objectsManager.SL_Y = 293;
	}
	if (slotNumber == 6) {
		_vm->_objectsManager.SL_X = 322;
		_vm->_objectsManager.SL_Y = 293;
	}
	if (slotNumber == 7 || !slotNumber) {
		_vm->_objectsManager.SL_X = 0;
		_vm->_objectsManager.SL_Y = 0;
	}
	return slotNumber;
}

} // End of namespace Hopkins
