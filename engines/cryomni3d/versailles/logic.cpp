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

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"
#include "common/file.h"
#include "common/system.h"

#include "cryomni3d/fixed_image.h"

#include "cryomni3d/versailles/engine.h"

namespace CryOmni3D {
namespace Versailles {

static const char *imagesObjects[] = {
	"PAMP.gif",   //  0:  96
	"PAPT_2.gif", //  1:  98
	"PAML.gif",   //  2: 101
	"ESQ1.gif",   //  3: 105a, 106b
	"ESQ2.gif",   //  4: 105b, 106c
	"ESQ3.gif",   //  5: 105c
	"ESQ4.gif",   //  6: 105d, 106a, 107a
	"ESQ4T.gif",  //  7: 107b
	"ESQ4D.gif",  //  8: 109
	"PAMA.gif",   //  9: 115
	"PAMM1.gif",  // 10: 118a
	"PAMM2.gif",  // 11: 118b
	"MEDP.gif",   // 12: 121a
	"MEDP2.gif",  // 13: 121b
	"PAMR1.gif",  // 14: 125a
	"PAMR4.gif",  // 15: 125b
	"EPIL.gif",   // 16: 126
	"PAMG.gif",   // 17: 127
	"PBETE.gif",  // 18: 129
	"VAU2.gif",   // 19: 131
	"VAU3.gif",   // 20: 132
	"GRAV2.gif",  // 21: 134
	"MEM.gif",    // 22: 137
	"VS1.gif",    // 23: 138
	"VS2.gif",    // 24: 139
	"FAB.gif",    // 25: 141
	"LABYR.gif",  // 26: 142
};

void CryOmni3DEngine_Versailles::setupObjects() {
	_objects.reserve(51);
#define SET_OBJECT(cursorId, nameId) _objects.push_back(Object(_sprites, cursorId, nameId))
#define SET_OBJECT_CB(cursorId, nameId, cb) do { \
        _objects.push_back(Object(_sprites, cursorId, nameId)); \
        _objects.back().setViewCallback(new Common::Functor0Mem<void, CryOmni3DEngine_Versailles>(this, &CryOmni3DEngine_Versailles::cb)); \
    } while (false)
#define SET_OBJECT_GENERIC_CB(cursorId, nameId, imageId) SET_OBJECT_CB(cursorId, nameId, genericDisplayObject<imageId>)
	SET_OBJECT(161, 93);
	SET_OBJECT(107, 94);
	SET_OBJECT(69, 95);
	SET_OBJECT_GENERIC_CB(230, 96, 0);
	SET_OBJECT(64, 97);
	SET_OBJECT_GENERIC_CB(250, 98, 1);
	SET_OBJECT(202, 99);
	SET_OBJECT(235, 100);
	SET_OBJECT_GENERIC_CB(167, 101, 2);
	SET_OBJECT(191, 102);
	SET_OBJECT(171, 103);
	SET_OBJECT(47, 104);
	SET_OBJECT(205, 105);
	SET_OBJECT(214, 106);
	SET_OBJECT(6, 107);
	SET_OBJECT(58, 108);
	SET_OBJECT_GENERIC_CB(5, 109, 8);
	SET_OBJECT(38, 110);
	SET_OBJECT(119, 113);
	SET_OBJECT(186, 114);
	SET_OBJECT_GENERIC_CB(246, 115, 9);
	SET_OBJECT(80, 116);
	SET_OBJECT(180, 117);
	SET_OBJECT(34, 118);
	SET_OBJECT(173, 119);
	SET_OBJECT(81, 120);
	SET_OBJECT(156, 121);
	SET_OBJECT(143, 122);
	SET_OBJECT(101, 123);
	SET_OBJECT(204, 124);
	SET_OBJECT(10, 125);
	SET_OBJECT(112, 126); // TODO: EPIL.gif
	SET_OBJECT_GENERIC_CB(90, 127, 17);
	SET_OBJECT(216, 128);
	SET_OBJECT_GENERIC_CB(32, 129, 18);
	SET_OBJECT(37, 130);
	SET_OBJECT_GENERIC_CB(134, 131, 19);
	SET_OBJECT_GENERIC_CB(150, 132, 20);
	SET_OBJECT(28, 133);
	SET_OBJECT_GENERIC_CB(22, 134, 21);
	SET_OBJECT(92, 135);
	SET_OBJECT(115, 136); // Out of order in EXE
	SET_OBJECT_GENERIC_CB(16, 137, 22);
	SET_OBJECT_GENERIC_CB(237, 138, 23);
	SET_OBJECT_GENERIC_CB(0, 139, 24);
	SET_OBJECT(31, 140);
	SET_OBJECT_GENERIC_CB(87, 141, 25);
	SET_OBJECT(95, 142); // TODO: LABYR.gif
	SET_OBJECT(157, 143);
	SET_OBJECT(168, 144);
	SET_OBJECT(65, 145);
#undef SET_OBJECT
}

template<unsigned int ID>
void CryOmni3DEngine_Versailles::genericDisplayObject() {
	displayObject(imagesObjects[ID]);
}

// This array contains images for all paintings it must be kept in sync with _paintingsTitles
static const char *imagesPaintings[] = {
	"10E_1.GIF",      //  0: 41201
	nullptr,          //  1: 41202
	"10E_3.GIF",      //  2: 41203
	"10E_4.GIF",      //  3: 41204
	"10E_5.GIF",      //  4: 41205
	"10D_1.GIF",      //  5: 41301
	"10D_2.GIF",      //  6: 41302
	"20C_1.GIF",      //  7: 42401
	"20G_11.GIF",     //  8: 42901
	"20G_12.GIF",     //  9: 42902
	"20G_13.GIF",     // 10: 42903
	"20G_14.GIF",     // 11: 42904
	"20G_15.GIF",     // 12: 42905
	"20G_16.GIF",     // 13: 42906
	"20G_21.GIF",     // 14: 42907
	"20G_22.GIF",     // 15: 42908
	"20G_23.GIF",     // 16: 42909
	"20G_31.GIF",     // 17: 42910
	"20G_32.GIF",     // 18: 42911
	"20G_33.GIF",     // 19: 42912
	"20G_34.GIF",     // 20: 42913
	"20G_35.GIF",     // 21: 42914
	"20G_36.GIF",     // 22: 42915
	"30N_1.GIF",      // 23: 43090
	"30N_2.GIF",      // 24: 43091
	"30N_3.GIF",      // 25: 43092
	"30O_1.GIF",      // 26: 43100
	"30O_2.GIF",      // 27: 43101
	"30O_31.GIF",     // 28: 43102
	"30O_32.GIF",     // 29: 43103
	"30O_33.GIF",     // 30: 43104
	"30M_1.GIF",      // 31: 43130
	"30M_2.GIF",      // 32: 43131
	"30M_3.GIF",      // 33: 43132
	"30L_11.GIF",     // 34: 43140
	"30L_12.GIF",     // 35: 43141
	"30L_21.GIF",     // 36: 43142
	nullptr,          // 37: 43143
	"30L_32.GIF",     // 38: 43144
	"30J_11.GIF",     // 39: 43150
	"30J_12.GIF",     // 40: 43151
	"30J_13.GIF",     // 41: 43152
	"30J_21.GIF",     // 42: 43153
	"30J_22.GIF",     // 43: 43154
	"30J_31.GIF",     // 44: 43155
	"30J_32.GIF",     // 45: 43156
	"30J_33.GIF",     // 46: 43157
	"51A_1.GIF",      // 47: 45260
	// Now let's put dumb images, those without description and any special action, they are not synced with _paintingsTitles
	"30Q_1.GIF",      // 48: 43060
	"30Q_2.GIF",      // 49: 43061
	"52M2.GIF",       // 50: 45130 // Almost dumb
	"53I_LUST.GIF",   // 51: 45280 // Almost dumb
	"DUC.GIF",        // 52: 46001
	"COQ.GIF",        // 53: 46002
	"CHAT.GIF",       // 54: 46003
	"DRAGON.GIF",     // 55: 46004
	"GRUE.GIF",       // 56: 46005
	"RENARD.GIF",     // 57: 46006
	"POULE.GIF",      // 58: 46007
	"LOUP.GIF",       // 59: 46008
	"MILAN.GIF",      // 60: 46009
	"GRENOU.GIF",     // 61: 46010
	"AIGLE.GIF",      // 62: 46011
	"SOURIS.GIF",     // 63: 46012
	"CYGNE.GIF",      // 64: 46013 and 46440
	"LOUPTETE.GIF",   // 65: 46014
	"CANNES.GIF",     // 66: 46015
};

// Setup array for all see actions
void CryOmni3DEngine_Versailles::setupImgScripts() {
	// First all paintings to keep it simple for counting
#define SET_SCRIPT_BY_ID(id) _imgScripts[id] = &CryOmni3DEngine_Versailles::img_ ## id
#define SET_SCRIPT_BY_PAINTING(id, image) _imgScripts[id] = &CryOmni3DEngine_Versailles::genericPainting<image>
	SET_SCRIPT_BY_PAINTING(41201,  0);
	SET_SCRIPT_BY_ID(41202);
	SET_SCRIPT_BY_PAINTING(41203,  2);
	SET_SCRIPT_BY_PAINTING(41204,  3);
	SET_SCRIPT_BY_PAINTING(41205,  4);
	SET_SCRIPT_BY_PAINTING(41301,  5);
	SET_SCRIPT_BY_PAINTING(41302,  6);
	SET_SCRIPT_BY_PAINTING(42401,  7);
	SET_SCRIPT_BY_PAINTING(42901,  8);
	SET_SCRIPT_BY_PAINTING(42902,  9);
	SET_SCRIPT_BY_PAINTING(42903, 10);
	SET_SCRIPT_BY_PAINTING(42904, 11);
	SET_SCRIPT_BY_PAINTING(42905, 12);
	SET_SCRIPT_BY_PAINTING(42906, 13);
	SET_SCRIPT_BY_PAINTING(42907, 14);
	SET_SCRIPT_BY_PAINTING(42908, 15);
	SET_SCRIPT_BY_PAINTING(42909, 16);
	SET_SCRIPT_BY_PAINTING(42910, 17);
	SET_SCRIPT_BY_PAINTING(42911, 18);
	SET_SCRIPT_BY_PAINTING(42912, 19);
	SET_SCRIPT_BY_PAINTING(42913, 20);
	SET_SCRIPT_BY_PAINTING(42914, 21);
	SET_SCRIPT_BY_PAINTING(42915, 22);
	SET_SCRIPT_BY_PAINTING(43090, 23);
	SET_SCRIPT_BY_PAINTING(43091, 24);
	SET_SCRIPT_BY_PAINTING(43092, 25);
	SET_SCRIPT_BY_PAINTING(43100, 26);
	SET_SCRIPT_BY_PAINTING(43101, 27);
	SET_SCRIPT_BY_PAINTING(43102, 28);
	SET_SCRIPT_BY_PAINTING(43103, 29);
	SET_SCRIPT_BY_PAINTING(43104, 30);
	SET_SCRIPT_BY_PAINTING(43130, 31);
	SET_SCRIPT_BY_PAINTING(43131, 32);
	SET_SCRIPT_BY_PAINTING(43132, 33);
	SET_SCRIPT_BY_PAINTING(43140, 34);
	SET_SCRIPT_BY_PAINTING(43141, 35);
	SET_SCRIPT_BY_PAINTING(43142, 36);
	//SET_SCRIPT_BY_ID(43143); // TODO: implement it
	SET_SCRIPT_BY_PAINTING(43144, 38);
	SET_SCRIPT_BY_PAINTING(43150, 39);
	SET_SCRIPT_BY_PAINTING(43151, 40);
	SET_SCRIPT_BY_PAINTING(43152, 41);
	SET_SCRIPT_BY_PAINTING(43153, 42);
	SET_SCRIPT_BY_PAINTING(43154, 43);
	SET_SCRIPT_BY_PAINTING(43155, 44);
	SET_SCRIPT_BY_PAINTING(43156, 45);
	SET_SCRIPT_BY_PAINTING(43157, 46);
	SET_SCRIPT_BY_PAINTING(45260, 47);
#undef SET_SCRIPT_BY_PAINTING
	// From now dumb images (like paintings but without interrogation mark handling)
#define SET_SCRIPT_BY_DUMB(id, image) _imgScripts[id] = &CryOmni3DEngine_Versailles::genericDumbImage<image>
	SET_SCRIPT_BY_DUMB(43060, 48);
	SET_SCRIPT_BY_DUMB(43061, 49);
	SET_SCRIPT_BY_DUMB(46001, 52);
	SET_SCRIPT_BY_DUMB(46002, 53);
	SET_SCRIPT_BY_DUMB(46003, 54);
	SET_SCRIPT_BY_DUMB(46004, 55);
	SET_SCRIPT_BY_DUMB(46005, 56);
	SET_SCRIPT_BY_DUMB(46006, 57);
	SET_SCRIPT_BY_DUMB(46007, 58);
	SET_SCRIPT_BY_DUMB(46008, 59);
	SET_SCRIPT_BY_DUMB(46009, 60);
	SET_SCRIPT_BY_DUMB(46010, 61);
	SET_SCRIPT_BY_DUMB(46011, 62);
	SET_SCRIPT_BY_DUMB(46012, 63);
	SET_SCRIPT_BY_DUMB(46013, 64);
	SET_SCRIPT_BY_DUMB(46014, 65);
	SET_SCRIPT_BY_DUMB(46015, 66);
	SET_SCRIPT_BY_DUMB(46440, 64); // Same as 46013
#undef SET_SCRIPT_BY_DUMB
	// From now specific handlers for anything that is not a painting
	SET_SCRIPT_BY_ID(41801);
	SET_SCRIPT_BY_ID(41802);
	//SET_SCRIPT_BY_ID(43145); // TODO: implement it
	//SET_SCRIPT_BY_ID(43146); // TODO: implement it
	//SET_SCRIPT_BY_ID(43160); // TODO: implement it
	//SET_SCRIPT_BY_ID(43190); // TODO: implement it
	//SET_SCRIPT_BY_ID(44071); // TODO: implement it
	//SET_SCRIPT_BY_ID(44161); // TODO: implement it
	//SET_SCRIPT_BY_ID(45130); // TODO: implement it // Almost dumb
	//SET_SCRIPT_BY_ID(45270); // TODO: implement it
	//SET_SCRIPT_BY_ID(45280); // TODO: implement it // Almost dumb
	//SET_SCRIPT_BY_ID(88001); // TODO: implement it
	//SET_SCRIPT_BY_ID(88002); // TODO: implement it
	//SET_SCRIPT_BY_ID(88003); // TODO: implement it
	//SET_SCRIPT_BY_ID(88004); // TODO: implement it
#undef SET_SCRIPT_BY_ID
}

// Generic handler for dumb fixed images
template<unsigned int ID>
void CryOmni3DEngine_Versailles::genericDumbImage(ZonFixedImage *fimg) {
	fimg->load(imagesPaintings[ID]);
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
	}
}

