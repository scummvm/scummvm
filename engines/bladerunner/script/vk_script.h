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

#ifndef BLADERUNNER_SCRIPT_KIA_H
#define BLADERUNNER_SCRIPT_KIA_H

#include "bladerunner/script/script.h"

namespace BladeRunner {

class BladeRunnerEngine;

class VKScript : ScriptBase {
	friend class Debugger;

	int _inScriptCounter;
	int _calibrationQuestionCounter;
	int _questionCounter;

public:
	VKScript(BladeRunnerEngine *vm);

	void initialize(int actorId);
	void calibrate(int actorId);
	void beginTest(int actorId);
	void mcCoyAsksQuestion(int actorId, int questionId);
	void questionAsked(int actorId, int questionId);
	void shutdown(int actorId, int humanPercentage, int replicantPercentage, int anxiety);

	bool isInsideScript() const;

private:
	bool SCRIPT_VK_DLL_Initialize(int actorId);
	void SCRIPT_VK_DLL_Calibrate(int actorId);
	bool SCRIPT_VK_DLL_Begin_Test(int actorId);
	void SCRIPT_VK_DLL_McCoy_Asks_Question(int actorId, int questionId);
	void SCRIPT_VK_DLL_Question_Asked(int actorId, int questionId);
 	void SCRIPT_VK_DLL_Shutdown(int actorId, int humanPercentage, int replicantPercentage, int anxiety);

	void askLucy(int questionId);
	void askGrigorian(int questionId);
	void askDektora(int questionId);
	void askRunciter(int questionId);
	void askBulletBob(int questionId);
	void askCalibrationQuestion1(int actorId, int notUsed);
	void askCalibrationQuestion2(int actorId, int notUsed);
	void askCalibrationQuestion3(int actorId, int notUsed);
};

} // End of namespace BladeRunner

#endif
