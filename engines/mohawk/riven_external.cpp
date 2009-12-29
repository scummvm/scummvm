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
 * $URL$
 * $Id$
 *
 */
 
#include "mohawk/graphics.h"
#include "mohawk/riven.h"
#include "mohawk/riven_external.h"

#include "common/EventRecorder.h"
#include "gui/message.h"
 
namespace Mohawk {

RivenExternal::RivenExternal(MohawkEngine_Riven *vm) : _vm(vm) {
	setupCommands();
	_rnd = new Common::RandomSource();
	g_eventRec.registerRandomSource(*_rnd, "riven");
}

RivenExternal::~RivenExternal() {
	delete _rnd;
	
	for (uint32 i = 0; i < _externalCommands.size(); i++)
		delete _externalCommands[i];
	
	_externalCommands.clear();
}

void RivenExternal::setupCommands() {
	// aspit (Main Menu, Books, Setup) external commands
	COMMAND(xastartupbtnhide);
	COMMAND(xasetupcomplete);
	COMMAND(xaatrusopenbook);
	COMMAND(xaatrusbookback);
	COMMAND(xaatrusbookprevpage);
	COMMAND(xaatrusbooknextpage);
	COMMAND(xacathopenbook);
	COMMAND(xacathbookback);
	COMMAND(xacathbookprevpage);
	COMMAND(xacathbooknextpage);
	COMMAND(xtrapbookback);
	COMMAND(xatrapbookclose);
	COMMAND(xatrapbookopen);
	COMMAND(xarestoregame);
	COMMAND(xadisablemenureturn);
	COMMAND(xaenablemenureturn);
	COMMAND(xalaunchbrowser);

	// bspit (Bookmaking Island) external commands
	COMMAND(xblabopenbook);
	COMMAND(xblabbookprevpage);
	COMMAND(xblabbooknextpage);
	COMMAND(xsoundplug);
	COMMAND(xbchangeboiler);
	COMMAND(xbupdateboiler);
	COMMAND(xbsettrap);
	COMMAND(xbcheckcatch);
	COMMAND(xbait);
	COMMAND(xbfreeytram);
	COMMAND(xbaitplate);
	COMMAND(xbisland190_opencard);
	COMMAND(xbisland190_resetsliders);
	COMMAND(xbisland190_slidermd);
	COMMAND(xbisland190_slidermw);
	COMMAND(xbscpbtn);
	COMMAND(xbisland_domecheck);
	COMMAND(xvalvecontrol);
	COMMAND(xbchipper);

	// gspit (Garden Island) external commands
	COMMAND(xgresetpins);
	COMMAND(xgrotatepins);
	COMMAND(xgpincontrols);
	COMMAND(xgisland25_opencard);
	COMMAND(xgisland25_resetsliders);
	COMMAND(xgisland25_slidermd);
	COMMAND(xgisland25_slidermw);
	COMMAND(xgscpbtn);
	COMMAND(xgisland1490_domecheck);
	COMMAND(xgplateau3160_dopools);
	COMMAND(xgwt200_scribetime);
	COMMAND(xgwt900_scribe);
	COMMAND(xgplaywhark);
	COMMAND(xgrviewer);
	COMMAND(xgwharksnd);
	COMMAND(xglview_prisonoff);
	COMMAND(xglview_villageoff);
	COMMAND(xglviewer);
	COMMAND(xglview_prisonon);
	COMMAND(xglview_villageon);

	// jspit (Jungle Island) external commands
	COMMAND(xreseticons);
	COMMAND(xicon);
	COMMAND(xcheckicons);
	COMMAND(xtoggleicon);
	COMMAND(xjtunnel103_pictfix);
	COMMAND(xjtunnel104_pictfix);
	COMMAND(xjtunnel105_pictfix);
	COMMAND(xjtunnel106_pictfix);
	COMMAND(xvga1300_carriage);
	COMMAND(xjdome25_resetsliders);
	COMMAND(xjdome25_slidermd);
	COMMAND(xjdome25_slidermw);
	COMMAND(xjscpbtn);
	COMMAND(xjisland3500_domecheck);
	COMMAND(xhandlecontroldown);
	COMMAND(xhandlecontrolmid);
	COMMAND(xhandlecontrolup);
	COMMAND(xjplaybeetle_550);
	COMMAND(xjplaybeetle_600);
	COMMAND(xjplaybeetle_950);
	COMMAND(xjplaybeetle_1050);
	COMMAND(xjplaybeetle_1450);
	COMMAND(xjlagoon700_alert);
	COMMAND(xjlagoon800_alert);
	COMMAND(xjlagoon1500_alert);
	COMMAND(xschool280_playwhark);
	COMMAND(xjatboundary);

	// ospit (Gehn's Office) external commands
	COMMAND(xorollcredittime);
	COMMAND(xbookclick);
	COMMAND(xooffice30_closebook);
	COMMAND(xobedroom5_closedrawer);
	COMMAND(xogehnopenbook);
	COMMAND(xogehnbookprevpage);
	COMMAND(xogehnbooknextpage);
	COMMAND(xgwatch);
	
	// pspit (Prison Island) external commands
	COMMAND(xpisland990_elevcombo);
	COMMAND(xpscpbtn);
	COMMAND(xpisland290_domecheck);
	COMMAND(xpisland25_opencard);
	COMMAND(xpisland25_resetsliders);
	COMMAND(xpisland25_slidermd);
	COMMAND(xpisland25_slidermw);

	// rspit (Rebel Age) external commands
	COMMAND(xrshowinventory);
	COMMAND(xrhideinventory);
	COMMAND(xrcredittime);
	COMMAND(xrwindowsetup);

	// tspit (Temple Island) external commands
	COMMAND(xtexterior300_telescopedown);
	COMMAND(xtexterior300_telescopeup);
	COMMAND(xtisland390_covercombo);
	COMMAND(xtatrusgivesbooks);
	COMMAND(xtchotakesbook);
	COMMAND(xthideinventory);
	COMMAND(xt7500_checkmarbles);
	COMMAND(xt7600_setupmarbles);
	COMMAND(xt7800_setup);
	COMMAND(xdrawmarbles);
	COMMAND(xtakeit);
	COMMAND(xtscpbtn);
	COMMAND(xtisland4990_domecheck);
	COMMAND(xtisland5056_opencard);
	COMMAND(xtisland5056_resetsliders);
	COMMAND(xtisland5056_slidermd);
	COMMAND(xtisland5056_slidermw);
	COMMAND(xtatboundary);
	
	// Common external commands
	COMMAND(xflies);
}

void RivenExternal::runCommand(uint16 argc, uint16 *argv) {
	Common::String externalCommandName = _vm->getName(ExternalCommandNames, argv[0]);
		
	for (uint16 i = 0; i < _externalCommands.size(); i++)
		if (externalCommandName == _externalCommands[i]->desc) {
			debug(0, "Running Riven External Command \'%s\'", externalCommandName.c_str());
			(this->*(_externalCommands[i]->proc)) (argv[1], argv[1] ? argv + 2 : NULL);
			return;
		}
		
	error("Unknown external command \'%s\'", externalCommandName.c_str());
}

void RivenExternal::runDemoBoundaryDialog() {
	GUI::MessageDialog dialog("This demo does not allow you\n"
							  "to visit that part of Riven.");
	dialog.runModal();
}

void RivenExternal::runEndGame(uint16 video) {
	_vm->_sound->stopAllSLST();
	_vm->_video->playMovieBlocking(video);
	
	// TODO: Play until the last frame and then run the credits
	_vm->_gameOver = true;
}

// ------------------------------------------------------------------------------------
// aspit (Main Menu, Books, Setup) external commands
// ------------------------------------------------------------------------------------

void RivenExternal::xastartupbtnhide(uint16 argc, uint16 *argv) {
	// The original game hides the start/setup buttons depending on an ini entry. It's safe to ignore this command.
}

void RivenExternal::xasetupcomplete(uint16 argc, uint16 *argv) {
	// The original game sets an ini entry to disable the setup button and use the start button only. It's safe to ignore this part of the command.
	_vm->_sound->stopSound();
	_vm->changeToCard(1);
}

void RivenExternal::xaatrusopenbook(uint16 argc, uint16 *argv) {
	// Get the variable
	uint32 page = *_vm->matchVarToString("aatruspage");
	
	// Set hotspots depending on the page
	if (page == 1) {
		_vm->_hotspots[1].enabled = false;
		_vm->_hotspots[2].enabled = false;
		_vm->_hotspots[3].enabled = true;
	} else {
		_vm->_hotspots[1].enabled = true;
		_vm->_hotspots[2].enabled = true;
		_vm->_hotspots[3].enabled = false;
	}
	
	// Draw the image of the page
	_vm->_gfx->drawPLST(page);
}

void RivenExternal::xaatrusbookback(uint16 argc, uint16 *argv) {
	// Return to where we were before entering the book
	_vm->changeToStack(*_vm->matchVarToString("returnstackid"));
	_vm->changeToCard(*_vm->matchVarToString("returncardid"));
}

void RivenExternal::xaatrusbookprevpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 *page = _vm->matchVarToString("aatruspage");
	