// Generic handler for interrogation mark action: display the painting title
#define HANDLE_QUESTION(ID) \
    do { \
        if (fimg->_zoneQuestion) { \
            displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), _paintingsTitles[ID], Common::Point(600, 400), \
                    Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage)); \
        } \
    } while (false)

// Generic handler for paintings fixed images
template<unsigned int ID>
void CryOmni3DEngine_Versailles::genericPainting(ZonFixedImage *fimg) {
	fimg->load(imagesPaintings[ID]);
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		HANDLE_QUESTION(ID);
	}
}

// Specific fixed images callbacks
#define IMG_CB(name) void CryOmni3DEngine_Versailles::img_ ## name(ZonFixedImage *fimg)

IMG_CB(31142) {
	fimg->load("10D2_4.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), 7,
			                  fimg->getZoneCenter(fimg->_currentZone),
			                  Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage));
		}
	}
}

IMG_CB(31142b) {
	fimg->load("11D2_2.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (_gameVariables[GameVariables::kCollectScissors] || _inventory.inInventoryByNameId(94)) {
				// Empty drawer
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_31142d);
				fimg->changeCallback(functor);
				break;
			} else {
				// Drawer with scissors in it
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_31142c);
				fimg->changeCallback(functor);
				break;
			}
		}
	}
}

