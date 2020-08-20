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

static const char *kImagesObjects[] = {
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
#define SET_OBJECT_AND_CB(cursorId, nameId, cb) do { \
        _objects.push_back(Object(_sprites, cursorId, nameId)); \
        _objects.back().setViewCallback(new Common::Functor0Mem<void, CryOmni3DEngine_Versailles>(this, &CryOmni3DEngine_Versailles::cb)); \
    } while (false)
#define SET_OBJECT_GENERIC_CB(cursorId, nameId, imageId) SET_OBJECT_AND_CB(cursorId, nameId, genericDisplayObject<imageId>)
#define SET_OBJECT_CB(cursorId, nameId) SET_OBJECT_AND_CB(cursorId, nameId, obj_ ## nameId)
	SET_OBJECT(161, 93); // 0
	SET_OBJECT(107, 94);
	SET_OBJECT(69, 95);
	SET_OBJECT_GENERIC_CB(230, 96, 0);
	SET_OBJECT(64, 97);
	SET_OBJECT_GENERIC_CB(250, 98, 1); // 5
	SET_OBJECT(202, 99);
	SET_OBJECT(235, 100);
	SET_OBJECT_GENERIC_CB(167, 101, 2);
	SET_OBJECT(191, 102);
	SET_OBJECT(171, 103); // 10
	SET_OBJECT(47, 104);
	SET_OBJECT_CB(205, 105);
	SET_OBJECT_CB(214, 106);
	SET_OBJECT_CB(6, 107);
	SET_OBJECT(58, 108); // 15
	SET_OBJECT_GENERIC_CB(5, 109, 8);
	SET_OBJECT(38, 110);
	SET_OBJECT(119, 113);
	SET_OBJECT(186, 114);
	SET_OBJECT_GENERIC_CB(246, 115, 9); // 20
	SET_OBJECT(80, 116);
	SET_OBJECT(180, 117);
	SET_OBJECT_CB(34, 118);
	SET_OBJECT(173, 119);
	SET_OBJECT(81, 120); // 25
	SET_OBJECT_CB(156, 121);
	SET_OBJECT(143, 122);
	SET_OBJECT(101, 123);
	SET_OBJECT(204, 124);
	SET_OBJECT_CB(10, 125); // 30
	SET_OBJECT_CB(112, 126);
	SET_OBJECT_GENERIC_CB(90, 127, 17);
	SET_OBJECT(216, 128);
	SET_OBJECT_CB(32, 129);
	SET_OBJECT(37, 130); // 35
	SET_OBJECT_GENERIC_CB(134, 131, 19);
	SET_OBJECT_GENERIC_CB(150, 132, 20);
	SET_OBJECT(28, 133);
	SET_OBJECT_GENERIC_CB(22, 134, 21);
	SET_OBJECT(92, 135); // 40
	SET_OBJECT_GENERIC_CB(16, 137, 22);
	SET_OBJECT_GENERIC_CB(237, 138, 23);
	SET_OBJECT_GENERIC_CB(0, 139, 24);
	SET_OBJECT(31, 140);
	SET_OBJECT_GENERIC_CB(87, 141, 25); // 45
	SET_OBJECT_CB(95, 142);
	SET_OBJECT(115, 136); // Out of order in EXE
	SET_OBJECT(157, 143);
	SET_OBJECT(168, 144);
	SET_OBJECT(65, 145); // 50
#undef SET_OBJECT_CB
#undef SET_OBJECT_GENERIC_CB
#undef SET_OBJECT_AND_CB
#undef SET_OBJECT
}

template<uint ID>
void CryOmni3DEngine_Versailles::genericDisplayObject() {
	displayObject(kImagesObjects[ID]);
}

void CryOmni3DEngine_Versailles::obj_105() {
	displayObject(kImagesObjects[3]);
	displayObject(kImagesObjects[4]);
	displayObject(kImagesObjects[5]);
	displayObject(kImagesObjects[6]);
}

void CryOmni3DEngine_Versailles::obj_106() {
	displayObject(kImagesObjects[6]);
	displayObject(kImagesObjects[3]);
	displayObject(kImagesObjects[4]);
}

void CryOmni3DEngine_Versailles::obj_107() {
	if (_gameVariables[GameVariables::kSketchState] == 3) {
		displayObject(kImagesObjects[7]);
	} else {
		displayObject(kImagesObjects[6]);
	}
}

void CryOmni3DEngine_Versailles::obj_118() {
	if (_gameVariables[GameVariables::kDecipherScore]) {
		displayObject(kImagesObjects[11]);
	} else {
		displayObject(kImagesObjects[10]);
	}
}

void CryOmni3DEngine_Versailles::obj_121() {
	if (_gameVariables[GameVariables::kGotMedalsSolution]) {
		displayObject(kImagesObjects[13]);
	} else {
		displayObject(kImagesObjects[12]);
	}
}

void CryOmni3DEngine_Versailles::obj_125() {
	if (_gameVariables[GameVariables::kStateLampoonReligion]) {
		displayObject(kImagesObjects[15]);
	} else {
		displayObject(kImagesObjects[14]);
	}
}

void CryOmni3DEngine_Versailles::obj_126() {
	displayObject(kImagesObjects[16], &CryOmni3DEngine_Versailles::obj_126hk);
}

void CryOmni3DEngine_Versailles::obj_126hk(Graphics::ManagedSurface &surface) {
	Graphics::Surface bmpLetters[28];
	loadBMPs("bomb_%02d.bmp", bmpLetters, 28);

	drawEpigraphLetters(surface, bmpLetters, _epigraphPassword);

	for (uint i = 0; i < 28; i++) {
		bmpLetters[i].free();
	}

	if (_messages.size() <= 148) {
		return;
	}

	Common::String &translation = _messages[148];

	if (translation.size() == 0) {
		return;
	}

	_fontManager.setCurrentFont(1);
	_fontManager.setTransparentBackground(true);
	_fontManager.setForeColor(0);
	_fontManager.setSurface(&surface);
	_fontManager.displayStr(9, 424, translation);
}

void CryOmni3DEngine_Versailles::obj_129() {
	displayObject(kImagesObjects[18], &CryOmni3DEngine_Versailles::obj_129hk);
}

void CryOmni3DEngine_Versailles::obj_129hk(Graphics::ManagedSurface &surface) {
	if (_messages.size() <= 149) {
		return;
	}

	Common::String &translation = _messages[149];

	if (translation.size() == 0) {
		return;
	}

	surface.fillRect(Common::Rect(0, 455, 640, 480), 247);

	_fontManager.setCurrentFont(8);
	_fontManager.setTransparentBackground(true);
	_fontManager.setForeColor(242);
	_fontManager.setSurface(&surface);
	_fontManager.displayStr(10, 460, translation);
}

void CryOmni3DEngine_Versailles::obj_142() {
	// Display a marker only when in maze
	if (_currentLevel == 6 && _currentPlaceId >= 14 && _currentPlaceId <= 44) {
		displayObject(kImagesObjects[26], &CryOmni3DEngine_Versailles::obj_142hk);
	} else {
		displayObject(kImagesObjects[26]);
	}
}

void CryOmni3DEngine_Versailles::obj_142hk(Graphics::ManagedSurface &surface) {
	const Common::Point markers[] = {
		Common::Point(135, 403), // 14
		Common::Point(136, 321), // 15
		Common::Point(225, 109),
		Common::Point(441,  88),
		Common::Point(505,  78),
		Common::Point(550,  82),
		Common::Point(479, 242), // 20
		Common::Point(529, 333),
		Common::Point(466, 407),
		Common::Point(359, 411),
		Common::Point(305, 415),
		Common::Point(217, 405), // 25
		Common::Point(216, 325),
		Common::Point(280, 378),
		Common::Point(340, 313),
		Common::Point(282, 313),
		Common::Point(253, 285), // 30
		Common::Point(225, 258),
		Common::Point(154, 255),
		Common::Point(219, 188),
		Common::Point(294, 251),
		Common::Point(341, 242), // 35
		Common::Point(308, 206),
		Common::Point(270, 172),
		Common::Point(363, 161),
		Common::Point(416, 201),
		Common::Point(513, 195), // 40
		Common::Point(412, 311),
		Common::Point(446, 280),
		Common::Point(377, 347),
		Common::Point(448, 356),
	};

	uint id = _currentPlaceId - 14;
	assert(id < ARRAYSIZE(markers));

	/*
	_fontManager.setSurface(&surface);
	_fontManager.setCurrentFont(4);
	_fontManager.setTransparentBackground(true);
	_fontManager.setForeColor(241);
	for(id = 0; id < ARRAYSIZE(markers); id++) {
	*/
	// Why - 20? Ask to game creators, it's like that in the code
	uint spriteX = markers[id].x - _sprites.getCursor(4).getWidth() / 2 - 20;
	uint spriteY = markers[id].y - _sprites.getCursor(4).getHeight() / 2;
	surface.transBlitFrom(_sprites.getSurface(4), Common::Point(spriteX, spriteY),
	                      _sprites.getKeyColor(4));
	/*
	_fontManager.displayInt(markers[id].x - 10, markers[id].y, id + 14);
	}
	*/
}

// This array contains images for all paintings it must be kept in sync with _paintingsTitles
static const char *kImagesPaintings[] = {
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
	"DUC.GIF",        // 50: 46001
	"COQ.GIF",        // 51: 46002
	"CHAT.GIF",       // 52: 46003
	"DRAGON.GIF",     // 53: 46004
	"GRUE.GIF",       // 54: 46005
	"RENARD.GIF",     // 55: 46006
	"POULE.GIF",      // 56: 46007
	"LOUP.GIF",       // 57: 46008
	"MILAN.GIF",      // 58: 46009
	"GRENOU.GIF",     // 59: 46010
	"AIGLE.GIF",      // 60: 46011
	"SOURIS.GIF",     // 61: 46012
	"CYGNE.GIF",      // 62: 46013 and 46440
	"LOUPTETE.GIF",   // 63: 46014
	"CANNES.GIF",     // 64: 46015
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
	SET_SCRIPT_BY_ID(43143);
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
	SET_SCRIPT_BY_DUMB(46001, 50);
	SET_SCRIPT_BY_DUMB(46002, 51);
	SET_SCRIPT_BY_DUMB(46003, 52);
	SET_SCRIPT_BY_DUMB(46004, 53);
	SET_SCRIPT_BY_DUMB(46005, 54);
	SET_SCRIPT_BY_DUMB(46006, 55);
	SET_SCRIPT_BY_DUMB(46007, 56);
	SET_SCRIPT_BY_DUMB(46008, 57);
	SET_SCRIPT_BY_DUMB(46009, 58);
	SET_SCRIPT_BY_DUMB(46010, 59);
	SET_SCRIPT_BY_DUMB(46011, 60);
	SET_SCRIPT_BY_DUMB(46012, 61);
	SET_SCRIPT_BY_DUMB(46013, 62);
	SET_SCRIPT_BY_DUMB(46014, 63);
	SET_SCRIPT_BY_DUMB(46015, 64);
	SET_SCRIPT_BY_DUMB(46440, 62); // Same as 46013
#undef SET_SCRIPT_BY_DUMB
	// From now specific handlers for anything that is not a painting
	SET_SCRIPT_BY_ID(41801);
	SET_SCRIPT_BY_ID(41802);
	SET_SCRIPT_BY_ID(43145);
	SET_SCRIPT_BY_ID(43146);
	SET_SCRIPT_BY_ID(43160);
	SET_SCRIPT_BY_ID(43190);
	SET_SCRIPT_BY_ID(44071);
	SET_SCRIPT_BY_ID(44161);
	SET_SCRIPT_BY_ID(45130);
	SET_SCRIPT_BY_ID(45270);
	SET_SCRIPT_BY_ID(45280);
	SET_SCRIPT_BY_ID(88001);
	SET_SCRIPT_BY_ID(88002);
	SET_SCRIPT_BY_ID(88003);
	SET_SCRIPT_BY_ID(88004);
#undef SET_SCRIPT_BY_ID
}

// Generic handler for dumb fixed images
template<uint ID>
void CryOmni3DEngine_Versailles::genericDumbImage(ZonFixedImage *fimg) {
	fimg->load(kImagesPaintings[ID]);
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
template<uint ID>
void CryOmni3DEngine_Versailles::genericPainting(ZonFixedImage *fimg) {
	fimg->load(kImagesPaintings[ID]);
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

IMG_CB(31101) {
	fimg->load("21F_11.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (fimg->_currentZone == 0) {
				// Collect key and change image
				collectObject(104, fimg);
				_gameVariables[GameVariables::kCollectKey] = 1;
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_31101b);
				fimg->changeCallback(functor);
				break;
			} else if (fimg->_currentZone == 1 && !_inventory.inInventoryByNameID(103)) {
				collectObject(103, fimg);
			}
		}
	}
}

