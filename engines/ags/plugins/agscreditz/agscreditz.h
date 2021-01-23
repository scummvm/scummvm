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

namespace AGS3 {
namespace Plugins {
namespace AgsCreditz {

class AgsCreditz : public DLL {
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);

	// Script methods
	static void SetCredit(const ScriptMethodParams &params);
	static void ScrollCredits(const ScriptMethodParams &params);
	static string GetCredit(const ScriptMethodParams &params);
	static int IsCreditScrollingFinished(const ScriptMethodParams &params);
	static void SetCreditImage(const ScriptMethodParams &params);
	static void PauseScroll(const ScriptMethodParams &params);
	static void ScrollReset(const ScriptMethodParams &params);
	static void SetEmptyLineHeight(const ScriptMethodParams &params);
	static int GetEmptyLineHeight(const ScriptMethodParams &params);
	static void SetStaticCredit(const ScriptMethodParams &params);
	static string GetStaticCredit(const ScriptMethodParams &params);
	static void StartEndStaticCredits(const ScriptMethodParams &params);
	static int GetCurrentStaticCredit(const ScriptMethodParams &params);
	static void SetDefaultStaticDelay(const ScriptMethodParams &params);
	static void SetStaticPause(const ScriptMethodParams &params);
	static void SetStaticCreditTitle(const ScriptMethodParams &params);
	static void ShowStaticCredit(const ScriptMethodParams &params);
	static void StaticReset(const ScriptMethodParams &params);
	static string GetStaticCreditTitle(const ScriptMethodParams &params);
	static void SetStaticCreditImage(const ScriptMethodParams &params);
	static int IsStaticCreditsFinished(const ScriptMethodParams &params);
public:
	AgsCreditz();
};

} // namespace AgsCreditz
} // namespace Plugins
} // namespace AGS3

#endif
