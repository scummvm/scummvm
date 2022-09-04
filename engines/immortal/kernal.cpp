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

/* This file covers both Kernal.GS and Driver.GS.
 * This is because most of Driver.GS is hardware specific,
 * and what is not (the slightly abstracted aspects), is
 * directly connected to kernal, and might as well be
 * considered part of the same process.
 */

#include "immortal/immortal.h"

namespace Immortal {

/* 
 *
 * -----                          -----
 * ----- Screen Drawing Functions -----
 * -----                          -----
 *
 */

void ImmortalEngine::drawUniv() {
	// This is where the entire screen actually comes together
	_myViewPortX = _viewPortX & 0xFFFE;
	_myViewPortY = _viewPortY;

	_num2DrawItems = 0;

	_myUnivPointX = !(_myViewPortX & (kChrW - 1)) + kViewPortSpX;
	_myUnivPointY = !(_myViewPortY & (kChrH - 1)) + kViewPortSpY;

	makeMyCNM();
	drawBGRND();							// Draw floor parts of leftmask rightmask and maskers
	addRows();								// Add rows to drawitem array
	addSprites();							// Add all active sprites that are in the viewport, into a list that will be sorted by priority
	sortDrawItems();						// Sort said items
	drawItems();							// Draw the items over the background

	/* copyRectToSurface will apply the screenbuffer to the ScummVM surface.
	 * We want to do 320 bytes per scanline, at location (0,0), with a
	 * size of 320x200.
	 */
	_mainSurface->copyRectToSurface(_screenBuff, kResH, 0, 0, kResH, kResV);

}

void ImmortalEngine::copyToScreen() {
	if (_draw == 1) {
		g_system->copyRectToScreen((byte *)_mainSurface->getPixels(), kResH, 0, 0, kResH, kResV);
		g_system->updateScreen();
	}
}

void ImmortalEngine::clearScreen() {
	//fill the visible screen with black pixels by drawing a rectangle

	//rect(32, 20, 256, 128, 0)
	
	if ((_dontResetColors & kMaskLow) == 0) {
		useNormal();
	}
}

void ImmortalEngine::whiteScreen() {
	//fill the visible screen with black pixels by drawing a rectangle

	//rect(32, 20, 256, 128, 13)
}

void ImmortalEngine::mungeBM() {}
void ImmortalEngine::blit() {}
void ImmortalEngine::blit40() {}
void ImmortalEngine::sBlit() {}
void ImmortalEngine::scroll() {}
void ImmortalEngine::makeMyCNM() {}									// ?

void ImmortalEngine::addRows() {
	// I'm not really sure how this works yet
	int i = _num2DrawItems;
	_tPriority[i] = !(!(_myViewPortY & (kChrH - 1)) + _myViewPortY);
	
	for (int j = 0; j != kViewPortCH+4; j++, i++) {
		_tIndex[i] = (j << 5) | 0x8000;
		_tPriority[i] = _tPriority[i] - kChrH;
	}
	_num2DrawItems = i;
}

void ImmortalEngine::addSprites() {
	// My goodness this routine is gross
	int tmpNum = _num2DrawItems;
	for (int i = 0; i < kMaxSprites; i++) {
		// If the sprite is active
		if (_sprites[i]._on == 1) {
			// If sprite X is an odd number???
			if ((_sprites[i]._X & 1) != 0) {
				debug("not good! BRK");
				return;
			}

			int tmpx = (_sprites[i]._X - kMaxSpriteW) - _myViewPortX;
			if (tmpx < 0) {
				if (tmpx + (kMaxSpriteW * 2) < 0) {
					continue;
				}
			} else if (tmpx >= kViewPortW) {
				continue;
			}

			int tmpy = (_sprites[i]._Y - kMaxSpriteH) - _myViewPortY;
			if (tmpy < 0) {
				if (tmpy + (kMaxSpriteH * 2) < 0) {
					continue;
				}
			} else if (tmpy >= kViewPortH) {
				continue;
			}

			DataSprite *tempD = _sprites[i]._dSprite;
			debug("what sprite is this: %d %d %d", i, _sprites[i]._image, _sprites[i]._dSprite->_images.size());
			Image *tempImg = &(tempD->_images[0/*_sprites[i]._image*/]);
			int sx = ((_sprites[i]._X + tempImg->_deltaX) - tempD->_cenX) - _myViewPortX;
			int sy = ((_sprites[i]._Y + tempImg->_deltaY) - tempD->_cenY) - _myViewPortY;

			if (sx >= 0 ) {
				if (sx >= kViewPortW) {
					continue;
				}
			} else if ((sx + tempImg->_rectW) <= 0) {
				continue;
			}

			if (sy >= 0 ) {
				if (sy >= kViewPortH) {
					continue;
				}
			} else if ((sy + tempImg->_rectH) <= 0) {
				continue;
			}

			// Sprite is actually in viewport, we can now enter it in the sorting array
			_tIndex[_num2DrawItems] = i;
			_tPriority[_num2DrawItems] = _sprites[i]._priority;
			tmpNum++;
			if (tmpNum == kMaxDrawItems) {
				break;
			}
		}
	}
	_num2DrawItems = tmpNum;
}

void ImmortalEngine::sortDrawItems() {
	/* Just an implementation of bubble sort.
	 * Sorting largest to smallest entry, simply
	 * swapping every two entries if they are not in order.
	 */

	int top = _num2DrawItems;
	bool bailout;

	do {
		// Assume that the list is sorted
		bailout = true;
		for (int i = 1; i < top; i++) {
			if (_tPriority[i] > _tPriority[i-1]) {
				uint16 tmp = _tPriority[i];
				_tPriority[i] = _tPriority[i-1];
				_tPriority[i-1] = tmp;

				// List was not sorted yet, therefor we need to check it again
				bailout = false;
			}
		}
		/* After every pass, the smallest entry is at the end of the array, so we move
		 * the end marker back by one
		 */
		top--;
	} while (bailout == false);
}

void ImmortalEngine::drawBGRND() {
	// 'tmp' is y, 'cmp' is x

	uint16 pointX = _myUnivPointX;
	uint16 pointY = _myUnivPointY;

	for (int y = kViewPortCH + 1, y2 = 0; y != 0; y--, y2++) {
		for (int x = 0; x < (kViewPortCW + 1); x += (kViewPortCW + 1)) {
			uint16 BTS = _myModLCNM[y2][x];

			if (kIsBackground[BTS] != 0) {
				// Low Floor value, draw tile as background
				drawSolid(_myCNM[y2][x], pointX, pointY);

			} else if (kChrMask[BTS] >= 0x8000) {
				// Right Mask, draw upper left hand corner (ULHC) of floor
				drawULHC(_myCNM[y2][x], pointX, pointY);

			} else if (kChrMask[BTS] != 0) {
				// Left Mask, draw upper right hand corner (UPHC) of floor
				drawURHC(_myCNM[y2][x], pointX, pointY);
			}
			pointX += kChrW;									// This (and the H version) could be added to the for loop iterator arugment
		}
		pointX -= (kChrW * (kViewPortCW + 1));					// They could have also just done pointX = _myUnivPointX
		pointY += kChrH;
	}
}

void ImmortalEngine::drawItems() {
	for (int i = 0; i < (kViewPortCW + 1); i++) {
		_columnIndex[i] = 0;
	}

	for (int i = 0; i < (kViewPortCW + 1); i++) {
		_columnTop[i] = _myUnivPointY;
	}

	_columnX[0] = _myUnivPointX;
	for (int i = 1; i < (kViewPortCW + 1); i++) {
		_columnX[i] = _myUnivPointX + kChrW;
	}

	// This is truly horrible, I should double check that this is the intended logic
	int n = 0;
	uint16 rowY = 0;
	do {
		uint16 index = _tIndex[n];
		if (index >= 0x8000) {								// If negative, it's a row to draw
			// rowY is (I think) the position of the start of the scroll window within the tile data
			rowY = (index & 0x7FFF) + _myUnivPointY;

			// The background is a matrix of rows and columns, so for each column, we draw each row tile
			for (int i = 0; (i < (kViewPortCW + 1)); i++) {
				//draw the column of rows
				while (_columnIndex[i] < ((kViewPortCW + 1) * (kViewPortCH + 1))) {

					uint16 k = _myModLCNM[i][_columnIndex[i]];
					// ******* This is just so that the array can be indexed right now, will remove when myModLCNM is actually useable
					k = 0;
					// *****************************
					if ((rowY - kChrDy[k]) < _columnTop[i]) {
						break;
					}
					if (kIsBackground[k] == 0) {
						// If it's a background tile, we already drew it (why is it in here then??)
						if (kChrMask[k] >= 0x8000) {
							// Right Mask, draw lower right hand corner (LRHC)
							drawLRHC(_myCNM[i][_columnIndex[i]], _columnTop[i], _columnX[i]);

						} else if (kChrMask[k] == 0) {
							// Floor or cover, draw the whole CHR
							drawSolid(_myCNM[i][_columnIndex[i]], _columnTop[i], _columnX[i]);

						} else {
							// Left Mask, draw lower left hand corner (LLHC)
							drawLLHC(_myCNM[i][_columnIndex[i]], _columnTop[i], _columnX[i]);
						}
					}
					_columnTop[i] += kChrH;
					_columnIndex[i] += (kViewPortCW + 1);
				}
			}

		} else {
			// If positive, it's a sprite
			uint16 x = (_sprites[index]._X - _myViewPortX) + kVSX;
			uint16 y = (_sprites[index]._Y - _myViewPortY) + kVSY;
			//superSprite(index, x, y, _sprites[index]._dSprite->_images[0/*_sprites[index]._image*/], kVSBMW, _screenBuff, kMySuperTop, kMySuperBottom);
		}
		n++;
	} while (n != _num2DrawItems);
}

void ImmortalEngine::backspace() {
	// Just moves the drawing position back by a char, and then draws an empty rect there (I think)
	_penX -= 8;
	//rect(_penX + 32, 40, 8, 16, 0);
}

void ImmortalEngine::printChr(char c) {
	// This draws a character from the font sprite table, indexed as an ascii char, using superSprite
	c &= kMaskASCII;				// Grab just the non-extended ascii part

	if (c == ' ') {
		_penX += 8;					// A space just moves the position on the screen to draw ahead by the size of a space
		return;
	}

	if (c == 0x27) {
		_penX -= 2;
	}

	if ((c >= 'A') && (c <= 'Z')) {
		_penX += 8;

	} else {
		switch (c) {
			// Capitals, the health bar icons, and lower case m/w are all 2 chars wide
			case 'm':
			case 'w':
			case 'M':
			case 'W':
			case 1:						// Can't use the constant for this for some reason
			case 0:
				_penX += 8;
				break;
			case 'i':
				_penX -= 3;
				break;
			case 'j':
			case 't':
				_penX -= 2;
				break;
			case 'l':
				_penX -= 4;
			default:
				break;
		}
	}

	uint16 x = _penX + kScreenLeft;
	if (x < _dataSprites[kFont]._cenX) {
		return;
	}

	uint16 y = _penY + kScreenTop;
	if (y < _dataSprites[kFont]._cenY) {
		return;
	}

	//superSprite(0, x, y, _dataSprites[kFont]._images[(int) c], kScreenBMW, _screenBuff, kSuperTop, kSuperBottom);
	if ((c == 0x27) || (c == 'T')) {
		_penX -= 2;					// Why is this done twice??
	}

	_penX += 8;
}

/*
 *
 * -----            -----
 * ----- Asset Init -----
 * -----            -----
 *
 */

void ImmortalEngine::initStoryStatic() {
	Common::Array<Common::String> s{"#" + Common::String(kSwordBigFrame) + "sword@",
									"You find an Elven sword of&agility. Take it?@",
									"Search the bones?%",
									"}The sword permanently endows you with Elven agility and quickness in combat.@",
									"}You notice something that looks wet and green under the pile. Search further?%",
									"#" + Common::String(kBagBigFrame) + "  dust@"
									"}You find a bag containing Dust of Complaisance.&@"
									"}Drop the bait on the ground here?%"
									"}To use this dust, you throw it in the air. Do that here?%"
									"_}Don+t bother me, I+m cutting a gem. Yes, you need it. No, you can+t have it. I wouldn+t give it to anyone, least of all you. Go away. ]]]]="
									"_}Let me help you. Please take this gem. No, really, I insist. Take it and go with my blessings. Good luck. ]]]]="
									"#" + Common::String(kCarpetBigFrame) + "carpet@",
									"#" + Common::String(kBombBigFrame) + " bomb@",
									"A gas bomb that goblins&use to paralyze trolls.&@",
									"Take it?<>@",
									"%",
									" other@",
									"#" + Common::String(kKeyBigFrame) + "  key@",
									"#" + Common::String(kKeyBigFrame) + "  key@",
									"A key to a chest.&@",
									"The chest is open. Examine&contents?%",
									"Put it on?%",
									"Drop it?%",
									"It+s unlocked. Open it?%",
									"It+s locked but you have&the key. Open it?%",
									"It+s locked and you don+t&have the key.@",
									"The lock, triggered by a&complicated set of latches,&is unfamiliar to you.@",
									"#" + Common::String(kGoldBigFrame) + "$0 gold@",
									"You find $0 gold pieces.&&^#" + Common::String(kPileFrame) + "@",
									"@",
									"You can+t plant them on&stone tiles.@",
									"It+s locked but you are&able to unlock it with&the key.@",
									"_}The king is not dead, but the poison is taking effect. When he sees you, he attempts to speak:[(Give me water... the fountain... I give you... information... peace...+[Give him water?%",
									"_}You dont have any water to give him. He mumbles something. Then silence... You find a key on his body.]]]]=",
									"_}He mumbles something. Then silence... You find a key on his body.]]]]=",
									"_}I+ll tell you how to... next level... past slime... three jewels... slime... rock becomes... floor... right, left, center of the... [Then silence. His hand opens, releasing a key.]]]]=",
									"You find a door key.&@",
									"You find a note.&@",
									"#" + Common::String(kNoteBigFrame) + "note@",
									"He+s dead.&Look for possessions?%",
									"You don+t have it. Check&your inventory.@",
									"Game Over&&Play again?@",
									"Congratulations!&&Play again?@",
									"You find a bag of bait.&@",
									"#" + Common::String(kBagBigFrame) + "   bait@",
									"You find a stone. @",
									"#" + Common::String(kStoneBigFrame) + " stone@",
									"You find a red gem.&@",
									"#" + Common::String(kGemBigFrame) + "  gem@",
									"You find a scroll with&fireball spells.&@"
									"#" + Common::String(kScrollBigFrame) + "$ shots@",
									"You find a map warning&you about pit traps.&@"
									"#" + Common::String(kMapBigFrame) + "  map@",
									"#" + Common::String(kVaseBigFrame) + "   oil@",
									"You apply the oil but notice&as you walk that the leather&is drying out quickly.@"
									"}You discover a scroll with a charm spell to use on will o+ the wisps.&@"
									"#" + Common::String(kScrollBigFrame) + " charm@",
									"}This charms the will o+ the wisps to follow you. Read the spell again to turn them against your enemies.@"
									"}It looks like water. Drink it?%",
									"Drink it?%",
									"}It works! You are much stronger.]]]=",
									"}It looks like it has green stuff inside. Open it?%",
									"Now this will take&effect when you press the&fire button.@",
									"You find a potion,&Magic Muscle.&@",
									"#" + Common::String(kVaseBigFrame) + "  potion@",
									"You find a bottle.&@",
									"#" + Common::String(kVaseBigFrame) + "  bottle@",
									"#" + Common::String(kRingBigFrame) + "Protean@",
									"You find a Protean Ring.&@",
									"You find a troll ritual knife,&used to declare a fight to&the death. @",
									"#" + Common::String(kKnifeBigFrame) + " knife@",
									"_}It is a fine woman+s garment. Folded inside is a ring with the words,[`To Ana, so harm will never find you. -Your loving father, Dunric.+&@",
									"You find a small, well&crafted ring. @",
									"#" + Common::String(kRingBigFrame) + "  gift@",
									"#" + Common::String(kRingBigFrame) + " Ana+s@",
									"_}She is hurt and upset when she finds you don+t have her ring or won+t give it to her. She scurries back into the hole. The hole is too small for you to follow.&@",
									"_}`Sir, can you help me,+ the girl pleads. `I was kidnapped and dragged down here. All the man would say is `Mordamir+s orders.+[I escaped using a ring my father gave me, but now I+ve lost it. Did you find it?+%",
									"_}We have met before, old man. Do you remember? Because you helped me, you may pass. But I warn you, we are at war with the trolls.[Over this ladder, across the spikes, is troll territory. Very dangerous.@",
									"_}You are an impostor!]]]]=",
									"_}Old man, do you remember me? I am king of the goblins. You didn+t give me the water. You left me to die after you took the key from me. Now you will pay.]]]]=",
									"_}You quickly fall into a deep, healing sleep...[Vivid images of a beautiful enchanted city pass by. All the city people are young and glowing. Fountains fill the city, and the splash and sparkle of water is everywhere...[Suddenly the images go black. A face appears... Mordamir!]][He is different from how you remember him. His gentle features are now withered. His kind eyes, now cold and sunken, seem to look through you with a dark, penetrating stare. You wake rejuvenated, but disturbed.]]]]]=",
									"_}Here, take this ring in return. [I don+t know if it will help, but I heard the unpleasant little dwarf say, (Clockwise, three rings around the triangle.+[Could that be a clue to his exit puzzle? I must go. Goodbye.]]]]=",
									"#" + Common::String(kSackBigFrame) + " spores@",
									"You find a sack of bad&smelling spores.&@",
									"Please insert play disk.@",
									"New game?%",
									"Enter certificate:&-=",
									"Invalid certificate.@",
									"End of level!&Here is your certificate:&&=",
									"&@",
									"   Electronic Arts presents&&       The Immortal&&&&      1990 Will Harvey|]]]]]]]]]=",
									"          written by&&         Will Harvey&         Ian Gooding&      Michael Marcantel&       Brett G. Durrett&        Douglas Fulton|]]]]]]]/=",
									"_}Greetings, friend! Come, I+ve got something you need. These parts are plagued with slime.[You can+t venture safely without my slime oil for boots, a bargain at only 80 gold pieces.%",
									"_}All right, 60 gold pieces for my oil. Rub it on your boots and slime won+t touch you. 60, friend.%",
									"This room doesn+t resemble&any part of the map.@",
									"This room resembles part&of the map.@"};
	_strPtrs = s;

	// Scope, amirite?
	Common::Array<int> cyc0{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1};
	Common::Array<int> cyc1{15,16,17,18,19,20,21,22,-1};
	Common::Array<int> cyc2{0,1,2,-1};
	Common::Array<int> cyc3{3,4,5,-1};
	Common::Array<int> cyc4{6,7,8,9,10,-1};
	Common::Array<int> cyc5{11,12,13,14,15,-1};
	Common::Array<int> cyc6{16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,-1};
	Common::Array<int> cyc7{0,1,2,3,4,-1};
	Common::Array<int> cyc8{5,1+5,2+5,3+5,4+5,-1};
	Common::Array<int> cyc9{10,1+10,2+10,3+10,4+10,-1};
	Common::Array<int> cyc10{15,1+15,2+15,3+15,4+15,-1};
	Common::Array<int> cyc11{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,-1};
	Common::Array<int> cyc12{0,1,2,3,4,5,6,7,8,9,-1};
	Common::Array<int> cyc13{0,1,2,3, 0,1,2,3, 0,1,2,3, 0,1,2,3, 0,1,2,3, 0,1,2,3, 0,1,2,3, 0,1,2,3, -1};
	Common::Array<int> cyc14{31,32,33,32, 34,35,36,35, 37,38,39,38, 40,41,42,41, 43,44,45,44, 46,47,48,47, 49,50,51,50, 52,53,54,53, -1};
	Common::Array<int> cyc15{55, -1};
	Common::Array<int> cyc16{63,64,65,66, 63,64,65,66, 63,64,65,66, 63,64,65,66, 63,64,65,66, 63,64,65,66, 63,64,65,66, 63,64,65,66,-1};
	Common::Array<int> cyc17{0,1,0,-1};
	Common::Array<int> cyc18{0,1,2,4,5,6,7,8,9,10,11,12,2,1,-1};
	Common::Array<int> cyc19{0,0,1,2,13,14,15,16,4,2,3,-1};
	Common::Array<int> cyc20{0,1,2,3,20,21,22,23,24,25,26,27,5,4,3,-1};
	Common::Array<int> cyc21{0,1,2,3,-1};
	Common::Array<int> cyc22{0,17,18,19,3,-1};
	Common::Array<int> cyc23{0,1,-1};
	Common::Array<int> cyc24{28,28,28,28,-1};
	Common::Array<int> cyc25{15,16,15,16,15,1+15,1+15,-1};
	Common::Array<int> cyc26{10+15,11+15,12+15,13+15,14+15,15+15,16+15,-1};
	Common::Array<int> cyc27{2+15,3+15,4+15,5+15,-1};
	Common::Array<int> cyc28{6+15,7+15,8+15,9+15,-1};
	Common::Array<int> cyc29{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1};
	Common::Array<int> cyc30{0,1,2,3,3,3,3,4,5,6,-1};
	Common::Array<int> cyc31{0,1,2,3,4,5,6,7,8,-1};

	Common::Array<SCycle> c{SCycle(kBubble,     false, cyc0),  SCycle(kBubble, 	   false, cyc1),
							SCycle(kSpark, 	    false, cyc2),  SCycle(kSpark, 	   false, cyc3),
							SCycle(kSpark, 	    false, cyc4),  SCycle(kSpark, 	   false, cyc5),  SCycle(kSpark,  false, cyc6),
							SCycle(kPipe, 	    false, cyc7),  SCycle(kPipe, 	   false, cyc8),
							SCycle(kPipe,	    false, cyc9),  SCycle(kPipe, 	   false, cyc10),
							SCycle(kAnaVanish,  false, cyc11), SCycle(kAnaGlimpse, false, cyc12),
							SCycle(kKnife, 	    true,  cyc13),
							SCycle(kSpark, 	    true,  cyc14), SCycle(kSpark, 	   true, cyc15), SCycle(kSpark,  true,  cyc16),
							SCycle(kBigBurst,   false, cyc17),
							SCycle(kFlame,      false, cyc18), SCycle(kFlame,      false, cyc19), SCycle(kFlame,  false, cyc20),
							SCycle(kFlame,      false, cyc21), SCycle(kFlame,      false, cyc22), SCycle(kFlame,  false, cyc23),
							SCycle(kFlame,      false, cyc24),
							SCycle(kCandle,     false, cyc25), SCycle(kCandle,     false, cyc26), SCycle(kCandle, false, cyc27),
							SCycle(kCandle,     false, cyc28), SCycle(kCandle,     false, cyc29),
							SCycle(kSink,       false, cyc30),
							SCycle(kNorlacDown, false, cyc31)};
	_cycPtrs = c;

	Common::Array<Motive>   m{};
	_motivePtrs = m;

	Common::Array<Damage>   d{};
	_damagePtrs = d;

	Common::Array<Use>      u{};
	_usePtrs = u;

	Common::Array<Pickup>   p{};
	_pickupPtrs = p;

	CArray2D<Motive>       pr{};
	_programPtrs = pr;

	Common::Array<ObjType>  o{};
	_objTypePtrs = o;

}

void ImmortalEngine::kernalAddSprite(uint16 x, uint16 y, SpriteName n, int img, uint16 p) {
	Utilities::addSprite(_sprites, _viewPortX, _viewPortY, &_numSprites, &_dataSprites[n], img, x, y, p);
}

void ImmortalEngine::clearSprites() {
	// Just sets the 'active' flag on all possible sprites to 0
	for (int i = 0; i < kMaxSprites; i++) {
		_sprites[i]._on = 0;
	}
}

void ImmortalEngine::cycleFreeAll() {
	// Sets all cycle indexes to -1, indicating they are available
	for (int i = 0; i < kMaxCycles; i++) {
		_cycles[i]._index = -1;
	}
}

void ImmortalEngine::loadSprites() {
	/* This is a bit weird, so I'll explain.
	 * In the source, this routine loads the files onto the heap, and then
	 * goes through a table of sprites in the form file_index, sprite_num, center_x, center_y.
	 * It uses file_index to get a pointer to the start of the file on the heap,
	 * which it then uses to set the center x/y variables in the file itself.
	 * ie. file_pointer[file_index]+((sprite_num<<3)+4) = center_x.
	 * We aren't going to have the sprite properties inside the file data, so instead
	 * we have an array of all game sprites _dataSprites which is indexed
	 * soley by a sprite number now. This also means that a sprite itself has a reference to
	 * a datasprite, instead of the sprite index and separate the file pointer. Datasprite
	 * is what needs the file, so that's where the pointer is. The index isn't used by
	 * the sprite or datasprite themselves, so it isn't a member of either of them.
	 */

	Common::String spriteNames[] = {"MORESPRITES.SPR", "NORLAC.SPR", "POWWOW.SPR", "TURRETS.SPR",
									"WORM.SPR", "IANSPRITES.SPR", "LAST.SPR", "DOORSPRITES.SPR",
									"GENSPRITES.SPR", "DRAGON.SPR", "MORDAMIR.SPR", "FLAMES.SPR",
									"ROPE.SPR", "RESCUE.SPR", "TROLL.SPR", "GOBLIN.SPR", "WIZARDA.SPR",
									"WIZARDB.SPR", "ULINDOR.SPR", "SPIDER.SPR", "DRAG.SPR"};

	// Number of sprites in each file
	int spriteNum[] = {10, 5, 7, 10, 4, 6, 3, 10, 5, 3, 2, 1, 3, 2, 9, 10, 8, 3, 9, 10, 9};

	// Pairs of (x,y) for each sprite
	// Should probably have made this a 2d array, oops
	uint16 centerXY[] = {16,56, 16,32, 27,39, 16,16, 32,16, 34,83, 28,37, 8,12, 8,19, 24,37,
	/* Norlac      */   46,18, 40,0, 8,13, 32,48, 32,40,
	/* Powwow      */   53,43, 28,37, 27,37, 26,30, 26,30, 26,29, 28,25,
	/* Turrets     */   34,42, 28,37, 24,32, 32,56, 26,56, 8,48, 8,32, 8,14, 8,24, 32,44,
	/* Worm        */   20,65, 25,46, 9,56, 20,53,
	/* Iansprites  */   24,50, 32,52, 32,53, 32,52, 40,16, 40,16,
	/* Last        */   32,56, 24,32, 24,36,
	/* Doorsprites */   0,64, 4,49, 18,49, 18,56, 24,32, 24,16, 24,56, 24,32, 24,32, 36,32,
	/* Gensprites  */   16,44, 16,28, 32,24, 34,45, 20,28,
	/* Dragon      */   24,93, 32,48, 0,64,
	/* Mordamir    */   104,104, 30,30,
	/* Flames      */   64,0,
	/* Rope        */   0,80, 32,52, 32,40,
	/* Rescue      */   0,112, 0,112,
	/* Troll       */   28,38, 28,37, 28,37, 31,38, 28,37, 25,39, 28,37, 28,37, 28,37,
	/* Goblin      */   28,38, 30,38, 26,37, 30,38, 26,37, 26,37, 26,37, 26,37, 26,36, 44,32,
	/* Wizarda	   */	28,37, 28,37, 28,37, 28,37, 28,37, 28,37, 28,37, 28,37,
	/* Wizardb	   */   28,37, 28,37, 28,37,
	/* Ulindor     */   42,42, 42,42, 42,42, 42,42, 42,42, 42,42, 42,42, 42,42, 42,42,
	/* Spider      */   64,44, 64,44, 64,44, 64,44, 64,44, 64,44, 64,44, 64,44, 64,44, 64,44,
	/* Drag        */   19,36, 19,36, 19,36, 19,36, 19,36, 19,36, 19,36, 19,36, 19,36};

	// s = current sprite index, f = current file index, n = current number of sprites for this file
	int s = 0;
	for (int f = 0; f < 21; f++) {
		// For every sprite file, open it and get the pointer
		Common::SeekableReadStream *file = loadIFF(spriteNames[f]);

		for (int n = 0; n < (spriteNum[f] * 2); n += 2, s++) {
			// For every data sprite in the file, make a datasprite and initialize it
			DataSprite d;
			initDataSprite(file, &d, n/2, centerXY[s * 2], centerXY[(s * 2) + 1]);
			_dataSprites[s] = d;
		}
	}
}

void ImmortalEngine::loadWindow() {
	// Initialize the window bitmap
	Common::File f;
	_window = new byte[kScreenSize];

	if (f.open("WINDOWS.BM")) {

		/* The byte buffer for the screen (_screenBuff) has one byte for
		 * every pixel, with the resolution of the game being 320x200.
		 * For a bitmap like the window frame, all we need to do is
		 * extract the pixel out of each nyble (half byte) of the data,
		 * by looping over it one row at a time.
		 */

		byte pixel;
		int pos;
		for (int y = 0; y < kResV; y++) {
			for (int x = 0; x < kResH; x += 2) {
				pos = (y * kResH) + x;
				pixel = f.readByte();
				_window[pos]     = (pixel & kMask8High) >> 4;
				_window[pos + 1] =  pixel & kMask8Low;
			}
		}

		// Now that the bitmap is processed and stored in a byte buffer, we can close the file
		f.close();

		// To start constructing the screen, we start with the frame as the base
		memcpy(_screenBuff, _window, kScreenSize);

	} else {
		// Should probably give an error or something here
		debug("oh nose :(");
	}
}

void ImmortalEngine::loadFont() {
	// Initialize the font data sprite
	Common::SeekableReadStream *f = loadIFF("FONT.SPR");
	DataSprite d;

	if (f) {
		initDataSprite(f, &d, 0, 16, 0);
		_dataSprites[kFont] = d;

	} else {
		debug("file doesn't exit?!");
	}

}

Common::SeekableReadStream *ImmortalEngine::loadIFF(Common::String fileName) {
	Common::File f;
	if (!f.open(fileName)) {
		debug("*surprised pikachu face*");
		return nullptr;
	}

	/* This isn't the most efficient way to do this (could just read a 32bit uint and compare),
	 * but this makes it more obvious what the source was doing. We want to know if the 4 bytes
	 * at file[8] are 'C' 'M' 'P' '0', so this grabs just the ascii bits of those 4 bytes,
	 * allowing us to directly compare it with 'CMP0'.
	 */
	char compSig[] = "CMP0";
		char sig[] = "0000";

	f.seek(8);

	for (int i = 0; i < 4; i++) {
		sig[i] = f.readByte() & kMaskASCII;
	}

	if (strcmp(sig, compSig) == 0) {
		debug("compressed");
		
		/* The size of the compressed data is stored in the header, but doesn't
		 * account for the FORM part?? Also, **technically** this is a uint32LE,
		 * but the engine itself actually /doesn't/ use it like that. It only
		 * decrements the first word (although it compares against the second half,
		 * as if it is expecting that to be zero? It's a little bizarre).
		 */
		f.seek(6);
		int len = f.readUint16LE() - 4;

		// Compressed files have a 12 byte header before the data
		f.seek(12);
		return unCompress(&f, len);
	}
	// Gotta remember we just moved the cursor around a bunch, need to reset it to read the file
	f.seek(SEEK_SET);

	byte *out = (byte *)malloc(f.size());
	f.read(out, f.size());
	return new Common::MemoryReadStream(out, f.size(), DisposeAfterUse::YES);
}


/*
 *
 * -----                   -----
 * ----- Palette Functions -----
 * -----                   -----
 *
 */

/* Palettes on the Apple IIGS:
 * In High-res mode you have 2 options: 320x200 @ 4bpp or 320x640 @ 2bpp.
 * The Immortal uses the former, giving us 16 colours to use
 * for any given pixel on the screen (ignoring per scanline palettes because
 * The Immortal does not use them). This 16 colour palette is made of 2 byte
 * words containing the RGB components in the form 0RGB.
 *
 * The equivalent palette for ScummVM is a byte stream of up to 256
 * colours composed of 3 bytes each, ending with a transparency byte.
 *
 * Because each colour in the game palette is only a single nyble (4 bits),
 * we also need to multiply the nyble up to the size of a byte (* 16, or << 4).
 */

void ImmortalEngine::loadPalette() {
	// The palettes are stored at a particular location in the disk, this just grabs them
	Common::File d;
	d.open("IMMORTAL.dsk");
	
	d.seek(kPaletteOffset);
	d.read(_palDefault, 32);
	d.read(_palWhite, 32);
	d.read(_palBlack, 32);
	d.read(_palDim, 32);

	d.close();
}

void ImmortalEngine::setColors(uint16 pal[]) {
	// The RGB palette is 3 bytes per entry, and each byte is a colour
	for (int i = 0; i < 16; i++) {

		// The palette gets masked so it can update only specific indexes and uses FFFF to do so. However the check is simply for a negative
		if (pal[i] < kMaskNeg) {

			// Green is already the correct size, being the second nyble (00G0)
			// Red is in the first nyble of the high byte, so it needs to move right by 4 bits (0R00 -> 00R0)
			// Blue is the first nyble of the first byte, so it needs to move left by 4 bits (000B -> 00B0)
			// We also need to repeat the bits so that the colour is the same proportion of 255 as it is of 15
			_palRGB[(i * 3)]     = ((pal[i] & kMaskRed) >> 4) | ((pal[i] & kMaskRed) >> 8);
			_palRGB[(i * 3) + 1] =  (pal[i] & kMaskGreen)     | ((pal[i] & kMaskGreen) >> 4);
			_palRGB[(i * 3) + 2] =  (pal[i] & kMaskBlue)      | ((pal[i] & kMaskBlue) << 4);
		}
	}
	// Palette index to update first is 0, and there are 16 colours to update
	g_system->getPaletteManager()->setPalette(_palRGB, 0, 16);
	g_system->updateScreen();
}

void ImmortalEngine::fixColors() {
	// Pretty silly that this is done with two separate variables, could just index by one...
	if (_dim == true) {
		if (_usingNormal == true) {
			useDim();
		}
	} else {
		if (_usingNormal == false) {
			useNormal();
		}
	}
}

void ImmortalEngine::pump() {
	// Flashes the screen (except the frame thankfully) white, black, white, black, then clears the screen and goes back to normal
	useWhite();
	g_system->updateScreen();
	Immortal::Utilities::delay(2);
	useBlack();
	g_system->updateScreen();
	Immortal::Utilities::delay(2);
	useWhite();
	g_system->updateScreen();
	Immortal::Utilities::delay(2);
	useBlack();
	g_system->updateScreen();
	clearScreen();
	// Why does it do this instead of setting _dontResetColors for clearScreen() instead?
	useNormal();
}

void ImmortalEngine::fadePal(uint16 pal[], int count, uint16 target[]) {
	/* This will fade the palette used by everything inside the game screen
	 * but will not touch the window frame palette. It essentially takes the
	 * color value nyble, multiplies it by a multiplier, then takes the whole
	 * number result and inserts it into the word at the palette index of the
	 * temporary palette. This could I'm sure, be done with regular multiplication
	 * and division operators, but in case the bits that get dropped are otherwise
	 * kept, this is a direct translation of the bit manipulation sequence.
	 */
	uint16 maskPal[16] = {0xFFFF, 0x0000, 0x0000, 0x0000,
					      0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
					      0xFFFF, 0xFFFF, 0xFFFF, 0x0000,
					      0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

	uint16 result;
	uint16 temp;

	for (int i = 15; i >= 0; i--) {
		result = maskPal[i];
		if (result == 0) {
			// If the equivalent maskPal entry is 0, then it is a colour we want to fade
			result = pal[i];
			if (result != 0xFFFF) {
				// If we have not reached FFFF in one direction or the other, we keep going

				// Blue = 0RGB -> 000B -> 0Bbb -> bb0B -> 000B
				result = (xba(mult16((result & kMaskFirst), count))) & kMaskFirst;

				// Green = 0RGB -> 00RG -> 000G -> 0Ggg -> gg0G -> 000G -> 00G0 -> 00GB
				temp = mult16(((pal[i] >> 4) & kMaskFirst), count);
				temp = (xba(temp) & kMaskFirst) << 4;
				result = temp | result;

				// Red = 0RGB -> GB0R -> 000R -> 0Rrr -> rr0R -> 000R -> 0R00 -> 0RGB
				temp = xba(pal[i]) & kMaskFirst;
				temp = xba(mult16(temp, count));
				temp = xba(temp & kMaskFirst);
				result = temp | result;
			}
		}
		target[i] = result;
	}
}

void ImmortalEngine::fade(uint16 pal[], int dir, int delay) {
	// This temp palette will have FFFF in it, which will be understood as masks by setColors()
	uint16 target[16];
	uint16 count;

	// Originally used a branch, but this is functionally identical and much cleaner
	count = dir * 256;

	while ((count >= 0) && (count <= 256)) {
		fadePal(pal, count, target);
		Immortal::Utilities::delay8(delay);
		setColors(target);

		// Same as above, it was originally a branch, this does the same thing
		count += (dir == 0) ? 16 : -16;
	}
}

// These two can probably be removed and instead use an enum to declare fadeout/in
void ImmortalEngine::fadeOut(int j) {
	fade(_palDefault, 1, j);
}

void ImmortalEngine::normalFadeOut() {
	fadeOut(15);
}

void ImmortalEngine::slowFadeOut() {
	fadeOut(28);
}

void ImmortalEngine::fadeIn(int j) {
	fade(_palDefault, 0, j);
}

void ImmortalEngine::normalFadeIn() {
	fadeIn(15);
}

// These two can probably be removed since the extra call in C doesn't have the setup needed in ASM
void ImmortalEngine::useBlack() {
	setColors(_palBlack);
}
void ImmortalEngine::useWhite() {
	setColors(_palBlack);
}

void ImmortalEngine::useNormal() {
	setColors(_palDefault);
	 _usingNormal = true;
}

void ImmortalEngine::useDim() {
	setColors(_palDim);
	_usingNormal = false;
}


/*
 *
 * -----                 -----
 * ----- Input Functions -----
 * -----                 -----
 *
 */

void ImmortalEngine::userIO() {}
void ImmortalEngine::pollKeys() {}
void ImmortalEngine::noNetwork() {}

void ImmortalEngine::waitKey() {
	bool wait = true;
	while (wait == true) {
		if (getInput() == true) {
			wait = false;
		}
	}
}

void ImmortalEngine::blit8() {}
bool ImmortalEngine::getInput() {
	return true;
}

void ImmortalEngine::addKeyBuffer() {}
void ImmortalEngine::clearKeyBuff() {}


/*
 *
 * -----                       -----
 * ----- Sound/Music Functions -----
 * -----                       -----
 *
 */

void ImmortalEngine::toggleSound() {
	// Interestingly, this does not mute or turn off the sound, it actually pauses it
	_themePaused = !_themePaused;
	fixPause();
}

void ImmortalEngine::fixPause() {
	/* The code for this is a little strange, but the idea is that you have
	 * a level theme, and a combat theme, that can both be active. So first you
	 * pause the level theme, and then you pause the combat theme.
	 * The way it does it is weird though. Here's the logic:
	 * if playing either text or maze song, check if the theme is paused. else, just go ahead and pause.
	 * Same thing for combat song. A little odd.
	 */

	// This is a nasty bit of code isn't it? It's accurate to the source though :D
	switch (_playing) {
		case kSongText:
		case kSongMaze:
			if (_themePaused) {
				musicUnPause(_themeID);
				break;
			}
		default:
			musicPause(_themeID);
			break;
	}

	// Strictly speaking this should probably be a single function called twice, but the source writes out both so I will too
	switch (_playing) {
		case kSongCombat:
			if (_themePaused) {
				musicUnPause(_combatID);
				break;
			}
		default:
			musicPause(_combatID);
			break;
	}

}

// *** These two functions will be in music.cpp, they just aren't implemented yet ***
void ImmortalEngine::musicPause(int sID) {}
void ImmortalEngine::musicUnPause(int sID) {}
// ***

Song ImmortalEngine::getPlaying() {
	return kSongMaze;
}

void ImmortalEngine::playMazeSong() {
}

void ImmortalEngine::playCombatSong() {
}

void ImmortalEngine::loadSingles(Common::String songName) {
	debug("%s", songName.c_str());
}

void ImmortalEngine::stopMusic() {
	//musicStop(-1)
	_playing = kSongNothing;
	//stopSound();
}

} // namespace Immortal




