IMG_CB(31101b) {
	fimg->load("21F_10.GIF");
	if (_inventory.inInventoryByNameID(103)) {
		fimg->disableZone(1);
	}
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse && !_inventory.inInventoryByNameID(103)) {
			collectObject(103, fimg);
			// Original game resets callback with this one, this is useless
			break;
		}
	}
}

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
			if (_gameVariables[GameVariables::kCollectScissors] || _inventory.inInventoryByNameID(94)) {
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
			if (!_inventory.inInventoryByNameID(94) && !_gameVariables[GameVariables::kCollectScissors]) {
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
			if (_inventory.inInventoryByNameID(96)) {
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
			if (!_inventory.inInventoryByNameID(96)) {
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

IMG_CB(32120) {
	if (currentGameTime() != 3) {
		fimg->_exit = true;
		return;
	}

	// Already painted
	if (_gameVariables[GameVariables::kSketchState] == 3 ||
	        _gameVariables[GameVariables::kSketchState] == 4) {
		fimg->_exit = true;
		return;
	}

	fimg->load("23I_10.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			_sprites.replaceSpriteColor(59, 254, 244);
			_sprites.replaceSpriteColor(63, 254, 247);
			break;
		}
		if (fimg->_zoneUse) {
			if (fimg->_currentZone == 0 && !_inventory.inInventoryByIconID(38)) {
				// Pick the brush
				Object *obj = _objects.findObjectByIconID(38);
				collectObject(obj, fimg);
				// collectObject animates the cursor
				// Make it selected
				_inventory.setSelectedObject(obj);
				setCursor(obj->idSA()); // 59 in original game
				fimg->_zonesMode = ZonFixedImage::kZonesMode_Object;
			}
		} else if (fimg->_usedObject && fimg->_usedObject->idCA() == 38) {
			// Brush used
			if (fimg->_currentZone == 1) { // on gold paint
				_inventory.removeByIconID(38);
				Object *obj = _objects.findObjectByIconID(38);
				obj->rename(111); // Brush has gold on it
				setMainPaletteColor(254, 128, 128, 0);
				_sprites.replaceSpriteColor(59, 244, 254);
				_sprites.replaceSpriteColor(63, 247, 254);
				// Collect only there once we set the color on the cursors
				collectObject(obj, fimg);
				_inventory.setSelectedObject(obj);
				setCursor(obj->idSA()); // 59 in original game
				fimg->_zonesMode = ZonFixedImage::kZonesMode_Object;
				_gameVariables[GameVariables::kBrushColor] = 1;
			} else if (fimg->_currentZone == 2) { // on red paint
				_inventory.removeByIconID(38);
				Object *obj = _objects.findObjectByIconID(38);
				obj->rename(112); // Brush has red on it
				setMainPaletteColor(254, 128, 0, 0);
				_sprites.replaceSpriteColor(59, 244, 254);
				_sprites.replaceSpriteColor(63, 247, 254);
				// Collect only there once we set the color on the cursors
				collectObject(obj, fimg);
				_inventory.setSelectedObject(obj);
				setCursor(obj->idSA()); // 59 in original game
				fimg->_zonesMode = ZonFixedImage::kZonesMode_Object;
				_gameVariables[GameVariables::kBrushColor] = 2;
			} else if (fimg->_currentZone == 3) { // on sketch
				if (fimg->_usedObject->idOBJ() == 111 &&
				        _gameVariables[GameVariables::kBrushColor] == 1) {
					// Gold brush used on sketch
					_gameVariables[GameVariables::kSketchState] = 3;
					playInGameVideo("23I_11");
					// Force reload of the place
					if (_nextPlaceId == uint(-1)) {
						_nextPlaceId = _currentPlaceId;
					}
					_inventory.removeByIconID(38);
					ZonFixedImage::CallbackFunctor *functor =
					    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
					            &CryOmni3DEngine_Versailles::img_32120b);
					fimg->changeCallback(functor);
					break;
				} else if (fimg->_usedObject->idOBJ() == 112 &&
				           _gameVariables[GameVariables::kBrushColor] == 2) {
					// Red brush used on sketch
					_gameVariables[GameVariables::kSketchState] = 4;
					playInGameVideo("23I_12");
					// Force reload of the place
					if (_nextPlaceId == uint(-1)) {
						_nextPlaceId = _currentPlaceId;
					}
					_inventory.removeByIconID(38);
					ZonFixedImage::CallbackFunctor *functor =
					    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
					            &CryOmni3DEngine_Versailles::img_32120c);
					fimg->changeCallback(functor);
					break;
				}
			}
		}
	}
}

IMG_CB(32120b) {
	fimg->load("23I_11.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneUse) {
			fimg->_exit = true;
			break;
		}
	}
	_inventory.removeByNameID(107);
	collectObject(107, fimg);

	_sprites.replaceSpriteColor(59, 254, 244);
	_sprites.replaceSpriteColor(63, 254, 247);

	setGameTime(4, 2);
}

IMG_CB(32120c) {
	fimg->load("23I_12.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneUse) {
			fimg->_exit = true;
			break;
		}
	}
	_inventory.removeByNameID(107);
	collectObject(109, fimg);

	_sprites.replaceSpriteColor(59, 254, 244);
	_sprites.replaceSpriteColor(63, 254, 247);

	setGameTime(4, 2);
}

IMG_CB(32201) {
	fimg->load("21E_41.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		// There is a check of use but only to change sound path
		// That must be a leftover
		/*
		if (fimg->_zoneUse) {
		}
		*/
	}
}

IMG_CB(32202) {
	fimg->load("21E_42.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
	}
}

IMG_CB(32203) {
	fimg->load("21E_43.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		// There is a check of use but only to change sound path
		// That must be a leftover
		/*
		if (fimg->_zoneUse) {
		}
		*/
	}
}

IMG_CB(32204) {
	fimg->load("21E_44.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse && !_inventory.inInventoryByNameID(105)) {
			// Collect portfolio
			collectObject(105, fimg);
			_gameVariables[GameVariables::kSketchState] = 1;
			_gameVariables[GameVariables::kCollectPortfolio] = 1;

			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_32204b);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(32204b) {
	fimg->load("21E_45.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
	}
}

IMG_CB(34131) {
	fimg->load("43ZA_1.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
	}
}

IMG_CB(34132) {
	fimg->load("43ZB_2.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
	}
}

IMG_CB(34172) {
	playInGameVideo("43X3_10");
	// Force reload of the place
	if (_nextPlaceId == uint(-1)) {
		_nextPlaceId = _currentPlaceId;
	}
	fimg->_exit = true;
}

IMG_CB(34173) {
	fimg->load("43X3_2.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			// WORKAROUND: The video doesn't exist there is only a fixed image unused in original game. We will use it.
			/*
			playInGameVideo("43X3_21");
			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
			    _nextPlaceId = _currentPlaceId;
			}
			*/

			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_34173b);
			fimg->changeCallback(functor);
			break;
		}
	}
}

// WORKAROUND: In original game an empty clickable drawer is displayed
// Happily for them, when you click on an area and change zones, the next zone
// under the cursor get activated too (fixed in fixed_image.cpp). So you don't
// see what happens. You just get the reminder and see the empty drawer.
IMG_CB(34173b) {
	// 43X3_21 doesn't have a ZON file, use the one of 43X3_22
	fimg->load("43X3_21.GIF", "43X3_22.ZON");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse && !_inventory.inInventoryByNameID(129)) {
			// Collect reminder
			collectObject(129, fimg);
			setGameTime(3, 4);

			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_34173c);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(34173c) {
	fimg->load("43X3_22.GIF");
	// WORKAROUND: Drawer is empty, just disable the use zone
	fimg->disableZone(0);
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
	}
}

IMG_CB(34174) {
	fimg->load("43X3_42.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			// Open the door
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_34174b);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(34174b) {
	// Door is open but safe is closed
	fimg->load("43X3_40.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (_gameVariables[GameVariables::kSafeUnlocked]) {
				// Open the safe
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_34174c);
				fimg->changeCallback(functor);
				break;
			}
			_dialogsMan["{JOUEUR-ALLER-BUREAU-LOUVOIS}"] = 'Y';
			if (handleSafe(fimg)) {
				// Unlocked the safe
				_gameVariables[GameVariables::kSafeUnlocked] = 1;
				_dialogsMan["{JOUEUR-ALLER-BUREAU-LOUVOIS}"] = 'N';
				// Open it
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_34174c);
				fimg->changeCallback(functor);
				break;
			}
			// If failed to solve: just display this image again
			break;
		}
	}
}

IMG_CB(34174c) {
	// Dispatch to the correct state
	if (_gameVariables[GameVariables::kCollectVaubanBlueprint1] &&
	        _gameVariables[GameVariables::kCollectVaubanBlueprint2]) {
		// Safe is empty
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_34174f);
		fimg->changeCallback(functor);
		return;
	}
	if (!_gameVariables[GameVariables::kCollectVaubanBlueprint1] &&
	        _gameVariables[GameVariables::kCollectVaubanBlueprint2] == 1) {
		// Blueprint 1 is there not the 2nd one
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_34174e);
		fimg->changeCallback(functor);
		return;
	}
	if (_gameVariables[GameVariables::kCollectVaubanBlueprint1] &&
	        !_gameVariables[GameVariables::kCollectVaubanBlueprint2]) {
		// Blueprint 2 is there not the 1st one
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_34174d);
		fimg->changeCallback(functor);
		return;
	}

	playInGameVideo("cofouv");
	// Force reload of the place
	if (_nextPlaceId == uint(-1)) {
		_nextPlaceId = _currentPlaceId;
	}

	// Safe has both blueprints
	fimg->load("43X3_30.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (fimg->_currentZone == 0) {
				// Collect 1st blueprint
				collectObject(131, fimg);
				_dialogsMan["{JOUEUR-TROUVE-PLANS-VAUBAN}"] = 'Y';
				_gameVariables[GameVariables::kCollectVaubanBlueprint1] = 1;

				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_34174d);
				fimg->changeCallback(functor);
				break;
			} else if (fimg->_currentZone == 1) {
				// Collect 2nd blueprint
				collectObject(132, fimg);
				_gameVariables[GameVariables::kCollectVaubanBlueprint2] = 1;

				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_34174e);
				fimg->changeCallback(functor);
				break;
			}
		}
	}
}

IMG_CB(34174d) {
	// Safe has only blueprint 2
	fimg->load("43X3_43.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			// Collect 2nd blueprint
			collectObject(132, fimg);
			_gameVariables[GameVariables::kCollectVaubanBlueprint2] = 1;

			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_34174f);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(34174e) {
	// Safe has only blueprint 1
	fimg->load("43X3_41.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			// Collect 1st blueprint
			collectObject(131, fimg);
			_dialogsMan["{JOUEUR-TROUVE-PLANS-VAUBAN}"] = 'Y';
			_gameVariables[GameVariables::kCollectVaubanBlueprint1] = 1;

			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_34174f);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(34174f) {
	// Safe is empty
	fimg->load("43X3_45.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			// Close safe
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_34174b);
			fimg->changeCallback(functor);
			break;
		}
	}
}

bool CryOmni3DEngine_Versailles::handleSafe(ZonFixedImage *fimg) {
	bool success = false;
	Common::RandomSource rnd("VersaillesSafe");
	Graphics::Surface bmpDigits[10];
	unsigned char safeDigits[kSafeDigitsCount];
	Graphics::ManagedSurface tempSurf;

	loadBMPs("coff_%02d.bmp", bmpDigits, 10);
	for (uint i = 0; i < kSafeDigitsCount; i++) {
		safeDigits[i] = rnd.getRandomNumber(9);
	}

	fimg->load("43x3_cof.GIF");
	const Graphics::Surface *fimgSurface = fimg->surface();
	tempSurf.create(fimgSurface->w, fimgSurface->h, fimgSurface->format);
	tempSurf.blitFrom(*fimgSurface);
	drawSafeDigits(tempSurf, bmpDigits, safeDigits);
	fimg->updateSurface(&tempSurf.rawSurface());

	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			break;
		}
		if (fimg->_zoneUse) {
			if (fimg->_currentZone == 15) {
				// Safe handle

				// Animate handle
				playInGameVideo("43x3_poi");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
					_nextPlaceId = _currentPlaceId;
				}

				// Redraw our safe image
				fimg->display();

				if (checkSafeDigits(safeDigits)) {
					success = true;
					break;
				}
			} else if (fimg->_currentZone < kSafeDigitsCount) {
				// Safe digit
				safeDigits[fimg->_currentZone] = (safeDigits[fimg->_currentZone] + 1) % 10;
				// Reset the surface and redraw digits on it
				tempSurf.blitFrom(*fimgSurface);
				drawSafeDigits(tempSurf, bmpDigits, safeDigits);
				fimg->updateSurface(&tempSurf.rawSurface());

				waitMouseRelease();
			}
		}
	}

	for (uint i = 0; i < 10; i++) {
		bmpDigits[i].free();
	}
	return success;
}

const uint16 CryOmni3DEngine_Versailles::kSafeDigitsX[] = { 267, 318, 370, 421 };
const uint16 CryOmni3DEngine_Versailles::kSafeDigitsY[] = { 148, 230, 311 };

void CryOmni3DEngine_Versailles::drawSafeDigits(Graphics::ManagedSurface &surface,
        const Graphics::Surface(&bmpDigits)[10], const unsigned char (&safeDigits)[kSafeDigitsCount]) {
	for (uint i = 0; i < ARRAYSIZE(safeDigits); i++) {
		const Graphics::Surface &digit = bmpDigits[safeDigits[i]];
		Common::Point dst(kSafeDigitsX[i % 4], kSafeDigitsY[i / 4]);
		surface.transBlitFrom(digit, dst);
	}
}