	// Decrement the page if it's not the first page
	if (*page == 1)
		return;
	(*page)--;
	
	// TODO: Play the page turning sound
	
	// Now update the screen :)
	_vm->_gfx->updateScreen();
}

void RivenExternal::xaatrusbooknextpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 *page = _vm->matchVarToString("aatruspage");
	
	// Increment the page if it's not the last page
	if (((_vm->getFeatures() & GF_DEMO) && *page == 6) || *page == 10)
		return;
	(*page)++;
	
	// TODO: Play the page turning sound
	
	// Now update the screen :)
	_vm->_gfx->updateScreen();
}

void RivenExternal::xacathopenbook(uint16 argc, uint16 *argv) {
	// Get the variable
	uint32 page = *_vm->matchVarToString("acathpage");

	// Set hotspots depending on the page
	if (page == 1) {
		_vm->_hotspots[1].enabled = false;
		_vm->_hotspots[2].enabled = false;
		_vm->_hotspots[3].enabled = true;
	} else {
		_vm->_hotspots[1].enabled = true;
		_vm->_hotspots[2].enabled = true;
		_vm->_hotspots[3].enabled = false;
	}
	
	// Draw the image of the page
	_vm->_gfx->drawPLST(page);
	
	// Draw the white page edges
	if (page > 1 && page < 5)
		_vm->_gfx->drawPLST(50);
	else if (page > 5)
		_vm->_gfx->drawPLST(51);
	
	if (page == 28) {
		// TODO: Draw telescope combination
	}
}

void RivenExternal::xacathbookback(uint16 argc, uint16 *argv) {
	// Return to where we were before entering the book
	_vm->changeToStack(*_vm->matchVarToString("returnstackid"));
	_vm->changeToCard(*_vm->matchVarToString("returncardid"));
}

void RivenExternal::xacathbookprevpage(uint16 argc, uint16 *argv) {
	// Get the variable
	uint32 *page = _vm->matchVarToString("acathpage");
	
	// Increment the page if it's not the first page
	if (*page == 1)
		return;
	(*page)--;
	
	// TODO: Play the page turning sound

	// Now update the screen :)
	_vm->_gfx->updateScreen();
}

void RivenExternal::xacathbooknextpage(uint16 argc, uint16 *argv) {
	// Get the variable
	uint32 *page = _vm->matchVarToString("acathpage");
	
	// Increment the page if it's not the last page
	if (*page == 49)
		return;
	(*page)++;
	
	// TODO: Play the page turning sound
	
	// Now update the screen :)
	_vm->_gfx->updateScreen();
}

void RivenExternal::xtrapbookback(uint16 argc, uint16 *argv) {
	// Return to where we were before entering the book
	_vm->changeToStack(*_vm->matchVarToString("returnstackid"));
	_vm->changeToCard(*_vm->matchVarToString("returncardid"));
}

