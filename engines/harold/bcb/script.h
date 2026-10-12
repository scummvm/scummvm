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

#ifndef BCB_SCRIPT_H
#define BCB_SCRIPT_H

namespace BCB {

enum GameStateOffset {
	kStateDispatcherControlCode = 0x0000,
	kStateMovieToPlayId = 0x0004,
	kStateRequiredDiscIndex = 0x0008,
	kStateGameClockTicks = 0x000c,
	kStateClearDisplayBeforeMediaCheckFlag = 0x0014,
	kStateSceneChoiceUiSetToShowId = 0x001c,
	kStateCurrentScriptSequenceId = 0x0020,
	kStateSelectedSceneChoiceValue = 0x0024,
	kStateSceneChoiceExtraButtonFlag = 0x0028,
	kStateStillImageToShowId = 0x0030,
	kStateMovieHistory = 0x0040,
	kStateMovieHistoryCount = 0x0088,
	kStateComputerChoiceStateByIndex = 0x0098,
	kStateMainSceneNightFlag = 0x0160
};

} // End of namespace BCB

#endif // BCB_SCRIPT_H
