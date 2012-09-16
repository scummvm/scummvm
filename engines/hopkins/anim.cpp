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
#include "hopkins/anim.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

AnimationManager::AnimationManager() {
	CLS_ANM = false;
	NO_SEQ = false;
}

void AnimationManager::PLAY_ANM(const Common::String &filename, uint32 rate1, uint32 rate2, uint32 rate3) {
	int doneFlag;
	byte *screenCopy = NULL;
	uint16 v14, v15, v16, v17, v18, v19;
	size_t nbytes;
	byte *screenP;
	Common::File f;
	char strBuffer[20];
	int idx = 0;

	doneFlag = 0;
	for (;;) {
//MAIN_LOOP:
		v14 = v15 = v16 = v17 = 0;
		v18 = 1;
		screenP = _vm->_graphicsManager.VESA_SCREEN;

		FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPANM, filename);

		if (!f.open(GLOBALS.NFICHIER))
			error("Not Found file %s", GLOBALS.NFICHIER.c_str());

		// TODO: Original above read seems to overlap the doneFlag
		f.skip(6);
		//buf = read(4); doneFlag = f.readUint16LE() != 0;
		f.read(_vm->_graphicsManager.Palette, PALETTE_EXT_BLOCK_SIZE);
		f.skip(4);
		nbytes = f.readUint32LE();

		// TODO: Original never seems to do anything with these. Or are these part of 
		// a bigger structure needed for sub-methods?
		v19 = f.readUint32LE();
		v18 = f.readUint16LE();
		v17 = f.readUint16LE();
		v16 = f.readUint16LE();
		v15 = f.readUint16LE();
		v14 = f.readUint16LE();

		f.read(screenP, nbytes);

		if (CLS_ANM) {
			_vm->_graphicsManager.DD_Lock();
			_vm->_graphicsManager.Cls_Video();
			_vm->_graphicsManager.DD_Unlock();
		}

		if (_vm->_graphicsManager.WinScan / _vm->_graphicsManager.Winbpp > SCREEN_WIDTH) {
			doneFlag = 1;
			screenCopy = GLOBALS.dos_malloc2(SCREEN_WIDTH * SCREEN_HEIGHT);
			memcpy((void *)screenCopy, screenP, SCREEN_WIDTH * SCREEN_HEIGHT);
		}

		if (NO_SEQ) {
			if (doneFlag == 1)
				memcpy((void *)screenCopy, _vm->_graphicsManager.VESA_BUFFER, SCREEN_WIDTH * SCREEN_HEIGHT);
			_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
		} else {
			_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
			_vm->_graphicsManager.DD_Lock();
		
			if (_vm->_graphicsManager.Winbpp == 2) {
				if (doneFlag)
					_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, 640, 480, 0, 0);
				else
					_vm->_graphicsManager.m_scroll16(screenP, 0, 0, 640, 480, 0, 0);
			}
			if (_vm->_graphicsManager.Winbpp == 1) {
				if (doneFlag)
					_vm->_graphicsManager.m_scroll2A(screenCopy, 0, 0, 640, 480, 0, 0);
				else
					_vm->_graphicsManager.m_scroll2(screenP, 0, 0, 640, 480, 0, 0);
			}
			_vm->_graphicsManager.DD_Unlock();
			_vm->_graphicsManager.DD_VBL();
		}

		_vm->_eventsManager.lItCounter = 0;
		_vm->_eventsManager.ESC_KEY = 0;
		_vm->_soundManager.LOAD_ANM_SOUND();
		if (_vm->_globals.iRegul == 1) {
			do {
				if (_vm->_eventsManager.ESC_KEY)
					goto FINISH;

// TODO: Original REDRAW_ANIM always returns false, so this isn't needed?
#if 0
				if (REDRAW_ANIM())
					goto REDRAW_ANIM;
#endif
				_vm->_eventsManager.CONTROLE_MES();
			} while (_vm->_eventsManager.lItCounter < rate1);
		}

		_vm->_eventsManager.lItCounter = 0;
		doneFlag = false;
		idx = 0;
		do {
			_vm->_soundManager.PLAY_ANM_SOUND(idx);

			// Get in string
			Common::fill(&strBuffer[0], &strBuffer[20], 0);
			if (f.read(strBuffer, 16) != 16)
				doneFlag = true;

			if (strncmp(strBuffer, "IMAGE=", 6) != 0)
				doneFlag = true;

			if (!doneFlag) {
				f.read(screenP, READ_LE_UINT32(strBuffer + 8));

				if (_vm->_globals.iRegul == 1) {
					do {
						if (_vm->_eventsManager.ESC_KEY)
							goto FINISH;

// TODO: Original REDRAW_ANIM always returns false, so this isn't needed?
#if 0
						if (REDRAW_ANIM()) {
							if (_vm->_graphicsManager.NOLOCK == 1)
								goto FINISH;

							f.close();
							if (doneFlag <= SCREEN_WIDTH)
								goto MAIN_LOOP;

							screenCopy = GLOBALS.dos_free2(screenCopy);
							goto MAIN_LOOP;
						}
#endif

						_vm->_eventsManager.CONTROLE_MES();
						_vm->_soundManager.VERIF_SOUND();
					} while (_vm->_eventsManager.lItCounter < rate2);
				}

				_vm->_eventsManager.lItCounter = 0;
				_vm->_graphicsManager.DD_Lock();

				if (!doneFlag) {
					if (*screenP != 252) {
						if (_vm->_graphicsManager.Winbpp == 1)
							_vm->_graphicsManager.Copy_Video_Vbe3(screenP);
						if (_vm->_graphicsManager.Winbpp == 2)
							_vm->_graphicsManager.Copy_Video_Vbe16(screenP);
					}
				} else if (*screenP != 252) {
					_vm->_graphicsManager.Copy_WinScan_Vbe3(screenP, screenCopy);

					if (_vm->_graphicsManager.Winbpp == 2)
						_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
					else
						_vm->_graphicsManager.m_scroll2A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				}

				_vm->_graphicsManager.DD_Unlock();
				_vm->_graphicsManager.DD_VBL();
				_vm->_soundManager.VERIF_SOUND();
			}
		} while (!doneFlag);

		if (_vm->_globals.iRegul != 1) {
			_vm->_eventsManager.lItCounter = 0;
			goto FINISH;
		}

		do {
			if (_vm->_eventsManager.ESC_KEY)
				goto FINISH;

// TODO: Original REDRAW_ANIM always returns false, so this isn't needed?
#if 0
			if (REDRAW_ANIM()) {
REDRAW_ANIM:
				if (_vm->_graphicsManager.NOLOCK == 1)
					goto FINISH;

				f.close();
				if (doneFlag != 1)
					goto MAIN_LOOP;

				screenCopy = GLOBALS.dos_free2(screenCopy);
				goto MAIN_LOOP;
			}
#endif

			_vm->_eventsManager.CONTROLE_MES();
			_vm->_soundManager.VERIF_SOUND();
		} while (_vm->_eventsManager.lItCounter < rate3);

		_vm->_eventsManager.lItCounter = 0;
		_vm->_soundManager.VERIF_SOUND();
		break;
	}

