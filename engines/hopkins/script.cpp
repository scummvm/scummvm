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

#include "common/system.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"
#include "hopkins/objects.h"
#include "hopkins/dialogs.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/sound.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

ScriptManager::ScriptManager() {
	TRAVAILOBJET = 0;
}

void ScriptManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

int ScriptManager::Traduction(byte *a1) {
	char v47; 
	char v48; 
	char v49;
	int v50; 
	CharacterLocation *v51; 
	CharacterLocation *v53; 
	Common::String file; 

	int v1 = 0;
	int v70 = 0;
	if (*a1 != 'F' || *(a1 + 1) != 'C')
		return 0;
	if (*(a1 + 2) == 'T' && *(a1 + 3) == 'X' && *(a1 + 4) == 'T') {
		v70 = *(a1 + 6);
		int v2 = *(a1 + 7);
		int v69 = *(a1 + 8);
		int v67 = (int16)READ_LE_UINT16(a1 + 9);
		int v65 = (int16)READ_LE_UINT16(a1 + 11);
		int v3 = (int16)READ_LE_UINT16(a1 + 13);
		v1 = 1;
		if (!TRAVAILOBJET) {
			if (_vm->_globals.SAUVEGARDE->data[svField356] == 1) {
				if (v3 == 53)
					v3 = 644;
				if (v3 == 624)
					v3 = 639;
				if (v3 == 627)
					v3 = 630;
				if (v3 == 625)
					v3 = 639;
				if (v3 == 8)
					v3 = 637;
				if (v3 == 53)
					v3 = 644;
				if (v3 == 557)
					v3 = 636;
				if (v3 == 51)
					v3 = 644;
				if (v3 == 287)
					v3 = 636;
				if (v3 == 619)
					v3 = 633;
				if (v3 == 620)
					v3 = 634;
				if (v3 == 622)
					v3 = 644;
				if (v3 == 297)
					v3 = 636;
				if (v3 == 612)
					v3 = 636;
				if ((uint16)(v3 - 613) <= 1u || v3 == 134)
					v3 = 636;
				if (v3 == 615)
					v3 = 635;
				if (v3 == 618)
					v3 = 632;
				if (v3 == 611)
					v3 = 642;
				if (v3 == 610)
					v3 = 641;
				if (v3 == 18)
					v3 = 643;
				if (v3 == 602)
					v3 = 645;
				if (v3 == 603)
					v3 = 646;
				if (v3 == 604)
					v3 = 647;
				if (v3 == 51)
					v3 = 644;
				if (v3 == 607)
					v3 = 650;
				if (v3 == 605)
					v3 = 648;
				if (v3 == 606)
					v3 = 649;
				if (v3 == 601)
					v3 = 652;
				if (v3 == 37)
					v3 = 636;
				if (v3 == 595)
					v3 = 633;
				if (v3 == 596)
					v3 = 634;
				if (v3 == 532)
					v3 = 636;
				if (v3 == 599)
					v3 = 636;
				if (v3 == 363)
					v3 = 636;
			}
			if (!_vm->_soundManager.SOUNDOFF && _vm->_soundManager.SOUND_FLAG == 1) {
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game
					_vm->_eventsManager.VBL();
				} while (_vm->_soundManager.SOUND_FLAG);
			}
			if (!_vm->_soundManager.TEXTOFF) {
				_vm->_fontManager.DOS_TEXT(9, v3, _vm->_globals.FICH_TEXTE, 2 * v67, 2 * v65 + 40, 20, 25, 6, v2, 253);
				if (!_vm->_soundManager.TEXTOFF)
					_vm->_fontManager.TEXTE_ON(9);
			}
			if (!_vm->_soundManager.VOICEOFF)
				_vm->_soundManager.VOICE_MIX(v3, 4);
		}
		if (TRAVAILOBJET == 1) {
			if (_vm->_globals.SAUVEGARDE->data[svField356]) {
				_vm->_fontManager.DOS_TEXT(9, 635, _vm->_globals.FICH_TEXTE, 55, 20, 20, 25, v69, 35, 253);
				if (!_vm->_soundManager.TEXTOFF)
					_vm->_fontManager.TEXTE_ON(9);
				if (!_vm->_soundManager.VOICEOFF)
					_vm->_soundManager.VOICE_MIX(635, 4);
				goto LABEL_104;
			}
			if (_vm->_globals.FR == 1 && !_vm->_soundManager.TEXTOFF)
				_vm->_fontManager.DOS_TEXT(9, v3, "OBJET1.TXT", 2 * v67, 60, 20, 25, 6, v2, 253);
			if (!_vm->_globals.FR && !_vm->_soundManager.TEXTOFF)
				_vm->_fontManager.DOS_TEXT(9, v3, "OBJETAN.TXT", 2 * v67, 60, 20, 25, 6, v2, 253);
			if (_vm->_globals.FR == 2) {
				if (_vm->_soundManager.TEXTOFF) {
LABEL_98:
					if (!_vm->_soundManager.VOICEOFF)
						_vm->_soundManager.VOICE_MIX(v3, 5);
					goto LABEL_104;
				}
				_vm->_fontManager.DOS_TEXT(9, v3, "OBJETES.TXT", 2 * v67, 60, 20, 25, 6, v2, 253);
			}
			if (!_vm->_soundManager.TEXTOFF)
				_vm->_fontManager.TEXTE_ON(9);
			goto LABEL_98;
		}
	}