const char *CryOmni3DEngine_Versailles::kSafeDates[] = { "1643", "1668", "1674" };
bool CryOmni3DEngine_Versailles::checkSafeDigits(unsigned char (&safeDigits)[kSafeDigitsCount]) {
	uint dateChecked;
	for (dateChecked = 0; dateChecked < ARRAYSIZE(kSafeDates); dateChecked++) {
		const char *checkDate = kSafeDates[dateChecked];
		// Find the date in one of safe digits lines
		uint line;
		for (line = 0; line < kSafeDigitsCount; line += 4) {
			uint digit;
			for (digit = 0; digit < 4; digit++) {
				if (safeDigits[line + digit] != checkDate[digit] - '0') {
					break;
				}
			}
			if (digit == 4) {
				// Check was a success: go to next date
				break;
			}
			// Failure: try next line
		}
		if (line >= kSafeDigitsCount) {
			// All lines were tested and none had the date: failure
			return false;
		}
	}
	// All dates were found
	return true;
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
			if (fimg->_currentZone == 2 && !_inventory.inInventoryByNameID(97)) {
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
			if (!_inventory.inInventoryByNameID(97)) {
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
				_inventory.removeByNameID(94);
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
					if (_nextPlaceId == uint(-1)) {
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
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (!_inventory.inInventoryByNameID(100)) {
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
			if (_nextPlaceId == uint(-1)) {
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
			uint objID = fimg->_usedObject->idOBJ();
			if (objID == 100) {
				playInGameVideo("12E2_24");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
					_nextPlaceId = _currentPlaceId;
				}
				_inventory.removeByNameID(100);
				// Revealed paper
				collectObject(98, fimg);
				_gameVariables[GameVariables::kGotRevealedPaper] = 1;
				setGameTime(3, 1);
			} else if (objID == 96) {
				// Lampoon about arts
				playInGameVideo("PAP-BRUL");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
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
			uint objID = fimg->_usedObject->idOBJ();
			if (objID == 100) {
				playInGameVideo("12E2_24");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
					_nextPlaceId = _currentPlaceId;
				}
				_inventory.removeByNameID(100);
				// Revealed paper
				collectObject(98, fimg);
				_gameVariables[GameVariables::kGotRevealedPaper] = 1;
				setGameTime(3, 1);
			} else if (objID == 96) {
				// Lampoon about arts
				playInGameVideo("PAP-BRUL");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
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
			uint objID = fimg->_usedObject->idOBJ();
			if (objID == 100) {
				playInGameVideo("12E2_24");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
					_nextPlaceId = _currentPlaceId;
				}
				_inventory.removeByNameID(100);
				// Revealed paper
				collectObject(98, fimg);
				_gameVariables[GameVariables::kGotRevealedPaper] = 1;
				setGameTime(3, 1);
			} else if (objID == 96) {
				// Lampoon about arts
				playInGameVideo("PAP-BRUL");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
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
			uint objID = fimg->_usedObject->idOBJ();
			if (objID == 100) {
				playInGameVideo("12E2_24");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
					_nextPlaceId = _currentPlaceId;
				}
				_inventory.removeByNameID(100);
				// Revealed paper
				collectObject(98, fimg);
				_gameVariables[GameVariables::kGotRevealedPaper] = 1;
				setGameTime(3, 1);
			} else if (objID == 96) {
				// Lampoon about arts
				playInGameVideo("PAP-BRUL");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
					_nextPlaceId = _currentPlaceId;
				}
				doGameOver();
			}
		}
	}
}

IMG_CB(43143) {
	// Lampoon is there: display it
	if (!_gameVariables[GameVariables::kCollectLampoonArchitecture] &&
	        (_currentLevel == 5 || currentGameTime() >= 3)) {
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_43143b);
		fimg->changeCallback(functor);
		return;
	}

	fimg->load("30L_31.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		HANDLE_QUESTION(37);
	}
}

IMG_CB(43143b) {
	fimg->load("30L_3101.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		HANDLE_QUESTION(37);
		if (fimg->_zoneUse) {
			// Paper is out of reach
			displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), 16,
			                  fimg->getZoneCenter(fimg->_currentZone),
			                  Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage));
		} else if (fimg->_usedObject && fimg->_usedObject->idOBJ() == 119 && fimg->_currentZone == 0) {
			_inventory.removeByNameID(119);
			collectLampoonArchitecture(fimg);
			// Display without the lampoon
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43143);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(43145) {
	fimg->load("30L_50.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (fimg->_currentZone == 0) {
				playInGameVideo("30L_51");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
					_nextPlaceId = _currentPlaceId;
				}

				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_43145b);
				fimg->changeCallback(functor);
				break;
			} else if (fimg->_currentZone == 1) {
				playInGameVideo("30L_52");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
					_nextPlaceId = _currentPlaceId;
				}

				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_43145c);
				fimg->changeCallback(functor);
				break;
			}
		}
	}
}

IMG_CB(43145b) {
	fimg->load("30L_51.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit) {
			break;
		}
		if (fimg->_zoneLow) {
			// Go back to drawer closed
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43145);
			fimg->changeCallback(functor);
			break;
		}
		if (fimg->_zoneUse) {
			if (_gameVariables[GameVariables::kCabinetDrawerStatus] == 1) {
				// Small key 2 has been put in it and not yet picked by us
				collectObject(116, fimg);
				_gameVariables[GameVariables::kCabinetDrawerStatus] = 2;
			} else {
				// Drawer is empty
				displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), 3,
				                  fimg->getZoneCenter(fimg->_currentZone),
				                  Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage));
			}
		}
	}
}

IMG_CB(43145c) {
	fimg->load("30L_52.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit) {
			break;
		}
		if (fimg->_zoneLow) {
			// Go back to drawer closed
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43145);
			fimg->changeCallback(functor);
			break;
		}
		if (fimg->_zoneUse) {
			// Drawer is empty
			displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), 3,
			                  fimg->getZoneCenter(fimg->_currentZone),
			                  Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage));
		}
	}
}

IMG_CB(43146) {
	fimg->load("30L_40.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (fimg->_currentZone == 0) {
				playInGameVideo("30L_41");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
					_nextPlaceId = _currentPlaceId;
				}

				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_43146b);
				fimg->changeCallback(functor);
				break;
			} else if (fimg->_currentZone == 1) {
				playInGameVideo("30L_42");
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
					_nextPlaceId = _currentPlaceId;
				}

				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_43146c);
				fimg->changeCallback(functor);
				break;
			}
		}
	}
}

IMG_CB(43146b) {
	fimg->load("30L_41.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit) {
			break;
		}
		if (fimg->_zoneLow) {
			// Go back to drawer closed
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43146);
			fimg->changeCallback(functor);
			break;
		}
		if (fimg->_zoneUse) {
			// Drawer is empty
			displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), 3,
			                  fimg->getZoneCenter(fimg->_currentZone),
			                  Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage));
		}
	}
}

IMG_CB(43146c) {
	fimg->load("30L_42.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit) {
			break;
		}
		if (fimg->_zoneLow) {
			// Go back to drawer closed
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43146);
			fimg->changeCallback(functor);
			break;
		}
		if (fimg->_zoneUse) {
			// Drawer is empty
			displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), 3,
			                  fimg->getZoneCenter(fimg->_currentZone),
			                  Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage));
		}
	}
}

IMG_CB(43160) {
	// Dispatch to the correct state
	bool inInvCharcoal = _inventory.inInventoryByNameID(113);
	bool inInvPaper = _inventory.inInventoryByNameID(114);
	if (inInvCharcoal && inInvPaper) {
		// When everything is collected, state of place change and we shouldn't be able to look at the table
		error("BUG: Shouldn't be here");
	} else if (inInvCharcoal && !inInvPaper) {
		// Draw table with paper but without charcoal
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_43160b);
		fimg->changeCallback(functor);
		return;
	} else if (!inInvCharcoal && inInvPaper) {
		// Draw table with charcoal but without paper
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_43160c);
		fimg->changeCallback(functor);
		return;
	}

	// There we have charcoal and paper on table
	fimg->load("31I01.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (fimg->_currentZone == 0) {
				// Collected charcoal
				collectObject(113, fimg);
				// Draw table with paper but without charcoal
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_43160b);
				fimg->changeCallback(functor);
				break;
			} else if (fimg->_currentZone == 1) {
				// Collected paper
				collectObject(114, fimg);
				// Draw table with charcoal but without paper
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_43160c);
				fimg->changeCallback(functor);
				break;
			}
		}
	}
}

IMG_CB(43160b) {
	// There we have paper on table but without charcoal
	fimg->load("31I02.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit) {
			break;
		}
		if (fimg->_zoneUse) {
			// Collected paper
			collectObject(114, fimg);
			// Draw table empty
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43160d);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(43160c) {
	// There we have charcoal on table but without paper
	fimg->load("31I03.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit) {
			break;
		}
		if (fimg->_zoneUse) {
			// Collected charcoal
			collectObject(113, fimg);
			// Draw table empty
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43160d);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(43160d) {
	// There we have neither charcoal nor paper on table
	fimg->load("31I04.GIF");
	setPlaceState(16, 1);
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
	}
}

IMG_CB(43190) {
	fimg->load("31L1_20.GIF");
	if (_gameVariables[GameVariables::kCollectScore]) {
		fimg->disableZone(0);
	}
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			playInGameVideo("31L1_2A");
			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43190b);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(43190b) {
	fimg->load("31L1_20B.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			playInGameVideo("31L1_2B");
			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43190c);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(43190c) {
	fimg->load("31L1_20C.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			playInGameVideo("31L1_2C");
			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43190d);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(43190d) {
	fimg->load("31L1_20D.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			playInGameVideo("31L1_2D");
			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43190e);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(43190e) {
	fimg->load("31L1_20E.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_43190f);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(43190f) {
	fimg->load("31L1_22.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit) {
			break;
		}
		if (fimg->_zoneUse) {
			_gameVariables[GameVariables::kCollectScore] = 1;
			collectObject(118, fimg);
			fimg->_exit = true;
			break;
		}
	}
}

IMG_CB(44071) {
	// Dispatch to the correct state
	if (_gameVariables[GameVariables::kCollectFood]) {
		// Draw plate with less food
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_44071b);
		fimg->changeCallback(functor);
		return;
	}

	// There we have a full plate
	fimg->load("41B_bboy.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			// Collected food
			collectObject(124, fimg);
			_gameVariables[GameVariables::kCollectFood] = 1;
			// Draw plate with less food
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_44071b);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(44071b) {
	// There we have less food on plate
	fimg->load("41B_bbo2.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
	}
}

IMG_CB(44161) {
	// Dispatch to the correct state
	if (_gameVariables[GameVariables::kCollectQuill] == 1 && !_inventory.inInventoryByNameID(126)) {
		// We have collected quill but not solved epigraph yet
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_44161b);
		fimg->changeCallback(functor);
		return;
	} else if (_gameVariables[GameVariables::kUsedVaubanBlueprint1] == 1 &&
	           _gameVariables[GameVariables::kUsedVaubanBlueprint2] == 1) {
		// We have used vauban blueprints: display the solution
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_44161f);
		fimg->changeCallback(functor);
		return;
	} else if (_gameVariables[GameVariables::kCollectQuill] == 1 &&
	           _inventory.inInventoryByNameID(126)) {
		// We have collected quill and epigraph
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_44161c);
		fimg->changeCallback(functor);
		return;
	}

	// There we have blueprints, quill and epigraph on desk
	fimg->load("42X2_20.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse && fimg->_currentZone == 0 && currentGameTime() >= 2) {
			// Collected quill
			collectObject(128, fimg);
			_gameVariables[GameVariables::kCollectQuill] = 1;
			// Try to solve epigraph
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_44161b);
			fimg->changeCallback(functor);
			break;
		} else if (fimg->_zoneSee) {
			// Look at blueprints
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_44161d);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(44161b) {
	// There we have blueprints and epigraph on desk.
	fimg->load("42X2_10.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_usedObject && fimg->_usedObject->idOBJ() == 128 && fimg->_currentZone == 1) {
			if (handleEpigraph(fimg)) {
				// Epigraph is solved
				_inventory.removeByNameID(128);
				collectObject(126, fimg, false);
				_dialogsMan["{JOUEUR_POSSEDE_EPIGRAPHE}"] = 'Y';
				setPlaceState(16, 2);
				// No more epigraphe nor quill
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_44161c);
				fimg->changeCallback(functor);
			}
			// If failed to solve: just display this image again
			break;
		} else if (fimg->_zoneSee) {
			// Look at blueprints
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_44161d);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(44161c) {
	// There we have no quill nor epigraph anymore
	fimg->load("42X2_11.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneSee) {
			// Look at blueprints
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_44161d);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(44161d) {
	// Look at blueprints
	fimg->load("VAU1.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_usedObject && fimg->_usedObject->idOBJ() == 131) {
			// Overlay blueprints
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_44161e);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(44161e) {
	// Look at blueprints
	fimg->load("VAUB22.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_usedObject && fimg->_usedObject->idOBJ() == 132) {
			// Overlay blueprints
			_gameVariables[GameVariables::kUsedVaubanBlueprint1] = 1;
			_gameVariables[GameVariables::kUsedVaubanBlueprint2] = 1;
			_inventory.removeByNameID(131);
			_inventory.removeByNameID(132);
			setGameTime(4, 4);
			// Look at the final result
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_44161f);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(44161f) {
	// Look at blueprints result
	fimg->load("VAU.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
	}
}

bool CryOmni3DEngine_Versailles::handleEpigraph(ZonFixedImage *fimg) {
	bool success = false;
	Graphics::Surface bmpLetters[28];
	Common::String password;
	Graphics::ManagedSurface tempSurf;

	loadBMPs("bomb_%02d.bmp", bmpLetters, 28);

	fimg->load("EPIL.GIF");
	const Graphics::Surface *fimgSurface = fimg->surface();
	tempSurf.create(fimgSurface->w, fimgSurface->h, fimgSurface->format);
	// No need to customize image yet

	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			break;
		}
		if (fimg->_zoneUse) {
			if (password.size() >= kEpigraphMaxLetters) {
				continue;
			}
			// Find which letter got clicked
			char letter = _epigraphContent[fimg->_currentZone];
			password += letter;
			// Reset the surface and redraw digits on it
			tempSurf.blitFrom(*fimgSurface);
			drawEpigraphLetters(tempSurf, bmpLetters, password);
			fimg->updateSurface(&tempSurf.rawSurface());

			waitMouseRelease();
		} else if (fimg->_key.keycode) {
			Common::KeyCode keyCode = fimg->_key.keycode;
			if (keyCode == Common::KEYCODE_BACKSPACE) {
				password.deleteLastChar();
			} else {
				if (password.size() >= kEpigraphMaxLetters) {
					continue;
				}
				if (keyCode >= Common::KEYCODE_a &&
				        keyCode <= Common::KEYCODE_z &&
				        _epigraphContent.contains((char)(keyCode - Common::KEYCODE_a + 'A'))) {
					password += keyCode - Common::KEYCODE_a + 'A';
				} else {
					continue;
				}
			}
			// Reset the surface and redraw digits on it
			tempSurf.blitFrom(*fimgSurface);
			drawEpigraphLetters(tempSurf, bmpLetters, password);
			fimg->updateSurface(&tempSurf.rawSurface());
		}

		if (password == _epigraphPassword) {
			success = true;
			break;
		}
	}

	for (uint i = 0; i < 28; i++) {
		bmpLetters[i].free();
	}
	return success;
}

void CryOmni3DEngine_Versailles::drawEpigraphLetters(Graphics::ManagedSurface &surface,
        const Graphics::Surface(&bmpLetters)[28], const Common::String &letters) {
	for (uint i = 0; i < letters.size() && i < kEpigraphMaxLetters; i++) {
		uint letterId = 0;
		if (letters[i] >= 'A' && letters[i] <= 'Z') {
			letterId = letters[i] - 'A';
		} else if (letters[i] == ' ') {
			letterId = 26;
		} else if (letters[i] == '\'') {
			letterId = 27;
		}
		const Graphics::Surface &letter = bmpLetters[letterId];
		Common::Point dst(34 * i + 4, 380);
		surface.transBlitFrom(letter, dst);
	}
}

IMG_CB(45130) {
	fimg->load("52M2.GIF");
	_dialogsMan["{JOUEUR-VU-PLANS-SALON-DIANE}"] = 'Y';
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
	}
}

IMG_CB(45270) {
	fimg->load("51A4_11.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (fimg->_currentZone == 0) {
				// Open left drawer
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_45270b);
				fimg->changeCallback(functor);
				break;
			} else if (fimg->_currentZone == 1) {
				// Open middle drawer
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_45270c);
				fimg->changeCallback(functor);
				break;
			} else if (fimg->_currentZone == 2) {
				// Open right drawer
				ZonFixedImage::CallbackFunctor *functor =
				    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
				            &CryOmni3DEngine_Versailles::img_45270d);
				fimg->changeCallback(functor);
				break;
			}
		}
	}
}

