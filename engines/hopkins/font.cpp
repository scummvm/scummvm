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
#include "common/file.h"
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
		Txt[idx].textOn = false;
		Txt[idx].lineCount = 0;
		Txt[idx].field3FC = 0;
		Txt[idx].field3FE = 0;
		Txt[idx].textBlock = NULL;
		Txt[idx].width = 0;
		Txt[idx].height = 0;
		Txt[idx].textLoaded = false;

		ListeTxt[idx].enabled = false;
	}
}

// Text On
void FontManager::TEXTE_ON(int idx) {
	if ((idx - 5) > 11)
		error("Attempted to display text > MAX_TEXT.");
  
	TxtItem &txt = Txt[idx - 5];
	txt.textOn = true;
	txt.textLoaded = false;
  
	if (txt.textBlock != g_PTRNUL) {
		_vm->_globals.dos_free2(txt.textBlock);
		txt.textBlock = g_PTRNUL;
	}
}

// Text Off
void FontManager::TEXTE_OFF(int idx) {
	if ((idx - 5) > 11)
			error("Attempted to display text > MAX_TEXT.");
  
	TxtItem &txt = Txt[idx - 5];
	txt.textOn = false;
	txt.textLoaded = false;

	if (txt.textBlock != g_PTRNUL) {
		_vm->_globals.dos_free2(txt.textBlock);
		txt.textBlock = g_PTRNUL;
	}
}

// Text Color
void FontManager::COUL_TXT(int idx, byte colByte) {
	Txt[idx - 5].colour = colByte;
}

// Text Optimal Color
void FontManager::OPTI_COUL_TXT(int idx1, int idx2, int idx3, int idx4) {
	COUL_TXT(idx1, 255);
	COUL_TXT(idx2, 255);
	COUL_TXT(idx3, 255);
	COUL_TXT(idx4, 253);
}

// 
void FontManager::DOS_TEXT(int idx, int messageId, const Common::String &filename, int xp, int yp, int a6, int a7, int a8, int a9, int colour) {
	if ((idx - 5) > 11)
		error("Attempted to display text > MAX_TEXT.");
  
	TxtItem &txt = Txt[idx - 5];
	txt.textOn = false;
	txt.filename = filename;
	txt.xp = xp;
	txt.yp = yp;
	txt.messageId = messageId;
	txt.fieldE = a6;
	txt.field10 = a7;
	txt.field3FC = a8;
	txt.field3FE = a9;
	txt.colour = colour;
}

