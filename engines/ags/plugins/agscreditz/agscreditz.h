/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_PLUGINS_AGSCREDITZ_AGSCREDITZ_H
#define AGS_PLUGINS_AGSCREDITZ_AGSCREDITZ_H

#include "ags/plugins/dll.h"

namespace AGS {
namespace Plugins {
namespace AgsCreditz {

class AgsCreditz : public DLL {
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);

	static void SetCredit(int ID, string credit, int colour, int font, int center, int xpos, int generateoutline);
	static void ScrollCredits(int onoff, int speed, int fromY, int toY, int isautom, int wait, int res);
	static string GetCredit(int ID);
	static int IsCreditScrollingFinished();
	static void SetCreditImage(int ID, int Slot, int center, int xpos, int pixtonext);
	static void PauseScroll(int onoff);
	static void ScrollReset();
	static void SetEmptyLineHeight(int Height);
	static int GetEmptyLineHeight();
	static void SetStaticCredit(int ID, int x, int y, int creditfont, int creditcolour, int centered, int generateoutline, string credit);
	static string GetStaticCredit(int ID);
	static void StartEndStaticCredits(int onoff, int res);
	static int GetCurrentStaticCredit();
	static void SetDefaultStaticDelay(int Cyclesperchar);
	static void SetStaticPause(int ID, int length);
	static void SetStaticCreditTitle(int ID, int x, int y, int titlefont, int titlecolour, int centered, int generateoutline, string title);
	static void ShowStaticCredit(int ID, int time, int style, int transtime, int sound, int resolution);
	static void StaticReset();
	static string GetStaticCreditTitle(int ID);
	static void SetStaticCreditImage(int ID, int x, int y, int Slot, int Hcentered, int Vcentered, int time);
	static int IsStaticCreditsFinished();
public:
	AgsCreditz();
};

} // namespace AgsCreditz
} // namespace Plugins
} // namespace AGS

#endif