IMG_CB(45270b) {
	fimg->load("51A4_22.GIF");
	if (!_gameVariables[GameVariables::kCollectSmallKey3]) {
		// Collected small key 3
		collectObject(135, fimg);
		_gameVariables[GameVariables::kCollectSmallKey3] = 1;
	}
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
			            &CryOmni3DEngine_Versailles::img_45270);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(45270c) {
	fimg->load("51A4_32.GIF");
	if (!_gameVariables[GameVariables::kCollectEngraving]) {
		// Collected engraving
		collectObject(134, fimg);
		_gameVariables[GameVariables::kCollectEngraving] = 1;
	}
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
			            &CryOmni3DEngine_Versailles::img_45270);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(45270d) {
	fimg->load("51A4_12.GIF");
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
			            &CryOmni3DEngine_Versailles::img_45270);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(45280) {
	if (_gameVariables[GameVariables::kOpenedCurtain]) {
		fimg->load("53I_LUST.GIF");
		_gameVariables[GameVariables::kSeenMemorandum] = 1;
		while (1) {
			fimg->manage();
			if (fimg->_exit || fimg->_zoneLow) {
				fimg->_exit = true;
				break;
			}
		}
	} else {
		fimg->_exit = true;
	}
}

IMG_CB(88001) {
	if (!_inventory.inInventoryByNameID(121) &&
	        _gameVariables[GameVariables::kMedalsDrawerStatus] == 3) {
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
		            &CryOmni3DEngine_Versailles::img_88001c);
		fimg->changeCallback(functor);
		return;
	}

	fimg->load("33P_10.GIF");
	if (_inventory.inInventoryByNameID(121)) {
		fimg->disableZone(0);
	}
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse &&
		        !_inventory.inInventoryByNameID(121)) {
			// Open the drawer

			playInGameVideo("33P_10");
			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}

			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_88001b);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(88001b) {
	_gameVariables[GameVariables::kMedalsDrawerStatus] = 2;

	fimg->load("33P_12.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit) {
			break;
		}
		if (fimg->_zoneLow) {
			_gameVariables[GameVariables::kMedalsDrawerStatus] = 0;
			// Go back to drawer closed
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_88001);
			fimg->changeCallback(functor);
			break;
		}
		if (fimg->_usedObject &&
		        fimg->_usedObject->idOBJ() == 114 &&
		        fimg->_currentZone == 0) {
			// Lay the paper on the medals
			_inventory.removeByNameID(114);

			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_88001c);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(88001c) {
	// Paper is laid on the medals
	_gameVariables[GameVariables::kMedalsDrawerStatus] = 3;

	fimg->load("33P_13.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_usedObject &&
		        fimg->_usedObject->idOBJ() == 113 &&
		        fimg->_currentZone == 0) {
			// Use charcoal on paper and medals
			_inventory.removeByNameID(113);

			playInGameVideo("33P_14");
			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}

			collectObject(121, fimg);
			_dialogsMan["{JOUEUR-POSSEDE-FUSAIN-MEDAILLES}"] = 'Y';

			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_88001);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(88002) {
	fimg->load("53Z1c_10.GIF");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (_currentLevel == 7) {
				// You will need something to reach the bomb
				displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), 10,
				                  fimg->getZoneCenter(fimg->_currentZone),
				                  Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage));
			}
		}
	}
}

IMG_CB(88003) {
	// Dispatch to the correct state
	if (_gameVariables[GameVariables::kBombState] >= 1 &&
	        _gameVariables[GameVariables::kBombState] <= 5) {
		FixedImgCallback callback = nullptr;
		switch (_gameVariables[GameVariables::kBombState]) {
		case 1:
			callback = &CryOmni3DEngine_Versailles::img_88003b;
			break;
		case 2:
			callback = &CryOmni3DEngine_Versailles::img_88003c;
			break;
		case 3:
			callback = &CryOmni3DEngine_Versailles::img_88003d;
			break;
		case 4:
			callback = &CryOmni3DEngine_Versailles::img_88003e;
			break;
		case 5:
			callback = &CryOmni3DEngine_Versailles::img_88003f;
			break;
		default:
			error("BUG: Invalid bomb state");
			break;
		}
		ZonFixedImage::CallbackFunctor *functor =
		    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this, callback);
		fimg->changeCallback(functor);
		return;
	}

	fimg->load("70Z_10.GIF");

	// Draw countdown there and not in fixed image class directly
	Graphics::ManagedSurface tempSurf;
	const Graphics::Surface *fimgSurface = fimg->surface();
	tempSurf.create(fimgSurface->w, fimgSurface->h, fimgSurface->format);
	tempSurf.blitFrom(*fimgSurface);
	drawCountdown(&tempSurf);
	fimg->updateSurface(&tempSurf.rawSurface());

	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_currentZone == 0 && fimg->_usedObject &&
		        fimg->_usedObject->idOBJ() == 145) {
			// Now we know how to reach the bomb
			_gameVariables[GameVariables::kBombState] = 1;
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_88003b);
			fimg->changeCallback(functor);
			break;
		}
		if (fimg->_zoneUse) {
			if (_currentLevel == 7) {
				// You will need something to reach the bomb
				displayMessageBox(kFixedimageMsgBoxParameters, fimg->surface(), 10,
				                  fimg->getZoneCenter(fimg->_currentZone),
				                  Common::Functor0Mem<void, ZonFixedImage>(fimg, &ZonFixedImage::manage));
			}
		}
		if (countDown()) {
			// Countdown has changed: refresh surface
			drawCountdown(&tempSurf);
			fimg->updateSurface(&tempSurf.rawSurface());
		}
	}
}

IMG_CB(88003b) {
	fimg->load("70Z_11.GIF");

	// Draw countdown there and not in fixed image class directly
	Graphics::ManagedSurface tempSurf;
	const Graphics::Surface *fimgSurface = fimg->surface();
	tempSurf.create(fimgSurface->w, fimgSurface->h, fimgSurface->format);
	tempSurf.blitFrom(*fimgSurface);
	drawCountdown(&tempSurf);
	fimg->updateSurface(&tempSurf.rawSurface());

	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_currentZone == 0 && fimg->_usedObject &&
		        fimg->_usedObject->idOBJ() == 97) {
			// Unlock first line with key 1
			_gameVariables[GameVariables::kBombState] = 2;
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_88003c);
			fimg->changeCallback(functor);
			break;
		}
		if (countDown()) {
			// Countdown has changed: refresh surface
			drawCountdown(&tempSurf);
			fimg->updateSurface(&tempSurf.rawSurface());
		}
	}
}

IMG_CB(88003c) {
	fimg->load("70Z_12.GIF");

	// Draw countdown there and not in fixed image class directly
	Graphics::ManagedSurface tempSurf;
	const Graphics::Surface *fimgSurface = fimg->surface();
	tempSurf.create(fimgSurface->w, fimgSurface->h, fimgSurface->format);
	tempSurf.blitFrom(*fimgSurface);
	drawCountdown(&tempSurf);
	fimg->updateSurface(&tempSurf.rawSurface());

	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_currentZone == 1 && fimg->_usedObject &&
		        fimg->_usedObject->idOBJ() == 116) {
			// Unlock second line with key 2
			_gameVariables[GameVariables::kBombState] = 3;
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_88003d);
			fimg->changeCallback(functor);
			break;
		}
		if (countDown()) {
			// Countdown has changed: refresh surface
			drawCountdown(&tempSurf);
			fimg->updateSurface(&tempSurf.rawSurface());
		}
	}
}

IMG_CB(88003d) {
	fimg->load("70Z_13.GIF");

	// Draw countdown there and not in fixed image class directly
	Graphics::ManagedSurface tempSurf;
	const Graphics::Surface *fimgSurface = fimg->surface();
	tempSurf.create(fimgSurface->w, fimgSurface->h, fimgSurface->format);
	tempSurf.blitFrom(*fimgSurface);
	drawCountdown(&tempSurf);
	fimg->updateSurface(&tempSurf.rawSurface());

	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_currentZone == 2 && fimg->_usedObject &&
		        fimg->_usedObject->idOBJ() == 135) {
			// Unlock third line with key 3
			_gameVariables[GameVariables::kBombState] = 4;
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_88003e);
			fimg->changeCallback(functor);
			break;
		}
		if (countDown()) {
			// Countdown has changed: refresh surface
			drawCountdown(&tempSurf);
			fimg->updateSurface(&tempSurf.rawSurface());
		}
	}
}

IMG_CB(88003e) {
	fimg->load("70Z_14.GIF");

	// Draw countdown there and not in fixed image class directly
	Graphics::ManagedSurface tempSurf;
	const Graphics::Surface *fimgSurface = fimg->surface();
	tempSurf.create(fimgSurface->w, fimgSurface->h, fimgSurface->format);
	tempSurf.blitFrom(*fimgSurface);
	drawCountdown(&tempSurf);
	fimg->updateSurface(&tempSurf.rawSurface());

	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_currentZone == 3 && fimg->_usedObject &&
		        fimg->_usedObject->idOBJ() == 136) {
			// Unlock fourth line with key 4
			_gameVariables[GameVariables::kBombState] = 5;
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_88003f);
			fimg->changeCallback(functor);
			break;
		}
		if (countDown()) {
			// Countdown has changed: refresh surface
			drawCountdown(&tempSurf);
			fimg->updateSurface(&tempSurf.rawSurface());
		}
	}
}