IMG_CB(31142c) {
	fimg->load("11D2_21.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (!_inventory.inInventoryByNameId(94) && !_gameVariables[GameVariables::kCollectScissors]) {
				collectObject(94, fimg);
			}
			_gameVariables[GameVariables::kCollectScissors] = 1;
			// Display empty drawer
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_31142d);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(31142d) {
	fimg->load("11D2_22.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			// Close drawer
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_31142b);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(31143) {
	fimg->load("10D2_3.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), 7,
			                  fimg->getZoneCenter(fimg->_currentZone),
			                  Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage));
		}
	}
}

IMG_CB(31143b) {
	fimg->load("11D2_1.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (_inventory.inInventoryByNameId(96)) {
				// Empty drawer
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_31143d);
				fimg->changeCallback(functor);
				break;
			} else {
				// Drawer with pamphlet about arts in it
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_31143c);
				fimg->changeCallback(functor);
				break;
			}
		}
	}
}

IMG_CB(31143c) {
	fimg->load("11D2_11.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (!_inventory.inInventoryByNameId(96)) {
				collectObject(96, fimg);
			}
			// Display empty drawer
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_31143d);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(31143d) {
	fimg->load("11D2_12.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			// Close drawer
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_31143b);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(41202) {
	fimg->load("10E_20.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		HANDLE_QUESTION(1);
		if (fimg->_zoneUse) {
			if (fimg->_currentZone == 2 && !_inventory.inInventoryByNameId(97)) {
				// Open the jar
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_41202b);
				fimg->changeCallback(functor);
				break;
			} else {
				displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), 11,
				                  fimg->getZoneCenter(fimg->_currentZone),
				                  Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage));
			}
		}
	}
}