LABEL_104:
	if (*(a1 + 2) == 'B' && *(a1 + 3) == 'O' && *(a1 + 4) == 'B') {
		if (_vm->_objectsManager.DESACTIVE != true) {
			int v72 = *(a1 + 5);
			int v70 = *(a1 + 6);
			int v4 = *(a1 + 7);
			int v68 = (int16)READ_LE_UINT16(a1 + 8);
			int v66 = (int16)READ_LE_UINT16(a1 + 10);
			if (v72 == 52) {
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, v68, (int16)READ_LE_UINT16(a1 + 10), v70);
			} else if (v72 == 51) {
				_vm->_objectsManager.BOB_VIVANT(v70);
			} else {
				if (v72 == 50)
					goto LABEL_1141;
				_vm->_objectsManager.VBOB(_vm->_globals.SPRITE_ECRAN, v72, v68, v66, v70);
				if (v4)
					v4 /= _vm->_globals.vitesse;
				if (v4 > 1) {
					do {
						if (_vm->shouldQuit())
							return -1; // Exiting game

						--v4;
						_vm->_eventsManager.VBL();
					} while (v4);
				}
				if (v72 == 50)
LABEL_1141:
					_vm->_objectsManager.AFFICHE_SPEED1(_vm->_globals.SPRITE_ECRAN, v68, v66, v70);
			}
		}
		v1 = 1;
	}
	if (*(a1 + 2) == 'S') {
		if (*(a1 + 3) == 'T' && *(a1 + 4) == 'P') {
			if (_vm->_objectsManager.DESACTIVE != true) {
				_vm->_objectsManager.DEUXPERSO = false;
				int v5 = *(a1 + 5);
				int v6 = (int16)READ_LE_UINT16(a1 + 8);
				_vm->_objectsManager.PERX = (int16)READ_LE_UINT16(a1 + 6);
				_vm->_objectsManager.PERY = v6;
				_vm->_objectsManager.PERI = v5;
				if (_vm->_objectsManager.CH_TETE == 1) {
					if (_vm->_globals.SAUVEGARDE->data[svField354] == 1
					        && _vm->_globals.SAUVEGARDE->data[svField180]
					        && _vm->_globals.SAUVEGARDE->data[svField181]
					        && _vm->_globals.SAUVEGARDE->data[svField182]
					        && _vm->_globals.SAUVEGARDE->data[svField183]) {
						_vm->_objectsManager.PERX = _vm->_globals.SAUVEGARDE->data[svField180];
						_vm->_objectsManager.PERY = _vm->_globals.SAUVEGARDE->data[svField181];
						_vm->_objectsManager.PERI = _vm->_globals.SAUVEGARDE->data[svField182];
					}
					if (_vm->_globals.SAUVEGARDE->data[svField356] == 1
					        && _vm->_globals.SAUVEGARDE->data[svField190]
					        && _vm->_globals.SAUVEGARDE->data[svField191]
					        && _vm->_globals.SAUVEGARDE->data[svField192]
					        && _vm->_globals.SAUVEGARDE->data[svField193]) {
						_vm->_objectsManager.PERX = _vm->_globals.SAUVEGARDE->data[svField190];
						_vm->_objectsManager.PERY = _vm->_globals.SAUVEGARDE->data[svField191];
						_vm->_objectsManager.PERI = _vm->_globals.SAUVEGARDE->data[svField192];
					}
					if (_vm->_globals.SAUVEGARDE->data[svField357] == 1
					        && _vm->_globals.SAUVEGARDE->data[svField185]
					        && _vm->_globals.SAUVEGARDE->data[svField186]
					        && _vm->_globals.SAUVEGARDE->data[svField187]
					        && _vm->_globals.SAUVEGARDE->data[svField188]) {
						_vm->_objectsManager.PERX = _vm->_globals.SAUVEGARDE->data[svField185];
						_vm->_objectsManager.PERY = _vm->_globals.SAUVEGARDE->data[svField186];
						_vm->_objectsManager.PERI = _vm->_globals.SAUVEGARDE->data[svField187];
					}
				}
				if (_vm->_globals.SAUVEGARDE->data[svField356] == 1 && _vm->_globals.SAUVEGARDE->data[svField188] == _vm->_globals.ECRAN) {
					_vm->_objectsManager.SPRITE(
					    _vm->_globals.TETE,
					    _vm->_globals.SAUVEGARDE->data[svField185],
					    _vm->_globals.SAUVEGARDE->data[svField186],
					    1,
					    2,
					    _vm->_globals.SAUVEGARDE->data[svField189],
					    0,
					    34,
					    190);
					_vm->_objectsManager.SPRITE_ON(1);
					_vm->_objectsManager.DEUXPERSO = true;
				}
				if (_vm->_globals.SAUVEGARDE->data[svField357] == 1
				        && _vm->_globals.SAUVEGARDE->data[svField355] == 1
				        && _vm->_globals.SAUVEGARDE->data[svField193] == _vm->_globals.ECRAN) {
					_vm->_objectsManager.SPRITE(
					    _vm->_globals.TETE,
					    _vm->_globals.SAUVEGARDE->data[svField190],
					    _vm->_globals.SAUVEGARDE->data[svField191],
					    1,
					    3,
					    _vm->_globals.SAUVEGARDE->data[svField194],
					    0,
					    20,
					    127);
					_vm->_objectsManager.SPRITE_ON(1);
					_vm->_objectsManager.DEUXPERSO = true;
				}
			}
			v1 = 1;
			_vm->_objectsManager.CH_TETE = 0;
		}
		if (*(a1 + 2) == 'S' && *(a1 + 3) == 'T' && *(a1 + 4) == 'E') {
			if (_vm->_objectsManager.DESACTIVE != true) {
				int v7 = *(a1 + 5);
				v70 = *(a1 + 6);
				int v8 = *(a1 + 7);
				int v9 = *(a1 + 8);
				_vm->_objectsManager.RECALL = 0;
				_vm->_globals.OLD_ECRAN = _vm->_globals.ECRAN;
				_vm->_globals.SAUVEGARDE->data[svField6] = _vm->_globals.ECRAN;
				_vm->_globals.ECRAN = v7;
				_vm->_globals.SAUVEGARDE->data[svField5] = v7;
				_vm->_objectsManager.PTAILLE = v70;
				_vm->_objectsManager.PEROFX = v8;
				_vm->_objectsManager.PEROFY = v9;
			}
			v1 = 1;
		}
	}
	if (*(a1 + 2) == 'B' && *(a1 + 3) == 'O' && *(a1 + 4) == 'F') {
		if (_vm->_objectsManager.DESACTIVE != true)
			_vm->_objectsManager.VBOB_OFF((int16)READ_LE_UINT16(a1 + 5));
		v1 = 1;
	}
	if (*(a1 + 2) == 'P' && *(a1 + 3) == 'E' && *(a1 + 4) == 'R') {
		int v73 = (int16)READ_LE_UINT16(a1 + 5);
		if (!_vm->_globals.SAUVEGARDE->data[svField122] && !_vm->_globals.SAUVEGARDE->data[svField356]) {
			v70 = 0;
			if ((int16)READ_LE_UINT16(a1 + 5) == 14)
				v73 = 1;
			if (v73 == 17)
				v73 = 7;
			if (v73 == 18)
				v73 = 8;
			if (v73 == 19)
				v73 = 4;
			if (v73 == 15)
				v73 = 11;
			if (v73 == 16)
				v73 = 12;
			if (v73 == 1) {
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(4);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(4);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(4);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(4);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(4);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(4);
			}
			if (v73 == 2) {
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(7);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(7);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(7);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(7);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(7);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(7);
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(8);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(8);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(8);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(8);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(8);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(8);
			}
			if (v73 == 4) {
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(1);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(1);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(1);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(1);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(1);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(1);
			}
			if (v73 == 5) {
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(5);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(5);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(5);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(5);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(5);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(5);
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(6);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(6);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(6);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(6);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(6);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(6);
			}
			if (v73 == 7) {
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(2);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(2);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(2);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(2);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(2);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(2);
			}
			if (v73 == 8) {
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(3);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(3);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(3);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(3);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(3);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(3);
			}
			if (v73 == 9) {
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(5);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(5);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(5);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(5);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(5);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(5);
			}
			if (v73 == 10) {
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(6);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(6);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(6);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(6);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(6);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(6);
			}
			if (v73 == 11) {
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(7);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(7);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(7);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(7);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(7);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(7);
			}
			if (v73 == 12) {
				if (_vm->_globals.ACTION_SENS == 1)
					_vm->_objectsManager.ACTION_DOS(8);
				if (_vm->_globals.ACTION_SENS == 3)
					_vm->_objectsManager.ACTION_DROITE(8);
				if (_vm->_globals.ACTION_SENS == 2)
					_vm->_objectsManager.Q_DROITE(8);
				if (_vm->_globals.ACTION_SENS == 5)
					_vm->_objectsManager.ACTION_FACE(8);
				if (_vm->_globals.ACTION_SENS == 8)
					_vm->_objectsManager.Q_GAUCHE(8);
				if (_vm->_globals.ACTION_SENS == 7)
					_vm->_objectsManager.ACTION_GAUCHE(8);
			}
		}
		v1 = 1;
	}
	if (*(a1 + 2) == 'M' && *(a1 + 3) == 'U' && *(a1 + 4) == 'S')
		v1 = 1;
	if (*(a1 + 2) == 'W' && *(a1 + 3) == 'A' && *(a1 + 4) == 'I') {
		int v74 = (int16)READ_LE_UINT16(a1 + 5) / _vm->_globals.vitesse;
		if (!v74)
			v74 = 1;
		int v10 = 0;
		if (v74 + 1 > 0) {
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
				++v10;
			} while (v10 < v74 + 1);
		}
		v1 = 1;
	}
	if (*(a1 + 2) == 'O') {
		if (*(a1 + 3) == 'B' && *(a1 + 4) == 'P') {
			v1 = 1;
			_vm->_objectsManager.AJOUTE_OBJET((int16)READ_LE_UINT16(a1 + 5));
		}
		if (*(a1 + 2) == 'O' && *(a1 + 3) == 'B' && *(a1 + 4) == 'M') {
			v1 = 1;
			_vm->_objectsManager.DELETE_OBJET((int16)READ_LE_UINT16(a1 + 5));
		}
	}
	if (*(a1 + 2) == 'G' && *(a1 + 3) == 'O' && *(a1 + 4) == 'T')
		v1 = 2;
	if (*(a1 + 2) == 'Z') {
		if (*(a1 + 3) == 'O' && *(a1 + 4) == 'N') {
			_vm->_objectsManager.ZONE_ON((int16)READ_LE_UINT16(a1 + 5));
			v1 = 1;
		}
		if (*(a1 + 2) == 'Z' && *(a1 + 3) == 'O' && *(a1 + 4) == 'F') {
			_vm->_objectsManager.ZONE_OFF((int16)READ_LE_UINT16(a1 + 5));
			v1 = 1;
		}
	}
	if (*(a1 + 2) == 'E' && *(a1 + 3) == 'X' && *(a1 + 4) == 'I')
		v1 = 5;
	if (*(a1 + 2) == 'S' && *(a1 + 3) == 'O' && *(a1 + 4) == 'R') {
		_vm->_globals.SORTIE = (int16)READ_LE_UINT16(a1 + 5);
		v1 = 5;
	}
	if (*(a1 + 2) == 'B' && *(a1 + 3) == 'C' && *(a1 + 4) == 'A') {
		_vm->_globals.CACHE_OFF((int16)READ_LE_UINT16(a1 + 5));
		v1 = 1;
	}
	if (*(a1 + 2) == 'A' && *(a1 + 3) == 'N' && *(a1 + 4) == 'I') {
		int v75 = (int16)READ_LE_UINT16(a1 + 5);
		if (v75 <= 100)
			_vm->_objectsManager.BOBANIM_ON(v75);
		else
			_vm->_objectsManager.BOBANIM_OFF(v75 - 100);
		v1 = 1;
	}
	if (*(a1 + 2) == 'S' && *(a1 + 3) == 'P' && *(a1 + 4) == 'E') {
		int v76 = (int16)READ_LE_UINT16(a1 + 5);
		if (v76 == 7)
			_vm->_talkManager.PARLER_PERSO("rueh1.pe2");
		if (v76 == 8)
			_vm->_talkManager.PARLER_PERSO("ruef1.pe2");
		if (v76 == 6) {
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.OPTI_ONE(20, 0, 14, 4);
		}
		if (v76 == 12) {
			_vm->_fontManager.TEXTE_OFF(9);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.VBL();
			_vm->_talkManager.PARLER_PERSO("bqetueur.pe2");
		}
		if (v76 == 600) {
			if (!_vm->_globals.internet) {
				_vm->_graphicsManager.FADE_LINUX = 2;
				_vm->_graphicsManager.FADESPD = 1;
				if (_vm->_globals.SVGA == 2)
					_vm->_animationManager.PLAY_ANM("BOMBE1A.ANM", 100, 18, 100);
				if (_vm->_globals.SVGA == 1)
					_vm->_animationManager.PLAY_ANM("BOMBE1.ANM", 100, 18, 100);
			}
			_vm->_graphicsManager.LOAD_IMAGE("BOMBEB");
			_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
			_vm->_graphicsManager.OPTI_INI("BOMBE", 2);
			_vm->_graphicsManager.FADE_INS();
		}
		if (v76 == 601) {
			_vm->_globals.NO_VISU = true;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = false;
			_vm->_objectsManager.OPTI_ONE(2, 0, 16, 4);
		}
		if (v76 == 602) {
			_vm->_globals.NO_VISU = true;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = false;
			_vm->_objectsManager.OPTI_ONE(4, 0, 16, 4);
		}
		if (v76 == 603) {
			_vm->_globals.NO_VISU = true;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = false;
			_vm->_objectsManager.OPTI_ONE(3, 0, 16, 4);
			_vm->_soundManager.SPECIAL_SOUND = 199;
			_vm->_graphicsManager.FADE_LINUX = 2;
			if (_vm->_globals.SVGA == 1)
				_vm->_animationManager.PLAY_ANM("BOMBE2.ANM", 50, 14, 500);
			if (_vm->_globals.SVGA == 2)
				_vm->_animationManager.PLAY_ANM("BOMBE2A.ANM", 50, 14, 500);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			memset(_vm->_graphicsManager.VESA_BUFFER, 0, 0x96000u);
			_vm->_graphicsManager.NOFADE = true;
			_vm->_globals.SORTIE = 151;
		}
		if (v76 == 604) {
			_vm->_globals.NO_VISU = true;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = false;
			_vm->_objectsManager.OPTI_ONE(1, 0, 16, 4);
			_vm->_soundManager.SPECIAL_SOUND = 199;
			if (_vm->_globals.SVGA == 1)
				_vm->_animationManager.PLAY_ANM("BOMBE2.ANM", 50, 14, 500);
			if (_vm->_globals.SVGA == 2)
				_vm->_animationManager.PLAY_ANM("BOMBE2A.ANM", 50, 14, 500);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_graphicsManager.NOFADE = true;
			memset(_vm->_graphicsManager.VESA_BUFFER, 0, 0x96000u);
			_vm->_globals.SORTIE = 151;
		}
		if (v76 == 605) {
			_vm->_globals.NO_VISU = true;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = false;
			_vm->_objectsManager.OPTI_ONE(5, 0, 16, 4);
			_vm->_graphicsManager.FADE_OUTS();
			_vm->_soundManager.SPECIAL_SOUND = 199;
			_vm->_graphicsManager.FADE_LINUX = 2;
			if (_vm->_globals.SVGA == 1)
				_vm->_animationManager.PLAY_ANM("BOMBE2.ANM", 50, 14, 500);
			if (_vm->_globals.SVGA == 2)
				_vm->_animationManager.PLAY_ANM("BOMBE2A.ANM", 50, 14, 500);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_graphicsManager.NOFADE = true;
			memset(_vm->_graphicsManager.VESA_BUFFER, 0, 0x96000u);
			_vm->_globals.SORTIE = 151;
		}
		if (v76 == 606) {
			_vm->_globals.NO_VISU = true;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = false;
			_vm->_objectsManager.OPTI_ONE(6, 0, 16, 4);
			if (_vm->_globals.SVGA == 1)
				_vm->_animationManager.PLAY_ANM("BOMBE3.ANM", 50, 14, 500);
			if (_vm->_globals.SVGA == 2)
				_vm->_animationManager.PLAY_ANM("BOMBE3A.ANM", 50, 14, 500);
			memset(_vm->_graphicsManager.VESA_BUFFER, 0, 0x96000u);
			_vm->_globals.SORTIE = 6;
		}
		if (v76 == 607) {
			if (!_vm->_globals.internet) {
				memcpy(_vm->_graphicsManager.OLD_PAL, _vm->_graphicsManager.Palette, 0x301u);
				v1 = _vm->_graphicsManager.OLD_PAL[769];
				_vm->_animationManager.PLAY_ANM2("PLAN.ANM", 50, 10, 800);
			}
			_vm->_globals.NBBLOC = 0;
		}
		if (v76 == 608) {
			_vm->_objectsManager.BOBANIM_OFF(2);
			_vm->_objectsManager.BOBANIM_OFF(3);
			_vm->_objectsManager.BOBANIM_OFF(4);
			_vm->_objectsManager.BOBANIM_OFF(6);
			_vm->_objectsManager.BOBANIM_OFF(11);
			_vm->_objectsManager.BOBANIM_OFF(10);
		}
		if (v76 == 609) {
			_vm->_objectsManager.BOBANIM_ON(2);
			_vm->_objectsManager.BOBANIM_ON(3);
			_vm->_objectsManager.BOBANIM_ON(4);
			_vm->_objectsManager.BOBANIM_ON(6);
			_vm->_objectsManager.BOBANIM_ON(11);
			_vm->_objectsManager.BOBANIM_ON(10);
		}
		if (v76 == 611) {
			_vm->_objectsManager.BOBANIM_ON(5);
			_vm->_objectsManager.BOBANIM_ON(7);
			_vm->_objectsManager.BOBANIM_ON(8);
			_vm->_objectsManager.BOBANIM_ON(9);
			_vm->_objectsManager.BOBANIM_ON(12);
			_vm->_objectsManager.BOBANIM_ON(13);
		}
		if (v76 == 610) {
			_vm->_objectsManager.BOBANIM_OFF(5);
			_vm->_objectsManager.BOBANIM_OFF(7);
			_vm->_objectsManager.BOBANIM_OFF(8);
			_vm->_objectsManager.BOBANIM_OFF(9);
			_vm->_objectsManager.BOBANIM_OFF(12);
			_vm->_objectsManager.BOBANIM_OFF(13);
		}
		if (v76 == 10)
			_vm->_talkManager.PARLER_PERSO("bqeflic1.pe2");
		if (v76 == 11)
			_vm->_talkManager.PARLER_PERSO("bqeflic2.pe2");
		if (v76 == 16)
			_vm->_talkManager.PARLER_PERSO("ftoubib.pe2");
		if (v76 == 17)
			_vm->_talkManager.PARLER_PERSO("flic2b.pe2");
		if (v76 == 18)
			_vm->_talkManager.PARLER_PERSO("fjour.pe2");
		if (v76 == 13) {
			_vm->_eventsManager.souris_b = _vm->_eventsManager.souris_bb;
			_vm->_globals.DESACTIVE_INVENT = true;
			_vm->_graphicsManager.FADE_OUTW();
			_vm->_globals.CACHE_OFF();
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_fontManager.TEXTE_OFF(5);
			_vm->_fontManager.TEXTE_OFF(9);
			_vm->_graphicsManager.FIN_VISU();
			_vm->_objectsManager.CLEAR_ECRAN();
			_vm->_soundManager.PLAY_SOUND("SOUND17.WAV");
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_animationManager.PLAY_SEQ2("HELICO.SEQ", 10, 4, 10);
			_vm->_animationManager.CHARGE_ANIM("otage");
			_vm->_graphicsManager.LOAD_IMAGE("IM05");
			_vm->_graphicsManager.VISU_ALL();

			for (int i = 0; i <= 4; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			}

			_vm->_eventsManager.MOUSE_OFF();
			_vm->_graphicsManager.FADE_INW_LINUX(_vm->_graphicsManager.VESA_BUFFER);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(3) != 100);
			_vm->_graphicsManager.FADE_OUTW_LINUX(_vm->_graphicsManager.VESA_BUFFER);
			_vm->_graphicsManager.FIN_VISU();
			if (!_vm->_globals.CENSURE) {
				_vm->_soundManager.SPECIAL_SOUND = 16;
				_vm->_graphicsManager.FADE_LINUX = 2;
				_vm->_animationManager.PLAY_ANM("EGORGE.ANM", 50, 28, 500);
				_vm->_soundManager.SPECIAL_SOUND = 0;
			}
			_vm->_animationManager.CHARGE_ANIM("ASCEN");
			_vm->_eventsManager.MOUSE_OFF();
			_vm->_graphicsManager.LOAD_IMAGE("ASCEN");
			_vm->_graphicsManager.VISU_ALL();

			for (int i = 0; i <= 4; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			}

			_vm->_eventsManager.MOUSE_OFF();
			_vm->_graphicsManager.FADE_INW_LINUX(_vm->_graphicsManager.VESA_BUFFER);
			_vm->_objectsManager.SCI_OPTI_ONE(1, 0, 17, 3);
			_vm->_graphicsManager.FADE_OUTW_LINUX(_vm->_graphicsManager.VESA_BUFFER);
			_vm->_graphicsManager.FIN_VISU();
			_vm->_soundManager.SPECIAL_SOUND = 14;
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_animationManager.PLAY_SEQ2("ASSOM.SEQ", 10, 4, 500);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_globals.DESACTIVE_INVENT = false;
			_vm->_globals.HELICO = 1;
		}
		if (v76 == 36) {
			if (_vm->_globals.SAUVEGARDE->data[svField270] == 2 && _vm->_globals.SAUVEGARDE->data[svField94] == 1 && _vm->_globals.SAUVEGARDE->data[svField95] == 1)
				_vm->_globals.SAUVEGARDE->data[svField270] = 3;
			if (!_vm->_globals.SAUVEGARDE->data[svField270])
				_vm->_talkManager.PARLER_PERSO2("PATRON0.pe2");
			if (_vm->_globals.SAUVEGARDE->data[svField270] == 1)
				_vm->_talkManager.PARLER_PERSO2("PATRON1.pe2");
			if (_vm->_globals.SAUVEGARDE->data[svField270] == 2)
				_vm->_talkManager.PARLER_PERSO2("PATRON2.pe2");
			if (_vm->_globals.SAUVEGARDE->data[svField270] == 3)
				_vm->_talkManager.PARLER_PERSO2("PATRON3.pe2");
			if (_vm->_globals.SAUVEGARDE->data[svField270] > 3) {
				_vm->_talkManager.PARLER_PERSO2("PATRON4.pe2");
				_vm->_globals.SAUVEGARDE->data[svField270] = 5;
			}
		}
		if (v76 == 37) {
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_animationManager.PLAY_SEQ2("corde.SEQ", 32, 32, 100);
			_vm->_graphicsManager.NOFADE = true;
		}
		if (v76 == 38) {
			_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND44.WAV");
			_vm->_soundManager.CHARGE_SAMPLE(2, "SOUND42.WAV");
			_vm->_soundManager.CHARGE_SAMPLE(3, "SOUND41.WAV");
			_vm->_soundManager.SPECIAL_SOUND = 17;
			_vm->_animationManager.PLAY_SEQ("grenade.SEQ", 1, 32, 100);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_animationManager.PLAY_ANM("CREVE17.ANM", 24, 24, 200);
			_vm->_soundManager.DEL_SAMPLE(1);
			_vm->_soundManager.DEL_SAMPLE(2);
			_vm->_soundManager.DEL_SAMPLE(3);
			_vm->_graphicsManager.NOFADE = true;
		}
		if (v76 == 29) {
			_vm->_globals.DESACTIVE_INVENT = true;
			_vm->_talkManager.OBJET_VIVANT("TELEP.pe2");
			_vm->_globals.DESACTIVE_INVENT = false;
		}
		if (v76 == 22)
			_vm->_talkManager.OBJET_VIVANT("CADAVRE1.pe2");
		if (v76 == 20)
			_vm->_talkManager.PARLER_PERSO("PUNK.pe2");
		if (v76 == 23)
			_vm->_talkManager.PARLER_PERSO2("CHERCHE1.pe2");
		if (v76 == 35) {
			if (!_vm->_soundManager.SOUNDOFF) {
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					_vm->_eventsManager.VBL();
				} while (_vm->_soundManager.SOUND_FLAG);
			}
			_vm->_talkManager.PARLER_PERSO("PTLAB.pe2");
		}
		if (v76 == 46) {
			_vm->_globals.NOT_VERIF = 1;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			int v13 = _vm->_objectsManager.YSPR(0);
			int v14 = _vm->_objectsManager.XSPR(0);
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v14, v13, 564, 420);
			_vm->_objectsManager.NUMZONE = -1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != (int16 *)g_PTRNUL);
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_globals.NOT_VERIF = 1;
			_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND44.WAV");
			_vm->_soundManager.CHARGE_SAMPLE(2, "SOUND45.WAV");
			_vm->_objectsManager.OPTI_BOBON(9, 10, -1, 0, 0, 0, 0);
			int v15 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.BOBPOSI(9) == 4 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(9) == 5)
					v15 = 0;
				if (_vm->_objectsManager.BOBPOSI(9) == 16 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(9) == 17)
					v15 = 0;
				if (_vm->_objectsManager.BOBPOSI(9) == 28 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(9) == 29)
					v15 = 0;
				if (_vm->_objectsManager.BOBPOSI(10) == 10 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(10) == 11)
					v15 = 0;
				if (_vm->_objectsManager.BOBPOSI(10) == 22 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(10) == 23)
					v15 = 0;
				if (_vm->_objectsManager.BOBPOSI(10) == 33 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(10) == 34)
					v15 = 0;
				if (_vm->_objectsManager.BOBPOSI(10) == 12)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 249, 1);
				if (_vm->_objectsManager.BOBPOSI(10) == 23)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 249, 2);
				if (_vm->_objectsManager.BOBPOSI(10) == 34)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 249, 3);
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(9) != 36);
			_vm->_objectsManager.SPRITE_ON(0);
			_vm->_objectsManager.BOBANIM_OFF(9);
			_vm->_objectsManager.BOBANIM_OFF(10);
			_vm->_soundManager.DEL_SAMPLE(1);
			_vm->_soundManager.DEL_SAMPLE(2);
		}
		if (v76 == 59) {
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.g_old_x = _vm->_objectsManager.XSPR(0);
			_vm->_globals.g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			int v16 = _vm->_objectsManager.YSPR(0);
			int v17 = _vm->_objectsManager.XSPR(0);
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v17, v16, 445, 332);
			_vm->_globals.NOT_VERIF = 1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != (int16 *)g_PTRNUL);
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.BOBANIM_ON(7);
			_vm->_objectsManager.SET_BOBPOSI(7, 0);
			int v18 = 0;
			_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND40.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.BOBPOSI(7) == 10 && !v18) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v18 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(7) == 11)
					v18 = 0;
				if (_vm->_objectsManager.BOBPOSI(7) == 18 && !v18) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v18 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(7) == 19)
					v18 = 0;
				if (_vm->_objectsManager.BOBPOSI(7) == 19)
					_vm->_objectsManager.BOBANIM_ON(3);
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(3) != 48);
			_vm->_soundManager.DEL_SAMPLE(1);
			_vm->_objectsManager.SETANISPR(0, 62);
			_vm->_objectsManager.SPRITE_ON(0);
			_vm->_objectsManager.BOBANIM_ON(6);
			_vm->_objectsManager.BOBANIM_OFF(7);
			_vm->_objectsManager.BOBANIM_OFF(3);
		}
		if (v76 == 50) {
			_vm->_soundManager.PLAY_SOUND("SOUND46.WAv");
			_vm->_objectsManager.OPTI_ONE(11, 0, 23, 0);
		}
		if (v76 == 49) {
			_vm->_globals.CACHE_OFF();
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.OPTI_BOBON(9, 10, -1, 0, 0, 0, 0);
			int v19 = 12;
			if (_vm->_globals.SAUVEGARDE->data[svField133] == 1)
				v19 = 41;
			int v20 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.BOBPOSI(9) == 4 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND44.WAV");
					v20 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(9) == 5)
					v20 = 0;
				if (_vm->_objectsManager.BOBPOSI(9) == 18 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND46.WAV");
					v20 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(9) == 19)
					v20 = 0;
				if (_vm->_objectsManager.BOBPOSI(10) == 11 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND45.WAV");
					v20 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(10) == 12)
					v20 = 0;
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(9) != v19);
			if (v19 == 12) {
				_vm->_objectsManager.SPRITE_ON(0);
				_vm->_objectsManager.BOBANIM_OFF(9);
			}
			_vm->_globals.CACHE_ON();
		}
		if (v76 == 80) {
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.BOBANIM_ON(12);
			_vm->_objectsManager.BOBANIM_ON(13);
			_vm->_objectsManager.SET_BOBPOSI(12, 0);
			_vm->_objectsManager.SET_BOBPOSI(13, 0);
			int v21 = 0;
			_vm->_soundManager.LOAD_WAV("SOUND44.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND71.WAV", 2);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.BOBPOSI(12) == 4 && !v21) {
					_vm->_soundManager.PLAY_WAV(1);
					v21 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(12) == 5)
					v21 = 0;
				if (_vm->_objectsManager.BOBPOSI(4) == 5 && !v21) {
					_vm->_soundManager.PLAY_WAV(2);
					v21 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(4) == 6)
					v21 = 0;
				if (_vm->_objectsManager.BOBPOSI(13) == 8) {
					_vm->_objectsManager.BOBANIM_OFF(13);
					_vm->_objectsManager.BOBANIM_OFF(3);
					_vm->_objectsManager.BOBANIM_ON(4);
					_vm->_objectsManager.SET_BOBPOSI(4, 0);
					_vm->_objectsManager.SET_BOBPOSI(13, 0);
				}
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(4) != 16);
			_vm->_objectsManager.BOBANIM_OFF(12);
			_vm->_objectsManager.BOBANIM_OFF(4);
			_vm->_objectsManager.SPRITE_ON(0);
			_vm->_objectsManager.OBSSEUL = 1;
			_vm->_objectsManager.INILINK("IM27a");
			_vm->_objectsManager.OBSSEUL = 0;
		}
		if (v76 == 81) {
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.g_old_x = _vm->_objectsManager.XSPR(0);
			_vm->_globals.g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			int v22 = _vm->_objectsManager.YSPR(0);
			int v23 = _vm->_objectsManager.XSPR(0);
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v23, v22, 119, 268);
			_vm->_globals.NOT_VERIF = 1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != (int16 *)g_PTRNUL);
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.BOBANIM_ON(11);
			_vm->_objectsManager.BOBANIM_ON(8);
			_vm->_objectsManager.SET_BOBPOSI(11, 0);
			_vm->_objectsManager.SET_BOBPOSI(8, 0);
			_vm->_soundManager.LOAD_WAV("SOUND44.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND48.WAV", 2);
			_vm->_soundManager.LOAD_WAV("SOUND49.WAV", 3);
			int v24 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.BOBPOSI(11) == 4 && !v24) {
					_vm->_soundManager.PLAY_WAV(1);
					v24 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(11) == 5)
					v24 = 0;
				if (_vm->_objectsManager.BOBPOSI(8) == 11 && !v24) {
					_vm->_soundManager.PLAY_WAV(2);
					v24 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(8) == 12)
					v24 = 0;
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(8) != 32);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 201, 14, 1);
			_vm->_objectsManager.SPRITE_ON(0);
			_vm->_objectsManager.BOBANIM_OFF(11);
			_vm->_objectsManager.BOBANIM_OFF(8);
			_vm->_objectsManager.BOBANIM_ON(5);
			_vm->_objectsManager.BOBANIM_ON(6);
			_vm->_objectsManager.SET_BOBPOSI(5, 0);
			_vm->_objectsManager.SET_BOBPOSI(6, 0);
			_vm->_soundManager.PLAY_WAV(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(5) != 74);
			_vm->_objectsManager.BOBANIM_OFF(5);
			_vm->_objectsManager.BOBANIM_OFF(6);
			_vm->_objectsManager.BOBANIM_ON(9);
			_vm->_objectsManager.BOBANIM_ON(7);
		}
		if (v76 == 95) {
			_vm->_objectsManager.BOBANIM_ON(9);
			_vm->_objectsManager.BOBANIM_ON(10);
			_vm->_objectsManager.BOBANIM_ON(12);
			_vm->_objectsManager.SET_BOBPOSI(9, 0);
			_vm->_objectsManager.SET_BOBPOSI(10, 0);
			_vm->_objectsManager.SET_BOBPOSI(12, 0);
			_vm->_objectsManager.SPRITE_OFF(0);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(9) != 15);
			_vm->_objectsManager.BOBANIM_OFF(9);
			_vm->_objectsManager.SPRITE_ON(0);
			_vm->_soundManager.PLAY_SOUND("SOUND50.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(12) != 117);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 830, 122, 0);
			_vm->_objectsManager.BOBANIM_OFF(12);
			_vm->_objectsManager.BOBANIM_OFF(10);
			_vm->_objectsManager.BOBANIM_ON(11);
		}
		if (v76 == 85) {
			_vm->_objectsManager.BOBANIM_OFF(3);
			_vm->_objectsManager.BOBANIM_ON(5);
			_vm->_objectsManager.SET_BOBPOSI(5, 0);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(5) != 6);
			_vm->_objectsManager.BOBANIM_OFF(5);
			_vm->_objectsManager.BOBANIM_ON(6);
			_vm->_objectsManager.OBSSEUL = 1;
			_vm->_objectsManager.INILINK("IM24a");
			_vm->_objectsManager.OBSSEUL = 0;
		}
		if (v76 == 88) {
			if (_vm->_globals.SAUVEGARDE->data[svField183] == 1) {
				_vm->_objectsManager.SET_BOBPOSI(1, 0);
				_vm->_objectsManager.SET_BOBPOSI(2, 0);
				_vm->_objectsManager.BOBANIM_ON(1);
				_vm->_objectsManager.BOBANIM_ON(2);
				_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND40.WAV");
				int v25 = 0;
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					if (_vm->_objectsManager.BOBPOSI(1) == 1 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(1) == 2)
						v25 = 0;
					if (_vm->_objectsManager.BOBPOSI(1) == 3 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(1) == 4)
						v25 = 0;
					if (_vm->_objectsManager.BOBPOSI(1) == 5 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(1) == 6)
						v25 = 0;
					if (_vm->_objectsManager.BOBPOSI(1) == 7 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(1) == 8)
						v25 = 0;
					_vm->_eventsManager.VBL();
				} while (_vm->_objectsManager.BOBPOSI(1) != 9);
				_vm->_objectsManager.BOBANIM_OFF(1);
				_vm->_objectsManager.BOBANIM_OFF(2);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 160, 6);
				_vm->_soundManager.DEL_SAMPLE(1);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField183] == 2) {
				_vm->_objectsManager.SET_BOBPOSI(1, 0);
				_vm->_objectsManager.SET_BOBPOSI(3, 0);
				_vm->_objectsManager.BOBANIM_ON(1);
				_vm->_objectsManager.BOBANIM_ON(3);
				_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND40.WAV");
				int v26 = 0;
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					if (_vm->_objectsManager.BOBPOSI(1) == 1 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(1) == 2)
						v26 = 0;
					if (_vm->_objectsManager.BOBPOSI(1) == 3 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(1) == 4)
						v26 = 0;
					if (_vm->_objectsManager.BOBPOSI(1) == 5 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(1) == 6)
						v26 = 0;
					if (_vm->_objectsManager.BOBPOSI(1) == 7 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(1) == 8)
						v26 = 0;
					_vm->_eventsManager.VBL();
				} while (_vm->_objectsManager.BOBPOSI(1) != 9);
				_vm->_objectsManager.BOBANIM_OFF(1);
				_vm->_objectsManager.BOBANIM_OFF(3);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 161, 8);
				_vm->_soundManager.DEL_SAMPLE(1);
			}
		}
		if (v76 == 90) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField186]) {
				_vm->_animationManager.PLAY_SEQ("CIB5A.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField186] == 1) {
				_vm->_animationManager.PLAY_SEQ("CIB5C.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 329, 87, 2);
			}
		}
		if (v76 == 91) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField186]) {
				_vm->_animationManager.PLAY_SEQ("CIB5B.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField186] == 1) {
				_vm->_animationManager.PLAY_SEQ("CIB5D.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 160, 6);
			}
		}
		if (v76 == 92) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField184]) {
				_vm->_animationManager.PLAY_SEQ("CIB6A.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField184] == 1) {
				_vm->_animationManager.PLAY_SEQ("CIB6C.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 293, 139, 3);
			}
		}
		if (v76 == 93) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField184]) {
				_vm->_animationManager.PLAY_SEQ("CIB6B.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField184] == 1) {
				_vm->_animationManager.PLAY_SEQ("CIB6D.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 161, 8);
			}
		}
		if (v76 == 62)
			_vm->_talkManager.OBJET_VIVANT("SBCADA.pe2");
		if (v76 == 65)
			_vm->_talkManager.OBJET_VIVANT("ScCADA.pe2");
		if (v76 == 105) {
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.g_old_x = _vm->_objectsManager.XSPR(0);
			_vm->_globals.g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 1) {
				int v27 = _vm->_objectsManager.YSPR(0);
				int v28 = _vm->_objectsManager.XSPR(0);
				_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v28, v27, 201, 294);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 2) {
				int v29 = _vm->_objectsManager.YSPR(0);
				int v30 = _vm->_objectsManager.XSPR(0);
				_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v30, v29, 158, 338);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] > 2) {
				int v31 = _vm->_objectsManager.YSPR(0);
				int v32 = _vm->_objectsManager.XSPR(0);
				_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v32, v31, 211, 393);
			}
			_vm->_globals.NOT_VERIF = 1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != (int16 *)g_PTRNUL);
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.SETANISPR(0, 60);
			_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND63.WAV");
			if (_vm->_globals.SAUVEGARDE->data[svField253] > 2) {
				_vm->_objectsManager.BOBANIM_ON(4);
				int v33 = 0;
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					if (_vm->_objectsManager.BOBPOSI(4) == 9 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(4) == 10)
						v33 = 0;
					if (_vm->_objectsManager.BOBPOSI(4) == 32 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(4) == 33)
						v33 = 0;
					if (_vm->_objectsManager.BOBPOSI(4) == 55 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(4) == 56)
						v33 = 0;
					_vm->_eventsManager.VBL();
				} while (_vm->_objectsManager.BOBPOSI(4) != 72);
				_vm->_objectsManager.BOBANIM_OFF(4);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 1) {
				_vm->_objectsManager.BOBANIM_ON(6);
				int v34 = 0;
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					if (_vm->_objectsManager.BOBPOSI(6) == 9 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(6) == 10)
						v34 = 0;
					if (_vm->_objectsManager.BOBPOSI(6) == 32 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(6) == 33)
						v34 = 0;
					if (_vm->_objectsManager.BOBPOSI(6) == 55 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(6) == 56)
						v34 = 0;
					_vm->_eventsManager.VBL();
				} while (_vm->_objectsManager.BOBPOSI(6) != 72);
				_vm->_objectsManager.BOBANIM_OFF(6);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 2) {
				_vm->_objectsManager.BOBANIM_ON(5);
				int v35 = 0;
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					if (_vm->_objectsManager.BOBPOSI(5) == 9 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(5) == 10)
						v35 = 0;
					if (_vm->_objectsManager.BOBPOSI(5) == 32 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(5) == 33)
						v35 = 0;
					if (_vm->_objectsManager.BOBPOSI(5) == 55 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if (_vm->_objectsManager.BOBPOSI(5) == 56)
						v35 = 0;
					_vm->_eventsManager.VBL();
				} while (_vm->_objectsManager.BOBPOSI(5) != 72);
				_vm->_objectsManager.BOBANIM_OFF(5);
			}
			_vm->_objectsManager.SPRITE_ON(0);
			_vm->_objectsManager.ACTION_DOS(1);
			_vm->_soundManager.DEL_SAMPLE(1);
		}
		if (v76 == 106) {
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.BOBANIM_ON(4);
			_vm->_objectsManager.SET_BOBPOSI(4, 0);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND62.WAV", 2);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(4) != 10);
			_vm->_soundManager.PLAY_WAV(1);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(4) != 18);
			_vm->_soundManager.PLAY_WAV(2);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(4) != 62);
			_vm->_soundManager.PLAY_WAV(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(4) != 77);
			_vm->_objectsManager.BOBANIM_OFF(4);
			_vm->_objectsManager.SPRITE_ON(0);
		}
		if (v76 == 107) {
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.BOBANIM_ON(5);
			_vm->_objectsManager.SET_BOBPOSI(5, 0);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND62.WAV", 2);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(5) != 10);
			_vm->_soundManager.PLAY_WAV(1);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(5) != 18);
			_vm->_soundManager.PLAY_WAV(2);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(5) != 38);
			_vm->_soundManager.PLAY_WAV(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(5) != 53);
			_vm->_objectsManager.BOBANIM_OFF(5);
			_vm->_objectsManager.SPRITE_ON(0);
		}
		if (v76 == 210) {
			_vm->_animationManager.NO_SEQ = true;
			_vm->_soundManager.SPECIAL_SOUND = 210;
			_vm->_animationManager.PLAY_SEQ2("SECRET1.SEQ", 1, 12, 1);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 192, 152, 0);
			_vm->_objectsManager.BOBANIM_ON(9);
			_vm->_objectsManager.OBSSEUL = 1;
			_vm->_objectsManager.INILINK("IM73a");
			_vm->_objectsManager.OBSSEUL = 0;
			_vm->_globals.CACHE_ON();
			_vm->_animationManager.NO_SEQ = false;
			_vm->_globals.CACHE_ADD(0);
			_vm->_globals.CACHE_ADD(1);
			_vm->_graphicsManager.SETCOLOR4(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(254, 0, 0, 0);
		}
		if (v76 == 211) {
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_globals.CACHE_OFF();
			_vm->_animationManager.NO_SEQ = true;
			_vm->_globals.NO_VISU = false;
			_vm->_soundManager.SPECIAL_SOUND = 211;
			_vm->_animationManager.PLAY_SEQ("SECRET2.SEQ", 1, 12, 100);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_animationManager.NO_SEQ = false;
			_vm->_graphicsManager.NOFADE = true;
			_vm->_graphicsManager.FADE_OUTW();

			for (int i = 1; i <= 39; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			}

			_vm->_graphicsManager.SETCOLOR4(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(254, 0, 0, 0);
		}
		if (v76 == 207)
			_vm->_talkManager.OBJET_VIVANT("PANNEAU.PE2");
		if (v76 == 208) {
			_vm->_globals.DESACTIVE_INVENT = true;
			if (_vm->_globals.SAUVEGARDE->data[svField6] != _vm->_globals.SAUVEGARDE->data[svField401]) {
				_vm->_soundManager.SPECIAL_SOUND = 208;
				_vm->_eventsManager.NOESC = true;
				_vm->_animationManager.PLAY_SEQ("SORT.SEQ", 10, 4, 10);
				_vm->_eventsManager.NOESC = false;
				_vm->_soundManager.SPECIAL_SOUND = 0;
			}
			_vm->_globals.NOT_VERIF = 1;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			int v37 = _vm->_objectsManager.YSPR(0);
			int v38 = _vm->_objectsManager.XSPR(0);
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v38, v37, 330, 418);
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.NUMZONE = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != (int16 *)g_PTRNUL);
			_vm->_objectsManager.SETANISPR(0, 64);
			_vm->_globals.SORTIE = _vm->_globals.SAUVEGARDE->data[svField401];
			_vm->_globals.DESACTIVE_INVENT = false;
		}
		if (v76 == 209) {
			_vm->_objectsManager.SET_BOBPOSI(1, 0);
			_vm->_objectsManager.SET_BOBPOSI(2, 0);
			_vm->_objectsManager.SETANISPR(0, 60);
			_vm->_objectsManager.BOBANIM_OFF(4);
			_vm->_objectsManager.BOBANIM_ON(1);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(1) != 9);
			_vm->_objectsManager.BOBANIM_OFF(1);
			_vm->_globals.NO_VISU = true;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			_vm->_globals.NOT_VERIF = 1;
			int v39 = _vm->_objectsManager.YSPR(0);
			int v40 = _vm->_objectsManager.XSPR(0);
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v40, v39, 330, 314);
			_vm->_objectsManager.NUMZONE = 0;
			_vm->_globals.NOT_VERIF = 1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != (int16 *)g_PTRNUL);
			_vm->_objectsManager.SETANISPR(0, 64);
			_vm->_objectsManager.BOBANIM_ON(2);
			_vm->_soundManager.PLAY_SOUND("SOUND66.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(2) != 10);
			_vm->_objectsManager.BOBANIM_OFF(2);
			_vm->_objectsManager.BOBANIM_ON(4);
		}
		if (v76 == 201) {
			_vm->_objectsManager.BOBANIM_ON(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(3) != 18);
			_vm->_objectsManager.BOBANIM_OFF(3);
			_vm->_objectsManager.BOBANIM_ON(4);
		}
		if (v76 == 203) {
			_vm->_globals.NO_VISU = true;
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.BOBANIM_ON(4);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
				if (_vm->_objectsManager.BOBPOSI(4) == 18)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 18, 334, 0);
			} while (_vm->_objectsManager.BOBPOSI(4) != 26);
			_vm->_objectsManager.BOBANIM_OFF(4);
			_vm->_globals.NO_VISU = false;
			_vm->_objectsManager.SPRITE_ON(0);
		}
		if (v76 == 204) {
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.BOBANIM_ON(3);
			_vm->_soundManager.LOAD_WAV("SOUND67.WAV", 1);
			int v41 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.BOBPOSI(3) == 10 && !v41) {
					_vm->_soundManager.PLAY_WAV(1);
					v41 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(3) == 11)
					v41 = 0;
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(3) != 50);
			_vm->_objectsManager.BOBANIM_OFF(3);
			_vm->_objectsManager.SPRITE_ON(0);
		}
		if (v76 == 205) {
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.BOBANIM_ON(4);
			_vm->_soundManager.LOAD_WAV("SOUND69.WAV", 1);
			int v42 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.BOBPOSI(4) == 10 && !v42) {
					_vm->_soundManager.PLAY_WAV(1);
					v42 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(4) == 11)
					v42 = 0;
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(4) != 24);
			_vm->_objectsManager.BOBANIM_OFF(4);
			_vm->_objectsManager.SPRITE_ON(0);
		}
		if (v76 == 175) {
			_vm->_objectsManager.SETANISPR(0, 55);
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.BOBANIM_ON(9);
			_vm->_objectsManager.BOBANIM_ON(10);
			_vm->_objectsManager.BOB_OFFSET(10, 300);
			_vm->_soundManager.PLAY_SOUND("SOUND44.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(10) != 7);
			_vm->_objectsManager.BOBANIM_ON(6);
			_vm->_objectsManager.BOBANIM_OFF(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(6) != 10);
			_vm->_soundManager.PLAY_SOUND("SOUND71.WAV");
			_vm->_objectsManager.BOBANIM_ON(7);
			_vm->_objectsManager.BOBANIM_OFF(4);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(7) != 15);
			_vm->_objectsManager.BOBANIM_OFF(5);
			_vm->_objectsManager.BOBANIM_ON(8);
			_vm->_soundManager.PLAY_SOUND("SOUND70.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(8) != 76);
			_vm->_objectsManager.BOBANIM_OFF(6);
			_vm->_objectsManager.BOBANIM_OFF(7);
			_vm->_objectsManager.BOBANIM_OFF(8);
			_vm->_objectsManager.BOBANIM_OFF(9);
			_vm->_objectsManager.BOBANIM_OFF(10);
			_vm->_objectsManager.SPRITE_ON(0);
		}
		if (v76 == 229) {
			_vm->_soundManager.SPECIAL_SOUND = 229;
			_vm->_animationManager.PLAY_SEQ("MUR.SEQ", 1, 12, 1);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 340, 157, 2);
		}
		if (v76 == 230) {
			_vm->_objectsManager.OBSSEUL = 1;
			_vm->_objectsManager.INILINK("IM93a");
			_vm->_objectsManager.OBSSEUL = 0;
			_vm->_globals.CACHE_ON();
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.g_old_x = _vm->_objectsManager.XSPR(0);
			_vm->_globals.g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			_vm->_globals.NOT_VERIF = 1;
			int v43 = _vm->_objectsManager.YSPR(0);
			int v44 = _vm->_objectsManager.XSPR(0);
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v44, v43, 488, 280);
			_vm->_globals.NOT_VERIF = 1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != (int16 *)g_PTRNUL);
			_vm->_objectsManager.SPRITE_OFF(0);
			int v45 = 0;
			_vm->_objectsManager.BOBANIM_ON(7);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.BOBPOSI(7) == 9 && !v45) {
					v45 = 1;
					_vm->_soundManager.PLAY_SOUND("SOUND81.WAV");
				}
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(7) != 15);
			_vm->_objectsManager.BOBANIM_OFF(7);
			_vm->_objectsManager.SETXSPR(0, 476);
			_vm->_objectsManager.SETYSPR(0, 278);
			_vm->_objectsManager.SPRITE_ON(0);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 337, 154, 3);
			_vm->_objectsManager.OBSSEUL = 1;
			_vm->_objectsManager.INILINK("IM93c");
			_vm->_objectsManager.OBSSEUL = 0;
			_vm->_globals.CACHE_ON();
		}
		if (v76 == 231) {
			_vm->_globals.CACHE_OFF();
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.BOBANIM_ON(12);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(12) != 6);
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("PRMORT.pe2");
			_vm->_globals.NOPARLE = false;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(12) != 12);
			_vm->_objectsManager.SPRITE_ON(0);
			_vm->_objectsManager.BOBANIM_OFF(12);
			_vm->_globals.CACHE_ON();
		}
		if (v76 == 233) {
			_vm->_globals.CACHE_OFF();
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.BOBANIM_ON(11);
			int v46 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
				if (_vm->_objectsManager.BOBPOSI(11) == 10 && !v46)
					v46 = 1;
			} while (_vm->_objectsManager.BOBPOSI(11) != 13);
			_vm->_objectsManager.BOBANIM_OFF(11);
			_vm->_globals.CACHE_ON();
			_vm->_objectsManager.BOBANIM_ON(13);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(13) != 48);
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("HRADIO.PE2");
			_vm->_globals.NOPARLE = false;
			_vm->_graphicsManager.FADE_OUTW();
			_vm->_objectsManager.BOBANIM_OFF(13);
			_vm->_graphicsManager.NOFADE = true;
			_vm->_globals.SORTIE = 94;
		}
		if (v76 == 52) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("GARDE.PE2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 53)
			_vm->_talkManager.PARLER_PERSO("GARDE1.pe2");
		if (v76 == 54)
			_vm->_talkManager.PARLER_PERSO("GARDE2.pe2");
		if (v76 == 40)
			_vm->_talkManager.PARLER_PERSO("MAGE.pe2");
		if (v76 == 236) {
			v47 = _vm->_globals.SAUVEGARDE->data[svField341];
			if (v47) {
				if (v47 == 2)
					v70 = 5;
				if (v47 == 3)
					v70 = 4;
				if (v47 == 1)
					v70 = 6;
				_vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
				_vm->_objectsManager.OPTI_ONE(v70, 26, 50, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 27, 117, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 145, 166, 2);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 296, 212, 4);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_globals.SAUVEGARDE->data[svField338] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_globals.SAUVEGARDE->data[svField339] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_globals.SAUVEGARDE->data[svField340] = 0;
			}
			_vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
			_vm->_objectsManager.OPTI_ONE(6, 0, 23, 0);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 27, 117, 1);
		}
		if (v76 == 237) {
			v48 = _vm->_globals.SAUVEGARDE->data[svField341];
			if (v48) {
				if (v48 == 2)
					v70 = 5;
				if (v48 == 3)
					v70 = 4;
				if (v48 == 1)
					v70 = 6;
				_vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
				_vm->_objectsManager.OPTI_ONE(v70, 26, 50, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 27, 117, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 145, 166, 2);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 296, 212, 4);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_globals.SAUVEGARDE->data[svField338] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_globals.SAUVEGARDE->data[svField339] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_globals.SAUVEGARDE->data[svField340] = 0;
			}
			_vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
			_vm->_objectsManager.OPTI_ONE(5, 0, 23, 0);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 145, 166, 3);
		}
		if (v76 == 238) {
			v49 = _vm->_globals.SAUVEGARDE->data[svField341];
			if (v49) {
				if (v49 == 2)
					v70 = 5;
				if (v49 == 3)
					v70 = 4;
				if (v49 == 1)
					v70 = 6;
				_vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
				_vm->_objectsManager.OPTI_ONE(v70, 26, 50, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 27, 117, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 145, 166, 2);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 296, 212, 4);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_globals.SAUVEGARDE->data[svField338] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_globals.SAUVEGARDE->data[svField339] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_globals.SAUVEGARDE->data[svField340] = 0;
			}
			_vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
			_vm->_objectsManager.OPTI_ONE(4, 0, 23, 0);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 296, 212, 5);
		}
		if (v76 == 239) {
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_soundManager.PLAY_SOUND("SOUND84.WAV");
			_vm->_objectsManager.OPTI_ONE(16, 0, 10, 0);
		}
		if (v76 == 240) {
			_vm->_objectsManager.BOBANIM_ON(1);
			v50 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
				if (_vm->_objectsManager.BOBPOSI(1) == 12 && !v50) {
					_vm->_soundManager.PLAY_SOUND("SOUND86.WAV");
					v50 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(1) == 13)
					v50 = 0;
				if (_vm->_objectsManager.BOBPOSI(1) == 25 && !v50) {
					_vm->_soundManager.PLAY_SOUND("SOUND85.WAV");
					v50 = 1;
				}
				if (_vm->_objectsManager.BOBPOSI(1) == 25)
					v50 = 0;
			} while (_vm->_objectsManager.BOBPOSI(1) != 32);
			_vm->_objectsManager.BOBANIM_OFF(1);
			_vm->_objectsManager.BOBANIM_ON(2);
			_vm->_fontManager.TEXTE_OFF(9);
			if (!_vm->_soundManager.TEXTOFF) {
				_vm->_fontManager.DOS_TEXT(9, 617, _vm->_globals.FICH_TEXTE, 91, 41, 20, 25, 3, 30, 253);
				_vm->_fontManager.TEXTE_ON(9);
			}
			if (!_vm->_soundManager.VOICEOFF)
				_vm->_soundManager.VOICE_MIX(617, 4);
			for (int i = 0; i <= 29; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			}
			v51 = &_vm->_globals.SAUVEGARDE->field370;
			v51->xp = _vm->_objectsManager.XSPR(0);
			v51->yp = _vm->_objectsManager.YSPR(0);
			v51->field2 = 57;
			v51->location = 97;
			_vm->_globals.SAUVEGARDE->data[svField121] = 1;
			_vm->_globals.SAUVEGARDE->data[svField352] = 1;
			_vm->_globals.SAUVEGARDE->data[svField353] = 1;
			_vm->_globals.SAUVEGARDE->data[svField354] = 1;
		}
		if (v76 == 56) {
			_vm->_fileManager.CONSTRUIT_SYSTEM("HOPFEM.SPR");
			_vm->_fileManager.CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
			_vm->_globals.PERSO_TYPE = 1;
			_vm->_globals.SAUVEGARDE->data[svField122] = 1;
			_vm->_globals.HOPKINS_DATA();
			_vm->_objectsManager.Sprite[0].field12 = 28;
			_vm->_objectsManager.Sprite[0].field14 = 155;
			_vm->_objectsManager.VERIFTAILLE();
		}
		if (v76 == 57) {
			_vm->_fileManager.CONSTRUIT_SYSTEM("PERSO.SPR");
			_vm->_fileManager.CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
			_vm->_globals.PERSO_TYPE = 0;
			_vm->_globals.SAUVEGARDE->data[svField122] = 0;
			_vm->_globals.HOPKINS_DATA();
			_vm->_objectsManager.Sprite[0].field12 = 34;
			_vm->_objectsManager.Sprite[0].field14 = 190;
			_vm->_objectsManager.VERIFTAILLE();
		}
		if (v76 == 25)
			_vm->_talkManager.PARLER_PERSO("AGENT1.pe2");
		if (v76 == 26)
			_vm->_talkManager.PARLER_PERSO("AGENT2.pe2");
		if (v76 == 87) {
			if (_vm->_globals.SAUVEGARDE->data[svField188])
				_vm->_talkManager.PARLER_PERSO("stand2.pe2");
			else
				_vm->_talkManager.PARLER_PERSO("stand1.pe2");
		}
		if (v76 == 86) {
			if (_vm->_globals.SAUVEGARDE->data[svField231] == 1) {
				_vm->_talkManager.PARLER_PERSO("chotess1.pe2");
			} else {
				_vm->_globals.NOPARLE = true;
				_vm->_talkManager.PARLER_PERSO("chotesse.pe2");
				_vm->_globals.NOPARLE = false;
			}
		}
		if (v76 == 51) {
			_vm->_graphicsManager.FADE_OUTW();
			_vm->_globals.CACHE_OFF();
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_fontManager.TEXTE_OFF(5);
			_vm->_fontManager.TEXTE_OFF(9);
			_vm->_graphicsManager.FIN_VISU();
			_vm->_graphicsManager.LOAD_IMAGE("IM20f");
			_vm->_animationManager.CHARGE_ANIM("ANIM20f");
			_vm->_graphicsManager.VISU_ALL();
			_vm->_eventsManager.MOUSE_OFF();
			_vm->_graphicsManager.FADE_INW();
			bool v52 = false;
			_vm->_soundManager.LOAD_WAV("SOUND46.WAV", 1);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.BOBPOSI(12) == 5 && !v52) {
					_vm->_soundManager.PLAY_WAV(1);
					v52 = true;
				}
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.BOBPOSI(12) != 34);
			_vm->_objectsManager.BOBANIM_OFF(2);
			_vm->_graphicsManager.FADE_OUTW();
			_vm->_graphicsManager.NOFADE = true;
			_vm->_globals.SORTIE = 20;
		}
		if (v76 == 41)
			_vm->_talkManager.PARLER_PERSO("MORT3.pe2");
		if (v76 == 44)
			_vm->_talkManager.PARLER_PERSO("MORT3A.pe2");
		if (v76 == 42)
			_vm->_talkManager.PARLER_PERSO("MORT2.pe2");
		if (v76 == 43)
			_vm->_talkManager.PARLER_PERSO("MORT1.pe2");
		if (v76 == 47)
			_vm->_talkManager.PARLER_PERSO("BARMAN.pe2");
		if (v76 == 45)
			_vm->_talkManager.PARLER_PERSO("FEM3.pe2");
		if (v76 == 48)
			_vm->_talkManager.PARLER_PERSO("SAMAN2.pe2");
		if (v76 == 21)
			_vm->_talkManager.PARLER_PERSO("MEDLEG.pe2");
		if (v76 == 94) {
			if (!_vm->_globals.SAUVEGARDE->data[svField228])
				_vm->_talkManager.PARLER_PERSO("flicn.pe2");
			if (_vm->_globals.SAUVEGARDE->data[svField228] == 1)
				_vm->_talkManager.PARLER_PERSO("flicn1.pe2");
		}
		if (v76 == 27) {
			if (_vm->_globals.SAUVEGARDE->data[svField94] != 1 || _vm->_globals.SAUVEGARDE->data[svField95] != 1)
				_vm->_talkManager.PARLER_PERSO("STANDAR.pe2");
			else
				_vm->_talkManager.PARLER_PERSO("STANDAR1.pe2");
		}
		if (v76 == 58) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("Gm1.PE2");
			_vm->_globals.SAUVEGARDE->data[svField176] = 1;
			_vm->_globals.SAUVEGARDE->data[svField270] = 2;
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 200) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("Gm2.PE2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 84) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("CVIGIL1.PE2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 98) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("CVIGIL2.PE2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 83)
			_vm->_talkManager.PARLER_PERSO("CVIGIL.pe2");
		if (v76 == 32)
			_vm->_talkManager.PARLER_PERSO("SAMAN.pe2");
		if (v76 == 215) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("aviat.pe2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 216) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("aviat1.pe2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 170)
			_vm->_talkManager.PARLER_PERSO("GRED.pe2");
		if (v76 == 172)
			_vm->_talkManager.PARLER_PERSO("GBLEU.pe2");
		if (v76 == 100)
			_vm->_talkManager.PARLER_PERSO("tourist.pe2");
		if (v76 == 103) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("tourist1.pe2");
			_vm->_globals.NOPARLE = false;
			if (_vm->_globals.SVGA == 1)
				_vm->_animationManager.PLAY_ANM2("T421.ANM", 100, 14, 500);
			if (_vm->_globals.SVGA == 2)
				_vm->_animationManager.PLAY_ANM2("T421a.ANM", 100, 14, 500);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.VBL();
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("tourist2.pe2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 104) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("tourist3.pe2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 108) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("peche1.pe2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 109) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("peche2.pe2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 110) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("peche3.pe2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 111) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("peche4.pe2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 112) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("teint1.pe2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 176) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("gred2.pe2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 177) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("gbleu2.pe2");
			_vm->_globals.NOPARLE = false;
		}
		if (v76 == 113)
			_vm->_talkManager.PARLER_PERSO("teint.pe2");
		if (v76 == 114)
			_vm->_talkManager.PARLER_PERSO("tahibar.pe2");
		if (v76 == 115)
			_vm->_talkManager.PARLER_PERSO("ilebar.pe2");
		if (v76 == 116)
			_vm->_talkManager.PARLER_PERSO("Profred.pe2");
		if (v76 == 101)
			_vm->_talkManager.PARLER_PERSO("tahi1.pe2");
		if (v76 == 243) {
			_vm->_soundManager.PLAY_SOUND("SOUND88.WAV");
			if (_vm->_globals.SAUVEGARDE->data[svField341] == 2) {
				_vm->_animationManager.NO_SEQ = true;
				_vm->_animationManager.PLAY_SEQ("RESU.SEQ", 2, 24, 2);
				_vm->_animationManager.NO_SEQ = false;
			} else {
				_vm->_objectsManager.OPTI_ONE(7, 0, 14, 0);
			}
		}
		if (v76 == 242) {
			_vm->_soundManager.PLAY_SOUND("SOUND87.WAV");
			_vm->_animationManager.NO_SEQ = true;
			_vm->_animationManager.PLAY_SEQ("RESUF.SEQ", 1, 24, 1);
			_vm->_animationManager.NO_SEQ = false;
			v53 = &_vm->_globals.SAUVEGARDE->field380;
			v53->xp = 404;
			v53->yp = 395;
			v53->field2 = 64;
			v53->location = _vm->_globals.ECRAN;

			int v54 = _vm->_globals.STAILLE[790 / 2];
			if (_vm->_globals.STAILLE[790 / 2] < 0)
				v54 = -_vm->_globals.STAILLE[790 / 2];
			v76 = -(100 * (67 - (100 - v54)) / 67);
			v53->field4 = v76;
			_vm->_globals.SAUVEGARDE->data[svField357] = 1;
			_vm->_globals.SAUVEGARDE->data[svField354] = 0;
			_vm->_globals.SAUVEGARDE->data[svField356] = 0;
			_vm->_globals.SAUVEGARDE->data[svField355] = 1;
			_vm->_objectsManager.DEUXPERSO = true;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 373, 191, 3);
			_vm->_objectsManager.SPRITE(_vm->_globals.TETE, v53->xp, v53->yp, 1, 3, v53->field4, 0, 20, 127);
			_vm->_objectsManager.SPRITE_ON(1);
		}
		if (v76 == 245) {
			_vm->_soundManager.PLAY_SOUND("SOUND89.WAV");
			_vm->_objectsManager.OPTI_ONE(5, 0, 6, 0);
			_vm->_globals.ZONEP[4].destX = 276;
			_vm->_objectsManager.VERBE_ON(4, 19);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 285, 379, 0);
			_vm->_globals.SAUVEGARDE->data[svField399] = 1;
		}
		if (v76 == 246) {
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.OPTI_ONE(6, 0, 15, 0);
			_vm->_objectsManager.PERSO_ON = true;
			_vm->_graphicsManager.NB_SCREEN();
			_vm->_animationManager.NO_SEQ = true;
			_vm->_animationManager.PLAY_SEQ2("TUNNEL.SEQ", 1, 18, 20);
			_vm->_animationManager.NO_SEQ = false;
			_vm->_graphicsManager.NOFADE = true;
			_vm->_graphicsManager.FADE_OUTW();
			_vm->_objectsManager.PERSO_ON = false;
			_vm->_globals.SORTIE = 100;
		}
		if (v76 == 55) {
			_vm->_objectsManager.BOBANIM_OFF(1);
			_vm->_objectsManager.OPTI_ONE(15, 0, 12, 0);
			_vm->_objectsManager.BOBANIM_OFF(15);
			_vm->_objectsManager.OBSSEUL = 1;
			_vm->_objectsManager.INILINK("IM19a");
			_vm->_objectsManager.OBSSEUL = 0;
		}
		if (v76 == 241)
			_vm->_talkManager.PARLER_PERSO("RECEP.PE2");
		if (v76 == 171) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("gred1.pe2");
			_vm->_globals.NOPARLE = false;
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.g_old_x = _vm->_objectsManager.XSPR(0);
			_vm->_globals.g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			_vm->_globals.NOT_VERIF = 1;
			int v55 = _vm->_objectsManager.YSPR(0);
			int v56 = _vm->_objectsManager.XSPR(0);
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v56, v55, 361, 325);
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.NUMZONE = -1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != (int16 *)g_PTRNUL);
			_vm->_globals.SORTIE = 59;
		}
		if (v76 == 173) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("gbleu1.pe2");
			_vm->_globals.NOPARLE = false;
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.g_old_x = _vm->_objectsManager.XSPR(0);
			_vm->_globals.g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			_vm->_globals.NOT_VERIF = 1;
			int v57 = _vm->_objectsManager.YSPR(0);
			int v58 = _vm->_objectsManager.XSPR(0);
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v58, v57, 361, 325);
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.NUMZONE = -1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != (int16 *)g_PTRNUL);
			_vm->_globals.SORTIE = 59;
		}
		if (v76 == 174)
			_vm->_talkManager.PARLER_PERSO("Profbl.pe2");
		if (v76 == 202) {
			_vm->_globals.NOPARLE = true;
			_vm->_talkManager.PARLER_PERSO("SVGARD2.PE2");
			_vm->_globals.NOPARLE = false;
		}
		v1 = 1;
	}
	if (*(a1 + 2) == 'E' && *(a1 + 3) == 'I' && *(a1 + 4) == 'F')
		v1 = 4;
	if (*(a1 + 2) == 'V' && *(a1 + 3) == 'A' && *(a1 + 4) == 'L') {
		v1 = 1;
		_vm->_globals.SAUVEGARDE->data[(int16)READ_LE_UINT16(a1 + 5)] = (int16)READ_LE_UINT16(a1 + 7);
	}
	if (*(a1 + 2) == 'A' && *(a1 + 3) == 'D' && *(a1 + 4) == 'D') {
		v1 = 1;
		_vm->_globals.SAUVEGARDE->data[(int16)READ_LE_UINT16(a1 + 5)] += *(a1 + 7);
	}
	if (*(a1 + 2) == 'B' && *(a1 + 3) == 'O' && *(a1 + 4) == 'S') {
		v1 = 1;
		_vm->_objectsManager.BOB_OFFSET((int16)READ_LE_UINT16(a1 + 5), (int16)READ_LE_UINT16(a1 + 7));
	}
	if (*(a1 + 2) == 'V' && *(a1 + 3) == 'O' && *(a1 + 4) == 'N') {
		_vm->_objectsManager.VERBE_ON((int16)READ_LE_UINT16(a1 + 5), (int16)READ_LE_UINT16(a1 + 7));
		v1 = 1;
	}
	if (*(a1 + 2) == 'Z' && *(a1 + 3) == 'C' && *(a1 + 4) == 'H') {
		_vm->_globals.ZONEP[(int16)READ_LE_UINT16(a1 + 5)].field12 = (int16)READ_LE_UINT16(a1 + 7);
		v1 = 1;
	}
	if (*(a1 + 2) == 'J' && *(a1 + 3) == 'U' && *(a1 + 4) == 'M') {
		int v59 = (int16)READ_LE_UINT16(a1 + 7);
		_vm->_objectsManager.NVZONE = (int16)READ_LE_UINT16(a1 + 5);
		_vm->_objectsManager.NVVERBE = v59;
		v1 = 6;
	}
	if (*(a1 + 2) == 'S' && *(a1 + 3) == 'O' && *(a1 + 4) == 'U') {
		int v60 = (int16)READ_LE_UINT16(a1 + 5);
		
		file = Common::String::format("SOUND%d.WAV", v60);
		_vm->_soundManager.PLAY_SOUND(file);
		v1 = 1;
	}
	if (*(a1 + 2) == 'V' && *(a1 + 3) == 'O' && *(a1 + 4) == 'F') {
		_vm->_objectsManager.VERBE_OFF((int16)READ_LE_UINT16(a1 + 5), (int16)READ_LE_UINT16(a1 + 7));
		v1 = 1;
	}
	if (*(a1 + 2) == 'I' && *(a1 + 3) == 'I' && *(a1 + 4) == 'F') {
		v1 = 3;
	}

	return v1;
}


