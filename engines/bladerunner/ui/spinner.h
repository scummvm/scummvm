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
#include "common/rect.h"

namespace BladeRunner {

class BladeRunnerEngine;
class SaveFileReadStream;
class SaveFileWriteStream;
class Shapes;
class UIImagePicker;
class VQAPlayer;

class Spinner {
	static const int kSpinnerDestinations = 10;

	struct Destination {
		int          id;
		Common::Rect rect;
		int          shapeId;
		int          shapeIdOver;
	};

	BladeRunnerEngine *_vm;
	bool               _isDestinationSelectable[kSpinnerDestinations];
	bool               _isOpen;
	VQAPlayer         *_vqaPlayer;
	const Destination *_destinations;
	int                _selectedDestination;
	Shapes            *_shapes;
	UIImagePicker     *_imagePicker;

	int                _actorId;
	int                _sentenceId;
	uint32             _timeSpeakDescriptionStart;

public:
	Spinner(BladeRunnerEngine *vm);
	~Spinner();

	void setSelectableDestinationFlag(int destination, bool selectable);
	bool querySelectableDestinationFlag(int destination) const;

	int chooseDestination(int vqaLoopId, bool immediately);

	void open();
	bool isOpen() const;

	int handleMouseUp(int x, int y);
	int handleMouseDown(int x, int y);

	void tick();
	void setSelectedDestination(int destination);
	void reset();
	void resume();

	void destinationFocus(int destination);
	void setupDescription(int actorId, int sentenceId);
	void resetDescription();
	void tickDescription();

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);

private:
	static void mouseInCallback(int, void *);
	static void mouseOutCallback(int, void *);
	static void mouseDownCallback(int, void *);
	static void mouseUpCallback(int, void *);
	static const Destination *getDestinationsFar();
	static const Destination *getDestinationsMedium();
	static const Destination *getDestinationsNear();
};

} // End of namespace BladeRunner

#endif
