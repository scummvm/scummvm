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

#ifndef ULTIMA3_VIEWS_H
#define ULTIMA3_VIEWS_H

#include "ultima/shared/engine/events.h"
#include "ultima/ultima3/views/character_details.h"
#include "ultima/ultima3/views/create_character.h"
#include "ultima/ultima3/views/disperse_party.h"
#include "ultima/ultima3/views/form_party.h"
#include "ultima/ultima3/views/journey_onward.h"
#include "ultima/ultima3/views/location_map.h"
#include "ultima/ultima3/views/logo_screen.h"
#include "ultima/ultima3/views/main_menu.h"
#include "ultima/ultima3/views/party_menu.h"
#include "ultima/ultima3/views/register.h"
#include "ultima/ultima3/views/terminate_character.h"
#include "ultima/ultima3/views/title.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

struct Views {
	CharacterDetails _characterDetails;
	CreateCharacter _createCharacter;
	DisperseParty _disperseParty;
	FormParty _formParty;
	JourneyOnward _journeyOnward;
	LocationMap _locationMap;
	LogoScreen _logoScreen;
	MainMenu _mainMenu;
	PartyMenu _partyMenu;
	Register _register;
	TerminateCharacter _terminateCharacter;
	Title _title;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
