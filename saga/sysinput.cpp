/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "reinherit.h"

#include <SDL.h>

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "actor_mod.h"
#include "console_mod.h"
#include "interface_mod.h"
#include "render_mod.h"
#include "scene_mod.h"
#include "script_mod.h"

namespace Saga {

/*
 * Begin module component
\*--------------------------------------------------------------------------*/

int SYSINPUT_Init(void)
{

	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(200, 30);

	return R_SUCCESS;
}

int SYSINPUT_ProcessInput(void)
{
	SDL_Event sdl_event;

	int mouse_x, mouse_y;
	R_POINT imouse_pt;

	SYSINPUT_GetMousePos(&mouse_x, &mouse_y);

	imouse_pt.x = mouse_x;
	imouse_pt.y = mouse_y;

	RENDER_ConvertMousePt(&imouse_pt);

	while (SDL_PollEvent(&sdl_event)) {

		int in_char;

		switch (sdl_event.type) {

		case SDL_KEYDOWN:

			if (CON_IsActive()) {

				in_char = sdl_event.key.keysym.sym;

				switch (sdl_event.key.keysym.sym) {

				case SDLK_BACKQUOTE:
					CON_Deactivate();
					break;

				case SDLK_PAGEUP:
					CON_PageUp();
					break;

				case SDLK_PAGEDOWN:
					CON_PageDown();
					break;

				case SDLK_UP:
				case SDLK_KP8:
					CON_CmdUp();
					break;

				case SDLK_DOWN:
				case SDLK_KP2:
					CON_CmdDown();
					break;

				default:

					if ((sdl_event.key.keysym.
						unicode & 0xFF80) == 0) {

						in_char =
						    sdl_event.key.keysym.
						    unicode & 0x7F;
						if (in_char) {
							CON_Type(in_char);
						}
					} else {
						R_printf(R_STDOUT,
						    "Ignored UNICODE character.\n");
					}

					break;
				}

				break;
			}

			switch (sdl_event.key.keysym.sym) {

			case SDLK_BACKQUOTE:
				CON_Activate();
				break;

			case SDLK_q:
				R_printf(R_STDOUT, "Quit key pressed.\n");
				/*goto done; */
				break;

			case SDLK_r:
				INTERFACE_Draw();
				break;

			case SDLK_F1:
				RENDER_ToggleFlag(RF_SHOW_FPS);
				break;

			case SDLK_F2:
				RENDER_ToggleFlag(RF_PALETTE_TEST);
				break;

			case SDLK_F3:
				RENDER_ToggleFlag(RF_TEXT_TEST);
				break;

			case SDLK_F4:
				RENDER_ToggleFlag(RF_OBJECTMAP_TEST);
				break;

			case SDLK_1:
				RENDER_SetMode(RM_NORMAL);
				break;

			case SDLK_4:
				RENDER_SetMode(RM_2XSAI);
				break;

			case SDLK_5:
				RENDER_SetMode(RM_SUPER2XSAI);
				break;

			case SDLK_6:
				RENDER_SetMode(RM_SUPEREAGLE);
				break;

			case SDLK_TAB:
				STHREAD_DebugStep();
				break;

				/* Actual game keys */

			case SDLK_SPACE:

				ACTOR_SkipDialogue();
				break;

			case SDLK_PAUSE:
			case SDLK_p:
				RENDER_ToggleFlag(RF_RENDERPAUSE);
				break;

			case SDLK_ESCAPE:
				/* Skip to next scene skip target */
				SCENE_Skip();
				break;

			default:
				break;
			}

			break;

		case SDL_KEYUP:
			break;

		case SDL_MOUSEBUTTONDOWN:
			INTERFACE_Update(&imouse_pt, UPDATE_MOUSECLICK);
			break;

		default:
			break;
		}
	}

	return R_SUCCESS;
}

int SYSINPUT_GetMousePos(int *mouse_x, int *mouse_y)
{

	SDL_GetMouseState(mouse_x, mouse_y);

	return R_SUCCESS;
}

int SYSINPUT_HideMouse(void)
{

	SDL_ShowCursor(SDL_DISABLE);

	return R_SUCCESS;
}

int SYSINPUT_ShowMouse(void)
{

	SDL_ShowCursor(SDL_ENABLE);

	return R_SUCCESS;
}

} // End of namespace Saga