IMG_CB(41202b) {
	fimg->load("10E_21.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit) {
			break;
		}
		HANDLE_QUESTION(1);
		if (fimg->_zoneLow) {
			// Go back to jars closed
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_41202);
			fimg->changeCallback(functor);
			break;
		}
		if (fimg->_zoneUse) {
			if (!_inventory.inInventoryByNameId(97)) {
				collectObject(97, fimg);
			}
			// Go back to jars closed
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_41202);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(41801) {
	fimg->load("12E2_10.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_currentZone == 0) {
			bool open = false;
			if (fimg->_zoneUse) {
				// Using without object
				if (_gameVariables[GameVariables::kUsedScissors]) {
					open = true;
				} else {
					// Closed
					displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), 8,
					                  fimg->getZoneCenter(fimg->_currentZone),
					                  Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage));
				}
			} else if (fimg->_usedObject && fimg->_usedObject->idOBJ() == 94) {
				_gameVariables[GameVariables::kUsedScissors] = 1;
				_inventory.removeByNameId(94);
				open = true;
			}
			if (open) {
				if (_gameVariables[GameVariables::kCollectedPaperInTrunk]) {
					// Display empty trunk
					ZonFixedImage::CallbackFunctor *functor =
					    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
					            &CryOmni3DEngine_Versailles::img_41801c);
					fimg->changeCallback(functor);
					break;
				} else {
					// Display trunk with paper in it
					// Animate opening
					playInGameVideo("12E2_11");
					// Force reload of the place
					if (_nextPlaceId == -1u) {
						_nextPlaceId = _currentPlaceId;
					}

					ZonFixedImage::CallbackFunctor *functor =
					    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
					            &CryOmni3DEngine_Versailles::img_41801b);
					fimg->changeCallback(functor);
					break;
				}
			}
		}
	}
}

