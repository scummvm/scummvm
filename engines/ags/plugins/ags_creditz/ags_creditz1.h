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

	static void SetCredit(const ScriptMethodParams &params);
	static void ScrollCredits(const ScriptMethodParams &params);
	static NumberPtr GetCredit(const ScriptMethodParams &params);
	static NumberPtr IsCreditScrollingFinished(const ScriptMethodParams &params);
	static void SetCreditImage(const ScriptMethodParams &params);
	static void PauseScroll(const ScriptMethodParams &params);
	static void ScrollReset(const ScriptMethodParams &params);
	static void SetEmptyLineHeight(const ScriptMethodParams &params);
	static NumberPtr GetEmptyLineHeight(const ScriptMethodParams &params);
	static void SetStaticCredit(const ScriptMethodParams &params);
	static NumberPtr GetStaticCredit(const ScriptMethodParams &params);
	static void StartEndStaticCredits(const ScriptMethodParams &params);
	static NumberPtr GetCurrentStaticCredit(const ScriptMethodParams &params);
	static void SetDefaultStaticDelay(const ScriptMethodParams &params);
	static void SetStaticPause(const ScriptMethodParams &params);
	static void SetStaticCreditTitle(const ScriptMethodParams &params);
	static void ShowStaticCredit(const ScriptMethodParams &params);
	static void StaticReset(const ScriptMethodParams &params);
	static NumberPtr GetStaticCreditTitle(const ScriptMethodParams &params);
	static void SetStaticCreditImage(const ScriptMethodParams &params);
	static NumberPtr IsStaticCreditsFinished(const ScriptMethodParams &params);

public:
	AGSCreditz1();
};

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3

#endif
