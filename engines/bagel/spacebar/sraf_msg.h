
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

#ifndef BAGEL_SPACEBAR_SRAF_MSG_H
#define BAGEL_SPACEBAR_SRAF_MSG_H

namespace Bagel {
namespace SpaceBar {

// Messages for "submit offer" button on the deal summary

#define kszOfferAccepted    "Your offer has been accepted!  You must now select a secure code name for this deal."
#define kszUnresolvedSeller "%s's terms have not yet been resolved.  You must chat with this seller first."
#define kszMiningConflict   "You have committed %s to %s and %s.  Please resolve this conflict."
#define kszOfferNotEnough   "Your offer is short by %dgZ.  Please raise additional funds."
#define kszGungNotHappy     "Gung-14 has backed out of this offer because you included %s in this offer."
#define kszHemNotHappy      "Hem-20 refuses to enter into this deal as long as Dippik-10 is one of the backers."
#define kszDorkNotHappy     "Dork-44 refuses to enter into this deal because the makeup of backers is not at least half female."
#define kszJellaNotHappy    "You have committed Crop Harvesting rights to %s but Jella-37's Lumber Harvesting bid implicitly includes Crop Harvesting.  Please resolve this conflict."
#define kszSinjinNotHappy   "Sinjin-11 has backed out of this offer because you included Dork-44 on this team.  He's still upset about the Zwix incident."
#define kszUnresolvedEnriro "The Environmentalists have not signed off on this deal yet.  You must resolve this situation first."
#define kszFinishCodeWords  "Please finish selecting your code words for this deal."

// Messages for "Dispatch negotiating Team" button in the dispatch section

#define kszNotEvenClose     "You have not assigned anyone to your team and you have not selected anyone for your team to meet with."
#define kszNoNegotiators    "You have not assigned anyone for your negotiation team to meet with."
#define kszNoTeamMembers    "You have not assigned anyone to your negotiation team."
#define ksz3MeetingTargets  "You have assigned your negotiating team to meet with %s, %s and %s, they can only meet with one at a time."
#define ksz2MeetingTargets  "You have assigned your negotiating team to meet with %s and %s, they can only meet with one at a time."

#define kszCantMeetQuosh    "But, boss!  Quosh-23 is with you!"
#define kszCantMeetIrk      "But, boss!  Irk-4 is with you!"

#define kszCantRenegSeller  "But, boss!  D7 Realty has already talked this seller down!"
#define kszCantRenegBuyer   "But, boss!  D7 Realty has already talked this buyer up!"
#define kszCantRenegOther   "But, boss!  D7 Realty has already resolved the terms for this party!"

} // namespace SpaceBar
} // namespace Bagel

#endif