IMG_CB(41801b) {
	fimg->load("12E2_11.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit) {
			break;
		}
		if (fimg->_zoneLow) {
			// Animate closing
			playInGameVideo("12E2_13");
			// Force reload of the place
			if (_nextPlaceId == -1u) {
				_nextPlaceId = _currentPlaceId;
			}
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (!_inventory.inInventoryByNameId(100)) {
				collectObject(100, fimg);
			}
			_gameVariables[GameVariables::kCollectedPaperInTrunk] = 1;

			// Go to empty trunk
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_41801c);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(41801c) {
	fimg->load("12E2_12.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit) {
			break;
		}
		if (fimg->_zoneLow) {
			// Animate closing
			playInGameVideo("12E2_13");
			// Force reload of the place
			if (_nextPlaceId == -1u) {
				_nextPlaceId = _currentPlaceId;
			}
			fimg->_exit = true;
			break;
		}
	}
}

IMG_CB(41802) {
	// Dispatch to the correct state
	if (_gameVariables[GameVariables::kInkSpilled] &&
	        !_gameVariables[GameVariables::kCollectedPaperOnTable]) {
		// Draw paper with ink on it
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_41802b);
		fimg->changeCallback(functor);
		return;
	}
	if (!_gameVariables[GameVariables::kInkSpilled] &&
	        _gameVariables[GameVariables::kCollectedPaperOnTable]) {
		// Draw table with ink in inkpot and without paper
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_41802c);
		fimg->changeCallback(functor);
		return;
	}
	if (_gameVariables[GameVariables::kInkSpilled] &&
	        _gameVariables[GameVariables::kCollectedPaperOnTable]) {
		// Draw table with ink directly on table
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_41802d);
		fimg->changeCallback(functor);
		return;
	}

	// There we have paper on table and ink is in its inkpot
	fimg->load("12E2_20.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse && fimg->_currentZone == 1) {
			// Collected paper
			collectObject(95, fimg);
			_gameVariables[GameVariables::kCollectedPaperOnTable] = 1;
			setPlaceState(8, 1);
			// Draw table with ink in inkpot and without paper
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_41802c);
			fimg->changeCallback(functor);
			break;
		}
		if (fimg->_zoneUse && fimg->_currentZone == 2) {
			_gameVariables[GameVariables::kInkSpilled] = 1;
			setPlaceState(8, 3);
			// Draw paper with ink on it
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_41802b);
			fimg->changeCallback(functor);
			break;
		}
		if (fimg->_usedObject && fimg->_currentZone == 0) {
			unsigned int objID = fimg->_usedObject->idOBJ();
			if (objID == 100) {
				playInGameVideo("12E2_24");
				// Force reload of the place
				if (_nextPlaceId == -1u) {
					_nextPlaceId = _currentPlaceId;
				}
				_inventory.removeByNameId(100);
				// Revealed paper
				collectObject(98, fimg);
				_gameVariables[GameVariables::kGotRevealedPaper] = 1;
				setGameTime(3, 1);
			} else if (objID == 96) {
				// Pamphlet about arts
				playInGameVideo("PAP_BRUL");
				// Force reload of the place
				if (_nextPlaceId == -1u) {
					_nextPlaceId = _currentPlaceId;
				}
				doGameOver();
			}
		}
	}
}