IMG_CB(88003f) {
	fimg->load("70Z_15.GIF");

	// Draw countdown there and not in fixed image class directly
	Graphics::ManagedSurface tempSurf;
	const Graphics::Surface *fimgSurface = fimg->surface();
	tempSurf.create(fimgSurface->w, fimgSurface->h, fimgSurface->format);
	tempSurf.blitFrom(*fimgSurface);
	drawCountdown(&tempSurf);
	fimg->updateSurface(&tempSurf.rawSurface());

	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			if (handleBomb(fimg)) {
				playInGameVideo("COFFRE");
				_forcePaletteUpdate = true;
				// Force reload of the place
				if (_nextPlaceId == uint(-1)) {
					_nextPlaceId = _currentPlaceId;
				}
				playTransitionEndLevel(7);
				break;
			}
		}
		if (countDown()) {
			// Countdown has changed: refresh surface
			drawCountdown(&tempSurf);
			fimg->updateSurface(&tempSurf.rawSurface());
		}
	}
}

bool CryOmni3DEngine_Versailles::handleBomb(ZonFixedImage *fimg) {
	bool success = false;
	Common::RandomSource rnd("VersaillesBomb");
	Graphics::Surface bmpLetters[28];
	uint32 bombPossibilites[60][5];
	byte bombCurrentLetters[60];
	Graphics::ManagedSurface tempSurf;

	const uint bombPasswordLength = _bombPassword.size();
	if (bombPasswordLength >= kBombPasswordMaxLength) {
		error("Bomb password is too long");
	}

	uint max = _bombAlphabet.size() - 1;
	if (getLanguage() != Common::JA_JPN) {
		// In bitmap mode we only have 28 images
		assert(max < 28);
		// BUG: in game the rand is modulo 27
		max = 26;
		loadBMPs("bomb_%02d.bmp", bmpLetters, 28);
	}
	for (uint i = 0; i < bombPasswordLength; i++) {
		bombPossibilites[i][0] = _bombPassword[i];
		for (uint j = 1; j < 5; j++) {
			bool foundSameLetter;
			do {
				foundSameLetter = false;
				bombPossibilites[i][j] = _bombAlphabet[rnd.getRandomNumber(max)];
				for (uint k = 0; k < j; k++) {
					if (bombPossibilites[i][k] == bombPossibilites[i][j]) {
						foundSameLetter = true;
					}
				}
			} while (foundSameLetter);
		}
		bombCurrentLetters[i] = rnd.getRandomNumber(4);
	}

	if (bombPasswordLength <= kBombPasswordSmallLength) {
		fimg->load("70z_16.GIF");
	} else {
		fimg->load("70z_17.GIF");
	}
	const Graphics::Surface *fimgSurface = fimg->surface();
	tempSurf.create(fimgSurface->w, fimgSurface->h, fimgSurface->format);
	tempSurf.blitFrom(*fimgSurface);
	drawBombLetters(tempSurf, bmpLetters, bombPasswordLength, bombPossibilites, bombCurrentLetters);
	drawCountdown(&tempSurf);
	fimg->updateSurface(&tempSurf.rawSurface());

	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			break;
		}
		if (fimg->_zoneUse) {
			if (fimg->_currentZone < bombPasswordLength) {
				// Safe digit
				bombCurrentLetters[fimg->_currentZone] = (bombCurrentLetters[fimg->_currentZone] + 1) % 5;
				// Reset the surface and redraw letters on it
				tempSurf.blitFrom(*fimgSurface);
				drawBombLetters(tempSurf, bmpLetters, bombPasswordLength, bombPossibilites, bombCurrentLetters);
				drawCountdown(&tempSurf);
				fimg->updateSurface(&tempSurf.rawSurface());

				waitMouseRelease();

				// Check if password is OK
				success = true;
				for (uint i = 0; i < bombPasswordLength; i++) {
					uint16 letterId = bombPossibilites[i][bombCurrentLetters[i]];
					if (letterId != _bombPassword[i]) {
						success = false;
						break;
					}
				}
				if (success) {
					handleBombTranslation(tempSurf);
					break;
				}
			}
		}
		if (countDown()) {
			// Countdown has changed: refresh surface
			drawCountdown(&tempSurf);
			fimg->updateSurface(&tempSurf.rawSurface());
		}
	}

	for (uint i = 0; i < 28; i++) {
		bmpLetters[i].free();
	}
	return success;
}

void CryOmni3DEngine_Versailles::handleBombTranslation(Graphics::ManagedSurface &surface) {
	if (_messages.size() <= 150) {
		return;
	}

	Common::String &translation = _messages[150];

	if (translation.size() == 0) {
		return;
	}

	surface.fillRect(Common::Rect(0, 430, 640, 480), 247);

	_fontManager.setCurrentFont(1);
	_fontManager.setTransparentBackground(true);
	_fontManager.setForeColor(242);
	_fontManager.setSurface(&surface);
	uint w = _fontManager.getStrWidth(translation);
	_fontManager.displayStr((640 - w) / 2, 440, translation);

	g_system->copyRectToScreen(surface.getPixels(), surface.pitch, 0, 0,
	                           surface.w, surface.h);
	g_system->updateScreen();

	uint32 end = g_system->getMillis() + 5000;
	bool exitImg = false;
	while (!shouldAbort() && !exitImg && g_system->getMillis() < end) {
		if (pollEvents()) {
			if (checkKeysPressed() || getCurrentMouseButton() == 1) {
				exitImg = true;
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

const uint16 CryOmni3DEngine_Versailles::kBombLettersPos[2][kBombPasswordMaxLength][2] = {
	{
		{26, 91},
		{84, 89},
		{141, 89},
		{202, 88},
		{261, 87},
		{322, 86},
		{384, 85},
		{448, 84},
		{512, 83},
		{576, 83},
		{26, 175},
		{84, 175},
		{142, 174},
		{202, 174},
		{260, 174},
		{322, 174},
		{384, 173},
		{448, 173},
		{512, 173},
		{576, 172},
		{26, 261},
		{84, 261},
		{141, 261},
		{202, 261},
		{261, 262},
		{322, 262},
		{383, 262},
		{447, 263},
		{512, 263},
		{576, 263},
		{26, 344},
		{84, 345},
		{142, 346},
		{202, 347},
		{260, 348},
		{322, 349},
		{384, 350},
		{448, 351},
		{512, 352},
		{576, 352}
	},
	{
		{42, 26},
		{100, 24},
		{155, 22},
		{214, 19},
		{271, 18},
		{330, 15},
		{389, 14},
		{451, 11},
		{515, 8},
		{576, 6},
		{45, 102},
		{100, 102},
		{156, 101},
		{215, 100},
		{272, 99},
		{331, 98},
		{391, 97},
		{453, 96},
		{515, 94},
		{578, 94},
		{44, 184},
		{101, 184},
		{157, 184},
		{215, 183},
		{272, 183},
		{331, 183},
		{391, 182},
		{453, 182},
		{515, 182},
		{577, 181},
		{44, 267},
		{101, 267},
		{157, 267},
		{215, 268},
		{272, 268},
		{331, 268},
		{390, 269},
		{453, 269},
		{515, 269},
		{578, 269},
		{45, 348},
		{101, 349},
		{156, 349},
		{215, 351},
		{271, 351},
		{331, 351},
		{391, 353},
		{453, 354},
		{515, 355},
		{577, 356},
		{44, 434},
		{101, 435},
		{156, 436},
		{215, 437},
		{272, 437},
		{331, 437},
		{391, 439},
		{453, 440},
		{515, 441},
		{578, 442}
	},
};

void CryOmni3DEngine_Versailles::drawBombLetters(Graphics::ManagedSurface &surface,
        const Graphics::Surface(&bmpLetters)[28], const uint bombPasswordLength,
        const uint32(&bombPossibilites)[kBombPasswordMaxLength][5],
        const byte(&bombCurrentLetters)[kBombPasswordMaxLength]) {
	uint table = bombPasswordLength <= kBombPasswordSmallLength ? 0 : 1;
	if (getLanguage() == Common::JA_JPN) {
		_fontManager.setCurrentFont(1);
		_fontManager.setTransparentBackground(true);
		_fontManager.setForeColor(0);
		_fontManager.setSurface(&surface);

		for (uint i = 0; i < bombPasswordLength; i++) {
			Common::Rect rct(34, 34);
			rct.moveTo(kBombLettersPos[table][i][0], kBombLettersPos[table][i][1]);
			surface.fillRect(rct, 239);

			uint32 letter = bombPossibilites[i][bombCurrentLetters[i]];
			Common::U32String str(&letter, 1);

			_fontManager.displayStr(rct.left + (34 - _fontManager.getStrWidth(str)) / 2,
			                        rct.top + 5, str);
		}
	} else {
		for (uint i = 0; i < bombPasswordLength; i++) {
			uint letterId = _bombAlphabet.find(bombPossibilites[i][bombCurrentLetters[i]]);
			const Graphics::Surface &letter = bmpLetters[letterId];
			Common::Point dst(kBombLettersPos[table][i][0], kBombLettersPos[table][i][1]);
			surface.transBlitFrom(letter, dst);
		}
	}
}

IMG_CB(88004) {
	fimg->load("31j31.gif");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_88004b);
			fimg->changeCallback(functor);
			break;
		}
	}
}

IMG_CB(88004b) {
	// Open the toilets
	playInGameVideo("31j32");
	// Force reload of the place
	if (_nextPlaceId == uint(-1)) {
		_nextPlaceId = _currentPlaceId;
	}
	fimg->load("31j32.gif");
	while (1) {
		fimg->manage();
		if (fimg->_exit || fimg->_zoneLow) {
			fimg->_exit = true;
			break;
		}
		if (fimg->_zoneUse) {
			ZonFixedImage::CallbackFunctor *functor =
			    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this,
			            &CryOmni3DEngine_Versailles::img_88004);
			fimg->changeCallback(functor);
			break;
		}
	}
	if (!shouldAbort()) {
		// Close the toilets
		playInGameVideo("31j32b");
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}
	}
}

#undef IMG_CB

// Init place and filter event
#define FILTER_EVENT(level, place) bool CryOmni3DEngine_Versailles::filterEventLevel ## level ## Place ## place(uint *event)
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
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}
		return false;
	}

	return true;
}

INIT_PLACE(1, 3) {
	if (!_gameVariables[GameVariables::kHasPlayedLebrun]) {
		Common::File *audioFile = new Common::File();
		if (!audioFile->open(_localizedFilenames[LocalizedFilenames::kLeb001])) {
			warning("Failed to open sound file %s", _localizedFilenames[LocalizedFilenames::kLeb001].c_str());
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
		while (!shouldAbort() && _mixer->isSoundIDActive(SoundIds::kLeb001)) {
			g_system->updateScreen();
			g_system->delayMillis(10);
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
		uint fakePlaceId = getFakeTransition(*event);
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
		} else {
			error("BUG: Shouldn't be here");
		}
	} else if (_currentLevel == 2 || _placeStates[14].state == 1) {
		if (*event == 31142) {
			video = "11D2_2";
			callback = &CryOmni3DEngine_Versailles::img_31142b;
		} else if (*event == 31143) {
			video = "11D2_1";
			callback = &CryOmni3DEngine_Versailles::img_31143b;
		} else {
			error("BUG: Shouldn't be here");
		}
	} else {
		error("Invalid state in filter event 1/14: level: %d/ placeState: %d", _currentLevel,
		      _placeStates[14].state);
	}

	uint fakePlaceId = getFakeTransition(*event);
	fakeTransition(fakePlaceId);

	playInGameVideo(video);

	// Force reload of the place
	if (_nextPlaceId == uint(-1)) {
		_nextPlaceId = _currentPlaceId;
	}

	handleFixedImg(callback);

	// Don't pass the event as we try to avoid implementing use
	return false;
}

FILTER_EVENT(2, 1) {
	if (*event == 22101 && _inventory.selectedObject()) {
		_dialogsMan["{JOUEUR-MONTRE-UN-PAMPHLET}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-PAPIER-ECRIT-ENCRE-SYMPATHIQUE}"] = 'N';
		uint idOBJ = _inventory.selectedObject()->idOBJ();
		if (idOBJ == 96  || idOBJ == 101 || idOBJ == 115 ||
		        idOBJ == 125 || idOBJ == 127) {
			_dialogsMan["{JOUEUR-MONTRE-UN-PAMPHLET}"] = 'Y';
		} else if (idOBJ == 98) {
			_dialogsMan["{JOUEUR-MONTRE-PAPIER-ECRIT-ENCRE-SYMPATHIQUE}"] = 'Y';
		} else {
			_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'Y';
		}
		_dialogsMan.play("21F_BON");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-MONTRE-UN-PAMPHLET}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-PAPIER-ECRIT-ENCRE-SYMPATHIQUE}"] = 'N';

		if (idOBJ == 98 && _dialogsMan["JOUEUR-CONFIE-MESSAGE-HUISSIER"] == 'Y') {
			_inventory.removeByNameID(98);
			setGameTime(2, 2);
		}
		_inventory.deselectObject();
	} else if (*event == 31101) {
		if (!_inventory.selectedObject() && currentGameTime() > 1) {
			const char *video;
			FixedImgCallback callback;

			if (!_gameVariables[GameVariables::kCollectKey]) {
				video = "21F_11";
				callback = &CryOmni3DEngine_Versailles::img_31101;
			} else {
				video = "21F_10";
				callback = &CryOmni3DEngine_Versailles::img_31101b;
			}

			playInGameVideo(video);

			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}

			handleFixedImg(callback);
		}
		// Don't pass the event as we try to avoid implementing use
		return false;
	} else if (*event >= 1 && *event <= 9999 && currentGameTime() == 2) {
		setPlaceState(1, 1);
	}
	return true;
}