void RivenExternal::xatrapbookclose(uint16 argc, uint16 *argv) {
	// Close the trap book
	_vm->_hotspots[1].enabled = false;
	_vm->_hotspots[2].enabled = false;
	_vm->_hotspots[3].enabled = false;
	_vm->_hotspots[4].enabled = true;
	_vm->_gfx->drawPLST(3);
	_vm->_gfx->updateScreen();
}

void RivenExternal::xatrapbookopen(uint16 argc, uint16 *argv) {
	// Open the trap book
	_vm->_hotspots[1].enabled = true;
	_vm->_hotspots[2].enabled = true;
	_vm->_hotspots[3].enabled = true;
	_vm->_hotspots[4].enabled = false;
	_vm->_gfx->drawPLST(1);
	// TODO: Play movie
	_vm->_gfx->updateScreen();
}

void RivenExternal::xarestoregame(uint16 argc, uint16 *argv) {
	// Launch the load game dialog
	_vm->runLoadDialog();
}

void RivenExternal::xadisablemenureturn(uint16 argc, uint16 *argv) {
	// Dummy function -- implemented in Mohawk::go
}

void RivenExternal::xaenablemenureturn(uint16 argc, uint16 *argv) {
	// Dummy function -- implemented in Mohawk::go
}

void RivenExternal::xalaunchbrowser(uint16 argc, uint16 *argv) {
	// Well, we can't launch a browser for obvious reasons ;)
	GUI::MessageDialog dialog("At this point, the Riven Demo would\n"
							  "open up a web browser to bring you to\n"
							  "the Riven website. ScummVM cannot do\n"
							  "that. Visit the site on your own.");
	dialog.runModal();
}

// ------------------------------------------------------------------------------------
// bspit (Bookmaking Island) external commands
// ------------------------------------------------------------------------------------

void RivenExternal::xblabopenbook(uint16 argc, uint16 *argv) {
	// Get the variable
	uint32 page = *_vm->matchVarToString("blabbook");

	// Draw the image of the page based on the blabbook variable
	_vm->_gfx->drawPLST(page);
	
	// TODO: Draw the dome combo
	if (page == 14) {
		warning ("Need to draw dome combo");
	}
}

void RivenExternal::xblabbookprevpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 *page = _vm->matchVarToString("blabbook");
	
	// Decrement the page if it's not the first page
	if (*page == 1)
		return;
	(*page)--;
	
	// Now update the screen :)
	_vm->_gfx->updateScreen();
}

void RivenExternal::xblabbooknextpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 *page = _vm->matchVarToString("blabbook");
	
	// Increment the page if it's not the last page
	if (*page == 22)
		return;
	(*page)++;
	
	// Now update the screen :)
	_vm->_gfx->updateScreen();
}

void RivenExternal::xsoundplug(uint16 argc, uint16 *argv) {
	uint32 heat = *_vm->matchVarToString("bheat");
	uint32 boilerInactive = *_vm->matchVarToString("bcratergg");
	
	if (heat != 0)
		_vm->_sound->playSLST(1, _vm->getCurCard());
	else if (boilerInactive != 0)
		_vm->_sound->playSLST(2, _vm->getCurCard());
	else
		_vm->_sound->playSLST(3, _vm->getCurCard());
}

void RivenExternal::xbchangeboiler(uint16 argc, uint16 *argv) {
	uint32 heat = *_vm->matchVarToString("bheat");
	uint32 water = *_vm->matchVarToString("bblrwtr");
	uint32 platform = *_vm->matchVarToString("bblrgrt");

	if (argv[0] == 1) {
		if (water == 0) {
			if (platform == 0)
				_vm->_video->activateMLST(10, _vm->getCurCard());
			else
				_vm->_video->activateMLST(12, _vm->getCurCard());
		} else if (heat == 0) {
			if (platform == 0)
				_vm->_video->activateMLST(19, _vm->getCurCard());
			else
				_vm->_video->activateMLST(22, _vm->getCurCard());
		} else {
			if (platform == 0)
				_vm->_video->activateMLST(13, _vm->getCurCard());
			else
				_vm->_video->activateMLST(16, _vm->getCurCard());
		}
	} else if (argv[0] == 2 && water != 0) {
		if (heat == 0) {
			if (platform == 0)
				_vm->_video->activateMLST(20, _vm->getCurCard());
			else
				_vm->_video->activateMLST(23, _vm->getCurCard());
		} else {
			if (platform == 0)
				_vm->_video->activateMLST(18, _vm->getCurCard());
			else
				_vm->_video->activateMLST(15, _vm->getCurCard());
		}
	} else if (argv[0] == 3) {
		if (platform == 0) {
			if (water == 0) {
				_vm->_video->activateMLST(11, _vm->getCurCard());	
			} else {
				if (heat == 0)
					_vm->_video->activateMLST(17, _vm->getCurCard());
				else
					_vm->_video->activateMLST(24, _vm->getCurCard());
			}
		} else {
			if (water == 0) {
				_vm->_video->activateMLST(9, _vm->getCurCard());	
			} else {
				if (heat == 0)
					_vm->_video->activateMLST(14, _vm->getCurCard());
				else
					_vm->_video->activateMLST(21, _vm->getCurCard());
			}
		}
	}
	
	if (argc > 1)
		_vm->_sound->playSLST(argv[1], _vm->getCurCard());
	else if (argv[0] == 2)
		_vm->_sound->playSLST(1, _vm->getCurCard());
	
	_vm->_video->playMovie(11);
}

void RivenExternal::xbupdateboiler(uint16 argc, uint16 *argv) {
	uint32 heat = *_vm->matchVarToString("bheat");
	uint32 platform = *_vm->matchVarToString("bblrgrt");
	
	if (heat) {
		if (platform == 0) {
			_vm->_video->activateMLST(7, _vm->getCurCard());
			// TODO: Play video (non-blocking)
		} else {
			_vm->_video->activateMLST(8, _vm->getCurCard());
			// TODO: Play video (non-blocking)
		}
	} else {
		// TODO: Stop MLST's 7 and 8
	}
	
	_vm->changeToCard();
}