IMG_CB(41802b) {
	// There we have paper on table with ink on it
	fimg->load("12E2_21.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse && fimg->_currentZone == 1) {
			// Collected paper with ink on it
			collectObject(99, fimg);
			_gameVariables[GameVariables::kCollectedPaperOnTable] = 1;
			setPlaceState(8, 2);
			// Draw table with ink spilled and without paper
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_41802d);
			fimg->changeCallback(functor);
			break;
		}
		if (fimg->_usedObject && fimg->_currentZone == 0) {
			unsigned int objID = fimg->_usedObject->idOBJ();
			if (objID == 100) {
				playInGameVideo("12E2_24");
				// Force reload of the place
				if (_nextPlaceId == -1u) {
					_nextPlaceId = _currentPlaceId;
				}
				_inventory.removeByNameId(100);
				// Revealed paper
				collectObject(98, fimg);
				_gameVariables[GameVariables::kGotRevealedPaper] = 1;
				setGameTime(3, 1);
			} else if (objID == 96) {
				// Pamphlet about arts
				playInGameVideo("PAP_BRUL");
				// Force reload of the place
				if (_nextPlaceId == -1u) {
					_nextPlaceId = _currentPlaceId;
				}
				doGameOver();
			}
		}
	}
}

IMG_CB(41802c) {
	// There we have ink in inkpot and without paper
	fimg->load("12E2_22.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse && fimg->_currentZone == 1) {
			_gameVariables[GameVariables::kInkSpilled] = 1;
			setPlaceState(8, 2);
			// Draw table with ink on it
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_41802d);
			fimg->changeCallback(functor);
			break;
		}
		if (fimg->_usedObject && fimg->_currentZone == 0) {
			unsigned int objID = fimg->_usedObject->idOBJ();
			if (objID == 100) {
				playInGameVideo("12E2_24");
				// Force reload of the place
				if (_nextPlaceId == -1u) {
					_nextPlaceId = _currentPlaceId;
				}
				_inventory.removeByNameId(100);
				// Revealed paper
				collectObject(98, fimg);
				_gameVariables[GameVariables::kGotRevealedPaper] = 1;
				setGameTime(3, 1);
			} else if (objID == 96) {
				// Pamphlet about arts
				playInGameVideo("PAP_BRUL");
				// Force reload of the place
				if (_nextPlaceId == -1u) {
					_nextPlaceId = _currentPlaceId;
				}
				doGameOver();
			}
		}
	}
}