FILTER_EVENT(2, 2) {
	if (*event < 32201 || *event > 32204) {
		// Not handled here
		return true;
	}

	const char *video = nullptr;
	FixedImgCallback callback = nullptr;

	const Object *obj = _inventory.selectedObject();
	bool deselectObj = false;

	if (*event == 32201) {
		if (!obj) {
			// Opening left drawer
			video = "21E_41";
			callback = &CryOmni3DEngine_Versailles::img_32201;
		} else {
			return false;
		}
	} else if (*event == 32202) {
		if (obj && obj->idOBJ() == 104) {
			// Using key on left door
			video = "21E_42";
			callback = &CryOmni3DEngine_Versailles::img_32202;
		} else {
			// This door is locked
			displayMessageBoxWarp(1);
			return false;
		}
	} else if (*event == 32203) {
		if (!obj) {
			// Opening right drawer
			video = "21E_43";
			callback = &CryOmni3DEngine_Versailles::img_32203;
		} else {
			return false;
		}
	} else if (*event == 32204) {
		if (obj && obj->idOBJ() == 104) {
			// Using key on right door
			if (_gameVariables[GameVariables::kCollectPortfolio]) {
				video = "21E_45";
				callback = &CryOmni3DEngine_Versailles::img_32204b;
			} else {
				video = "21E_44";
				callback = &CryOmni3DEngine_Versailles::img_32204;
			}
			deselectObj = true;
		} else {
			// This door is locked
			displayMessageBoxWarp(1);
			return false;
		}
	}

	assert(video != nullptr);
	assert(callback != nullptr);

	// Adjust viewpoint for video
	uint fakePlaceId = getFakeTransition(*event);
	fakeTransition(fakePlaceId);

	playInGameVideo(video);

	// Force reload of the place
	if (_nextPlaceId == uint(-1)) {
		_nextPlaceId = _currentPlaceId;
	}

	handleFixedImg(callback);

	if (deselectObj) {
		_inventory.deselectObject();
	}

	// Don't pass the event: it has been handled
	return false;
}

FILTER_EVENT(2, 5) {
	if (*event == 22501 && _inventory.selectedObject()) {
		uint idOBJ = _inventory.selectedObject()->idOBJ();
		if (idOBJ == 96) {
			if (!_inventory.inInventoryByNameID(101)) {
				_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-ARTS}"] = 'Y';
			}
		} else {
			_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-CHOSE}"] = 'Y';
			_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'Y';
		}

		_dialogsMan.play("21B1_HUI");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-ARTS}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-CHOSE}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'N';

		_inventory.deselectObject();
	} else if (*event >= 1 && *event <= 9999 &&
	           _inventory.inInventoryByNameID(96) && !_inventory.inInventoryByNameID(101)) {
		// Give your clues at the bailiff
		displayMessageBoxWarp(15);
		return false;
	}
	return true;
}

INIT_PLACE(2, 9) {
	// BUG: This dialog gets played twice when Monseigneur is waiting for the sketches and we speak to him
	// The bug is in original version too
	if (_gameVariables[GameVariables::kSketchState] == 1 && currentGameTime() == 2) {
		// Sketches are not yet sorted
		_dialogsMan["{JOUEUR-SE-DIRIGE-VERS-MONSEIGNEUR-AVEC-ESQUISSES}"] = 'Y';

		_dialogsMan.play("22G_DAU");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		// Change warp viewpoint
		_omni3dMan.setAlpha(4.17);
		_omni3dMan.setBeta(0.097);

		_inventory.deselectObject();

		_dialogsMan["{JOUEUR-SE-DIRIGE-VERS-MONSEIGNEUR-AVEC-ESQUISSES}"] = 'N';
	}
}

FILTER_EVENT(2, 9) {
	if (*event == 22902 && _inventory.selectedObject() &&
	        _inventory.selectedObject()->idOBJ() == 105) {
		_dialogsMan["{JOUEUR-DONNE-ESQUISSES}"] = 'Y';

		_dialogsMan.setIgnoreNoEndOfConversation(true);
		_dialogsMan.play("22G_DAU");
		_dialogsMan.setIgnoreNoEndOfConversation(false);

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-DONNE-ESQUISSES}"] = 'N';

		_inventory.deselectObject();
	} else if (*event >= 1 && *event <= 9999 && currentGameTime() == 3 &&
	           _placeStates[9].state != 2) {
		setPlaceState(9, 2);
	}
	return true;
}

FILTER_EVENT(2, 11) {
	if (*event == 22111 && _inventory.selectedObject()) {
		bool gameOver = false;
		uint idOBJ = _inventory.selectedObject()->idOBJ();
		if (idOBJ == 107) {
			_dialogsMan["{JOUEUR-MONTRE-TITRE-FABLE-APPARU-SUR-ESQUISSE}"] = 'Y';
		} else if (idOBJ == 109) {
			_dialogsMan["{JOUEUR-MONTRE-ESQUISSE-DETRUITE}"] = 'Y';
			gameOver = true;
		}

		_dialogsMan.play("24Z_BON");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-MONTRE-TITRE-FABLE-APPARU-SUR-ESQUISSE}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-ESQUISSE-DETRUITE}"] = 'N';

		_inventory.deselectObject();

		if (gameOver) {
			doGameOver();
		}
	}
	return true;
}

FILTER_EVENT(2, 12) {
	if (*event == 22121 && _inventory.selectedObject()) {
		uint idOBJ = _inventory.selectedObject()->idOBJ();
		if (idOBJ == 105) {
			_dialogsMan["{LE JOUEUR-PRESENTE-AUTRES-ESQUISSES-OU-ESQUISSE-NON-TRIEES}"] = 'Y';
			_dialogsMan["{JOUEUR-A-MONTRE-ESQUISSES-NON-TRIEES-LEBRUN}"] = 'Y';
		} else if (idOBJ == 106) {
			_dialogsMan["{LE JOUEUR-PRESENTE-ESQUISSES-TRIEES}"] = 'Y';
			_inventory.removeByNameID(106);
		} else if (idOBJ == 107 && _gameVariables[GameVariables::kSketchState] == 2) {
			if (_gameVariables[GameVariables::kFakeSketchChatState] == 0) {
				_dialogsMan["{JOUEUR-PRESENTE-FAUX-CROQUIS}"] = 'Y';
				_gameVariables[GameVariables::kFakeSketchChatState] = 1;
			} else if (_gameVariables[GameVariables::kFakeSketchChatState] == 1) {
				_dialogsMan["{JOUEUR-PRESENTE-FAUX-CROQUIS2}"] = 'Y';
				_gameVariables[GameVariables::kFakeSketchChatState] = 2;
			} else if (_gameVariables[GameVariables::kFakeSketchChatState] == 2) {
				_dialogsMan["{JOUEUR-PRESENTE-FAUX-CROQUIS3}"] = 'Y';
			}
		} else if (idOBJ == 96) {
			_dialogsMan["{JOUEUR-PRESENTE-PAMPHLET-SUR-LEBRUN}"] = 'Y';
		} else {
			_dialogsMan["{JOUEUR-PRESENTE-TOUT-AUTRE-PAMPHLET-OU-LETTRE}"] = 'Y';
		}

		_dialogsMan.play("23I_LEB");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-PRESENTE-PAMPHLET-SUR-LEBRUN}"] = 'N';
		_dialogsMan["{LE JOUEUR-PRESENTE-AUTRES-ESQUISSES-OU-ESQUISSE-NON-TRIEES}"] = 'N';
		_dialogsMan["{LE JOUEUR-PRESENTE-ESQUISSES-TRIEES}"] = 'N';
		_dialogsMan["{JOUEUR-PRESENTE-FAUX-CROQUIS}"] = 'N';
		_dialogsMan["{JOUEUR-PRESENTE-FAUX-CROQUIS2}"] = 'N';
		_dialogsMan["{JOUEUR-PRESENTE-FAUX-CROQUIS3}"] = 'N';
		_dialogsMan["{JOUEUR-PRESENTE-TOUT-AUTRE-PAMPHLET-OU-LETTRE}"] = 'N';

		_inventory.deselectObject();
	} else if (*event == 32120) {
		if (_inventory.selectedObject() &&
		        _inventory.selectedObject()->idOBJ() == 107 &&
		        _gameVariables[GameVariables::kSketchState] == 2) {
			handleFixedImg(&CryOmni3DEngine_Versailles::img_32120);
		}
		// We handle use here
		return false;
	}
	return true;
}

FILTER_EVENT(2, 14) {
	return filterEventLevel1Place14(event);
}

FILTER_EVENT(3, 3) {
	if (*event == 23030 && _inventory.selectedObject() &&
	        _inventory.selectedObject()->idOBJ() == 118 &&
	        _gameVariables[GameVariables::kDecipherScore]) {
		_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-DECHIFFRE-PAR-LULLY}"] = 'Y';
		_dialogsMan.play("31X_BON");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-DECHIFFRE-PAR-LULLY}"] = 'N';
		_inventory.deselectObject();
	}
	return true;
}

FILTER_EVENT(3, 10) {
	if (*event == 23101 && _inventory.selectedObject() &&
	        _inventory.selectedObject()->idOBJ() == 120) {
		_inventory.removeByNameID(120);

		_dialogsMan["{JOUEUR-MONTRE-AUTORISATION-DE-BONTEMPS}"] = 'Y';
		_dialogsMan.play("31O_SUIP");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-MONTRE-AUTORISATION-DE-BONTEMPS}"] = 'N';
		_inventory.deselectObject();
		return true;
	} else if (*event == 21) {
		if (_dialogsMan["SUISSE-VU-AUTORISATION"] == 'Y') {
			fakeTransition(*event);
			playInGameVideo("33O_SUIP");
			playInGameVideo("33O_P");
			executeSeeAction(88001);
			if (!shouldAbort()) {
				playInGameVideo("33P_O");
			}
			_forcePaletteUpdate = true;
			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}
		}
		// This place is a fake one: so never go in there
		return false;
	}
	return true;
}

FILTER_EVENT(3, 13) {
	_dialogsMan["{JOUEUR-MONTRE-FUSAIN-MEDAILLES}"] = 'N';
	_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'N';
	if (*event == 33130 && !_inventory.inInventoryByNameID(119)) {
		collectObject(119);
		if (_placeStates[13].state) {
			setPlaceState(13, 3);
		} else {
			setPlaceState(13, 1);
		}
		// We handle use here
		return false;
	} else if (*event == 23131 && _inventory.selectedObject()) {
		if (_inventory.selectedObject()->idOBJ() == 121) {
			_dialogsMan["{JOUEUR-MONTRE-FUSAIN-MEDAILLES}"] = 'Y';
		} else {
			_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'Y';
		}
		_dialogsMan.play("32M_MR");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-MONTRE-FUSAIN-MEDAILLES}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'N';
		_inventory.deselectObject();
		return true;
	} else {
		return true;
	}
}

FILTER_EVENT(3, 15) {
	if (*event == 23151 && _inventory.selectedObject()) {
		return filterEventLevel3Obj23151();
	}
	return true;
}

FILTER_EVENT(3, 17) {
	if (*event == 18) {
		if (_inventory.selectedObject() &&
		        _inventory.selectedObject()->idOBJ() == 123) {
			_gameVariables[GameVariables::kUnlockHiddenDoor] = 1;
			_inventory.removeByNameID(123);
			return true;
		} else if (_gameVariables[GameVariables::kUnlockHiddenDoor] != 1) {
			// Locked
			displayMessageBoxWarp(1);
			_dialogsMan["{LE JOUEUR-A-TENTE-OUVRIR-PETITE-PORTE}"] = 'Y';
			return false;
		} else {
			return true;
		}
	} else if (*event == 23151) {
		return filterEventLevel3Obj23151();
	} else {
		return true;
	}
}

FILTER_EVENT(3, 18) {
	if (*event != 19) {
		return true;
	}

	// Only take care of event 19

	// Adjust camera
	fakeTransition(*event);
	// As we have just adjusted camera, don't do it later
	_transitionAnimateWarp = false;

	if (_placeStates[22].state) {
		playInGameVideo("31J1_L2");
	} else if (_gameVariables[GameVariables::kAlreadyWent3_19]) {
		playInGameVideo("31J1_L1");
	} else {
		playInGameVideo("31J1_L0");
		playInGameVideo("31L1_AL2");
		playInGameVideo("31L1_AL3");
		_gameVariables[GameVariables::kAlreadyWent3_19] = 1;
		_gameVariables[GameVariables::kCabinetDrawerStatus] = 1;
	}

	_forcePaletteUpdate = true;
	// Force reload of the place
	if (_nextPlaceId == uint(-1)) {
		_nextPlaceId = _currentPlaceId;
	}
	return true;
}