void RivenExternal::xbsettrap(uint16 argc, uint16 *argv) {
	// TODO: Set the Ytram trap
}

void RivenExternal::xbcheckcatch(uint16 argc, uint16 *argv) {
	// TODO: Check if we've caught a Ytram
}

void RivenExternal::xbait(uint16 argc, uint16 *argv) {	
	// Set the cursor to the pellet
	_vm->_gfx->changeCursor(kRivenPelletCursor);
	
	// Loop until the player lets go (or quits)
	Common::Event event;
	bool mouseDown = true;
	while (mouseDown) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONUP)
				mouseDown = false;
			else if (event.type == Common::EVENT_MOUSEMOVE)
				_vm->_system->updateScreen();
			else if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RTL)
				return;
		}
		
		_vm->_system->delayMillis(10); // Take it easy on the CPU
	}
	
	// Set back the cursor
	_vm->_gfx->changeCursor(kRivenMainCursor);
	
	// Set the bait if we put it on the plate
	if (_vm->_hotspots[9].rect.contains(_vm->_system->getEventManager()->getMousePos())) {
		*_vm->matchVarToString("bbait") = 1;
		_vm->_gfx->drawPLST(4);
		_vm->_gfx->updateScreen();
		_vm->_hotspots[3].enabled = false; // Disable bait hotspot
		_vm->_hotspots[9].enabled = true; // Enable baitplate hotspot
	}
}

void RivenExternal::xbfreeytram(uint16 argc, uint16 *argv) {
	// TODO: Play a random Ytram movie
}

void RivenExternal::xbaitplate(uint16 argc, uint16 *argv) {
	// Remove the pellet from the plate and put it in your hand
	_vm->_gfx->drawPLST(3);
	_vm->_gfx->updateScreen();
	_vm->_gfx->changeCursor(kRivenPelletCursor);
	
	// Loop until the player lets go (or quits)
	Common::Event event;
	bool mouseDown = true;
	while (mouseDown) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONUP)
				mouseDown = false;
			else if (event.type == Common::EVENT_MOUSEMOVE)
				_vm->_system->updateScreen();
			else if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RTL)
				return;
		}
		
		_vm->_system->delayMillis(10); // Take it easy on the CPU
	}
	
	// Set back the cursor
	_vm->_gfx->changeCursor(kRivenMainCursor);
	
	// Set the bait if we put it on the plate, remove otherwise
	if (_vm->_hotspots[9].rect.contains(_vm->_system->getEventManager()->getMousePos())) {
		*_vm->matchVarToString("bbait") = 1;
		_vm->_gfx->drawPLST(4);
		_vm->_gfx->updateScreen();
		_vm->_hotspots[3].enabled = false; // Disable bait hotspot
		_vm->_hotspots[9].enabled = true; // Enable baitplate hotspot
	} else {
		*_vm->matchVarToString("bbait") = 0;
		_vm->_hotspots[3].enabled = true; // Enable bait hotspot
		_vm->_hotspots[9].enabled = false; // Disable baitplate hotspot
	}
}