// Box
void FontManager::BOITE(int idx, int messageId, const Common::String &filename, int xp, int yp) {
	int filesize;
	byte *v9; 
	const byte *v10;
	int v11; 
	byte v13;
	byte v14;
	int v15;
	byte v16; 
	int v17; 
	int v18;
	int v19;
	int v20; 
	int v21; 
	int v22; 
	int v23;
	byte v24; 
	int v25; 
	int v27; 
	int v28; 
	int v29; 
	int v32; 
	int v34; 
	int v36; 
	int v37;
	int ptrb;
	int ptrc; 
	byte *ptrd; 
	byte *ptre; 
	Common::String s; 
	int v49; 
	int blockSize; 
	int v51;
	int blockHeight;
	int v53;
	int blockWidth; 
	int v55; 
	int v56; 
	int lineSize; 
	int lineCount; 
	byte *v59; 
	byte *v60; 
	byte *v61; 
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
	int v73; 
	int i; 
	int v75;
	Common::String file; 
	Common::File f;

	v73 = xp;
	v70 = yp;
	lineCount = 0;
	if (idx < 0)
		error("Bad number for text");
	_vm->_globals.police_l = 11;

	_vm->_globals.largeur_boite = 11 * Txt[idx].field3FE;
	if (Txt[idx].textLoaded) {
		v34 = Txt[idx].field3FC;
		if (v34 != 6 && v34 != 1 && v34 != 3 && v34 != 5) {
			int yCurrent = yp + 5;
			if (Txt[idx].lineCount > 0) {
				for (int lineNum = 0; lineNum < Txt[idx].lineCount; ++lineNum) {
					TEXT_NOW1(xp + 5, yCurrent, Txt[idx].lines[lineNum], Txt[idx].colour);
					yCurrent += _vm->_globals.police_h + 1;
				} 
			}
		} else {
			v36 = Txt[idx].height;
			v37 = Txt[idx].width;
			_vm->_graphicsManager.Restore_Mem(
				_vm->_graphicsManager.VESA_BUFFER,
				Txt[idx].textBlock,
			    xp,
			    yp,
			    Txt[idx].width,
			    Txt[idx].height);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(xp, yp, xp + v37, yp + v36);
		}
	} else {
		v62 = 0;
		do {
			TRIER_TEXT[v62++] = 0;
		} while (v62 <= 19);
		Txt[idx].textLoaded = true;
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, filename);

		file = _vm->_globals.NFICHIER;
		if (strncmp(file.c_str(), oldname.c_str(), strlen(file.c_str())) != 0) {
			// Starting to access a new file, so read in the index file for the file
			oldname = file;
			nom_index = Common::String(file.c_str(), file.size() - 3);
			nom_index += "IND";
			
			if (!f.open(nom_index))
				error("Error opening file - %s", nom_index.c_str());
			filesize = f.size();
			for (i = 0; i < (filesize / 4); ++i)
				Index[i] = f.readUint32LE();
			f.close();
		}
		if (filename[0] != 'Z' || filename[1] != 'O') {
			if (!f.open(file))
				error("Error opening file - %s", nom_index.c_str());

			v69 = 2048;
			f.seek(Index[messageId]);

			texte_tmp = _vm->_globals.dos_malloc2(0x80Au);
			if (texte_tmp == g_PTRNUL)
				error("Error allocating text");
			
			Common::fill(&texte_tmp[0], &texte_tmp[0x80a], 0);
			f.read(texte_tmp, 0x800u);
			f.close();
			_vm->_globals.texte_long = 2048;
		} else {
			v69 = 100;
			_vm->_globals.texte_long = 100;
			v9 = _vm->_globals.dos_malloc2(0x6Eu);
			Common::fill(&v9[0], &v9[0x6e], 0);

			texte_tmp = v9;
			v10 = _vm->_globals.BUF_ZONE + Index[messageId];
			memcpy(v9, v10, 0x60u);
			v11 = 0;
			WRITE_LE_UINT16((uint16 *)v9 + 48, (int16)READ_LE_UINT16(v10 + 96));
		}
		v59 = texte_tmp;
		v63 = 0;
		if (!v69)
			goto LABEL_43;
		do {
			v13 = *v59;
			if ((byte)(*v59 + 46) > 0x1Bu) {
				if ((byte)(v13 + 80) > 0x1Bu) {
					if ((byte)(v13 - 65) <= 0x19u || (byte)(v13 - 97) <= 0x19u)
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
			while (1) {
				v14 = *(v60 + v64);
				if (v14 == '\r' || v14 == '\n') {
					*(v60 + v64) = 0;
					if (!Txt[idx].field3FE)
						break;
				}
				++v64;
				if (v69 <= v64)
					goto LABEL_43;
			}
			Txt[idx].field3FE = v64;
			_vm->_globals.largeur_boite = 0;

			v15 = 0;
			if (v64 + 1 > 0) {
				do {
					v16 = *(v60 + v15);
					if ((byte)v16 <= 0x1Fu)
						v16 = 32;
					_vm->_globals.largeur_boite += _vm->_objectsManager.Get_Largeur(_vm->_globals.police, (byte)v16 - 32);
					++v15;
				} while (v15 < v64 + 1);
			}
			_vm->_globals.largeur_boite += 2;
			v17 = _vm->_globals.largeur_boite / 2;
			if (v17 < 0)
				v17 = -v17;
			Txt[idx].xp = 320 - v17;
			v73 = _vm->_eventsManager.start_x + 320 - v17;
			lineCount = 1;
			v18 = 0;
			if (v64 + 1 > 0) {
				Txt[idx].lines[0] = Common::String((const char *)v60, v64);
			}
		} else {
LABEL_43:
			if (!_vm->_globals.largeur_boite)
				_vm->_globals.largeur_boite = 240;
			v65 = 0;
			v61 = texte_tmp;
			do {
				v19 = 0;
				ptrb = _vm->_globals.largeur_boite - 4;
				while (1) {
					lineSize = v19;
					do
						v11 = *(v61 + v65 + v19++);
					while (v11 != 32 && v11 != 37);
					if (v19 >= ptrb / _vm->_globals.police_l)
						break;
					if (v11 == '%') {
						if (v19 < ptrb / _vm->_globals.police_l)
							goto LABEL_55;
						break;
					}
				}
				if (v11 != '%')
					goto LABEL_57;
				v11 = 32;
LABEL_55:
				if (v11 == '%')
					lineSize = v19;
LABEL_57:
				v20 = lineCount;
				v21 = v11;

				// WORKAROUND: Perhaps due to the usage of ScummVM strings here, recalculate what the
				// actual length of the line to be copied will be. Otherwise, you can see artifacts,
				// such as a single character beyond the end of string NULL.
				int actualSize = 0;
				while (actualSize < lineSize && *(v61 + v65 + actualSize))
					++actualSize;

				Txt[idx].lines[v20] = Common::String((const char *)v61 + v65, actualSize);
				TRIER_TEXT[lineCount++] = lineSize;

				v65 += lineSize;
				v11 = v21;
			} while (v21 != 37);
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
							Common::String &line = Txt[idx].lines[v66];
							v24 = (v23 >= (int)line.size()) ? '\0' : line[v23];
							if ((byte)v24 <= 0x1Fu)
								v24 = 32;
							ptrc += _vm->_objectsManager.Get_Largeur(_vm->_globals.police, (byte)v24 - 32);
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
				} while (v25 != (int16)v67);
				++v67;
			} while (v67 <= 19);
			v68 = 0;
			do {
				if (TRIER_TEXT[v68])
					_vm->_globals.largeur_boite = TRIER_TEXT[v68];
				++v68;
			} while (v68 <= 19);

			if ((uint16)(Txt[idx].field3FC - 2) > 1u) {
				for (i = xp - _vm->_eventsManager.start_x; _vm->_globals.largeur_boite + i > 638 && i > -2 && Txt[idx].field3FC; i -= 2)
					;
				Txt[idx].xp = i;
				v73 = _vm->_eventsManager.start_x + i;
			} else {
				if (_vm->_globals.nbrligne == (SCREEN_WIDTH - 1)) {
					while (_vm->_globals.largeur_boite + v73 > 638 && v73 > -2)
						v73 -= 2;
				}
				if (_vm->_globals.nbrligne == (SCREEN_WIDTH * 2)) {
					while (_vm->_globals.largeur_boite + v73 > 1278 && v73 > -2)
						v73 -= 2;
				}
				Txt[idx].xp = v73;
			}
		}
		_vm->_globals.hauteur_boite = (_vm->_globals.police_h + 1) * lineCount + 2;
		v56 = v73;
		v55 = yp;
		v53 = _vm->_globals.largeur_boite + 10;
		v51 = (_vm->_globals.police_h + 1) * lineCount + 12;
		if (Txt[idx].field3FC == 6) {
			v27 = v53 / 2;
			if (v27 < 0)
				v27 = -v27;
			Txt[idx].xp = 315 - v27;
			v28 = _vm->_eventsManager.start_x + 315 - v27;
			v73 = _vm->_eventsManager.start_x + 315 - v27;
			Txt[idx].yp = 50;
			v70 = 50;
			v55 = 50;
			v56 = v28;
		}
		v29 = Txt[idx].field3FC;
		if (v29 == 1 || v29 == 3 || (uint16)(v29 - 5) <= 1u) {
			v49 = v51 * v53;
			ptrd = _vm->_globals.dos_malloc2(v51 * v53);
			if (ptrd == g_PTRNUL) {
				error("Cutting a block for text box (%d)", v49);
			}
			_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_BUFFER, ptrd, v56, v55, v53, v51);
			_vm->_graphicsManager.Trans_bloc2(ptrd, _vm->_graphicsManager.TABLE_COUL, v49);
			_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, ptrd, v56, v55, v53, v51);
			_vm->_globals.dos_free2(ptrd);
			
			_vm->_graphicsManager.Plot_Hline(_vm->_graphicsManager.VESA_BUFFER, v56, v55, v53, (byte)-2);
			_vm->_graphicsManager.Plot_Hline(_vm->_graphicsManager.VESA_BUFFER, v56, v51 + v55, v53, (byte)-2);
			_vm->_graphicsManager.Plot_Vline(_vm->_graphicsManager.VESA_BUFFER, v56, v70, v51, (byte)-2);
			_vm->_graphicsManager.Plot_Vline(_vm->_graphicsManager.VESA_BUFFER, v53 + v56, v70, v51, (byte)-2);
		}
		Txt[idx].lineCount = lineCount;
		v75 = v73 + 5;
		v71 = v70 + 5;

		if (lineCount > 0) {
			for (int lineNum = 0; lineNum < lineCount; ++lineNum) {
				TEXT_NOW1(v75, v71, Txt[idx].lines[lineNum], Txt[idx].colour);
				v71 += _vm->_globals.police_h + 1;
			}
		}

		blockWidth = v53 + 1;
		blockHeight = v51 + 1;
		
		Txt[idx].width = blockWidth;
		Txt[idx].height = blockHeight;
		v32 = Txt[idx].field3FC;
		if (v32 == 6 || v32 == 1 || v32 == 3 || v32 == 5) {
			if (Txt[idx].textBlock != g_PTRNUL)
				Txt[idx].textBlock = _vm->_globals.dos_free2(Txt[idx].textBlock);
			blockSize = blockHeight * blockWidth;
			ptre = _vm->_globals.dos_malloc2(blockSize + 20);
			if (ptre == g_PTRNUL)
				error("Cutting a block for text box (%d)", blockSize);

			Txt[idx].textBlock = ptre;
			Txt[idx].width = blockWidth;
			Txt[idx].height = blockHeight;
			_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_BUFFER, Txt[idx].textBlock, v56, v55, Txt[idx].width, blockHeight);
		}
		texte_tmp = _vm->_globals.dos_free2(texte_tmp);
	}
}