FILTER_EVENT(3, 19) {
	if (*event != 18) {
		return true;
	}
	if (currentGameTime() != 3 || _placeStates[22].state) {
		return true;
	}

	if (_gameVariables[GameVariables::kCollectLampoonArchitecture]) {
		setPlaceState(22, 2);
	} else {
		setPlaceState(22, 1);
	}
	setPlaceState(19, 1);

	return true;
}

FILTER_EVENT(3_5, 20) {
	if (*event != 25) {
		return true;
	}

	fakeTransition(*event);
	playInGameVideo("31j31");

	// Force reload of the place
	if (_nextPlaceId == uint(-1)) {
		_nextPlaceId = _currentPlaceId;
	}

	// Toilets
	executeSeeAction(88004);

	_forcePaletteUpdate = true;

	return false;
}

FILTER_EVENT(3, 22) {
	if (*event == 33220) {
		if (!_gameVariables[GameVariables::kCollectLampoonArchitecture]) {
			if (_inventory.selectedObject() &&
			        _inventory.selectedObject()->idOBJ() == 119) {
				// Using pool cue
				_inventory.removeByNameID(119);
				collectLampoonArchitecture();
				_forcePaletteUpdate = true;
			} else {
				// Paper is out of reach
				displayMessageBoxWarp(16);
			}
		}
		// We handle use here
		return false;
	}

	if (*event >= 20000 && *event < 30000 &&
	        _inventory.selectedObject() &&
	        _inventory.selectedObject()->idOBJ() == 118) {
		_dialogsMan["{JOUEUR-PRESENTE-PAMPHLET-PARTITION}"] = 'Y';
		_dialogsMan.play("31L1_LUL");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-PRESENTE-PAMPHLET-PARTITION}"] = 'N';
		if (_dialogsMan["LULLY-DONNE-MISSION1-JOUEUR"] != 'Y' ||
		        _gameVariables[GameVariables::kDecipherScore]) {
			_inventory.deselectObject();
		} else {
			_inventory.removeByNameID(118);
		}
	}
	return true;
}

FILTER_EVENT(3, 23) {
	if (*event != 32) {
		return true;
	}

	if (_inventory.selectedObject() &&
	        _inventory.selectedObject()->idOBJ() == 140) {
		_gameVariables[GameVariables::kUnlockedAttic] = 1;
		_inventory.removeByNameID(140);
		return true;
	} else if (_gameVariables[GameVariables::kUnlockedAttic] != 1) {
		// Locked
		displayMessageBoxWarp(1);
		return false;
	} else {
		return true;
	}
}

bool CryOmni3DEngine_Versailles::filterEventLevel3Obj23151() {
	if (_inventory.selectedObject() &&
	        _inventory.selectedObject()->idOBJ() == 115) {
		_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-ARCHITECTURE}"] = 'Y';
	} else if (_inventory.selectedObject() &&
	           _inventory.selectedObject()->idOBJ() == 121 &&
	           _gameVariables[GameVariables::kGotMedalsSolution]) {
		_inventory.removeByNameID(121);
		_dialogsMan["{JOUEUR-MONTRE-EPIGRAPHE-MEDAILLES}"] = 'Y';
	} else {
		_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-CHOSE}"] = 'Y';
	}

	_dialogsMan.play("32J_CRO");

	_forcePaletteUpdate = true;
	// Force reload of the place
	if (_nextPlaceId == uint(-1)) {
		_nextPlaceId = _currentPlaceId;
	}

	_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-ARCHITECTURE}"] = 'N';
	_dialogsMan["{JOUEUR-MONTRE-EPIGRAPHE-MEDAILLES}"] = 'N';
	_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-CHOSE}"] = 'N';

	_inventory.deselectObject();

	return true;
}

void CryOmni3DEngine_Versailles::collectLampoonArchitecture(const ZonFixedImage *fimg) {
	_gameVariables[GameVariables::kCollectLampoonArchitecture] = 1;
	collectObject(115, fimg);
	if (_currentLevel == 3) {
		setPlaceState(22, 2);
	}
	_dialogsMan["{JOUEUR_POSSEDE_PAMPHLET_ARCHI}"] = 'Y';
}

INIT_PLACE(4, 9) {
	if (currentGameTime() == 4 && !_inventory.inInventoryByNameID(125)) {
		_dialogsMan.play("4_MAI");
		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}
	}
}

FILTER_EVENT(4, 10) {
	if (*event == 24104 && _inventory.selectedObject()) {
		_dialogsMan["{JOUEUR-PRESENTE-OBJET-HUISSIER}"] = 'Y';
		_dialogsMan.play("41C_HUI");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-PRESENTE-OBJET-HUISSIER}"] = 'N';
		_inventory.deselectObject();
		return true;
	} else if (*event == 24105 && _inventory.selectedObject()) {
		if (_inventory.selectedObject()->idOBJ() == 127) {
			_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-GOUVERNEMENT}"] = 'Y';
		} else if (_inventory.selectedObject()->idOBJ() == 125) {
			_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-RELIGION}"] = 'Y';
		} else if (_inventory.selectedObject()->idOBJ() == 126) {
			_dialogsMan["{JOUEUR-MONTRE-PAPIER-CROISSY}"] = 'Y';
		} else {
			_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'Y';
		}
		_dialogsMan.play("42C_BON");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-GOUVERNEMENT}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-RELIGION}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-PAPIER-CROISSY}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'N';
		_inventory.deselectObject();
		return true;
	} else if (*event == 11 && currentGameTime() < 3) {
		// Closed
		displayMessageBoxWarp(2);
		return false;
	} else {
		return true;
	}
}

FILTER_EVENT(4, 12_13_14) {
	if (*event != 34131 && *event != 34132) {
		// Not for us
		return true;
	}

	if (!_inventory.selectedObject() ||
	        _inventory.selectedObject()->idOBJ() != 130) {
		// Not using scope: do nothing
		return false;
	}

	// Using scope
	const char *video;
	FixedImgCallback callback;

	if (*event == 34131) {
		video = "43ZA_1";
		callback = &CryOmni3DEngine_Versailles::img_34131;
	} else if (*event == 34132) {
		video = "43ZB_2";
		callback = &CryOmni3DEngine_Versailles::img_34132;
	} else {
		error("BUG: Shouldn't be here");
	}

	playInGameVideo(video);

	// Force reload of the place
	if (_nextPlaceId == uint(-1)) {
		_nextPlaceId = _currentPlaceId;
	}

	handleFixedImg(callback);

	// Don't pass the event: it has been handled
	return false;
}

FILTER_EVENT(4, 15) {
	if (*event == 17 && (_dialogsMan["BONTEMPS-VU-PAPIER-CROISSY"] == 'N' ||
	                     _dialogsMan["BONTEMPS-VU-PAMPHLET-GOUVERNEMENT"] == 'N')) {
		// Closed
		displayMessageBoxWarp(2);
		return false;
	}

	return true;
}

FILTER_EVENT(4, 16) {
	if (*event == 24161 && _inventory.selectedObject()) {
		uint idOBJ = _inventory.selectedObject()->idOBJ();
		if (idOBJ == 124) {
			_dialogsMan["{JOUEUR-DONNE-REPAS}"] = 'Y';
		} else {
			_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'Y';
		}
		_dialogsMan.play("41X2_CRO");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-DONNE-REPAS}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'N';

		if (idOBJ == 124) {
			_inventory.removeByNameID(124);
			playInGameVideo("41X2_CR1");
			setGameTime(2, 4);
		}
		_inventory.deselectObject();
		return true;
	} else if (*event == 34162) {
		if (!_inventory.inInventoryByNameID(127)) {
			collectObject(127);
			_forcePaletteUpdate = true;
		} else {
			// Nothing there anymore
			displayMessageBoxWarp(21);
		}

		// Don't pass the event: it has been handled
		return false;
	}
	return true;
}

FILTER_EVENT(4, 17) {
	if (*event == 34171) {
		collectObject(130);
		setPlaceState(17, 1);
		return false;
	} else if (*event == 34172) {
		uint fakePlaceId = getFakeTransition(*event);
		fakeTransition(fakePlaceId);
		handleFixedImg(&CryOmni3DEngine_Versailles::img_34172);
		return false;
	} else if (*event == 34173) {
		handleFixedImg(&CryOmni3DEngine_Versailles::img_34173);
		return false;
	} else if (*event == 34174) {
		handleFixedImg(&CryOmni3DEngine_Versailles::img_34174);
		return false;
	}

	return true;
}

INIT_PLACE(5, 6) {
	if (currentGameTime() == 2) {
		setPlaceState(27, 2);
	}
}

FILTER_EVENT(5, 9) {
	if (*event == 25090 && _inventory.selectedObject()) {
		uint idOBJ = _inventory.selectedObject()->idOBJ();
		if (currentGameTime() < 4) {
			if (idOBJ == 125 && _gameVariables[GameVariables::kStateLampoonReligion] == 3) {
				_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-RELIGION}"] = 'Y';
			} else if (idOBJ == 115) {
				_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-ARCHITECTURE}"] = 'Y';
			} else {
				_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'Y';
			}

			_dialogsMan.play("53N_BON");

			_forcePaletteUpdate = true;
			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}

			if (_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-RELIGION}"] == 'Y' && currentGameTime() != 3) {
				setGameTime(3, 5);
				_inventory.removeByNameID(125);
			}

			_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-RELIGION}"] = 'N';
			_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-ARCHITECTURE}"] = 'N';
			_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'N';

			_inventory.deselectObject();
		} else {
			if (_inventory.inInventoryByNameID(135) && _inventory.inInventoryByNameID(116)) {
				_dialogsMan["{JOUEUR-POSSEDE-CLEF-3-ET-4}"] = 'Y';
			}
			// Useless?
			_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'N';

			if (idOBJ == 137) {
				_dialogsMan["{JOUEUR-MONTRE-MEMORANDUM}"] = 'Y';
			} else {
				_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'Y';
			}

			_dialogsMan.play("54I_BON");

			_forcePaletteUpdate = true;
			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}

			_dialogsMan["{JOUEUR-MONTRE-MEMORANDUM}"] = 'N';
			_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'N';

			_inventory.deselectObject();
		}
	}

	return true;
}

FILTER_EVENT(5, 14) {
	if (*event == 25142 && _inventory.selectedObject()) {
		uint idOBJ = _inventory.selectedObject()->idOBJ();
		if (idOBJ == 125) {
			_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-RELIGION}"] = 'Y';
		} else {
			_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'Y';
		}

		_dialogsMan.play("52L_BOU");

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-RELIGION}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}"] = 'N';
		_inventory.deselectObject();
	}

	return true;
}

FILTER_EVENT(5, 15) {
	if (*event == 16 && _gameVariables[GameVariables::kLoweredChandelier]) {
		*event = 29;
	}

	return true;
}

FILTER_EVENT(5, 16) {
	if (*event == 35162) {
		// Don't move the ladder when there is a guard
		if (_placeStates[16].state != 0) {
			// Take back the ladder from the scaffolding
			if (_gameVariables[GameVariables::kLadderState] == 2) {
				collectObject(108);
				_gameVariables[GameVariables::kLadderState] = 1;
				filterEventLevel5UpdatePlaceStates();
			}
		}
		// Handled here
		return false;
	} else if (*event == 35160) {
		// Don't move the ladder when there is a guard
		if (_placeStates[16].state != 0) {
			if (_gameVariables[GameVariables::kLadderState] == 0) {
				// Take the ladder from the curtain
				collectObject(108);
				_gameVariables[GameVariables::kLadderState] = 1;
				filterEventLevel5UpdatePlaceStates();
			} else if (_gameVariables[GameVariables::kLadderState] == 1 &&
			           _inventory.selectedObject() &&
			           _inventory.selectedObject()->idOBJ() == 108) {
				// Put back the ladder
				_inventory.removeByNameID(108);
				_gameVariables[GameVariables::kLadderState] = 0;
				filterEventLevel5UpdatePlaceStates();
			}
		}
		// Handled here
		return false;
	} else if (*event == 35161) {
		// Don't move the ladder when there is a guard
		if (_placeStates[16].state != 0) {
			if (!_gameVariables[GameVariables::kOpenedCurtain] &&
			        _inventory.selectedObject() &&
			        _inventory.selectedObject()->idOBJ() == 133) {
				// Try to open the curtain
				if (_gameVariables[GameVariables::kLadderState]) {
					// Ladder is not near the curtain
					// Cannot reach the covering
					displayMessageBoxWarp(4);
				} else {
					_inventory.removeByNameID(133);
					_gameVariables[GameVariables::kOpenedCurtain] = 1;
					filterEventLevel5UpdatePlaceStates();
				}
			}
		}
		// Handled here
		return false;
	} else if (*event == 28) {
		// Try to go to scaffolding
		if (_gameVariables[GameVariables::kLadderState] == 1 &&
		        _inventory.selectedObject() &&
		        _inventory.selectedObject()->idOBJ() == 108) {
			// Put the ladder on the scaffolding
			_inventory.removeByNameID(108);
			_gameVariables[GameVariables::kLadderState] = 2;
			filterEventLevel5UpdatePlaceStates();
		}
		// Don't move if there is no ladder on the scaffolding
		// Don't take selected object into account
		return _gameVariables[GameVariables::kLadderState] == 2;
	} else if (*event == 15 && _inventory.inInventoryByNameID(108)) {
		// Cannot move carrying ladder
		displayMessageBoxWarp(20);
		return false;
	} else {
		return true;
	}
}