int ScriptManager::Control_Goto(const byte *dataP) {
	return (int16)READ_LE_UINT16(dataP + 5);
}

int ScriptManager::Control_If(const byte *dataP, int a2) {
	int v2; 
	int v3; 
	int v4; 
	int v5; 
	int v6; 
	int v7; 
	const byte *v8; 
	int v9; 
	int v10; 
	int v11; 
	int v12; 
	int v14; 
	int v15; 
	int v16; 
	int v17; 
	int v18; 
	int v19; 
	int v20; 

	v2 = a2;
LABEL_2:
	v3 = v2;
	do {
		if (_vm->shouldQuit())
			return 0; // Exiting game

		++v3;
		v4 = Traduction2(dataP + 20 * v3);
		if (v3 > 400) {
			v5 = v4;
			error("Control if failed");
			v4 = v5;
		}
	} while (v4 != 4);
	v20 = v3;
	v6 = v2;
	v7 = 0;
	do {
		if (_vm->shouldQuit())
			return 0; // Exiting game

		++v6;
		if (Traduction2(dataP + 20 * v6) == 3)
			v7 = 1;
		if (v6 > 400)
			error("Control if failed ");
		if (v7 == 1) {
			v2 = v20;
			goto LABEL_2;
		}
	} while (v20 != v6);
	v8 = dataP + 20 * a2;
	v9 = *(v8 + 13);
	v17 = *(v8 + 14);
	v16 = *(v8 + 15);
	v10 = (int16)READ_LE_UINT16(v8 + 5);
	v11 = (int16)READ_LE_UINT16(v8 + 7);
	v19 = (int16)READ_LE_UINT16(v8 + 9);
	v18 = (int16)READ_LE_UINT16(v8 + 11);
	v14 = 0;
	v15 = 0;
	if (v9 == 1 && _vm->_globals.SAUVEGARDE->data[v10] == v11)
		v14 = 1;
	if (v9 == 2 && _vm->_globals.SAUVEGARDE->data[v10] != v11)
		v14 = 1;
	if (v9 == 3 && _vm->_globals.SAUVEGARDE->data[v10] <= v11)
		v14 = 1;
	if (v9 == 4 && _vm->_globals.SAUVEGARDE->data[v10] >= v11)
		v14 = 1;
	if (v9 == 5 && _vm->_globals.SAUVEGARDE->data[v10] > v11)
		v14 = 1;
	if (v9 == 6 && _vm->_globals.SAUVEGARDE->data[v10] < v11)
		v14 = 1;
	if (v16 == 3)
		goto LABEL_68;
	if (v17 == 1 && v18 == _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v17 == 2 && v18 != _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v17 == 3 && v18 >= _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v17 == 4 && v18 <= _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v17 == 5 && v18 < _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v17 == 6 && v18 > _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v16 == 3) {
LABEL_68:
		if (v14 == 1) {
			v12 = a2;
			return (v12 + 1);
		}
		if (!v14) {
LABEL_63:
			v12 = v20;
			return (v12 + 1);
		}
	}
	if (v16 == 1) {
		if (v14 == 1 && v15 == 1) {
			v12 = a2;
			return (v12 + 1);
		}
		goto LABEL_63;
	}
	if (v16 == 2) {
		if (v14 == 1 || v15 == 1) {
			v12 = a2;
			return (v12 + 1);
		}
		goto LABEL_63;
	}
	return -1;
}

