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

#ifndef AGS_PLUGINS_AGSCREDITZ_AGSCREDITZ1_H
#define AGS_PLUGINS_AGSCREDITZ_AGSCREDITZ1_H

#include "ags/plugins/ags_creditz/ags_creditz.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

class AGSCreditz1 : public AGSCreditz {
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);
	static int64 AGS_EngineOnEvent(int event, NumberPtr data);

	static void SetCredit(ScriptMethodParams &params);
	static void ScrollCredits(ScriptMethodParams &params);
	static void GetCredit(ScriptMethodParams &params);
	static void IsCreditScrollingFinished(ScriptMethodParams &params);
	static void SetCreditImage(ScriptMethodParams &params);
	static void PauseScroll(ScriptMethodParams &params);
	static void ScrollReset(ScriptMethodParams &params);
	static void SetEmptyLineHeight(ScriptMethodParams &params);
	static void GetEmptyLineHeight(ScriptMethodParams &params);
	static void SetStaticCredit(ScriptMethodParams &params);
	static void GetStaticCredit(ScriptMethodParams &params);
	static void StartEndStaticCredits(ScriptMethodParams &params);
	static void GetCurrentStaticCredit(ScriptMethodParams &params);
	static void SetDefaultStaticDelay(ScriptMethodParams &params);
	static void SetStaticPause(ScriptMethodParams &params);
	static void SetStaticCreditTitle(ScriptMethodParams &params);
	static void ShowStaticCredit(ScriptMethodParams &params);
	static void StaticReset(ScriptMethodParams &params);
	static void GetStaticCreditTitle(ScriptMethodParams &params);
	static void SetStaticCreditImage(ScriptMethodParams &params);
	static void IsStaticCreditsFinished(ScriptMethodParams &params);

public:
	AGSCreditz1();
};

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3

#endif