void CryOmni3DEngine_Versailles::filterEventLevel5UpdatePlaceStates() {
	// Place 28 (mirror of 16 with chandelier on the floor) only depends on curtain state
	setPlaceState(28, _gameVariables[GameVariables::kOpenedCurtain]);
	if (!_gameVariables[GameVariables::kOpenedCurtain]) {
		// Curtain is closed
		switch (_gameVariables[GameVariables::kLadderState]) {
		case 0:
			// Ladder is near the curtain
			setPlaceState(16, 2);
			break;
		case 1:
			// Ladder is with us
			setPlaceState(16, 4);
			break;
		case 2:
			// Ladder is on the scaffolding
			setPlaceState(16, 5);
			break;
		default:
			error("BUG: Invalid ladder state");
			break;
		}
	} else {
		// Curtain is opened
		switch (_gameVariables[GameVariables::kLadderState]) {
		case 0:
			// Ladder is near the curtain
			setPlaceState(16, 1);
			break;
		case 1:
			// Ladder is with us
			setPlaceState(16, 3);
			break;
		case 2:
			// Ladder is on the scaffolding
			setPlaceState(16, 6);
			break;
		default:
			error("BUG: Invalid ladder state");
			break;
		}
	}
}

FILTER_EVENT(5, 23) {
	if (*event != 32) {
		return true;
	}

	// Event 32 only
	// Try to open attic door
	if (_inventory.selectedObject() &&
	        _inventory.selectedObject()->idOBJ() == 140) {
		_gameVariables[GameVariables::kUnlockedAttic] = 1;
		_inventory.removeByNameID(140);
		return true;
	} else if (_gameVariables[GameVariables::kUnlockedAttic] != 1) {
		// Locked
		displayMessageBoxWarp(1);
		return false;
	} else {
		return true;
	}
}

FILTER_EVENT(5, 27) {
	if (*event == 25270) {
		if (_inventory.selectedObject()) {
			uint idOBJ = _inventory.selectedObject()->idOBJ();
			if (idOBJ == 115) {
				_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-ARCHITECTURE}"] = 'Y';
			} else if (idOBJ == 125) {
				_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-RELIGION}"] = 'Y';
			} else if (idOBJ == 134) {
				_dialogsMan["{JOUEUR-MONTRE-ECROUELLES}"] = 'Y';
			}

			_dialogsMan.play("52A4_LAC");

			_forcePaletteUpdate = true;
			// Force reload of the place
			if (_nextPlaceId == uint(-1)) {
				_nextPlaceId = _currentPlaceId;
			}

			_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-ARCHITECTURE}"] = 'N';
			_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-RELIGION}"] = 'N';
			_dialogsMan["{JOUEUR-MONTRE-ECROUELLES}"] = 'N';

			if (_dialogsMan["LACHAIZE-TROUVE-ECROUELLES"] == 'Y') {
				_inventory.removeByNameID(134);
			}
			_inventory.deselectObject();
		}
	} else if (*event == 35270) {
		if (!_inventory.inInventoryByNameID(133)) {
			collectObject(133);
			_gameVariables[GameVariables::kCollectCord] = 1;
			setPlaceState(27, 1);
		}
		// Handled here
		return false;
	} else if (*event > 0 && *event < 10000 && currentGameTime() == 1 &&
	           _gameVariables[GameVariables::kCollectCord]) {
		setGameTime(2, 5);
	}

	return true;
}

FILTER_EVENT(5, 28) {
	if (*event == 45280 && !_gameVariables[GameVariables::kOpenedCurtain]) {
		// Too dark
		displayMessageBoxWarp(7);
	}

	return true;
}

FILTER_EVENT(5, 29) {
	if (*event == 35290 && _placeStates[29].state == 0) {
		// Collect memorandum
		collectObject(137);
		setPlaceState(29, 1);
		// Handled here
		return false;
	}

	return true;
}

FILTER_EVENT(5, 33) {
	if (*event == 35330 && !_gameVariables[GameVariables::kLoweredChandelier]) {
		uint fakePlaceId = getFakeTransition(*event);
		fakeTransition(fakePlaceId);

		playInGameVideo("LUSTRE");
		// setPlaceState will force reload

		setPlaceState(33, 1);
		setGameTime(4, 5);

		_gameVariables[GameVariables::kLoweredChandelier] = 1;

		// Handled here
		return false;
	}

	return true;
}

FILTER_EVENT(5, 34) {
	if (*event == 35) {
		fakeTransition(*event);

		playInGameVideo("53z1c_10");

		executeSeeAction(88002);

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}
		// Handled here
		return false;
	}

	return true;
}

FILTER_EVENT(6, 1) {
	if (*event == 36010 && _placeStates[1].state == 0) {
		collectObject(144);
		setPlaceState(1, 1);
		// Handled here
		return false;
	}

	return true;
}

FILTER_EVENT(6, 3) {
	if (!filterEventLevel6PlaceOrangery(event)) {
		// Handled
		return false;
	} else if (*event == 36030 && _placeStates[3].state == 0) {
		collectObject(143);
		setPlaceState(3, 1);
		// Handled here
		return false;
	} else if (*event == 1) {
		// To apothecary
		displayMessageBoxWarp(17);
	}

	return true;
}

FILTER_EVENT(6, Orangery) {
	if (*event == 36000) {
		if (_inventory.selectedObject() &&
		        _inventory.selectedObject()->idOBJ() == 143) {
			_gameVariables[GameVariables::kCombedOrangeTree]++; // Not used afterwards
			displayMessageBoxWarp(5);
		}
		// Handled here
		return false;
	} else if (*event == 36001) {
		if (_inventory.selectedObject() &&
		        _inventory.selectedObject()->idOBJ() == 143) {
			displayMessageBoxWarp(6);
		}
		// Handled here
		return false;
	}

	return true;
}

FILTER_EVENT(6, 19) {
	if (*event == 26190 && _inventory.selectedObject() &&
	        _placeStates[19].state == 0) {
		if (!_gameVariables[GameVariables::kMaineTalked]) {
			if (_inventory.selectedObject()->idOBJ() == 144) {
				_dialogsMan["{JOUEUR-DONNE-AUTRE-MEDICAMENT}"] = 'Y';
			}

			_dialogsMan.play("61_DUC");
		} else {
			if (_inventory.selectedObject()->idOBJ() == 144) {
				_dialogsMan["{JOUEUR-DONNE-SIROP-DE-ROSE}"] = 'Y';
				_dialogsMan.setIgnoreNoEndOfConversation(true);
			}

			_dialogsMan.play("62_DUC");
			_dialogsMan.setIgnoreNoEndOfConversation(false);
		}

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}

		_dialogsMan["{JOUEUR-DONNE-AUTRE-MEDICAMENT}"] = 'N';
		_dialogsMan["{JOUEUR-DONNE-SIROP-DE-ROSE}"] = 'N';

		_inventory.deselectObject();
	} else if (*event > 0 && *event < 10000 && _dialogsMan["{DUC_MAIN_A_PARLE}"] == 'Y') {
		_gameVariables[GameVariables::kMaineTalked] = 1;
		_whoSpeaksWhere[PlaceActionKey(19, 16190)] = "62_DUC";
	} else if (*event == 36190 && _placeStates[19].state == 1) {
		collectObject(142);
		setGameTime(2, 6);
		// Handled here
		return false;
	}

	return true;
}

FILTER_EVENT(7, 2) {
	if (*event == 37021) {
		if (_inventory.selectedObject() &&
		        _inventory.selectedObject()->idOBJ() == 103) {
			// Light the candle
			_inventory.removeByNameID(103);
			collectObject(102);
		}
		// Handled here
		return false;
	} else if (*event == 37022) {
		if (!_inventory.inInventoryByNameID(97)) {
			collectObject(97);
			_inventory.deselectObject();
		} else {
			// Jar is empty
			displayMessageBoxWarp(11);
		}
		// Handled here
		return false;
	} else if (*event == 7) {
		// Stairs
		if (_gameVariables[GameVariables::kUsedLitCandle]) {
			return true;
		}

		if (_inventory.selectedObject() &&
		        _inventory.selectedObject()->idOBJ() == 102) {
			// Now you can go
			displayMessageBoxWarp(12);
			_inventory.removeByNameID(102);
			_inventory.deselectObject();
			// Save it for later
			_gameVariables[GameVariables::kUsedLitCandle] = 1;

			return true;
		}

		// Didn't used lit candle and not using it now: denied
		// Too dark
		displayMessageBoxWarp(7);
		return false;
	}

	return true;
}

FILTER_EVENT(7, 9) {
	if (*event == 37090) {
		if (_placeStates[9].state == 0) {
			// Get the candle snuffer
			collectObject(145);
			_inventory.deselectObject();
			setPlaceState(9, 1);
		}
		// Handled here
		return false;
	}

	return true;
}

FILTER_EVENT(7, 10_11_13) {
	if (*event == 37131) {
		if (_inventory.selectedObject() &&
		        _inventory.selectedObject()->idOBJ() == 143 &&
		        !_inventory.inInventoryByNameID(136)) {
			collectObject(136);
			// WORKAROUND: Deselect the tool
			_inventory.deselectObject();
		}
		// Handled here
		return false;
	} else if (*event == 37132) {
		if (_inventory.selectedObject() &&
		        _inventory.selectedObject()->idOBJ() == 143) {
			// Nothing in this orange tree
			displayMessageBoxWarp(5);
		}
		// Handled here
		return false;
	}

	return true;
}

FILTER_EVENT(7, 20) {
	if (*event == 21) {
		fakeTransition(*event);

		playInGameVideo("70z_10");

		executeSeeAction(88003);

		_forcePaletteUpdate = true;
		// Force reload of the place
		if (_nextPlaceId == uint(-1)) {
			_nextPlaceId = _currentPlaceId;
		}
		// Handled here
		return false;
	}

	return true;
}

#undef FILTER_EVENT
#undef INIT_PLACE

// Countdown

void CryOmni3DEngine_Versailles::initCountdown() {
	strcpy(_countdownValue, "05:00");
	if (_gameVariables[GameVariables::kSavedCountdown]) {
		uint counter = _gameVariables[GameVariables::kSavedCountdown];
		_countdownValue[4] = counter;
		counter >>= 8;
		_countdownValue[3] = counter;
		counter >>= 8;
		_countdownValue[1] = counter;
		counter >>= 8;
		_countdownValue[0] = counter;
	}
}

void CryOmni3DEngine_Versailles::syncCountdown() {
	uint counter = 0;
	counter |= _countdownValue[0];
	counter <<= 8;
	counter |= _countdownValue[1];
	counter <<= 8;
	counter |= _countdownValue[3];
	counter <<= 8;
	counter |= _countdownValue[4];
	_gameVariables[GameVariables::kSavedCountdown] = counter;
}

bool CryOmni3DEngine_Versailles::doCountDown() {
	if (g_system->getMillis() > _countdownNextEvent) {
		_countdownNextEvent = g_system->getMillis() + 1000;
		// Decrement countdown
		_countdownValue[4]--;
		if (_countdownValue[4] < '0') {
			_countdownValue[4] = '9';
			_countdownValue[3]--;
			if (_countdownValue[3] < '0') {
				_countdownValue[3] = '5';
				_countdownValue[1]--;
				if (_countdownValue[1] < '0') {
					_countdownValue[1] = '9';
					_countdownValue[0]--;
					if (_countdownValue[0] < '0') {
						// Finished!
						_countingDown = false;
						playTransitionEndLevel(8);
						_abortCommand = kAbortGameOver;
					}
				}
			}
		}

		// Redraw surface
		_countdownSurface.clear(0);
		_fontManager.setCurrentFont(3);
		_fontManager.setTransparentBackground(true);
		_fontManager.setForeColor(241);
		_fontManager.setLineHeight(14);
		_fontManager.setSpaceWidth(0);
		_fontManager.setCharSpacing(1);
		_fontManager.setSurface(&_countdownSurface);

		_fontManager.displayStr(0, 2, _countdownValue);

		// Surface changed: need redraw
		return true;
	} else {
		return false;
	}
}

void CryOmni3DEngine_Versailles::doDrawCountdown(Graphics::ManagedSurface *surface) {
	if (surface) {
		surface->blitFrom(_countdownSurface, Common::Point(600, 0));
	} else {
		g_system->copyRectToScreen(_countdownSurface.getPixels(), _countdownSurface.pitch, 600, 0,
		                           _countdownSurface.w, _countdownSurface.h);
	}
}

} // End of namespace Versailles
} // End of namespace CryOmni3D