IMG_CB(41802d) {
	// There we have ink directly on table
	fimg->load("12E2_23.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_usedObject && fimg->_currentZone == 0) {
			unsigned int objID = fimg->_usedObject->idOBJ();
			if (objID == 100) {
				playInGameVideo("12E2_24");
				// Force reload of the place
				if (_nextPlaceId == -1u) {
					_nextPlaceId = _currentPlaceId;
				}
				_inventory.removeByNameId(100);
				// Revealed paper
				collectObject(98, fimg);
				_gameVariables[GameVariables::kGotRevealedPaper] = 1;
				setGameTime(3, 1);
			} else if (objID == 96) {
				// Pamphlet about arts
				playInGameVideo("PAP_BRUL");
				// Force reload of the place
				if (_nextPlaceId == -1u) {
					_nextPlaceId = _currentPlaceId;
				}
				doGameOver();
			}
		}
	}
}

#undef IMG_CB

// Init place and filter event
#define FILTER_EVENT(level, place) bool CryOmni3DEngine_Versailles::filterEventLevel ## level ## Place ## place(unsigned int *event)
#define INIT_PLACE(level, place) void CryOmni3DEngine_Versailles::initPlaceLevel ## level ## Place ## place()

FILTER_EVENT(1, 1) {
	if (*event > 0 && *event < 9999) {
		_gameVariables[GameVariables::kWarnedIncomplete] = 0;
	}
	if (*event == 11015 && currentGameTime() < 3) {
		return false;
	} else {
		return true;
	}
}