FINISH:
	if (_vm->_graphicsManager.FADE_LINUX == 2 && !doneFlag) {
		screenCopy = GLOBALS.dos_malloc2(SCREEN_WIDTH * SCREEN_HEIGHT);

		f.skip(6);
		f.read(_vm->_graphicsManager.Palette, PALETTE_EXT_BLOCK_SIZE);
		f.skip(4);
		nbytes = f.readUint32LE();
		v19 = f.readUint32LE();
		v18 = f.readUint16LE();
		v17 = f.readUint16LE();
		v16 = f.readUint16LE();
		v15 = f.readUint16LE();
		v14 = f.readUint16LE();

		f.read(screenCopy, nbytes);
		Common::copy(screenP, screenP + SCREEN_WIDTH * SCREEN_HEIGHT, screenCopy);

		idx = 0;
		doneFlag = false;
		do {
			// Get in string
			Common::fill(&strBuffer[0], &strBuffer[20], 0);
			if (f.read(strBuffer, 16) != 16)
				doneFlag = true;

			if (strncmp(strBuffer, "IMAGE=", 7) != 0)
				doneFlag = true;

			if (!doneFlag) {
				f.read(screenP, READ_LE_UINT32(strBuffer + 8));

				if (*screenP != 252)
					_vm->_graphicsManager.Copy_WinScan_Vbe3(screenP, screenCopy);
			}
		} while (!doneFlag);

		_vm->_graphicsManager.FADE_OUTW_LINUX(screenCopy);
		screenCopy = GLOBALS.dos_free2(screenCopy);
	}
			
	if (doneFlag == 1) {
		if (_vm->_graphicsManager.FADE_LINUX == 2)
			_vm->_graphicsManager.FADE_OUTW_LINUX(screenCopy);
		GLOBALS.dos_free2(screenCopy);
	}
  
	_vm->_graphicsManager.FADE_LINUX = 0;
	f.close();
	GLOBALS.dos_free2(screenCopy);
	_vm->_graphicsManager.NOLOCK = false;
}

