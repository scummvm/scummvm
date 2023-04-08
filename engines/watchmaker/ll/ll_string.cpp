/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy

#include "watchmaker/ll/ll_string.h"
#include "watchmaker/globvar.h"
#include "watchmaker/define.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/game.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/classes/do_dialog.h"
#include "watchmaker/t2d/expr.h"
#include "watchmaker/renderer.h"
#include "watchmaker/3d/t3d_body.h"

namespace Watchmaker {

// locals
int32 LastTextTime;
int console_3_4_xoffs = 0;

/* -----------------17/03/98 17.48-------------------
 *                      Text
 * --------------------------------------------------*/
void Text(uint16 x, uint16 y, uint16 dx, char *text) {
	if (!text) return;

	LastTextTime = TheTime;

	TheString.x = x;
	TheString.y = y;
	TheString.dx = dx;
	strcpy(TheString.text, text);
}

/* -----------------17/03/98 17.48-------------------
 *                      ClearText
 * --------------------------------------------------*/
void ClearText(void) {
	if (!(bUseWith & UW_ON)) {
		TheString.x = 0;
		TheString.y = 0;
		TheString.dx = 0;
		memset(TheString.text, 0, sizeof(TheString.text));
	}
}

/*-----------------17/02/95 09.53-------------------
 TextLen - calcola lunghezza str dal car 0 a num
--------------------------------------------------*/
uint16 TextLen(Fonts &fonts, char *sign, uint16 num) {
	uint16 Len, b, c;
	b = 0;

	if (sign == nullptr)
		return (0);

	if (num == 0)
		Len = strlen(sign);
	else
		Len = num;

	for (c = 0; c < Len; c++)
		b += (fonts.StandardFont.table[(uint16)((uint8)sign[c]) * 4 + 2]);

	return (b);
}

/*-----------------14/05/95 12.12-------------------
   CheckText - returns how many lines it will write
--------------------------------------------------*/
uint16 CheckText(Fonts &fonts, uint16 dx, char *sign) {
	uint16 a, b;
	uint16 CurInit;
	uint16 LastSpace;
	uint16 CurLine;

	if (sign == NULL) return (0);
//	Azzera tutto
	memset(TextLines, 0, sizeof(TextLines));

	a = 0;
	LastSpace = 0;
	CurInit = 0;
	CurLine = 0;

//	Caso piu' semplice: sta tutto su una riga
	if (TextLen(fonts, sign, 0) <= dx) {
		strcpy((char *)TextLines[CurLine], sign);
		return (1);
	}

	while (a < strlen(sign)) {
		a++;
		if (sign[a] == ' ') {
			if (TextLen(fonts, (char *)(sign + CurInit), (uint16)(a - CurInit)) <= dx)
				LastSpace = a;
			else if (TextLen(fonts, (char *)(sign + CurInit), (uint16)(LastSpace - CurInit)) <= dx) {
				for (b = CurInit; b < LastSpace; b++)
					TextLines[CurLine][b - CurInit] = sign[b];

				TextLines[CurLine][b - CurInit] = '\0';
				CurInit = LastSpace + 1;
				a = CurInit;
				CurLine++;
			} else
				return (0);
		} else if (sign[a] == '\0') {
			if (TextLen(fonts, (char *)(sign + CurInit), (uint16)(a - CurInit)) <= dx) {
				for (b = CurInit; b < a; b++)
					TextLines[CurLine][b - CurInit] = sign[b];
				TextLines[CurLine][b - CurInit] = '\0';

				CurLine++;
				CurInit = a + 1;
				return (CurLine);
			} else if (TextLen(fonts, (char *)(sign + CurInit), (uint16)(LastSpace - CurInit)) <= dx) {
				for (b = CurInit; b < LastSpace; b++)
					TextLines[CurLine][b - CurInit] = sign[b];

				TextLines[CurLine][b - CurInit] = '\0';
				CurLine++;
				CurInit = LastSpace + 1;

				if (CurInit < strlen(sign)) {
					for (b = CurInit; b < strlen(sign); b++)
						TextLines[CurLine][b - CurInit] = sign[b];

					TextLines[CurLine][b - CurInit] = '\0';
					CurLine++;
				}
				return (CurLine);
			} else
				return (0);
		}
	}
	return (0);
}


/* -----------------18/03/98 9.57--------------------
 *                  PaintText
 * --------------------------------------------------*/
void PaintText(WGame &game) {
	uint16 lines, i;
	int32 dx, obj;
	FontColor color;
	Init &init = game.init;

	if (bTitoliCodaStatic || bTitoliCodaScrolling)  return;

	if (TheString.text) {
		if (bDialogActive) {
			obj = init.Anim[TimeAnim].obj;
			if (obj == ocCURPLAYER)
				Player->Mesh->ExpressionFrame = VisemaTimeRecon(TheTime - LastTextTime);
			else if ((obj >= ocDARRELL) && (obj <= ocLASTCHAR) && (obj) && (Character[obj]->Mesh))
				Character[obj]->Mesh->ExpressionFrame = VisemaTimeRecon(TheTime - LastTextTime);
		}

		lines = (uint16)CheckText(game._fonts, (uint16)game._renderer->rFitY((int32)TheString.dx), TheString.text);
		for (i = 0; i < lines; i++) {
			dx = (TheString.dx - (TextLen(game._fonts, TextLines[i], 0) * SCREEN_RES_X) / game._renderer->rFitX(SCREEN_RES_X)) / 2;
			obj = init.Anim[TimeAnim].obj;
			color = WHITE_FONT;
			if ((obj >= ocCUOCO) && (obj <= ocCURPLAYER)) {
				switch (obj) {
				case ocNOTAIO:
				case ocSUPERVISORE:
				case ocCUOCO:
					color = RED_FONT;
					break;
				case ocVALENCIA:
				case ocCHIRURGO:
				case ocGIARDINIERE:
					color = GREEN_FONT;
					break;
				case ocKRENN:
				case ocOROLOGIAIO:
				case ocVICTORIA:
					color = CYAN_FONT;
					break;
				case ocMOORE:
				case ocMOOREBUCATO:
				case ocDUKES:
				case ocTRADUTTORE:
				case ocCUSTODE:
					color = MAGENTA_FONT;
					break;
				case ocCORONA:
				case ocMOGLIESUPERVISORE:
				case ocMOGLIE_KIMONO:
				case ocSERVETTA:
					color = YELLOW_FONT;
					break;
				case ocVECCHIO:
				case ocCACCIATORE:
				case ocCACCIATOREMALPRESO:
				case ocDOMESTICA:
					color = GRAY_FONT;
					break;
				case ocDARRELL:
				case ocDARRELLALETTO:
				case ocCURPLAYER:
				default:
					color = WHITE_FONT;
					break;
				}
			}
			game._renderer->_2dStuff.displayDDText(TextLines[i], FontKind::Standard, color, TheString.x + dx, TheString.y + i * 12, 0, 0, 0, 0);
		}
	}
}

/* -----------------06/04/98 10.34-------------------
 *                  PaintInventory
 * --------------------------------------------------*/
void PaintInventory(WGame &game) {
	int32 a, ci;
	Init &init = game.init;
	Renderer &renderer = *game._renderer;

	if ((InvStatus & INV_ON) || ((bT2DActive == tOPTIONS) && !bShowOnlyLoadWindow)) {
		if (bT2DActive != tOPTIONS) {
			DisplayD3DRect(renderer, 27, 77, 188, 490, 18, 25, 18, 128);
			DisplayD3DRect(renderer, 13, 124, 14, 49, 18, 25, 18, 128);
			DisplayD3DRect(renderer, 215, 472, 12, 50, 18, 25, 18, 128);
			renderer._2dStuff.displayDDBitmap(Console1, 3, 73, 0, 0, 0, 0);

			if (InvLen[CurPlayer] > MAX_SHOWN_ICONS) {
				if (InvBase[CurPlayer] > 0)
					renderer._2dStuff.displayDDBitmap(ConsoleFrecciaSu, 3 + 14, 73 + 66, 0, 0, 0, 0);

				if (InvBase[CurPlayer] < (InvLen[CurPlayer] - MAX_SHOWN_ICONS))
					renderer._2dStuff.displayDDBitmap(ConsoleFrecciaGiu, 3 + 206, 73 + 416, 0, 0, 0, 0);
			}
		}

		if ((InvStatus & INV_MODE1) && PlayerCanCall(game._gameVars)) {
			if (CurPlayer == VICTORIA)
				renderer._2dStuff.displayDDBitmap(Console5, 22, 13, 0, 0, 0, 0);
			else
				renderer._2dStuff.displayDDBitmap(Console6, 22, 13, 0, 0, 0, 0);
		}

		if ((InvStatus & INV_MODE2) || (bT2DActive == tOPTIONS)) {
			int ox;

			if (bT2DActive == tOPTIONS) {
				ox = 192;
				console_3_4_xoffs = -94;
			} else {
				ox = 0;
				console_3_4_xoffs = 0;
			}

			if (CurPlayer == DARRELL)
				renderer._2dStuff.displayDDBitmap(Console3, 22 + console_3_4_xoffs, 13, ox, 0, 0, 0);
			else
				renderer._2dStuff.displayDDBitmap(Console4, 22 + console_3_4_xoffs, 13, ox, 0, 0, 0);

			if (!PlayerCanSave())
				renderer._2dStuff.displayDDBitmap(ConsoleNoSave, 227 + 22 + console_3_4_xoffs, 13,  0, 0, 0, 0);

			if ((bT2DActive != tOPTIONS) && (!PlayerCanSwitch(game._gameVars, 0))) {
				if (CurPlayer == DARRELL)
					renderer._2dStuff.displayDDBitmap(ConsoleNoSwitchDar, 61 + 22 + console_3_4_xoffs, 13,  0, 0, 0, 0);
				else
					renderer._2dStuff.displayDDBitmap(ConsoleNoSwitchVic, 61 + 22 + console_3_4_xoffs, 13,  0, 0, 0, 0);
			}
		}

		if (bT2DActive != tOPTIONS) {
			for (a = 0; a < MAX_SHOWN_ICONS; a++) {
				if (ci = Inv[CurPlayer][InvBase[CurPlayer] + a]) {
					if (CurInvObj == ci)
						renderer._2dStuff.displayDDText(ObjName[init.InvObj[ci].name], FontKind::Standard, RED_FONT, INV_MARG_SX, INV_MARG_UP + ICON_DY * a, 0, 0, 0, 0);
					else
						renderer._2dStuff.displayDDText(ObjName[init.InvObj[ci].name], FontKind::Standard, WHITE_FONT, INV_MARG_SX, INV_MARG_UP + ICON_DY * a, 0, 0, 0, 0);
				}
			}
		}
	} else if ((bUseWith & UW_ON) && (bUseWith & UW_USEDI)) {
		DisplayD3DRect(renderer, game._gameRect._useIconRect.x1 + 3, game._gameRect._useIconRect.y1 + 3, 63, 63, 22, 31, 22, 75);
		renderer._2dStuff.displayDDBitmap(IconsPics[UseWith[USED]], game._gameRect._useIconRect.x1 + 3, game._gameRect._useIconRect.y1 + 3,  0, 0, 0, 0);
		renderer._2dStuff.displayDDBitmap(Console2, game._gameRect._useIconRect.x1, game._gameRect._useIconRect.y1,  0, 0, 0, 0);
	}
	PaintDialog(game);
}

} // End of namespace Watchmaker
