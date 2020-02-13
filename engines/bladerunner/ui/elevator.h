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

#ifndef BLADERUNNER_ELEVATOR_H
#define BLADERUNNER_ELEVATOR_H

#include "common/array.h"

namespace BladeRunner {

class BladeRunnerEngine;
class Shapes;
class VQAPlayer;
class UIImagePicker;

class Elevator {
	BladeRunnerEngine *_vm;
	bool               _isOpen;
	VQAPlayer         *_vqaPlayer;
	int                _buttonClicked;
	Shapes            *_shapes;
	UIImagePicker     *_imagePicker;
	int                _actorId;
	int                _sentenceId;
	uint32             _timeSpeakDescriptionStart;

public:
	Elevator(BladeRunnerEngine *vm);
	~Elevator();

	int  activate(int elevatorId);
	void open();
	bool isOpen() const;
	int  handleMouseUp(int x, int y);
	int  handleMouseDown(int x, int y);
	void tick();
	void buttonClick(int buttonId);
	void reset();
	void buttonFocus(int buttonId);
	void setupDescription(int actorId, int sentenceId);
	void resetDescription();
	void tickDescription();

private:
	static void mouseInCallback(int, void *);
	static void mouseOutCallback(int, void *);
	static void mouseDownCallback(int, void *);
	static void mouseUpCallback(int, void *);
};

} // End of namespace BladeRunner

#endif