bool AnimationManager::REDRAW_ANIM() {
	return false;
}

void AnimationManager::BOBANIM_ON(int idx) {
	if (GLOBALS.Bob[idx].field16) {
		GLOBALS.Bob[idx].field16 = 0;
		GLOBALS.Bob[idx].field10 = 5;
		GLOBALS.Bob[idx].fieldC = 250;
		GLOBALS.Bob[idx].field12 = 0;
		GLOBALS.Bob[idx].field14 = 0;
	}
}

void AnimationManager::BOBANIM_OFF(int idx) {
	GLOBALS.Bob[idx].field16 = 1;

}

void AnimationManager::CHARGE_ANIM(const Common::String &animName) {
	byte v20[15];
	char header[10];
	char filename1[15];
	char filename2[15];
	char filename3[15];
	char filename4[15];
	char filename5[15];
	char filename6[15];

	CLEAR_ANIM();

	Common::String filename = animName + ".ANI";
	FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPANIM, filename);
	
	Common::File f;
	if (!f.open(GLOBALS.NFICHIER))
		error("Failed to open %s", GLOBALS.NFICHIER);
	
	int filesize = f.size();
	int nbytes = filesize - 115;
	f.read(header, 10);
	f.read(v20, 15);
	f.read(filename1, 15);
	f.read(filename2, 15);
	f.read(filename3, 15);
	f.read(filename4, 15);
	f.read(filename5, 15);
	f.read(filename6, 15);

	if (header[0] != 'A' || header[1] != 'N' || header[2] != 'I' || header[3] != 'S')
		error("File incompatible with this soft.");

	const char *files[6] = { &filename1[0], &filename2[0], &filename3[0], &filename4[0],
			&filename5[0], &filename6[0] };

	for (int idx = 1; idx <= 6; ++idx) {
		if (files[idx - 1][0]) {
			FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPANIM, files[idx - 1]);
			
			if (!f.exists(GLOBALS.NFICHIER))
				error("File not found");
			if (CHARGE_BANK_SPRITE1(idx, files[idx - 1]))
				error("File not compatible with this soft.");
		}
	}

	byte *data = GLOBALS.dos_malloc2(nbytes + 1);
	f.read(data, nbytes);
	f.close();

	for (int idx = 1; idx <= 20; ++idx) {
		RECHERCHE_ANIM(data, idx, nbytes);
	}

	GLOBALS.dos_free2(data);
}

void AnimationManager::CLEAR_ANIM() {
	for (int idx = 0; idx < 35; ++idx) {
		if (GLOBALS.Bqe_Anim[idx].data != PTRNUL)
			GLOBALS.Bqe_Anim[idx].data = GLOBALS.dos_free2(GLOBALS.Bqe_Anim[idx].data);
		GLOBALS.Bqe_Anim[idx].field4 = 0;
	}

	for (int idx = 0; idx < 8; ++idx) {
		if (GLOBALS.Bank[idx].data != PTRNUL)
			GLOBALS.Bank[idx].data = GLOBALS.dos_free2(GLOBALS.Bank[idx].data);
		GLOBALS.Bank[idx].field4 = 0;
		GLOBALS.Bank[idx].filename1 = "";
		GLOBALS.Bank[idx].fileHeader = 0;
		GLOBALS.Bank[idx].field1C = 0;
	}
}

