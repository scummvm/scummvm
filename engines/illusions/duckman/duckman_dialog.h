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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ILLUSIONS_DUCKMAN_DUCKMAN_DIALOG_H
#define ILLUSIONS_DUCKMAN_DUCKMAN_DIALOG_H

#include "illusions/illusions.h"
#include "common/algorithm.h"
#include "common/stack.h"

namespace Illusions {

struct DialogItem {
	int16 _choiceJumpOffs;
	uint32 _sequenceId;
};

class DuckmanDialogSystem {
public:
	DuckmanDialogSystem(IllusionsEngine_Duckman *vm);
	~DuckmanDialogSystem();
	void addDialogItem(int16 choiceJumpOffs, uint32 sequenceId);
	void startDialog(int16 *choiceOfsPtr, uint32 actorTypeId, uint32 callerThreadId);
	void updateDialogState();
public:
	IllusionsEngine_Duckman *_vm;
	Common::Array<DialogItem> _dialogItems;
};

} // End of namespace Illusions

#endif // ILLUSIONS_DUCKMAN_DUCKMAN_DIALOG_H
