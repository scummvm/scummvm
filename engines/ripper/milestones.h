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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_MILESTONES_H
#define RIPPER_MILESTONES_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
class Serializer;
}

namespace Ripper {

class ResourceManager;

enum MilestoneFlag {
	kMilestoneCompletedPrologue = 1,
	kMilestoneCompletedAct1 = 2,
	kMilestoneCompletedAct2 = 3,
	kMilestoneCompletedAct3 = 4,
	kMilestoneFirstRipperIdentity = 6,
	kMilestoneLastRipperIdentity = 9,
	kMilestoneFirstTravelLocation = 20,
	kMilestoneSecretAnimalLabOpen = 29,
	kMilestoneMagnottaApartmentInteriorOpen = 31,
	kMilestoneWebRunnersLoftOpen = 41,
	kMilestoneLastTravelLocation = 44,
	kMilestoneFirstWacDatabaseEntry = 70,
	kMilestoneScannedMug = 71,
	kMilestoneCompletedMug = 72,
	kMilestoneWacAudioEditorAvailable = 84,
	kMilestonePlayedFinalPrologueScene = 300,
	kMilestonePlayedFirstWacMessage = 301,
	kMilestoneReceivedJordanWacMessage = 305,
	kMilestoneGotFarleyWacMessage = 307,
	kMilestonePlayedGambitEmail = 309,
	kMilestonePlayedThirdRipperWacMessage = 314,
	kMilestoneFalconettisWell = 406
};

class Milestones {
public:
	static const uint kFlagCount = 1000;
	static const uint kStorageByteCount = kFlagCount / 8;

	Milestones();

	bool initialize(ResourceManager &resources);
	bool isSet(uint flag) const;
	bool set(uint flag, bool value, const char *source);
	bool hasRipperIdentity() const;
	bool selectRipperIdentity(uint candidate, const char *source);
	bool syncGame(Common::Serializer &serializer);

	const Common::String &label(uint flag) const;
	static const char *domain(uint flag);

private:
	void debugState(const char *source) const;
	void setBit(uint flag, bool value);

	byte _flags[kStorageByteCount];
	Common::Array<Common::String> _labels;
};

} // End of namespace Ripper

#endif // RIPPER_MILESTONES_H
