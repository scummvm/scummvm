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

#ifndef TWINE_PARSER_HOLOMAP_H
#define TWINE_PARSER_HOLOMAP_H

#include "common/memstream.h"
#include "twine/parser/parser.h"

namespace TwinE {

enum HolomapVehicle {
	FerryBoat = 31,
	Motorbike = 33,
	Car = 35,
	FishingBoat = 37,
	Catamaran = 39,
	Hovercraft = 41,
	Dino = 43,
	ArmyBoat = 45,
	HamalayiTransporter = 47
};

struct TrajectoryPos {
	int16 x = 0;
	int16 y = 0;
};

struct Trajectory {
	int16 locationIdx = -1;
	int16 trajLocationIdx = -1;
	int16 vehicleIdx = -1;
	IVec3 angle;
	int16 numAnimFrames = 0;
	TrajectoryPos positions[512];

	bool isValid() const {
		return locationIdx != -1;
	}

	/**
	 * The HQR index of the vehicle model for the holomap
	 * @note Multiplied by 2 because the model index is always followed by the corresponding animation index for that model
	 */
	int32 getModel() const {
		return 2 * vehicleIdx + HolomapVehicle::FerryBoat;
	}

	int32 getAnimation() const {
		return getModel() + 1;
	}
};

class TrajectoryData : public Parser {
private:
	Common::Array<Trajectory> _trajectories;
protected:
	void reset() override;
public:
	bool loadFromStream(Common::SeekableReadStream &stream, bool lba1) override;

	const Trajectory *getTrajectory(uint index) const {
		if (index >= _trajectories.size()) {
			return nullptr;
		}
		return &_trajectories[index];
	}

	const Common::Array<Trajectory> &getTrajectories() const {
		return _trajectories;
	}
};

} // End of namespace TwinE

#endif