void RivenExternal::xbisland190_opencard(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xbisland190_resetsliders(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xbisland190_slidermd(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xbisland190_slidermw(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xbscpbtn(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xbisland_domecheck(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xvalvecontrol(uint16 argc, uint16 *argv) {
	// Get the variable for the valve
	uint32 *valve = _vm->matchVarToString("bvalve");
	
	Common::Event event;
	int changeX = 0;
	int changeY = 0;
	
	// Set the cursor to the closed position
	_vm->_gfx->changeCursor(2004);
	_vm->_system->updateScreen();
	
	for (;;) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					changeX = event.mouse.x - _vm->_mousePos.x;
					changeY = _vm->_mousePos.y - event.mouse.y;
					_vm->_system->updateScreen();
					break;
				case Common::EVENT_LBUTTONUP:
					// FIXME: These values for changes in x/y could be tweaked.
					if (*valve == 0 && changeY <= -10) {
						*valve = 1;
						// TODO: Play movie
						_vm->changeToCard(); // Refresh
					} else if (*valve == 1) {
						if (changeX >= 0 && changeY >= 10) {
							*valve = 0;
							// TODO: Play movie
							_vm->changeToCard(); // Refresh
						} else if (changeX <= -10 && changeY <= 10) {
							*valve = 2;
							// TODO: Play movie
							_vm->changeToCard(); // Refresh
						}
					} else if (*valve == 2 && changeX >= 10) {
						*valve = 1;
						// TODO: Play movie
						_vm->changeToCard(); // Refresh
					}
					return;
				default:
					break;
			}
		}
		_vm->_system->delayMillis(10);
	}
}

void RivenExternal::xbchipper(uint16 argc, uint16 *argv) {
	// Why is this an external command....?
	if (*_vm->matchVarToString("bvalve") == 2)
		_vm->_video->playMovieBlocking(2);
}

// ------------------------------------------------------------------------------------
// gspit (Garden Island) external commands
// ------------------------------------------------------------------------------------

void RivenExternal::xgresetpins(uint16 argc, uint16 *argv) {
	// TODO: Map related
}

void RivenExternal::xgrotatepins(uint16 argc, uint16 *argv) {
	// TODO: Map related
}

void RivenExternal::xgpincontrols(uint16 argc, uint16 *argv) {
	// TODO: Map related
}

void RivenExternal::xgisland25_opencard(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xgisland25_resetsliders(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xgisland25_slidermd(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xgisland25_slidermw(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xgscpbtn(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xgisland1490_domecheck(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xgplateau3160_dopools(uint16 argc, uint16 *argv) {
	// TODO: "Bubble" map related
}

void RivenExternal::xgwt200_scribetime(uint16 argc, uint16 *argv) {
	// Get the current time
	*_vm->matchVarToString("gscribetime") = _vm->_system->getMillis();
}

void RivenExternal::xgwt900_scribe(uint16 argc, uint16 *argv) {
	uint32 *scribeVar = _vm->matchVarToString("gscribe");

	if (*scribeVar == 1 && _vm->_system->getMillis() > *_vm->matchVarToString("gscribetime") + 40000)
		*scribeVar = 2;
}

void RivenExternal::xgplaywhark(uint16 argc, uint16 *argv) {
	// TODO: Whark response to using the lights
}

void RivenExternal::xgrviewer(uint16 argc, uint16 *argv) {
	// TODO: Image viewer related
}

void RivenExternal::xgwharksnd(uint16 argc, uint16 *argv) {
	// TODO: Image viewer related
}

void RivenExternal::xglview_prisonoff(uint16 argc, uint16 *argv) {
	// TODO: Image viewer related
}

void RivenExternal::xglview_villageoff(uint16 argc, uint16 *argv) {
	// TODO: Image viewer related
}

void RivenExternal::xglviewer(uint16 argc, uint16 *argv) {
	// TODO: Image viewer related
}

void RivenExternal::xglview_prisonon(uint16 argc, uint16 *argv) {
	// TODO: Image viewer related
}

void RivenExternal::xglview_villageon(uint16 argc, uint16 *argv) {
	// TODO: Image viewer related
}

// ------------------------------------------------------------------------------------
// jspit (Jungle Island) external commands
// ------------------------------------------------------------------------------------

void RivenExternal::xreseticons(uint16 argc, uint16 *argv) {
	// Reset the icons when going to Tay (rspit)
	*_vm->matchVarToString("jicons") = 0;
	*_vm->matchVarToString("jiconorder") = 0;
	*_vm->matchVarToString("jrbook") = 0;
}

// Count up how many icons are pressed
static byte countDepressedIcons(uint32 iconOrderVar) {
	if (iconOrderVar >= (1 << 20))
		return 5;
	else if (iconOrderVar >= (1 << 15))
		return 4;
	else if (iconOrderVar >= (1 << 10))
		return 3;
	else if (iconOrderVar >= (1 << 5))
		return 2;
	else if (iconOrderVar >= (1 << 1))
		return 1;
	else
		return 0; 
}

void RivenExternal::xicon(uint16 argc, uint16 *argv) {
	// Set atemp as the status of whether or not the icon can be depressed.
	if (*_vm->matchVarToString("jicons") & (1 << (argv[0] - 1))) {
		// This icon is depressed. Allow depression only if the last depressed icon was this one.
		if ((*_vm->matchVarToString("jiconorder") & 0x1f) == argv[0])
			*_vm->matchVarToString("atemp") = 1;
		else
			*_vm->matchVarToString("atemp") = 2;
	} else
		*_vm->matchVarToString("atemp") = 0;
}

void RivenExternal::xcheckicons(uint16 argc, uint16 *argv) {
	// Reset the icons if this is the sixth icon
	uint32 *iconOrderVar = _vm->matchVarToString("jiconorder");
	if (countDepressedIcons(*iconOrderVar) == 5) {
		*iconOrderVar = 0;
		*_vm->matchVarToString("jicons") = 0;
		_vm->_sound->playSound(46, false);
	}
}

void RivenExternal::xtoggleicon(uint16 argc, uint16 *argv) {
	// Get the variables
	uint32 *iconsDepressed = _vm->matchVarToString("jicons");
	uint32 *iconOrderVar = _vm->matchVarToString("jiconorder");
	
	if (*iconsDepressed & (1 << (argv[0] - 1))) {
		// The icon is depressed, now unpress it
		*iconsDepressed &= ~(1 << (argv[0] - 1));
		*iconOrderVar >>= 5;
	} else {
		// The icon is not depressed, now depress it
		*iconsDepressed |= 1 << (argv[0] - 1);
		*iconOrderVar = (*iconOrderVar << 5) + argv[0];
	}
	
	// Check if the puzzle is complete now and assign 1 to jrbook if the puzzle is complete.
	if (*iconOrderVar == *_vm->matchVarToString("jiconcorrectorder"))
		*_vm->matchVarToString("jrbook") = 1;
}

void RivenExternal::xjtunnel103_pictfix(uint16 argc, uint16 *argv) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = *_vm->matchVarToString("jicons");
	
	// Now, draw which icons are depressed based on the bits of the variable
	if (iconsDepressed & (1 << 0))
		_vm->_gfx->drawPLST(2);
	if (iconsDepressed & (1 << 1))
		_vm->_gfx->drawPLST(3);
	if (iconsDepressed & (1 << 2))
		_vm->_gfx->drawPLST(4);
	if (iconsDepressed & (1 << 3))
		_vm->_gfx->drawPLST(5);
	if (iconsDepressed & (1 << 22))
		_vm->_gfx->drawPLST(6);
	if (iconsDepressed & (1 << 23))
		_vm->_gfx->drawPLST(7);
	if (iconsDepressed & (1 << 24))
		_vm->_gfx->drawPLST(8);
}

void RivenExternal::xjtunnel104_pictfix(uint16 argc, uint16 *argv) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = *_vm->matchVarToString("jicons");
	
	// Now, draw which icons are depressed based on the bits of the variable
	if (iconsDepressed & (1 << 9))
		_vm->_gfx->drawPLST(2);
	if (iconsDepressed & (1 << 10))
		_vm->_gfx->drawPLST(3);
	if (iconsDepressed & (1 << 11))
		_vm->_gfx->drawPLST(4);
	if (iconsDepressed & (1 << 12))
		_vm->_gfx->drawPLST(5);
	if (iconsDepressed & (1 << 13))
		_vm->_gfx->drawPLST(6);
	if (iconsDepressed & (1 << 14))
		_vm->_gfx->drawPLST(7);
	if (iconsDepressed & (1 << 15))
		_vm->_gfx->drawPLST(8);
	if (iconsDepressed & (1 << 16))
		_vm->_gfx->drawPLST(9);
}

void RivenExternal::xjtunnel105_pictfix(uint16 argc, uint16 *argv) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = *_vm->matchVarToString("jicons");
	
	// Now, draw which icons are depressed based on the bits of the variable
	if (iconsDepressed & (1 << 3))
		_vm->_gfx->drawPLST(2);
	if (iconsDepressed & (1 << 4))
		_vm->_gfx->drawPLST(3);
	if (iconsDepressed & (1 << 5))
		_vm->_gfx->drawPLST(4);
	if (iconsDepressed & (1 << 6))
		_vm->_gfx->drawPLST(5);
	if (iconsDepressed & (1 << 7))
		_vm->_gfx->drawPLST(6);
	if (iconsDepressed & (1 << 8))
		_vm->_gfx->drawPLST(7);
	if (iconsDepressed & (1 << 9))
		_vm->_gfx->drawPLST(8);
}

void RivenExternal::xjtunnel106_pictfix(uint16 argc, uint16 *argv) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = *_vm->matchVarToString("jicons");
	
	// Now, draw which icons are depressed based on the bits of the variable
	if (iconsDepressed & (1 << 16))
		_vm->_gfx->drawPLST(2);
	if (iconsDepressed & (1 << 17))
		_vm->_gfx->drawPLST(3);
	if (iconsDepressed & (1 << 18))
		_vm->_gfx->drawPLST(4);
	if (iconsDepressed & (1 << 19))
		_vm->_gfx->drawPLST(5);
	if (iconsDepressed & (1 << 20))
		_vm->_gfx->drawPLST(6);
	if (iconsDepressed & (1 << 21))
		_vm->_gfx->drawPLST(7);
	if (iconsDepressed & (1 << 22))
		_vm->_gfx->drawPLST(8);
	if (iconsDepressed & (1 << 23))
		_vm->_gfx->drawPLST(9);
}

void RivenExternal::xvga1300_carriage(uint16 argc, uint16 *argv) {
	// TODO: This function is supposed to do a lot more, something like this (pseudocode):
	
	// Show level pull movie
	// Set transition up
	// Change to up card
	// Show movie of carriage coming down
	// Set transition down
	// Change back to card 276
	// Show movie of carriage coming down
	// if jgallows == 0
	//	Set up timer
	// 	Enter new input loop
	//	if you click within the time
	//		move forward
	//		set transition right
	//		change to card right
	//		show movie of ascending
	//		change to card 263
	//	else
	//		show movie of carriage ascending only
	// else
	//	show movie of carriage ascending only
	
	
	// For now, if the gallows base is closed, assume ascension and move to that card.
	if (*_vm->matchVarToString("jgallows") == 0)
		_vm->changeToCard(_vm->matchRMAPToCard(0x17167));
}

void RivenExternal::xjdome25_resetsliders(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xjdome25_slidermd(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xjdome25_slidermw(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xjscpbtn(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xjisland3500_domecheck(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

int RivenExternal::jspitElevatorLoop() {
	Common::Event event;
	int changeLevel = 0;
	
	_vm->_gfx->changeCursor(2004);
	_vm->_system->updateScreen();
	for (;;) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					if (event.mouse.y > (_vm->_mousePos.y + 10)) {
						changeLevel = -1;
					} else if (event.mouse.y < (_vm->_mousePos.y - 10)) {
						changeLevel = 1;
					} else {
						changeLevel = 0;
					}
					_vm->_system->updateScreen();
					break;
				case Common::EVENT_LBUTTONUP:
					_vm->_gfx->changeCursor(kRivenMainCursor);
					_vm->_system->updateScreen();
					return changeLevel;
				default:
					break;
			}
		}
		_vm->_system->delayMillis(10);
	}	
}

void RivenExternal::xhandlecontrolup(uint16 argc, uint16 *argv) {	
	int changeLevel = jspitElevatorLoop();
	
	if (changeLevel == -1) {
		// TODO: Run movie
		_vm->changeToCard(_vm->matchRMAPToCard(0x1e374));
	}
}

void RivenExternal::xhandlecontroldown(uint16 argc, uint16 *argv) {	
	int changeLevel = jspitElevatorLoop();
	
	if (changeLevel == 1) {
		// TODO: Run movie
		_vm->changeToCard(_vm->matchRMAPToCard(0x1e374));
	}
}

void RivenExternal::xhandlecontrolmid(uint16 argc, uint16 *argv) {
	int changeLevel = jspitElevatorLoop();
	
	if (changeLevel == 1) {
		// TODO: Run movie
		_vm->changeToCard(_vm->matchRMAPToCard(0x1e597));
	} else if (changeLevel == -1) {
		// TODO: Run movie
		_vm->changeToCard(_vm->matchRMAPToCard(0x1e29c));
	}
}

void RivenExternal::xjplaybeetle_550(uint16 argc, uint16 *argv) {
	// Play a beetle animation 25% of the time
	*_vm->matchVarToString("jplaybeetle") = (_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void RivenExternal::xjplaybeetle_600(uint16 argc, uint16 *argv) {
	// Play a beetle animation 25% of the time
	*_vm->matchVarToString("jplaybeetle") = (_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void RivenExternal::xjplaybeetle_950(uint16 argc, uint16 *argv) {
	// Play a beetle animation 25% of the time
	*_vm->matchVarToString("jplaybeetle") = (_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void RivenExternal::xjplaybeetle_1050(uint16 argc, uint16 *argv) {
	// Play a beetle animation 25% of the time
	*_vm->matchVarToString("jplaybeetle") = (_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void RivenExternal::xjplaybeetle_1450(uint16 argc, uint16 *argv) {
	// Play a beetle animation 25% of the time as long as the girl is not present
	*_vm->matchVarToString("jplaybeetle") = (_rnd->getRandomNumberRng(0, 3) == 0 && *_vm->matchVarToString("jgirl") != 1) ? 1 : 0;
}

void RivenExternal::xjlagoon700_alert(uint16 argc, uint16 *argv) {
	// TODO: Sunner related
}

void RivenExternal::xjlagoon800_alert(uint16 argc, uint16 *argv) {
	// TODO: Sunner related
}

void RivenExternal::xjlagoon1500_alert(uint16 argc, uint16 *argv) {
	// Have the sunners move a bit as you get closer ;)
	uint32 *sunners = _vm->matchVarToString("jsunners");
	if (*sunners == 0) {
		_vm->_video->playMovieBlocking(3);
	} else if (*sunners == 1) {
		_vm->_video->playMovieBlocking(2);
		*sunners = 2;
	}
}

void RivenExternal::xschool280_playwhark(uint16 argc, uint16 *argv) {
	// TODO: The "monstrous" whark puzzle that teaches the number system
}

void RivenExternal::xjatboundary(uint16 argc, uint16 *argv) {
	runDemoBoundaryDialog();
}

// ------------------------------------------------------------------------------------
// ospit (Gehn's Office) external commands
// ------------------------------------------------------------------------------------

void RivenExternal::xorollcredittime(uint16 argc, uint16 *argv) {
	// WORKAROUND: The special change stuff only handles one destination and it would
	// be messy to modify the way that currently works. If we use the trap book on Tay,
	// we should be using the Tay end game sequences.
	if (*_vm->matchVarToString("returnstackid") == rspit) {
		_vm->changeToStack(rspit);
		_vm->changeToCard(2);
		return;
	}

	// You used the trap book... why? What were you thinking?
	uint32 *gehnState = _vm->matchVarToString("agehn");
	
	if (*gehnState == 0)		// Gehn who?
		runEndGame(1);
	else if (*gehnState == 4)	// You freed him? Are you kidding me?
		runEndGame(2);
	else						// You already spoke with Gehn. What were you thinking?
		runEndGame(3);
}

void RivenExternal::xbookclick(uint16 argc, uint16 *argv) {
	// TODO: This fun external command is probably one of the most complex,
	// up there with the marble puzzle ones. It involves so much... Basically,
	// it's playing when Gehn holds the trap book up to you and you have to
	// click on the book (hence the name of the function). Yeah, not fun.
	// Lots of timing stuff needs to be done for a couple videos.
}

void RivenExternal::xooffice30_closebook(uint16 argc, uint16 *argv) {
	// Close the blank linking book if it's open
	uint32 *book = _vm->matchVarToString("odeskbook");
	if (*book != 1)
		return;

	// Set the variable to be "closed"
	*book = 0;

	// Play the movie
	_vm->_video->playMovieBlocking(1);

	// Set the hotspots into their correct states
	_vm->_hotspots[2].enabled = false;
	_vm->_hotspots[5].enabled = false;
	_vm->_hotspots[6].enabled = true;

	// We now need to draw PLST 1 and refresh, but PLST 1 is
	// drawn when refreshing anyway, so don't worry about that.
	_vm->changeToCard();
}

void RivenExternal::xobedroom5_closedrawer(uint16 argc, uint16 *argv) {
	// Close the drawer if open when clicking on the journal.
	_vm->_video->playMovieBlocking(2);
	*_vm->matchVarToString("ostanddrawer") = 0;
}

void RivenExternal::xogehnopenbook(uint16 argc, uint16 *argv) {
	_vm->_gfx->drawPLST(*_vm->matchVarToString("ogehnpage"));
}

void RivenExternal::xogehnbookprevpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 *page = _vm->matchVarToString("ogehnpage");
	
	// Decrement the page if it's not the first page
	if (*page == 1)
		return;
	(*page)--;
	
	// TODO: Play the page turning sound
	
	// Now update the screen :)
	_vm->_gfx->updateScreen();
}

void RivenExternal::xogehnbooknextpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 *page = _vm->matchVarToString("ogehnpage");
	
	// Increment the page if it's not the last page
	if (*page == 13)
		return;
	(*page)++;
	
	// TODO: Play the page turning sound
	
	// Now update the screen :)
	_vm->_gfx->updateScreen();
}

void RivenExternal::xgwatch(uint16 argc, uint16 *argv) {
	// TODO: Plays the prison combo on the watch
}

// ------------------------------------------------------------------------------------
// pspit (Prison Island) external commands
// ------------------------------------------------------------------------------------

// Yeah, none of these are done yet :P

void RivenExternal::xpisland990_elevcombo(uint16 argc, uint16 *argv) {
	// TODO: Elevator combo check
}

void RivenExternal::xpscpbtn(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xpisland290_domecheck(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xpisland25_opencard(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xpisland25_resetsliders(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xpisland25_slidermd(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xpisland25_slidermw(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

// ------------------------------------------------------------------------------------
// rspit (Rebel Age) external commands
// ------------------------------------------------------------------------------------

void RivenExternal::xrcredittime(uint16 argc, uint16 *argv) {
	// Nice going, you used the trap book on Tay.
	
	// The game chooses what ending based on agehn for us,
	// so we just have to play the video and credits.
	// For the record, when agehn == 4, Gehn will thank you for
	// showing him the rebel age and then leave you to die.
	// Otherwise, the rebels burn the book. Epic fail either way.
	runEndGame(1);
}

void RivenExternal::xrshowinventory(uint16 argc, uint16 *argv) {
	// Give the trap book and Catherine's journal to the player
	*_vm->matchVarToString("atrapbook") = 1;
	*_vm->matchVarToString("acathbook") = 1;
	_vm->_gfx->showInventory();
}

void RivenExternal::xrhideinventory(uint16 argc, uint16 *argv) {
	_vm->_gfx->hideInventory();
}

void RivenExternal::xrwindowsetup(uint16 argc, uint16 *argv) {
	// TODO: Randomizing what effect happens when you look out into the middle of Tay (useless! :P)
}

// ------------------------------------------------------------------------------------
// tspit (Temple Island) external commands
// ------------------------------------------------------------------------------------

void RivenExternal::xtexterior300_telescopedown(uint16 argc, uint16 *argv) {
	// First, show the button movie
	_vm->_video->playMovieBlocking(3);

	// Don't do anything else if the telescope power is off
	if (*_vm->matchVarToString("ttelevalve") == 0)
		return;

	uint32 *telescopePos = _vm->matchVarToString("ttelescope");
	uint32 *telescopeCover = _vm->matchVarToString("ttelecover");

	if (*telescopePos == 1) {
		// We're at the bottom, which means one of two things can happen...
		if (*telescopeCover == 1 && *_vm->matchVarToString("ttelepin") == 1) {
			// ...if the cover is open and the pin is up, the game is now over.
			if (*_vm->matchVarToString("pcage") == 2) {
				// The best ending: Catherine is free, Gehn is trapped, Atrus comes to rescue you.
				// And now we fall back to Earth... all the way...
				warning("xtexterior300_telescopedown: Good ending");
				_vm->_video->activateMLST(8, _vm->getCurCard());
				runEndGame(8);
			} else if (*_vm->matchVarToString("agehn") == 4) {
				// The ok ending: Catherine is still trapped, Gehn is trapped, Atrus comes to rescue you.
				// Nice going! Catherine and the islanders are all dead now! Just go back to your home...
				warning("xtexterior300_telescopedown: OK ending");
				_vm->_video->activateMLST(9, _vm->getCurCard());
				runEndGame(9);
			} else if (*_vm->matchVarToString("atrapbook") == 1) {
				// The bad ending: Catherine is trapped, Gehn is free, Atrus gets shot by Gehn,
				// And then you get shot by Cho. Nice going! Catherine and the islanders are dead
				// and you have just set Gehn free from Riven, not to mention you're dead.
				warning("xtexterior300_telescopedown: Bad ending");
				_vm->_video->activateMLST(10, _vm->getCurCard());
				runEndGame(10);
			} else {
				// The impossible ending: You don't have Catherine's journal and yet you were somehow
				// able to open the hatch on the telescope. The game provides an ending for those who
				// cheat, load a saved game with the combo, or just guess the telescope combo. Atrus
				// doesn't come and you just fall into the fissure.
				warning("xtexterior300_telescopedown: Wtf ending");
				_vm->_video->activateMLST(11, _vm->getCurCard());
				runEndGame(11);
			}
		} else {
			// ...the telescope can't move down anymore.
			// TODO: Play sound
		}
	} else {
		// We're not at the bottom, and we can move down again
		
		// TODO: Down movie, it involves playing a chunk of a movie
		
		// Now move the telescope down a position and refresh
		*telescopePos -= 1;
		_vm->changeToCard();
	}
}

void RivenExternal::xtexterior300_telescopeup(uint16 argc, uint16 *argv) {
	// First, show the button movie
	_vm->_video->playMovieBlocking(3);

	// Don't do anything else if the telescope power is off
	if (*_vm->matchVarToString("ttelevalve") == 0)
		return;

	uint32 *telescopePos = _vm->matchVarToString("ttelescope");

	// Check if we can't move up anymore
	if (*telescopePos == 5) {
		// TODO: Play sound
		return;
	}

	// TODO: Up movie, it involves playing a chunk of a movie
	
	// Now move the telescope up a position and refresh
	*telescopePos += 1;
	_vm->changeToCard();
}

void RivenExternal::xtisland390_covercombo(uint16 argc, uint16 *argv) {
	// Called when clicking the telescope cover buttons. button is the button number (1...5).
	uint32 *pressedButtons = _vm->matchVarToString("tcovercombo");
	
	// We pressed a button! Yay! Add it to the queue.
	*pressedButtons *= 10;
	*pressedButtons += argv[0];
	
	if (*pressedButtons == *_vm->matchVarToString("tcorrectorder")) {
		_vm->_hotspots[9].enabled = true;
	} else {
		_vm->_hotspots[9].enabled = false;
		
		// Set the buttons to the last one pressed if we've
		// pressed more than 5 buttons.
		if (*pressedButtons > 55555)
			*pressedButtons = argv[0];
	}
}

// Atrus' Journal and Trap Book are added to inventory
void RivenExternal::xtatrusgivesbooks(uint16 argc, uint16 *argv) {
	// Give the player Atrus' Journal and the Trap book
	*_vm->matchVarToString("aatrusbook") = 1;
	*_vm->matchVarToString("atrapbook") = 1;
	
	// Randomize the telescope combination
	uint32 *teleCombo = _vm->matchVarToString("tcorrectorder");
	for (byte i = 0; i < 5; i++) {
		*teleCombo *= 10;
		*teleCombo += _rnd->getRandomNumberRng(1, 5);
	}
	
	// TODO: Randomize Dome Combination
}

// Trap Book is removed from inventory
void RivenExternal::xtchotakesbook(uint16 argc, uint16 *argv) {
	// And now Cho takes the trap book. Sure, this isn't strictly
	// necessary to add and them remove the trap book... but it
	// seems better to do this ;)
	*_vm->matchVarToString("atrapbook") = 0;
}

void RivenExternal::xthideinventory(uint16 argc, uint16 *argv) {
	_vm->_gfx->hideInventory();
}

void RivenExternal::xt7500_checkmarbles(uint16 argc, uint16 *argv) {
	// TODO: Lots of stuff to do here, eventually we have to check each individual
	// marble position and set apower based on that. The game handles the video playing
	// so we don't have to. For the purposes of making the game progress further, we'll
	// just turn the power on for now. 
	*_vm->matchVarToString("apower") = 1;
}

void RivenExternal::xt7600_setupmarbles(uint16 argc, uint16 *argv) {
	// TODO: Marble puzzle related
}

void RivenExternal::xt7800_setup(uint16 argc, uint16 *argv) {
	// TODO: Marble puzzle related
}

void RivenExternal::xdrawmarbles(uint16 argc, uint16 *argv) {
	// TODO: Marble puzzle related
}

void RivenExternal::xtakeit(uint16 argc, uint16 *argv) {
	// TODO: Marble puzzle related
}

void RivenExternal::xtscpbtn(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xtisland4990_domecheck(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xtisland5056_opencard(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xtisland5056_resetsliders(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xtisland5056_slidermd(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xtisland5056_slidermw(uint16 argc, uint16 *argv) {
	// TODO: Dome related
}

void RivenExternal::xtatboundary(uint16 argc, uint16 *argv) {
	runDemoBoundaryDialog();
}

// ------------------------------------------------------------------------------------
// Common external commands
// ------------------------------------------------------------------------------------

void RivenExternal::xflies(uint16 argc, uint16 *argv) {
	// TODO: Activate the "flies" effect
}

} // End of namespace Mohawk
