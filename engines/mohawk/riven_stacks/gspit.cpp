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

#include "mohawk/riven_stacks/gspit.h"

#include "mohawk/cursors.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_sound.h"
#include "mohawk/riven_video.h"

#include "common/events.h"

namespace Mohawk {
namespace RivenStacks {

GSpit::GSpit(MohawkEngine_Riven *vm) :
		DomeSpit(vm, kStackGspit, "gsliders.190", "gsliderbg.190") {

//	REGISTER_COMMAND(GSpit, xgresetpins);
//	REGISTER_COMMAND(GSpit, xgrotatepins);
//	REGISTER_COMMAND(GSpit, xgpincontrols);
//	REGISTER_COMMAND(GSpit, xgisland25_opencard);
//	REGISTER_COMMAND(GSpit, xgisland25_resetsliders);
//	REGISTER_COMMAND(GSpit, xgisland25_slidermd);
//	REGISTER_COMMAND(GSpit, xgisland25_slidermw);
//	REGISTER_COMMAND(GSpit, xgscpbtn);
//	REGISTER_COMMAND(GSpit, xgisland1490_domecheck);
//	REGISTER_COMMAND(GSpit, xgplateau3160_dopools);
//	REGISTER_COMMAND(GSpit, xgwt200_scribetime);
//	REGISTER_COMMAND(GSpit, xgwt900_scribe);
//	REGISTER_COMMAND(GSpit, xgplaywhark);
//	REGISTER_COMMAND(GSpit, xgrviewer);
//	REGISTER_COMMAND(GSpit, xgwharksnd);
//	REGISTER_COMMAND(GSpit, xglview_prisonoff);
//	REGISTER_COMMAND(GSpit, xglview_villageoff);
//	REGISTER_COMMAND(GSpit, xglviewer);
//	REGISTER_COMMAND(GSpit, xglview_prisonon);
//	REGISTER_COMMAND(GSpit, xglview_villageon);
}

void GSpit::lowerPins() {
	// Lower the pins

	uint32 &pinUp = _vm->_vars["gpinup"];

	if (pinUp == 0)
		return;

	uint32 &pinPos = _vm->_vars["gpinpos"];
	uint32 startTime = (pinPos - 1) * 600 + 4830;
	pinUp = 0;

	// Play the down sound
	_vm->_sound->playSound(13);

	uint32 &upMovie = _vm->_vars["gupmoov"];

	// Play the video of the pins going down
	RivenVideo *video = _vm->_video->openSlot(upMovie);
	assert(video);
	video->setBounds(startTime, startTime + 550);
	video->playBlocking();

	upMovie = 0;
}

void GSpit::xgresetpins(uint16 argc, uint16 *argv) {
	// As the function name suggests, this resets the pins
	lowerPins();
	_vm->_vars["gupmoov"] = 0;
}

void GSpit::xgrotatepins(uint16 argc, uint16 *argv) {
	// Rotate the pins, if necessary

	if (_vm->_vars["gpinup"] == 0)
		return;

	uint32 &pinPos = _vm->_vars["gpinpos"];
	uint32 startTime = (pinPos - 1) * 1200;

	if (pinPos == 4)
		pinPos = 1;
	else
		pinPos++;

	// Play the rotating sound
	_vm->_sound->playSound(12);

	// Play the video of the pins rotating
	RivenVideo *video = _vm->_video->openSlot(_vm->_vars["gupmoov"]);
	assert(video);
	video->setBounds(startTime, startTime + 1215);
	video->playBlocking();
}

void GSpit::xgpincontrols(uint16 argc, uint16 *argv) {
	// Handle a click on a section of an island

	RivenHotspot *panel = _vm->getCard()->getHotspotByBlstId(13);

	// Get our mouse position and adjust it to the beginning of the hotspot
	Common::Point mousePos = _vm->_system->getEventManager()->getMousePos();
	mousePos.x -= panel->getRect().left;
	mousePos.y -= panel->getRect().top;

	// And now adjust it to which box we hit
	mousePos.x /= 10;
	mousePos.y /= 11;

	// Lastly, adjust it based on the rotational position
	uint32 &pinPos = _vm->_vars["gpinpos"];
	switch (pinPos) {
		case 1:
			mousePos.x = 5 - mousePos.x;
			mousePos.y = (4 - mousePos.y) * 5;
			break;
		case 2:
			mousePos.x = (4 - mousePos.x) * 5;
			mousePos.y = 1 + mousePos.y;
			break;
		case 3:
			mousePos.x = 1 + mousePos.x;
			mousePos.y = mousePos.y * 5;
			break;
		case 4:
			mousePos.x = mousePos.x * 5;
			mousePos.y = 5 - mousePos.y;
			break;
		default:
			// (Should never happen)
			error("Bad pin pos");
	}

	// Now check to see if this section of the island exists
	uint32 islandIndex = _vm->_vars["glkbtns"] - 1;
	uint16 imagePos = mousePos.x + mousePos.y;

	static const uint16 islandImages[5][11] = {
			{ 1, 2, 6, 7 },
			{ 11, 16, 21, 22 },
			{ 12, 13, 14, 15, 17, 18, 19, 20, 23, 24, 25 },
			{ 5 },
			{ 3, 4, 8, 9, 10 }
	};

	// The scripts set gimagemax to hold the max pin array length in islandPins above
	uint32 imageCount = _vm->_vars["gimagemax"];
	uint32 image = 0;
	for (; image < imageCount; image++)
		if (islandImages[islandIndex][image] == imagePos)
			break;

	// If we past it, we don't have a valid map coordinate
	if (image == imageCount)
		return;

	uint32 &pinUp = _vm->_vars["gpinup"];
	uint32 &curImage = _vm->_vars["gimagecurr"];

	// Lower the pins if they are currently raised
	if (pinUp == 1) {
		lowerPins();

		// If we just lowered the selected section, don't raise it up again
		if (curImage == image)
			return;
	}

	// Raise the pins by translating the position to the movie code
	static const uint16 pinMovieCodes[] = { 1, 2, 1, 2, 1, 3, 4, 3, 4, 5, 1, 1, 2, 3, 4, 2, 5, 6, 7, 8, 3, 4, 9, 10, 11 };

	// Play the up sound
	_vm->_sound->playSound(14);

	// Actually play the movie
	RivenVideo *handle = _vm->_video->openSlot(pinMovieCodes[imagePos - 1]);
	assert(handle);
	uint32 startTime = 9630 - pinPos * 600;
	handle->setBounds(startTime, startTime + 550);
	handle->playBlocking();

	// Update the relevant variables
	_vm->_vars["gupmoov"] = pinMovieCodes[imagePos - 1];
	pinUp = 1;
	curImage = image;
}

void GSpit::xgisland25_opencard(uint16 argc, uint16 *argv) {
	checkDomeSliders();
}

void GSpit::xgisland25_resetsliders(uint16 argc, uint16 *argv) {
	resetDomeSliders(16, 11);
}

void GSpit::xgisland25_slidermd(uint16 argc, uint16 *argv) {
	dragDomeSlider(16, 11);
}

void GSpit::xgisland25_slidermw(uint16 argc, uint16 *argv) {
	checkSliderCursorChange(11);
}

void GSpit::xgscpbtn(uint16 argc, uint16 *argv) {
	runDomeButtonMovie();
}

void GSpit::xgisland1490_domecheck(uint16 argc, uint16 *argv) {
	runDomeCheck();
}

void GSpit::xgplateau3160_dopools(uint16 argc, uint16 *argv) {
	// Play the deactivation of a pool if one is active and a different one is activated
	_vm->_cursor->setCursor(kRivenHideCursor);
	_vm->_system->updateScreen();
	RivenVideo *video = _vm->_video->openSlot(_vm->_vars["glkbtns"] * 2);
	video->playBlocking();
}

void GSpit::xgwt200_scribetime(uint16 argc, uint16 *argv) {
	// Get the current time
	_vm->_vars["gscribetime"] = _vm->_system->getMillis();
}

void GSpit::xgwt900_scribe(uint16 argc, uint16 *argv) {
	uint32 &scribeVar = _vm->_vars["gscribe"];

	if (scribeVar == 1 && _vm->_system->getMillis() > _vm->_vars["gscribetime"] + 40000)
		scribeVar = 2;
}

static const uint16 s_viewerTimeIntervals[] = { 0, 816, 1617, 2416, 3216, 4016, 4816, 5616, 6416, 7216, 8016, 8816 };

void GSpit::xgrviewer(uint16 argc, uint16 *argv) {
	// This controls the viewer on the right side of the 'throne' on Garden Island
	// (It shows the colors of the marbles)

	// If the light is on, turn it off
	uint32 &viewerLight = _vm->_vars["grview"];
	if (viewerLight == 1) {
		viewerLight = 0;
		_vm->_sound->playSound(27);
		_vm->refreshCard();

		// Delay a bit before turning
		_vm->_system->delayMillis(200);
	}

	// Calculate how much we're moving
	Common::String buttonName = _vm->getCard()->getCurHotspot()->getName();
	uint32 buttonPos = buttonName.lastChar() - '0';

	uint32 &curPos = _vm->_vars["grviewpos"];
	uint32 newPos = curPos + buttonPos;

	// Now play the movie
	RivenVideo *video = _vm->_video->openSlot(1);
	assert(video);
	video->setBounds(s_viewerTimeIntervals[curPos], s_viewerTimeIntervals[newPos]);
	video->playBlocking();

	// Set the new position and let the card's scripts take over again
	curPos = newPos % 6; // Clip it to 0-5
	_vm->refreshCard();
}

void GSpit::xgplaywhark(uint16 argc, uint16 *argv) {
	// The whark response to using the lights

	// If we've gotten a visit already since we turned out the light, bail out
	uint32 &wharkState = _vm->_vars["gwharktime"];

	if (wharkState != 1)
		return;

	wharkState = 0;

	// Increase the amount of times the whark has visited
	uint32 &wharkVisits = _vm->_vars["gwhark"];
	wharkVisits++;

	// If we're at 5 or more, the whark will no longer visit us :(
	if (wharkVisits >= 5) {
		wharkVisits = 5;
		return;
	}

	// Activate the correct video based on the amount of times we've been visited
	switch (wharkVisits) {
		case 1:
			_vm->getCard()->playMovie(3);
			break;
		case 2:
			// One of two random videos
			_vm->getCard()->playMovie(4 + _vm->_rnd->getRandomBit());
			break;
		case 3:
			// One of two random videos
			_vm->getCard()->playMovie(6 + _vm->_rnd->getRandomBit());
			break;
		case 4:
			// Red alert! Shields online! Brace yourself for impact!
			_vm->getCard()->playMovie(8);
			break;
	}

	// For whatever reason the devs felt fit, code 31 is used for all of the videos
	RivenVideo *video = _vm->_video->openSlot(31);
	video->playBlocking();
	_vm->refreshCard();
}

void GSpit::xgwharksnd(uint16 argc, uint16 *argv) {
	// TODO: Random background whark videos
}

void GSpit::xglviewer(uint16 argc, uint16 *argv) {
	// This controls the viewer on the left side of the 'throne' on Garden Island
	// (It shows the village from the middle of the lake)

	// Calculate how much we're moving
	Common::String buttonName = _vm->getCard()->getCurHotspot()->getName();
	uint32 buttonPos = buttonName.lastChar() - '0';

	uint32 &curPos = _vm->_vars["glviewpos"];
	uint32 newPos = curPos + buttonPos;

	// Now play the movie
	RivenVideo *video = _vm->_video->openSlot(1);
	assert(video);
	video->setBounds(s_viewerTimeIntervals[curPos], s_viewerTimeIntervals[newPos]);
	video->playBlocking();

	// Set the new position to the variable
	curPos = newPos % 6; // Clip it to 0-5

	// And update the screen with the new image
	_vm->getCard()->drawPicture(curPos + 2);
}

void GSpit::xglview_villageon(uint16 argc, uint16 *argv) {
	// Turn on the left viewer to 'village mode'
	_vm->_vars["glview"] = 2;
	_vm->getCard()->drawPicture(_vm->_vars["glviewpos"] + 2);
}

void GSpit::xglview_villageoff(uint16 argc, uint16 *argv) {
	// Turn off the left viewer when in 'village mode' (why is this external?)
	_vm->_vars["glview"] = 0;
	_vm->getCard()->drawPicture(1);
}

void GSpit::catherineViewerIdleTimer() {
	uint32 &cathState = _vm->_vars["gcathstate"];
	uint16 movie;

	// Choose a new movie
	if (cathState == 1) {
		static const int movieList[] = { 9, 10, 19, 19, 21, 21 };
		movie = movieList[_vm->_rnd->getRandomNumber(5)];
	} else if (cathState == 2) {
		static const int movieList[] = { 18, 20, 22 };
		movie = movieList[_vm->_rnd->getRandomNumber(2)];
	} else {
		static const int movieList[] = { 11, 11, 12, 17, 17, 17, 17, 23 };
		movie = movieList[_vm->_rnd->getRandomNumber(7)];
	}

	// Update Catherine's state
	if (movie == 10 || movie == 17 || movie == 18 || movie == 20)
		cathState = 1;
	else if (movie == 19 || movie == 21 || movie == 23)
		cathState = 2;
	else
		cathState = 3;

	// Begin playing the new movie
	_vm->getCard()->playMovie(movie);
	RivenVideo *video = _vm->_video->openSlot(30);
	video->play();

	// Reset the timer
	_vm->installTimer(TIMER(GSpit, catherineViewerIdleTimer), video->getDuration() + _vm->_rnd->getRandomNumber(60) * 1000);
}

void GSpit::xglview_prisonon(uint16 argc, uint16 *argv) {
	// Activate random background Catherine videos

	// Turn on the left viewer to 'prison mode'
	_vm->_vars["glview"] = 1;

	// Get basic starting states
	uint16 cathMovie = _vm->_rnd->getRandomNumberRng(8, 23);
	uint16 turnOnMovie = 4;
	uint32 &cathState = _vm->_vars["gcathstate"];

	// Adjust the turn on movie
	if (cathMovie == 14)
		turnOnMovie = 6;
	else if (cathMovie == 15)
		turnOnMovie = 7;

	// Adjust Catherine's state
	if (cathMovie == 9 || cathMovie == 11 || cathMovie == 12 || cathMovie == 22)
		cathState = 3;
	else if (cathMovie == 19 || cathMovie == 21 || cathMovie == 23 || cathMovie == 14)
		cathState = 2;
	else
		cathState = 1;

	// Turn on the viewer
	_vm->_cursor->hideCursor();
	RivenVideo *turnOn = _vm->_video->openSlot(turnOnMovie);
	turnOn->playBlocking();
	_vm->_cursor->showCursor();

	uint32 timeUntilNextMovie;

	// Begin playing a movie immediately if Catherine is already in the viewer
	if (cathMovie == 8 || (cathMovie >= 13 && cathMovie <= 16)) {
		_vm->getCard()->playMovie(cathMovie);
		RivenVideo *video = _vm->_video->openSlot(30);
		video->play();

		timeUntilNextMovie = video->getDuration() + _vm->_rnd->getRandomNumber(60) * 1000;
	} else {
		// Otherwise, just redraw the imager
		timeUntilNextMovie = _vm->_rnd->getRandomNumberRng(10, 20) * 1000;
		_vm->getCard()->drawPicture(8);
	}

	// Create the timer for the next video
	_vm->installTimer(TIMER(GSpit, catherineViewerIdleTimer), timeUntilNextMovie);
}

void GSpit::xglview_prisonoff(uint16 argc, uint16 *argv) {
	// Deactivate random background Catherine videos

	// Update the viewer state (now off)
	_vm->_vars["glview"] = 0;

	// Remove the timer we set in xglview_prisonon()
	_vm->removeTimer();

	// Play the 'turn off' movie after stopping any videos still playing
	_vm->_video->closeVideos();
	_vm->_cursor->hideCursor();
	RivenVideo *video = _vm->_video->openSlot(5);
	video->playBlocking();
	_vm->_cursor->showCursor();

	// Redraw the viewer
	_vm->getCard()->drawPicture(1);
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