int AnimationManager::CHARGE_BANK_SPRITE1(int idx, const Common::String &filename) {
	byte *v3;
	byte *v4; 
	int v7; 
	int v8; 
	int width; 
	int height; 
	byte *v13;
	__int16 v16;
	__int16 v17;
	byte *ptr; 
	byte *v19;
	__int16 v20; 
	__int16 v21; 
	int result = 0;
	FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPANIM, filename);
	GLOBALS.Bank[idx].field1C = FileManager::FLONG(GLOBALS.NFICHIER);
	GLOBALS.Bank[idx].field4 = 1;
	GLOBALS.Bank[idx].filename1 = filename;
	GLOBALS.Bank[idx].filename2 = GLOBALS.REP_SPR;

	v3 = FileManager::CHARGE_FICHIER(GLOBALS.NFICHIER);
	v4 = v3;

	GLOBALS.Bank[idx].fileHeader = 0;
	if (*(v3 + 1) == 'L' && *(v3 + 2) == 'E')
	    GLOBALS.Bank[idx].fileHeader = 1;
	if (*(v3 + 1) == 'O' && *(v3 + 2) == 'R')
		GLOBALS.Bank[184].fileHeader = 2;
	
	if (GLOBALS.Bank[idx].fileHeader) {
		GLOBALS.Bank[idx].data = v3;

		v7 = 0;
		v8 = 0;
		do {
			ptr = v4;
			width = Get_Largeur(v4, v8);
			height = Get_Hauteur(ptr, v8);
			v4 = ptr;
			if (!width && !height)
				v7 = 1;
			if (!v7)
				++v8;
			if (v8 > 249)
				v7 = 1;
		} while (v7 != 1);
    
		if (v8 <= 249) {
			GLOBALS.Bank[idx].field1A = v8;
			
			Common::String ofsFilename = GLOBALS.Bank[idx].filename1;
			while (ofsFilename.lastChar() != '.')
				ofsFilename.deleteLastChar();
			ofsFilename += ".OFS";
			
			FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPANIM, ofsFilename);
			Common::File f;
			if (!f.exists(GLOBALS.NFICHIER)) {
				v19 = FileManager::CHARGE_FICHIER(GLOBALS.NFICHIER);
				v13 = v19;
				
				if (GLOBALS.Bank[idx].field1A > 0) {
					for (int v14 = 0; v14 < GLOBALS.Bank[idx].field1A; ++v14) {
						v16 = READ_LE_UINT16(v13);
						v17 = READ_LE_UINT16(v13 + 2);
						v21 = READ_LE_UINT16(v13 + 4);
						v20 = READ_LE_UINT16(v13 + 6);
						v13 += 8;

						set_offsetxy(GLOBALS.Bank[idx].data, v14, v16, v17, 0);
						if (GLOBALS.Bank[idx].fileHeader == 2)
							set_offsetxy(GLOBALS.Bank[idx].data, v14, v21, v20, 1);
					}
				}
			
				GLOBALS.dos_free2(v19);
			}
      
			result = 0;
		} else {
			GLOBALS.dos_free2(ptr);
			GLOBALS.Bank[idx].field4 = 0;
			result = -2;
		}
	} else {
		GLOBALS.dos_free2(v3);
		GLOBALS.Bank[idx].field4 = 0;
		result = -1;
	}

	return result;
}

void AnimationManager::set_offsetxy(byte *data, int idx, int xp, int yp, bool isSize) {
	byte *startP = data + 3;
	for (int i = idx; i; --i)
		startP += READ_LE_UINT32(startP) + 16;
	
	byte *rectP = startP + 8;
	if (isSize == 1) {
		// Set size
		byte *pointP = rectP + 4;
		WRITE_LE_UINT16(pointP, xp);
		WRITE_LE_UINT16(pointP + 2, yp);
	} else {
		// Set position
		WRITE_LE_UINT16(rectP, xp);
		WRITE_LE_UINT16(rectP + 2, yp);
	}
}

