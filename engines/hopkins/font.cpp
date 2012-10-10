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
#include "common/textconsole.h"
#include "hopkins/font.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/objects.h"

namespace Hopkins {

void FontManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void FontManager::clearAll() {
	for (int idx = 0; idx < 11; ++idx) {
		Txt[idx].field0 = 0;
		Txt[idx].field12 = 0;
		Txt[idx].field3FC = 0;
		Txt[idx].field3FE = 0;
		Txt[idx].field400 = 0;
		Txt[idx].field404 = 0;
		Txt[idx].field406 = 0;
		Txt[idx].field408 = 0;

		ListeTxt[idx].field0 = 0;
	}
}

void FontManager::TEXTE_ON(int idx) {
	if ((idx - 5) > 11)
		error("Attempted to display text > MAX_TEXT.");
  
	TxtItem &txt = Txt[idx - 5];
	txt.field0 = 1;
	txt.field408 = 0;
  
	if (txt.field400 != g_PTRNUL) {
		_vm->_globals.dos_free2(txt.field400);
		txt.field400 = g_PTRNUL;
	}
}


void FontManager::TEXTE_OFF(int idx) {
	if ((idx - 5) > 11)
			error("Attempted to display text > MAX_TEXT.");
  
	TxtItem &txt = Txt[idx - 5];
	txt.field0 = 0;
	txt.field408 = 0;

	if (txt.field400 != g_PTRNUL) {
		_vm->_globals.dos_free2(txt.field400);
		txt.field400 = g_PTRNUL;
	}
}

void FontManager::COUL_TXT(int idx, byte colByte) {
	Txt[idx - 5].field40A = colByte;
}

void FontManager::OPTI_COUL_TXT(int idx1, int idx2, int idx3, int idx4) {
	COUL_TXT(idx1, 255);
	COUL_TXT(idx2, 255);
	COUL_TXT(idx3, 255);
	COUL_TXT(idx4, 253);
}

void FontManager::DOS_TEXT(int idx, int a2, const Common::String &filename, int a4, int a5, int a6, int a7, int a8, int a9, int a10) {
	if ((idx - 5) > 11)
		error("Attempted to display text > MAX_TEXT.");
  
	TxtItem &txt = Txt[idx - 5];
	txt.field0 = 0;
	txt.field4 = filename;
	txt.field8 = a4;
	txt.fieldA = a5;
	txt.fieldC = a2;
	txt.fieldE = a6;
	txt.field10 = a7;
	txt.field3FC = a8;
	txt.field3FE = a9;
	txt.field40A = a10;
}

void FontManager::BOITE(int idx, int fileIndex, const Common::String &filename, int xp, int yp) {
	/* There's something seriously wrong with the automatic disassembly.. it seems to have
	 * an extra code end block. I'll likely need to manually disassemble the method

	byte *v5; 
	int v6; 
	int v7; 
	int v8;
	byte *v9; 
	const byte *v10; 
	int v11; 
	int v12; 
	char v13; 
	char v14; 
	int v15; 
	char v16; 
	int v17; 
	int v18; 
	int v19; 
	int v20; 
	int v21; 
	int v22; 
	int v23; 
	char v24; 
	int v25; 
	int v26; 
	int v27; 
	int v28; 
	int v29; 
	int v30; 
	int v31; 
	int v32; 
	int v33;
	int v34; 
	int v35; 
	int v36; 
	int v37; 
	int v38; 
	int v39; 
	int v40; 
	int ptr; 
	char *ptra; 
	int ptrb; 
	int ptrc; 
	void *ptrd; 
	void *ptre; 
	int s; 
	int v49; 
	int v50; 
	int v51; 
	int v52; 
	int v53; 
	int v54; 
	int v55; 
	int v56; 
	int v57; 
	int v58; 
	void *v59; 
	void *v60; 
	void *v61; 
	int v62;
	int v63;
	int v64;
	int v65;
	int v66;
	int v67;
	int v68; 
	int v69;
	int v70; 
	int v71;
	int v72;
	int v73;
	int i; 
	int v75;
	Common::String fname; 

	v73 = xp;
	v70 = yp;
	v58 = 0;
	if (idx < 0)
		error("Bad number for text");
  
	_vm->_globals.police_l = 11;

	v5 = idx;
	largeur_boite = 11 * Txt[idx].field3FE;
	if (Txt[idx].field408) {
		v34 = Txt[idx].field3FC;
		if (v34 != 6 && v34 != 1 && v34 != 3 && v34 != 5) {
			v72 = yp + 5;
			v38 = 0;
			if (Txt[idx].field12 > 0) {
				do {
					v40 = idx;
					TEXT_NOW1(xp + 5, v72, Txt[idx].field14[v38], Txt[idx].field40A);
					v5 = police_h + v72 + 1;
					v72 += police_h + 1;
					++v38;
					idx = v40;
				} while (Txt[v40].field12 > v38);
			}
		} else {
			v35 = idx;
			v36 = *(_WORD *)&Txt[v35 + 1030];
			v37 = *(_WORD *)&Txt[v35 + 1028];
      
			_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER,
				Txt[v35].field400, xp, yp, Txt[v35].field404, Txt[v35].field406);
			v5 = _vm->_graphicsManager.Ajoute_Segment_Vesa(xp, yp, xp + v37, yp + v36);
		}
	} else {
		v62 = 0;
		do {
			TRIER_TEXT[v62++] = 0;
		while (v62 <= 19);
    
		&Txt[idx].field408 = 1;
		_vm->_fileManager.CONSTRUIT_FICHIER(HOPLINK, filename);
		fname = _vm->_globals.NFICHIER;
    
		if (strncmp(fname.c_str(), oldname.c_str(), fname.size())) {
			oldname = fname;
			nom_indexoldname = fname;

			// *(int *)((char *)&dword_80AE4DC + strlen(nom_index) + 1) = dword_807C98D;
			Common::File f;
			if (!f.open(nom_index))
				error("error opening file - %s", nom_index.c_str());

			int fileSize = f.size();
			for (int i = 0; i < (fileSize / 4); ++i)
				Index[i] = f.readUint32LE();
			f.close();
		}
    
		if (fname[0] != 'Z' || fname[1] != 'O') {
			Common::File f;
			if (!f.open(fname))
				error("error opening file - %s", fname.c_str());

			v69 = 2048;
			f.seek(Index[fileIndex]);
			texte_tmp = _vm->_globals.dos_malloc2(2058);
			if (texte_tmp == g_PTRNUL)
				error("temporary text");
      
			f.read(texte_temp, 2048);
			f.close();
			texte_long = 2048;
		} else {
			v69 = 100;
			texte_long = 100;
			v9 = _vm->_globals.dos_malloc2(110);
			texte_tmp = v9;
			v10 = BUF_ZONE + Index[fileIndex];
			memcpy(v9, v10, 96);

			v11 = 0;
			WRITE_LE_UINT16((uint16 *)v9 + 48, READ_LE_UINT16((uint16 *)v10 + 48));
		}

		v59 = texte_tmp;
		v63 = 0;
    
		if (!v69)
			goto LABEL_43;
    
		do {
			v13 = v59;
			if ((unsigned __int8)(*v59 + 46) > 0x1Bu) {
				if ((unsigned __int8)(v13 + 80) > 0x1Bu) {
					if ((unsigned __int8)(v13 - 65) <= 0x19u || (unsigned __int8)(v13 - 97) <= 0x19u)
						v13 = 32;
				} else {
					v13 -= 79;
				}
			} else {
				v13 += 111;
			}
      
			*v59 = v13;
			v59 = v59 + 1;
			++v63;
		} while (v63 < v69);
    
		v60 = texte_tmp;
		v64 = 0;
		if (v69) {
			ptr = idx;
			
			for (;;) {
				v14 = *(v60 + v64);
				if (v14 == 10 || v14 == 13) {
					*(v60 + v64) = 0;
					v11 = &Txt[0];
					if (!Txt[ptr].field3FE)
						break;
				}
        
				++v64;
				if (v69 <= v64)
					goto LABEL_43;
			}
      
			Txt[ptr].field3FE = v64;
			largeur_boite = 0;
      
			v15 = 0;
			if (v64 + 1 > 0) {
				do {
					v16 = *(v60 + v15);
					if ((unsigned __int8)v16 <= 0x1Fu)
						v16 = 32;
					largeur_boite += _vm->_objectManager.Get_Largeur(police, (unsigned __int8)v16 - 32);
					++v15;
				} while (v15 < v64 + 1);
			}
      
			largeur_boite += 2;
			v17 = largeur_boite / 2;
			if (v17 < 0)
				v17 = -v17;
			*Txt[idx].field8 = 320 - v17;
			v73 = _vm->_eventsManager.start_x + 320 - v17;
			v58 = 1;
			v18 = 0;
		
			if (v64 + 1 > 0) {
				ptra = Txt[ptr];
        
				do {
					ptra.field14[v18].field0 = *(v60 + v18);
					++v18;
				} while (v18 < v64 + 1);
			}
		} else {
LABEL_43:
      
			if (!largeur_boite)
				largeur_boite = 240;
			v65 = 0;
			v61 = texte_tmp;
      
			do {
				v19 = 0;
				ptrb = largeur_boite - 4;
				
				for (;;) {
					v57 = v19;
					do {
						v11 = *(v61 + v65 + v19++);
					} while (v11 != 32 && v11 != 37);
          
					if (v19 >= ptrb / police_l)
						break;
					if (v11 == 37) {
						if (v19 < ptrb / police_l)
							goto LABEL_55;
						break;
					}
				}
        
				if (v11 != 37)
					goto LABEL_57;
				v11 = 32;
LABEL_55:
				if (v11 == 37)
					v57 = v19;
LABEL_57:
				v20 = v58;
				v21 = v11;
				
				Txt[idx].field14[v20] = (const char *)v61 + v65;
				//strncpy((char *)(v20 + 1036 * idx + 134911728), (const char *)v61 + v65, v57);
				TRIER_TEXT[v58++] = v57;
        
				v65 += v57;
				v11 = v21;
			} while ((byte)v21 != 37);
      
			v66 = 0;
			do {
				v22 = TRIER_TEXT[v66];
				if (v22 <= 0) {
					TRIER_TEXT[v66] = 0;
				} else {
					ptrc = 0;
					v23 = 0;
					if (v22 - 1 > 0) {
						do {
							v24 = *(&Txt[1036 * idx + 20] + 100 * v66 + v23);
							if ((unsigned __int8)v24 <= 0x1Fu)
								v24 = 32;
              
							ptrc += _vm->_objectManager.Get_Largeur(police, (unsigned __int8)v24 - 32);
							++v23;
						} while (v23 < TRIER_TEXT[v66] - 1);
					}
          
					TRIER_TEXT[v66] = ptrc;
				}
				++v66;
			} while (v66 <= 19);
      
			v67 = 0;
			do {
				v25 = v67;
				do {
					++v25;
					if (v25 == 20)
						v25 = 0;
					if (TRIER_TEXT[v67] < TRIER_TEXT[v25])
						TRIER_TEXT[v67] = 0;
				} while (v25 != v67);
				++v67;
			} while (v67 <= 19);
      
			v68 = 0;
			do {
				if (TRIER_TEXT[v68])
					largeur_boite = TRIER_TEXT[v68];
				++v68;
			} while (v68 <= 19);
      
			if ((Txt[idx].field3FC - 2) > 1u) {
				for (i = xp - _vm->_eventsManager.start_x; largeur_boite + i > 638 && i > -2 && Txt[idx].field3FC]; i -= 2)
					;
				Txt[idx].field8 = i;
				v73 = _vm->_eventsManager.start_x + i;
			} else {
				if (nbrligne == 639) {
					while (largeur_boite + v73 > 638 && v73 > -2)
						v73 -= 2;
				}
				if (nbrligne == (SCREEN_WIDTH * 2)) {
					while (largeur_boite + v73 > 1278 && v73 > -2)
						v73 -= 2;
				}
				Txt[idx].field8 = v73;
			}
		}
    
		hauteur_boite = (police_h + 1) * v58 + 2;
		v56 = v73;
		v55 = yp;
		v53 = largeur_boite + 10;
		v51 = (police_h + 1) * v58 + 12;

		v26 = idx;
		if (Txt[idx].field3FC == 6) {
			v27 = v53 / 2;
			if (v27 < 0)
				v27 = -v27;
      
			Txt[v26].field8 = 315 - v27;
			v28 = _vm->_eventsManager.start_x + 315 - v27;
			v73 = _vm->_eventsManager.start_x + 315 - v27;
			Txt[v26].fieldA = 50;
			v70 = 50;
			v55 = 50;
			v56 = v28;
		}
    
		v29 = Txt[idx].field3FC;
		if (v29 == 1 || v29 == 3 || (uint16)(v29 - 5) <= 1u) {
			v49 = v51 * v53;
			ptrd = _vm->_globals.dos_malloc2(v51 * v53);
			
			if (ptrd == g_PTRNUL) 
				error("Error allocating block (%d)", v49);
		}

		_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_BUFFER, ptrd, v56, v55, v53, v51);
		_vm->_graphicsManager.Trans_bloc2(ptrd, TABLE_COUL, v49);
		_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, ptrd, v56, v55, v53, v51);
		_vm->_globals.dos_free2(ptrd);
      
		_vm->_graphicsManager.Plot_Hline(_vm->_graphicsManager.VESA_BUFFER, v56, v55, v53, -2);
		_vm->_graphicsManager.Plot_Hline(_vm->_graphicsManager.VESA_BUFFER, v56, (v51 + v55), v53, -2);
		_vm->_graphicsManager.Plot_Vline(_vm->_graphicsManager.VESA_BUFFER, v56, v70, v51, -2);
		_vm->_graphicsManager.Plot_Vline(_vm->_graphicsManager.VESA_BUFFER, (v53 + v56), v70, v51, -2);
    }

	Txt[idx].field12 = v58;
    v75 = v73 + 5;
    v71 = v70 + 5;
    v30 = 0;
    
	if (v58 > 0) {
		do {
			TEXT_NOW1(v75, v71, Txt[idx].field14[v30], Txt[idx].field40A);
			v71 += police_h + 1;
			++v30;
		} while ( v58 > v30 );
	}

    v54 = v53 + 1;
    v52 = v51 + 1;
    v31 = 1036 * idx;
    Txt[v31].field404 = v54;
    Txt[v31].field406 = v52;
    v32 = Txt[v31].field3FC;

	if (v32 == 6 || v32 == 1 || v32 == 3 || v32 == 5) {
		v33 = idx;
		if (Txt[v33].field400 != g_PTRNUL)
			Txt[v33].field400 = _vm->_globals.dos_free2(Txt[v33].field400);

		v50 = v52 * v54;
		ptre = dos_malloc2(v50 + 20);
		if (ptre == g_PTRNUL)
			error("Error allocating block (%d)", v50);
	}

	Txt[v33].field400 = ptre;
    Txt[v33].field404 = v54;
    Txt[v33].field406 = v52;
    _vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_BUFFER, Txt[v33].field400, v56, v55, 
		Txt[v33].field404, v52);
	}

    texte_tmp = _vm->_globals.dos_free2(texte_tmp);
	*/
}

void FontManager::TEXT_NOW1(int xp, int yp, const Common::String &message, int transColour) {
	for (uint idx = 0; idx < message.size(); ++idx) {
		char currentChar = message[idx];

		if (currentChar > 31) {
			int characterIndex = currentChar - 32;
			_vm->_graphicsManager.Affiche_Fonte(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.police, 
				xp, yp, characterIndex, transColour);
			xp += _vm->_objectsManager.Get_Largeur(_vm->_globals.police, characterIndex);
		}
	}
}

} // End of namespace Hopkins
