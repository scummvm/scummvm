/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "wince.h"

extern bool select_game;
extern bool _get_key_mapping;
extern bool draw_keyboard;
extern bool hide_toolbar;
extern bool freelook;
extern bool is_simon;
extern bool is_bass;
extern int num_of_dirty_square;
extern bool toolbar_drawn;
extern Engine *engine;
extern bool sound_activated;
extern char is_demo;

extern ToolbarSelected getToolbarSelection(int, int);
extern void redrawSoundItem();
extern int mapKey(int key);
extern void handleSelectGame(int, int);
extern void do_quit();

const char KEYBOARD_MAPPING_ALPHA_HIGH[] = {"abcdefghijklm"};
const char KEYBOARD_MAPPING_NUMERIC_HIGH[] = {"12345"};
const char KEYBOARD_MAPPING_ALPHA_LOW[] = {"nopqrstuvwxyz"};
const char KEYBOARD_MAPPING_NUMERIC_LOW[] = {"67890"};



BOOL PPCWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, OSystem_WINCE3 *wm) {
	switch(message) {
		case WM_KEYDOWN:

		/*
		if (wParam == VK_ESCAPE)   // FIXME
			do_quit();
		*/

		if(wParam && wParam != 0x84 && wParam != 0x5B) { // WHAT THE ???			

			if (select_game) {
				GAPIKeysHandleSelect((int)wParam);
				break;
			}

			/*
			unsigned char GAPI_key;

			GAPI_key = getGAPIKeyMapping((short)wParam);
			if (GAPI_key) {
			*/
				if (_get_key_mapping) {
					wm->_event.kbd.flags = 0xff;
					wm->_event.kbd.ascii = GAPIKeysTranslate((unsigned int)(wParam));
					wm->_event.event_code = OSystem::EVENT_KEYDOWN;
					break;
				}					
				/*
				else
					processAction((short)wParam);
				*/
			/*}*/
			if (!processAction(GAPIKeysTranslate((unsigned int)(wParam))))
			/*else*/ {
				wm->_event.kbd.ascii = mapKey(wParam);
				wm->_event.kbd.keycode = tolower(wm->_event.kbd.ascii);
				wm->_event.event_code = OSystem::EVENT_KEYDOWN;								
			}
		}

		break;

	case WM_KEYUP:
		if (_get_key_mapping) {
			_get_key_mapping = false;
			wm->_event.kbd.flags = 0xff;
			wm->_event.kbd.ascii = GAPIKeysTranslate((int)wParam);
			wm->_event.event_code = OSystem::EVENT_KEYUP;
			break;
		}
		else {
			wm->_event.kbd.ascii = mapKey(wParam);
			wm->_event.kbd.keycode = mapKey(wParam);
			wm->_event.event_code = OSystem::EVENT_KEYUP;
		}
		break;

	case WM_MOUSEMOVE:
		{
			int x = ((int16*)&lParam)[0];
			int y = ((int16*)&lParam)[1];
			/*if (select_game) {
				handleSelectGame(x, y);
				break;
			}*/
			if (select_game)
				break;
			Translate(&x, &y);
			wm->_event.event_code = OSystem::EVENT_MOUSEMOVE;
			wm->_event.mouse.x = x;
			wm->_event.mouse.y = y;
			wm->_last_mouse_event = wm->_event;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			ToolbarSelected toolbar_selection;
			int x = ((int16*)&lParam)[0];
			int y = ((int16*)&lParam)[1];
			if (select_game) {
				handleSelectGame(x, y);
				break;
			}

			Translate(&x, &y);

			wm->_event.kbd.flags = 0;

			if (freelook) {
				wm->_event.event_code = OSystem::EVENT_MOUSEMOVE;
				wm->_event.mouse.x = x;
				wm->_event.mouse.y = y;
				break;
			}

			if (draw_keyboard) {
				// Handle keyboard selection
				int offset_y;
				int saved_x = x;
				int saved_y = y;

				/*
				if (!GetScreenMode()) {
					x = ((int16*)&lParam)[0];
					y = ((int16*)&lParam)[1];
				}
				*/

				offset_y = (GetScreenMode() ? 0 : 40 + 22);

				if (x<185 && y>=(200 + offset_y)) {
					//Alpha selection
					wm->_event.event_code = OSystem::EVENT_KEYDOWN;
					wm->_event.kbd.ascii = 
						(y <= (220 + offset_y)? KEYBOARD_MAPPING_ALPHA_HIGH[((x + 10) / 14) - 1] :
												KEYBOARD_MAPPING_ALPHA_LOW[((x + 10) / 14) - 1]);
					wm->_event.kbd.keycode = tolower(wm->_event.kbd.ascii);
					break;
				} 
				else
				if (x>=186 && y>=(200 + offset_y) && x<=255) {
				   // Numeric selection
					wm->_event.event_code = OSystem::EVENT_KEYDOWN;				   
				   wm->_event.kbd.ascii =
					   (y <= (220 + offset_y) ? KEYBOARD_MAPPING_NUMERIC_HIGH[((x - 187 + 10) / 14) - 1] :
												KEYBOARD_MAPPING_NUMERIC_LOW[((x - 187 + 10) / 14) - 1]);
				   wm->_event.kbd.keycode = wm->_event.kbd.ascii;
				   break;
				}
				else
				if (x>=302 && x <= 316 && y >= (200 + offset_y) && y <= (220 + offset_y)) {
				  // Backspace
				  wm->_event.event_code = OSystem::EVENT_KEYDOWN;
				  wm->_event.kbd.keycode = 8;
				  wm->_event.kbd.ascii = mapKey(VK_BACK);
				  break;
				}
				else
				if (x>=302 && x<= 316 && y >= (220 + offset_y)) { 
				  // Enter
				  wm->_event.event_code = OSystem::EVENT_KEYDOWN;
				  wm->_event.kbd.keycode = '\n';
				  wm->_event.kbd.ascii = mapKey(VK_RETURN);
				  break;
				}

				x = saved_x;
				y = saved_y;

				wm->_event.event_code = OSystem::EVENT_LBUTTONDOWN;
				wm->_event.mouse.x = x;
				wm->_event.mouse.y = y;
				wm->_last_mouse_event = wm->_event;
				break;

			}
					

			toolbar_selection = (hide_toolbar || _get_key_mapping ? ToolbarNone : 
									 getToolbarSelection(
										 (GetScreenMode() ? x : ((int16*)&lParam)[0]), 
										 (GetScreenMode() ? y : ((int16*)&lParam)[1])));
			if (toolbar_selection == ToolbarNone) {				
				wm->_event.event_code = OSystem::EVENT_LBUTTONDOWN;
				wm->_event.mouse.x = x;
				wm->_event.mouse.y = y;
				wm->_last_mouse_event = wm->_event;			
			}
			else {
				switch(toolbar_selection) {
					case ToolbarSaveLoad:
						if (is_simon) 
							break;

						/*if (GetScreenMode()) {*/
						/*
							draw_keyboard = true;
							if (!hide_toolbar)
								toolbar_drawn = false;
						*/
						/*}*/
						wm->_event.event_code = OSystem::EVENT_KEYDOWN;
						if (is_bass)
							wm->_event.kbd.ascii = 63;
						else
							wm->_event.kbd.ascii = 319;	// "F5" key; opens up the save menu in Scumm games
						break;
					case ToolbarMode:
						SetScreenMode(!GetScreenMode());
						num_of_dirty_square = MAX_NUMBER_OF_DIRTY_SQUARES;
						if (!hide_toolbar)
							toolbar_drawn = false;
						break;
					case ToolbarSkip:
						if (is_demo)
							do_quit();

						wm->_event.event_code = OSystem::EVENT_KEYDOWN;
						if (is_simon || is_bass) {
							wm->_event.kbd.ascii = mapKey(VK_ESCAPE);
							wm->_event.kbd.keycode = mapKey(VK_ESCAPE);
							break;
						}
						/*
						if (g_scumm->vm.cutScenePtr[g_scumm->vm.cutSceneStackPointer] || g_scumm->_insaneState)
							wm->_event.kbd.ascii = g_scumm->_vars[g_scumm->VAR_CUTSCENEEXIT_KEY];
						else
						if (g_scumm->_talkDelay > 0)
							wm->_event.kbd.ascii = g_scumm->VAR_TALKSTOP_KEY;						
						else
							wm->_event.kbd.ascii = mapKey(VK_ESCAPE);
						*/
						wm->_event.kbd.ascii = KEY_ALL_SKIP;
						break;
					case ToolbarSound:
						sound_activated = !sound_activated;
						redrawSoundItem();
						break;
					default:
						break;
				}
			}
		}
		break;

	case WM_LBUTTONUP:
		{
			// pinched from the SDL code. Distinguishes between taps and not
			int x = ((int16*)&lParam)[0];
			int y = ((int16*)&lParam)[1];
			/*
			if (select_game) {
				handleSelectGame(x, y);
				break;
			}
			*/
			if (select_game)
				break;
			Translate(&x, &y);
			wm->_event.event_code = OSystem::EVENT_LBUTTONUP;
			wm->_event.mouse.x = x;
			wm->_event.mouse.y = y;
			wm->_last_mouse_event = wm->_event;
		}
		break;

	case WM_LBUTTONDBLCLK:  // doesn't seem to work right now
		//wm->_scumm->_rightBtnPressed |= msClicked | msDown;
		break;

	default:
		return FALSE;

	}

	return TRUE;
}