void AnimationManager::RECHERCHE_ANIM(const byte *data, int idx, int nbytes) {
	int v4; 
	const byte *v5; 
	int v6; 
	int v7; 
	byte *v9; 
	__int16 v10;
	__int16 v11;
	__int16 v12;
	char v13;
	signed int v14;
	__int16 v15;
	__int16 v16;
	char v17;
	int v19; 
	char v20;
	int v21; 
	__int16 v22;
	const byte *v23;
	int v24;

	bool doneFlag = false;
	bool breakFlag;

	v21 = 0;
	v4 = 8 * idx;
	v19 = 8 * idx;
  
	do {
		v20 = *(v21 + data);
		if (*(data + v20) == 'A' && *(data + v20 + 1) == 'N' && *(data + v20 + 2) == 'I'
				&& *(data + v20 + 3) == 'M') {
            v4 = *(data + v21 + 4);
            if (idx == v4) {
				v5 = v21 + data + 5;
				v6 = v21 + 5;
				v7 = 0;
				breakFlag = false;
              
				do {
					if (*v5 == 'A' && *(v5 + 1) == 'N' && *(v5 + 2) == 'I' && *(v5 + 3) == 'M')
						breakFlag = true;
                
					if (*v5 == 'F' && *(v5 + 1) == 'I' && *(v5 + 2) == 'N')
						breakFlag = true;

					if (nbytes < v6) {
						GLOBALS.Bqe_Anim[idx].field4 = 0;
						GLOBALS.Bqe_Anim[idx].data = PTRNUL;
					}
                
					++v6;
					++v7;
					++v5;
				} while (!breakFlag);

				GLOBALS.Bqe_Anim[idx].data = GLOBALS.dos_malloc2(v7 + 50);
				GLOBALS.Bqe_Anim[idx].field4 = 1;
				memcpy(GLOBALS.Bqe_Anim[idx].data, v21 + data + 5, 20);
				
				byte *dataP = GLOBALS.Bqe_Anim[idx].data;

				v9 = dataP + 20;
				v23 = v21 + data + 25;
				v10 = READ_LE_UINT16(v21 + data + 25);
				v11 = READ_LE_UINT16(v21 + data + 27);
				v22 = READ_LE_UINT16(v21 + data + 29);
				v12 = READ_LE_UINT16(v21 + data + 31);
				v13 = READ_LE_UINT16(v21 + data + 33);
				*(dataP + 29) = *(v21 + data + 34);
				WRITE_LE_UINT16(dataP + 20, v10);
				WRITE_LE_UINT16(dataP + 22, v11);
				WRITE_LE_UINT16(dataP + 24, v22);
				WRITE_LE_UINT16(dataP + 26, v12);
				WRITE_LE_UINT16(dataP + 28, v13);
				v14 = 1;
              
				do {
					v9 += 10;
					v23 += 10;
					if (!v22)
						break;
                
					v24 = READ_LE_UINT16(v23);
					v15 = READ_LE_UINT16(v23 + 2);
					v22 = READ_LE_UINT16(v23 + 4);
					v16 = READ_LE_UINT16(v23 + 6);
					v17 = READ_LE_UINT16(v23 + 8);
					*(v9 + 9) = *(v23 + 9);
					WRITE_LE_UINT16(v9, v24);
					WRITE_LE_UINT16(v9 + 2, v15);
					WRITE_LE_UINT16(v9 + 4, v22);
	                WRITE_LE_UINT16(v9 + 6, v16);
					WRITE_LE_UINT16(v9 + 8, v17);
					++v14;
				} while (v14 <= 4999);
              
				doneFlag = 1;
			}
		}
    
		if (*(data + v20) == 'F' && *(data + v21 + 1) == 'I' && *(data + v21 + 2) == 'N')
			doneFlag = 1;
		++v21;
	} while (v21 <= nbytes && !doneFlag);
}

int AnimationManager::Get_Largeur(const byte *data, int idx) {
	const byte *rectP = data + 3;
	for (int i = idx; i; --i)
		rectP += READ_LE_UINT32(rectP) + 16;
	
	return (int16)READ_LE_UINT16(rectP + 4);
}

int AnimationManager::Get_Hauteur(const byte *data, int idx) {
	const byte *rectP = data + 3;
	for (int i = idx; i; --i)
		rectP += READ_LE_UINT32(rectP) + 16;
	
	return (int16)READ_LE_UINT16(rectP + 6);
}

} // End of namespace Hopkins