FILTER_EVENT(1, 2) {
	if (*event == 7 && currentGameTime() < 2) {
		// Closed
		displayMessageBoxWarp(2);
		return false;
	}

	if (*event == 1 && currentGameTime() < 3) {
		_dialogsMan.play("11E_HUI");
		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == -1u) {
			_nextPlaceId = _currentPlaceId;
		}
		return false;
	}

	return true;
}

INIT_PLACE(1, 3) {
	if (!_gameVariables[GameVariables::kHasPlayedLebrun]) {
		Common::File *audioFile = new Common::File();
		if (!audioFile->open("LEB001__.WAV")) {
			warning("Failed to open sound file %s", "LEB001__.WAV");
			delete audioFile;
			return;
		}

		Audio::SeekableAudioStream *audioDecoder = Audio::makeWAVStream(audioFile, DisposeAfterUse::YES);
		// We lost ownership of the audioFile just set it to nullptr and don't use it
		audioFile = nullptr;
		if (!audioDecoder) {
			return;
		}

		_mixer->playStream(Audio::Mixer::kSpeechSoundType, nullptr, audioDecoder, SoundIds::kLeb001);
		// We lost ownership of the audioDecoder just set it to nullptr and don't use it
		audioDecoder = nullptr;

		_gameVariables[GameVariables::kHasPlayedLebrun] = 1;
	}
}

FILTER_EVENT(1, 3) {
	if (*event == 11301) {
		while (!g_engine->shouldQuit() && _mixer->isSoundIDActive(SoundIds::kLeb001)) {
			g_system->updateScreen();
			pollEvents();
		}
		clearKeys();
		return true;
	}

	if (*event > 0 && *event < 10000) {
		_mixer->stopID(SoundIds::kLeb001);
		return true;
	}
	return true;
}

// Event 19 is not in this room: must be a leftover
/*
FILTER_EVENT(1, 7) {
    if (*event == 19) {
        // Too dark
        displayMessageBoxWarp(7);
        return false;
    }

    return true;
}
*/

FILTER_EVENT(1, 14) {
	if (*event == 31141 && _placeStates[14].state == 0) {
		// Open the curtain
		unsigned int fakePlaceId = getFakeTransition(*event);
		fakeTransition(fakePlaceId);
		playInGameVideo("10D2_1");
		setPlaceState(14, 1);
		// setPlaceState will force reload
		// Don't pass the event as we try to avoid implementing use
		return false;
	}

	if (*event != 31142 && *event != 31143) {
		// Not for us
		return true;
	}

	const char *video;
	FixedImgCallback callback;

	if (_currentLevel == 1 && _placeStates[14].state == 0) {
		if (*event == 31142) {
			video = "10D2_4";
			callback = &CryOmni3DEngine_Versailles::img_31142;
		} else if (*event == 31143) {
			video = "10D2_3";
			callback = &CryOmni3DEngine_Versailles::img_31143;
		}
	} else if (_currentLevel == 2 || _placeStates[14].state == 1) {
		if (*event == 31142) {
			video = "11D2_2";
			callback = &CryOmni3DEngine_Versailles::img_31142b;
		} else if (*event == 31143) {
			video = "11D2_1";
			callback = &CryOmni3DEngine_Versailles::img_31143b;
		}
	} else {
		error("Invalid state in filter event 1/14: level: %d/ placeState: %d", _currentLevel,
		      _placeStates[14].state);
	}

	unsigned int fakePlaceId = getFakeTransition(*event);
	fakeTransition(fakePlaceId);

	playInGameVideo(video);

	// Force reload of the place
	if (_nextPlaceId == -1u) {
		_nextPlaceId = _currentPlaceId;
	}

	handleFixedImg(callback);

	// Don't pass the event as we try to avoid implementing use
	return false;
}

#undef FILTER_EVENT
#undef INIT_PLACE

} // End of namespace Versailles
} // End of namespace CryOmni3D