int ScriptManager::Traduction2(const byte *dataP) {
	int16 result; 

	result = 0;
	if (*dataP != 'F' || *(dataP + 1) != 'C') {
		result = 0;
	} else {
		if (*(dataP + 'T') == 84 && *(dataP + 3) == 'X' && *(dataP + 4) == 'T')
			result = 1;
		if (*(dataP + 2) == 'B') {
			if (*(dataP + 3) == 'O' && *(dataP + 4) == 'B')
				result = 1;
			if (*(dataP + 2) == 'B' && *(dataP + 3) == 'O' && *(dataP + 4) == 'F')
				result = 1;
		}
		if (*(dataP + 2) == 'P' && *(dataP + 3) == 'E' && *(dataP + 4) == 'R')
			result = 1;
		if (*(dataP + 2) == 'S' && *(dataP + 3) == 'O' && *(dataP + 4) == 'U')
			result = 1;
		if (*(dataP + 2) == 'M' && *(dataP + 3) == 'U' && *(dataP + 4) == 'S')
			result = 1;
		if (*(dataP + 2) == 'O') {
			if (*(dataP + 3) == 'B' && *(dataP + 4) == 'P')
				result = 1;
			if (*(dataP + 2) == 'O' && *(dataP + 3) == 'B' && *(dataP + 4) == 'M')
				result = 1;
		}
		if (*(dataP + 2) == 'G' && *(dataP + 3) == 'O' && *(dataP + 4) == 'T')
			result = 2;
		if (*(dataP + 2) == 'Z') {
			if (*(dataP + 3) == 'O' && *(dataP + 4) == 'N')
				result = 1;
			if (*(dataP + 2) == 'Z' && *(dataP + 3) == 'O' && *(dataP + 4) == 'F')
				result = 1;
		}
		if (*(dataP + 2) == 'E' && *(dataP + 3) == 'X' && *(dataP + 4) == 'I')
			result = 5;
		if (*(dataP + 2) == 'S' && *(dataP + 3) == 'O' && *(dataP + 4) == 'R')
			result = 5;
		if (*(dataP + 2) == 'A' && *(dataP + 3) == 'N' && *(dataP + 4) == 'I')
			result = 1;
		if (*(dataP + 2) == 'S' && *(dataP + 3) == 'P' && *(dataP + 4) == 'E')
			result = 1;
		if (*(dataP + 2) == 'E' && *(dataP + 3) == 'I' && *(dataP + 4) == 'F')
			result = 4;
		if (*(dataP + 2) == 'V') {
			if (*(dataP + 3) == 'A' && *(dataP + 4) == 'L')
				result = 1;
			if (*(dataP + 2) == 'V') {
				if (*(dataP + 3) == 'O' && *(dataP + 4) == 'N')
					result = 1;
				if (*(dataP + 2) == 'V' && *(dataP + 3) == 'O' && *(dataP + 4) == 'F')
					result = 1;
			}
		}
		if (*(dataP + 2) == 'I' && *(dataP + 3) == 'I' && *(dataP + 4) == 'F')
			result = 3;
		if (*(dataP + 2) == 'J' && *(dataP + 3) == 'U' && *(dataP + 4) == 'M')
			result = 6;
		if (*(dataP + 2) == 'B') {
			if (*(dataP + 3) == 'C' && *(dataP + 4) == 'A')
				result = 1;
			if (*(dataP + 2) == 'B' && *(dataP + 3) == 'O' && *(dataP + 4) == 'S')
				result = 1;
		}
		if (*(dataP + 2) == 'Z' && *(dataP + 3) == 'C') {
			if (*(dataP + 4) == 'H')
				result = 1;
		}
	}
	return result;
}

} // End of namespace Hopkins
