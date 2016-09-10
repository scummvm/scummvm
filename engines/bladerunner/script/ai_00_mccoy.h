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

#include "bladerunner/script/script.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

class AIScript_McCoy : public AIScriptBase {
	int dword_45A0D0_animation_state;
	int dword_45A0D4;
	int dword_45A0D8;
	int dword_45A0DC;
	int dword_45A0E0;
	int dword_45A0E4;
	int dword_45A0E8;
public:
	AIScript_McCoy(BladeRunnerEngine *vm);

	void Initialize();
	void UpdateAnimation(int *animation, int *frame);
	void ChangeAnimationMode(int mode);
};

} // End of namespace BladeRunner
