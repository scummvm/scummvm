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

/* The task loading works as follows:
 *   - the task has a constructor MyPrivateTask(Process &, Serializer &)
 *   - DECLARE_TASK implements a global function to call that constructor
 *     void constructTask_MyPrivateTask(Process &, Serializer
 *   - in Process::syncGame we first forward-declare that function
 *   - we go through every task to compare task name and call the factory
 */

#ifndef DEFINE_TASK
#define DEFINE_TASK(TaskName)
#endif

DEFINE_TASK(CamLerpPosTask)
DEFINE_TASK(CamLerpScaleTask)
DEFINE_TASK(CamLerpPosScaleTask)
DEFINE_TASK(CamLerpRotationTask)
DEFINE_TASK(CamShakeTask)
DEFINE_TASK(CamWaitToStopTask)
DEFINE_TASK(CamSetInactiveAttributeTask)
DEFINE_TASK(SayTextTask)
DEFINE_TASK(AnimateCharacterTask)
DEFINE_TASK(LerpLodBiasTask)
DEFINE_TASK(ArriveTask)
DEFINE_TASK(DialogMenuTask)
DEFINE_TASK(AnimateTask)
DEFINE_TASK(CenterBottomTextTask)
DEFINE_TASK(FadeTask)
DEFINE_TASK(DoorTask)
DEFINE_TASK(ScriptTimerTask)
DEFINE_TASK(ScriptTask)
DEFINE_TASK(PlaySoundTask)
DEFINE_TASK(WaitForMusicTask)
DEFINE_TASK(DelayTask)  

#undef DEFINE_TASK
