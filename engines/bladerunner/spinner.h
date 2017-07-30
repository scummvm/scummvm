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

#ifndef BLADERUNNER_SPINNER_H
#define BLADERUNNER_SPINNER_H

#include "common/array.h"

namespace BladeRunner {

class BladeRunnerEngine;
class Shape;
class VQAPlayer;
class UIImagePicker;

#define SPINNER_DESTINATIONS 10

struct SpinnerDestination {
	int id;
	int left;
	int top;
	int right;
	int bottom;
};

class Spinner {
	BladeRunnerEngine     *_vm;
	bool                   _isDestinationSelectable[SPINNER_DESTINATIONS];
	bool                   _isOpen;
	VQAPlayer             *_vqaPlayer;
	SpinnerDestination    *_destinations;
	int                    _selectedDestination;
	Common::Array<Shape*>  _shapes;
	UIImagePicker         *_imagePicker;

public:
	Spinner(BladeRunnerEngine *vm);
	~Spinner();

	void setSelectableDestinationFlag(int destination, bool selectable);
	bool querySelectableDestinationFlag(int destination) const;

	int interfaceChooseDest(int vqaLoopId, int loopFlag);

	void setIsOpen();
	bool isOpen() const;

	int handleMouseUp(int x, int y);
	int handleMouseDown(int x, int y);
	void tick();
	void setSelectedDestination(int destination);
	void reset();
	void resume();
};

} // End of namespace BladeRunner

#endif
