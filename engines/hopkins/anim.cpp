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
MAIN_LOOP:
		v15 = 0;
		v14 = 0;
		v17 = 0;
		v16 = 0;
		v18 = 1;
		screenP = _vm->_graphicsManager.VESA_SCREEN;

		FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPANM, filename);

		if (!f.open(GLOBALS.NFICHIER))
			error("Not Found file %s", GLOBALS.NFICHIER.c_str());

		f.skip(6);
		// TODO: Original above read seems to overlap the doneFlag
		//doneFlag = f.readUint16LE() != 0;
		f.read(_vm->_graphicsManager.Palette, 800);
		f.skip(4);
		nbytes = f.readUint32LE();
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

				if (REDRAW_ANIM())
					goto REDRAW_ANIM;

				_vm->_eventsManager.CONTROLE_MES();
			} while (_vm->_eventsManager.lItCounter < rate1);
		}

		_vm->_eventsManager.lItCounter = 0;
		doneFlag = false;
		idx = 0;
		do {
			_vm->_soundManager.PLAY_ANM_SOUND(idx);
			f.skip(6);

			// Get in string
			Common::fill(&strBuffer[0], &strBuffer[20], 0);
			if (f.read(strBuffer, 16) != 16)
				doneFlag = true;

			if (strncmp(strBuffer, "IMAGE=", 7) != 0)
				doneFlag = true;

			if (!doneFlag) {
				f.read(screenP, READ_LE_UINT32(strBuffer + 8));

				if (_vm->_globals.iRegul == 1) {
					do {
						if (_vm->_eventsManager.ESC_KEY)
							goto FINISH;

						if (REDRAW_ANIM()) {
							if (_vm->_graphicsManager.NOLOCK == 1)
								goto FINISH;

							f.close();
							if (doneFlag <= SCREEN_WIDTH)
								goto MAIN_LOOP;

							screenCopy = GLOBALS.dos_free2(screenCopy);
							goto MAIN_LOOP;
						}

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
		f.read(_vm->_graphicsManager.Palette, 800);
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

} // End of namespace Hopkins
