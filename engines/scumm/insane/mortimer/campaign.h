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

#ifndef SCUMM_INSANE_MORTIMER_CAMPAIGN_H
#define SCUMM_INSANE_MORTIMER_CAMPAIGN_H

#include "common/str.h"

namespace Scumm {

struct MortimerRegion {
	const char *directory;
	char prefix;
	const char *animals[7];
	const char *names[7];
	const char *flight;
	const char *subtitles;
};

const MortimerRegion &mortimerRegion(uint region);
uint mortimerGuessCost(uint region, uint difficulty);
int mortimerGuessAllowance(uint difficulty);

// Retail scene IDs and ordering from SHELL's campaign tables.
class MortimerCampaign {
public:
	enum Kind { kMovie, kFlight, kInterview, kCastle, kCastleRiddle, kCredits };
	struct Scene {
		int id;
		uint region;
		Kind kind;
		const char *movie;
		const char *subtitles;
		bool doubled;
	};

	MortimerCampaign();
	bool start(int scene = 24);
	bool advance();
	bool returnToFlight();
	const Scene &scene() const;
	Common::String movieName(bool girl) const;
	bool finished() const;

private:
	uint _index;
};

} // End of namespace Scumm

#endif
