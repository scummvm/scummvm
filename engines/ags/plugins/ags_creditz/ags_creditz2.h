/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGSCREDITZ_AGSCREDITZ2_H
#define AGS_PLUGINS_AGSCREDITZ_AGSCREDITZ2_H

#include "ags/plugins/ags_creditz/ags_creditz.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

class AGSCreditz2 : public AGSCreditz {
	SCRIPT_HASH_SUB(AGSCreditz2, AGSCreditz)
private:
	void RunCreditSequence(ScriptMethodParams &params);
	void SetCredit(ScriptMethodParams &params);
	void GetCredit(ScriptMethodParams &params);
	void CreditsSettings(ScriptMethodParams &params);
	void SequenceSettings(ScriptMethodParams &params);
	void IsSequenceFinished(ScriptMethodParams &params);
	void PauseScrolling(ScriptMethodParams &params);
	void SetCreditImage(ScriptMethodParams &params);
	void ResetSequence(ScriptMethodParams &params);

	void SetStaticCredit(ScriptMethodParams &params);
	void SetStaticCreditTitle(ScriptMethodParams &params);
	void SetStaticPause(ScriptMethodParams &params);
	void RunStaticCreditSequence(ScriptMethodParams &params);
	void IsStaticSequenceFinished(ScriptMethodParams &params);
	void ShowStaticCredit(ScriptMethodParams &params);
	void SetStaticImage(ScriptMethodParams &params);
	void GetCurrentStaticCredit(ScriptMethodParams &params);

public:
	AGSCreditz2();

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *engine) override;
	int64 AGS_EngineOnEvent(int event, NumberPtr data) override;
};

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3

#endif
