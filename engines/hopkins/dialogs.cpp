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
#include "common/events.h"
#include "common/util.h"
#include "hopkins/dialogs.h"
#include "hopkins/events.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/sound.h"

namespace Hopkins {

void OptionsDialog::show(HopkinsEngine *vm) {
	bool doneFlag;

	doneFlag = false;
	vm->_eventsManager.CHANGE_MOUSE(0);
	vm->_eventsManager.VBL();
	if (vm->_globals.FR == 1)
		vm->_fileManager.CONSTRUIT_SYSTEM("OPTIFR.SPR");
	if (!vm->_globals.FR)
		vm->_fileManager.CONSTRUIT_SYSTEM("OPTIAN.SPR");
	if (vm->_globals.FR == 2)
		vm->_fileManager.CONSTRUIT_SYSTEM("OPTIES.SPR");
  
	vm->_globals.OPTION_SPR = vm->_fileManager.CHARGE_FICHIER(vm->_globals.NFICHIER);
	vm->_globals.OPTION_FLAG = true;

	do {
		if (vm->_eventsManager.BMOUSE()) {
			Common::Point mousePos(vm->_eventsManager.XMOUSE(), vm->_eventsManager.YMOUSE());
			mousePos.x = vm->_eventsManager.XMOUSE();
			mousePos.y = vm->_eventsManager.YMOUSE();
	      
			if (!vm->_soundManager.MUSICOFF) {
				if (mousePos.x >= vm->_graphicsManager.ofscroll + 300 && mousePos.y > 113 && mousePos.x <= vm->_graphicsManager.ofscroll + 327 && mousePos.y <= 138) {
					++vm->_soundManager.MUSICVOL;
					vm->_soundManager.OLD_MUSICVOL = vm->_soundManager.MUSICVOL;
	          
					if (vm->_soundManager.MUSICVOL <= 12)
						vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						vm->_soundManager.MUSICVOL = 12;
					vm->_soundManager.MODSetMusicVolume(vm->_soundManager.MUSICVOL);
				}
	        
				if (!vm->_soundManager.MUSICOFF && mousePos.x >= vm->_graphicsManager.ofscroll + 331 && mousePos.y > 113 && mousePos.x <= vm->_graphicsManager.ofscroll + 358 && mousePos.y <= 138) {
					--vm->_soundManager.MUSICVOL;
					if (vm->_soundManager.MUSICVOL >= 0)
						vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						vm->_soundManager.MUSICVOL = 0;
				
					vm->_soundManager.OLD_MUSICVOL = vm->_soundManager.MUSICVOL;
					vm->_soundManager.MODSetMusicVolume(vm->_soundManager.MUSICVOL);
				}
			}
			if (!vm->_soundManager.SOUNDOFF) {
				if (mousePos.x >= vm->_graphicsManager.ofscroll + 300 && mousePos.y > 140 && mousePos.x <= vm->_graphicsManager.ofscroll + 327 && mousePos.y <= 165) {
					++vm->_soundManager.SOUNDVOL;
					if (vm->_soundManager.SOUNDVOL <= 16)
						vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						vm->_soundManager.SOUNDVOL = 16;
					vm->_soundManager.OLD_SOUNDVOL = vm->_soundManager.SOUNDVOL;
					vm->_soundManager.MODSetSampleVolume();
				}
	        
				if (!vm->_soundManager.SOUNDOFF && mousePos.x >= vm->_graphicsManager.ofscroll + 331 && mousePos.y > 140 && mousePos.x <= vm->_graphicsManager.ofscroll + 358 && mousePos.y <= 165) {
					--vm->_soundManager.SOUNDVOL;
					if (vm->_soundManager.SOUNDVOL >= 0)
						vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						vm->_soundManager.SOUNDVOL = 0;
					vm->_soundManager.OLD_SOUNDVOL = vm->_soundManager.SOUNDVOL;
					vm->_soundManager.MODSetSampleVolume();
				}
			}
	      
			if (!vm->_soundManager.VOICEOFF) {
				if (mousePos.x >= vm->_graphicsManager.ofscroll + 300 && mousePos.y > 167 && mousePos.x <= vm->_graphicsManager.ofscroll + 327 && mousePos.y <= 192) {
					++vm->_soundManager.VOICEVOL;
	          
					if (vm->_soundManager.VOICEVOL <= 16)
						vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						vm->_soundManager.VOICEVOL = 16;
					vm->_soundManager.OLD_VOICEVOL = vm->_soundManager.VOICEVOL;
					vm->_soundManager.MODSetVoiceVolume();
				}
	        
				if (!vm->_soundManager.VOICEOFF && mousePos.x >= vm->_graphicsManager.ofscroll + 331 && mousePos.y > 167 && mousePos.x <= vm->_graphicsManager.ofscroll + 358 && mousePos.y <= 192) {
					--vm->_soundManager.VOICEVOL;
					if (vm->_soundManager.VOICEVOL >= 0)
						vm->_soundManager.PLAY_SOUND("bruit2.wav");
					else
						vm->_soundManager.VOICEVOL = 0;
					vm->_soundManager.OLD_VOICEVOL = vm->_soundManager.VOICEVOL;
					vm->_soundManager.MODSetVoiceVolume();
				}
			}
	      
			if (mousePos.x >= vm->_graphicsManager.ofscroll + 431) {
				if (mousePos.y > 194 && mousePos.x <= vm->_graphicsManager.ofscroll + 489 && mousePos.y <= 219)
					vm->_soundManager.TEXTOFF = vm->_soundManager.TEXTOFF != 1;

				if (mousePos.x >= vm->_graphicsManager.ofscroll + 431) {
					if (mousePos.y > 167 && mousePos.x <= vm->_graphicsManager.ofscroll + 489 && mousePos.y <= 192)
						vm->_soundManager.VOICEOFF = vm->_soundManager.VOICEOFF != 1;
					if (mousePos.x >= vm->_graphicsManager.ofscroll + 431) {
						if (mousePos.y > 113 && mousePos.x <= vm->_graphicsManager.ofscroll + 489 && mousePos.y <= 138) {
							if (vm->_soundManager.MUSICOFF == 1) {
								vm->_soundManager.MUSICOFF = 0;
								vm->_soundManager.MODSetMusicVolume(vm->_soundManager.MUSICVOL);
							} else {
								vm->_soundManager.MUSICOFF = 1;
								vm->_soundManager.MODSetMusicVolume(0);
							}
						}
	        
						if (mousePos.x >= vm->_graphicsManager.ofscroll + 431 && mousePos.y > 140 && mousePos.x <= vm->_graphicsManager.ofscroll + 489 && mousePos.y <= 165)
							vm->_soundManager.SOUNDOFF = vm->_soundManager.SOUNDOFF != 1;
					}
				}
			}
	      
			if (mousePos.x >= vm->_graphicsManager.ofscroll + 175 && mousePos.y > 285 && mousePos.x <= vm->_graphicsManager.ofscroll + 281 && mousePos.y <= 310) {
				vm->_globals.SORTIE = 300;
				doneFlag = true;
			}
			if (mousePos.x >= vm->_graphicsManager.ofscroll + 355 && mousePos.y > 285 && mousePos.x <= vm->_graphicsManager.ofscroll + 490 && mousePos.y <= 310)
				doneFlag = true;
			if (mousePos.x >= vm->_graphicsManager.ofscroll + 300 && mousePos.y > 194 && mousePos.x <= vm->_graphicsManager.ofscroll + 358 && mousePos.y <= 219) {
				switch (vm->_graphicsManager.SPEED_SCROLL) {
				case 1:
					vm->_graphicsManager.SPEED_SCROLL = 2;
					break;
				case 2:
					vm->_graphicsManager.SPEED_SCROLL = 4;
					break;
				case 4:
					vm->_graphicsManager.SPEED_SCROLL = 8;
					break;
				case 8:
					vm->_graphicsManager.SPEED_SCROLL = 16;
					break;
				case 16:
					vm->_graphicsManager.SPEED_SCROLL = 32;
					break;
				case 32:
					vm->_graphicsManager.SPEED_SCROLL = 48;
					break;
				case 48:
					vm->_graphicsManager.SPEED_SCROLL = 64;
					break;
				case 64:
					vm->_graphicsManager.SPEED_SCROLL = 128;
					break;
				case 128:
					vm->_graphicsManager.SPEED_SCROLL = 160;
					break;
				case 160:
					vm->_graphicsManager.SPEED_SCROLL = 320;
					break;
				case 320:
					vm->_graphicsManager.SPEED_SCROLL = 1;
					break;
				}
			}

			if (mousePos.x >= vm->_graphicsManager.ofscroll + 348 && mousePos.y > 248 && mousePos.x <= vm->_graphicsManager.ofscroll + 394 && mousePos.y <= 273)
				vm->_globals.vitesse = 2;
			if (mousePos.x >= vm->_graphicsManager.ofscroll + 300 && mousePos.y > 221 && mousePos.x <= vm->_graphicsManager.ofscroll + 358 && mousePos.y <= 246)
				vm->_globals.SVGA = 2;
			if (mousePos.x < vm->_graphicsManager.ofscroll + 165 || mousePos.x > vm->_graphicsManager.ofscroll + 496 || (uint)(mousePos.y - 107) > 0xD3u)
				doneFlag = true;
		}

		if (!vm->_graphicsManager.MANU_SCROLL)
			vm->_globals.opt_scrtype = 2;
		if (vm->_graphicsManager.MANU_SCROLL == 1)
			vm->_globals.opt_scrtype = 1;
		if (vm->_globals.vitesse == 1)
			vm->_globals.opt_vitesse = 6;
		if (vm->_globals.vitesse == 2)
			vm->_globals.opt_vitesse = 5;
		if (vm->_globals.vitesse == 3)
			vm->_globals.opt_vitesse = 4;

		vm->_globals.opt_txt = !vm->_soundManager.TEXTOFF ? 7 : 8;
		vm->_globals.opt_voice = !vm->_soundManager.VOICEOFF ? 7 : 8;
		vm->_globals.opt_sound = !vm->_soundManager.SOUNDOFF ? 7 : 8;
		vm->_globals.opt_music = !vm->_soundManager.MUSICOFF ? 7 : 8;
			
		if (vm->_globals.SVGA == 1)
			vm->_globals.opt_anm = 10;
		if (vm->_globals.SVGA == 2)
			vm->_globals.opt_anm = 9;
		if (vm->_globals.SVGA == 3)
			vm->_globals.opt_anm = 11;
		if (vm->_graphicsManager.SPEED_SCROLL == 1)
			vm->_globals.opt_scrspeed = 12;
		if (vm->_graphicsManager.SPEED_SCROLL == 2)
			vm->_globals.opt_scrspeed = 13;
		if (vm->_graphicsManager.SPEED_SCROLL == 4)
			vm->_globals.opt_scrspeed = 14;
		if (vm->_graphicsManager.SPEED_SCROLL == 8)
			vm->_globals.opt_scrspeed = 15;
		if (vm->_graphicsManager.SPEED_SCROLL == 16)
			vm->_globals.opt_scrspeed = 16;
		if (vm->_graphicsManager.SPEED_SCROLL == 32)
			vm->_globals.opt_scrspeed = 17;
		if (vm->_graphicsManager.SPEED_SCROLL == 48)
			vm->_globals.opt_scrspeed = 18;
		if (vm->_graphicsManager.SPEED_SCROLL == 64)
			vm->_globals.opt_scrspeed = 19;
		if (vm->_graphicsManager.SPEED_SCROLL == 128)
			vm->_globals.opt_scrspeed = 20;
		if (vm->_graphicsManager.SPEED_SCROLL == 160)
			vm->_globals.opt_scrspeed = 21;
		if (vm->_graphicsManager.SPEED_SCROLL == 320)
			vm->_globals.opt_scrspeed = 22;
		if (vm->_graphicsManager.SPEED_SCROLL == 640)
			vm->_globals.opt_scrspeed = 23;

		vm->_eventsManager.VBL();
	} while (!doneFlag);

	vm->_graphicsManager.SCOPY(vm->_graphicsManager.VESA_SCREEN, vm->_graphicsManager.ofscroll + 164, 
		107, 335, 215, vm->_graphicsManager.VESA_BUFFER, vm->_graphicsManager.ofscroll + 164, 107);
	vm->_graphicsManager.Ajoute_Segment_Vesa(vm->_graphicsManager.ofscroll + 164, 107, 
		vm->_graphicsManager.ofscroll + 498, 320);

	vm->_globals.OPTION_SPR = vm->_globals.dos_free2(vm->_globals.OPTION_SPR);
	vm->_globals.OPTION_FLAG = false;
}

} // End of namespace Hopkins