void FontManager::TEXT_NOW(int xp, int yp, const Common::String &message, int col) {
	const char *srcP;
	char currChar; 
	int charIndex; 

	srcP = message.c_str();
	for (;;) {
		currChar = *srcP++;
		if (!currChar)
			break;
		if (currChar >= 32) {
			charIndex = currChar - 32;
			_vm->_graphicsManager.Affiche_Fonte(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.police, 
				xp, yp, currChar - 32, col);
			xp += _vm->_objectsManager.Get_Largeur(_vm->_globals.police, charIndex);
		}
	}
	_vm->_graphicsManager.Ajoute_Segment_Vesa(xp, yp, xp, yp + 12);
}


void FontManager::TEXT_NOW1(int xp, int yp, const Common::String &message, int col) {
	for (uint idx = 0; idx < message.size(); ++idx) {
		char currentChar = message[idx];

		if (currentChar > 31) {
			int characterIndex = currentChar - 32;
			_vm->_graphicsManager.Affiche_Fonte(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.police, 
				xp, yp, characterIndex, col);
			xp += _vm->_objectsManager.Get_Largeur(_vm->_globals.police, characterIndex);
		}
	}
}

void FontManager::TEXT_COMPUT(int xp, int yp, const Common::String &msg, int col) {
	const char *srcP; 
	int v5; 
	int v6; 
	byte v7; 
	int fontCol; 
	int v9; 

	srcP = msg.c_str();
	v9 = xp;
	fontCol = col;
	do {
		v7 = *srcP++;
		if (v7 == '&') {
			fontCol = 2;
			v7 = *srcP++;
		}
		if (v7 == '$') {
			fontCol = 4;
			v7 = *srcP++;
		}
		if (!v7)
			break;
		if (v7 >= 32) {
			v5 = v7 - 32;
			_vm->_graphicsManager.Affiche_Fonte(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.police, v9, yp, v7 - 32, fontCol);
			v9 += _vm->_objectsManager.Get_Largeur(_vm->_globals.police, v5);
			v6 = _vm->_objectsManager.Get_Largeur(_vm->_globals.police, v5);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(v9 - v6, yp, v9, yp + 12);
			if (_vm->_eventsManager.ESC_KEY) {
				_vm->_globals.iRegul = 1;
				_vm->_eventsManager.VBL();
			} else {
				_vm->_globals.iRegul = 4;
				_vm->_eventsManager.VBL();
				_vm->_globals.iRegul = 1;
			}
		}
	} while (v7);
}

} // End of namespace Hopkins
