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

#include "common/file.h"
#include "common/textconsole.h"

#include "tot/chrono.h"
#include "tot/vars.h"

namespace Tot {

int doserror = 0;
int exitcode = 0;

Common::MemorySeekableReadWriteStream *conversationData;
Common::MemorySeekableReadWriteStream *rooms;
Common::MemorySeekableReadWriteStream *invItemData;

byte mouseMaskIndex;

uint mouseX, mouseY;

uint mouseClickX, mouseClickY;

uint npraton2, npraton;

uint oldGridX, oldGridY;

SavedGame savedGame;

bool isSealRemoved;

bool roomChange;
bool isTVOn,
	isVasePlaced,
	isScytheTaken,
	isTridentTaken,
	isPottersWheelDelivered,
	isMudDelivered,
	isGreenDevilDelivered,
	isRedDevilCaptured,
	isPottersManualDelivered,
	isCupboardOpen,
	isChestOpen,
	isTrapSet,
	isPeterCoughing;

bool inGame;

bool firstTimeDone;

bool isIntroSeen;

bool shouldQuitGame;

bool startNewGame;

bool continueGame;

bool isSavingDisabled;

bool isDrawingEnabled;

bool isSecondaryAnimationEnabled;

palette palAnimSlice;

palette pal;

InventoryEntry mobj[inventoryIconCount];

byte *inventoryIconBitmaps[inventoryIconCount];

byte palAnimStep;

byte inventoryPosition;

byte actionCode;

byte oldActionCode;

byte steps;

byte doorIndex;

byte isPaletteAnimEnabled;

byte gamePart;

byte secondaryAnimationFrameCount;

byte secondaryAnimDirCount;

byte cpCounter, cpCounter2;

byte destinationStepX, destinationStepY;

byte charFacingDirection;

uint secondaryAnimWidth, secondaryAnimHeight;

uint backpackObjectCode;

uint foo;

uint oldposx, oldposy;
uint rightSfxVol, leftSfxVol;

uint musicVolRight, musicVolLeft;

int element1, element2;

int characterPosX, characterPosY;

int xframe2, yframe2;

Common::File verb;

Common::String oldInventoryObjectName, inventoryObjectName;

Common::String photoFileName;

Common::String characterName;

// Text decryption key
Common::String decryptionKey;

uint niche[2][4];

RoomFileRegister *currentRoomData;

ScreenObject regobj;

route mainRoute;

Common::Point trajectory[300];

uint trajectoryLength;

uint currentTrajectoryIndex;

uint currentSecondaryTrajectoryIndex;

byte currentZone, targetZone, oldTargetZone;

byte maxXGrid, maxYGrid;

byte movementGridForSecondaryAnim[10][10];

byte mouseGridForSecondaryAnim[10][10];

byte maskGridSecondaryAnim[10][10];

byte maskMouseSecondaryAnim[10][10];

bool list1Complete, list2Complete,
	obtainedList1, obtainedList2;

bool firstTimeTopicA[characterCount],
	firstTimeTopicB[characterCount],
	firstTimeTopicC[characterCount],
	bookTopic[characterCount],
	mintTopic[characterCount];

bool caves[5];

uint16 firstList[5], secondList[5];

CharacterAnim mainCharAnimation;
SecondaryAnim secondaryAnimation;
uint mainCharFrameSize,
	secondaryAnimFrameSize;

byte maxSecondaryAnimationFrames;

byte transitionEffect;

byte iframe, iframe2;

long screenSize;

ObjectInfo depthMap[numScreenOverlays];

byte *screenLayers[numScreenOverlays];

byte *curCharacterAnimationFrame;

byte *curSecondaryAnimationFrame;

byte *sceneBackground;

byte *characterDirtyRect;

byte *backgroundCopy;

uint currentRoomNumber;

bool isLoadingFromLauncher;

bool saveAllowed = true;

void clearObj() {

	regobj.code = 0;
	regobj.height = 0;
	regobj.name = "";
	regobj.lookAtTextRef = 0;
	regobj.beforeUseTextRef = 0;
	regobj.afterUseTextRef = 0;
	regobj.pickTextRef = 0;
	regobj.useTextRef = 0;
	regobj.speaking = 0;
	regobj.openable = false;
	regobj.closeable = false;
	for (int i = 0; i <= 7; i++)
		regobj.used[i] = 0;
	regobj.pickupable = false;
	regobj.useWith = 0;
	regobj.replaceWith = 0;
	regobj.depth = 0;
	regobj.bitmapPointer = 0;
	regobj.bitmapSize = 0;
	regobj.rotatingObjectAnimation = 0;
	regobj.rotatingObjectPalette = 0;
	regobj.dropOverlayX = 0;
	regobj.dropOverlayY = 0;
	regobj.dropOverlay = 0;
	regobj.dropOverlaySize = 0;
	regobj.objectIconBitmap = 0;
	regobj.xrej1 = 0;
	regobj.yrej1 = 0;
	regobj.xrej2 = 0;
	regobj.yrej2 = 0;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			regobj.walkAreasPatch[i][j] = 0;
			regobj.mouseGridPatch[i][j] = 0;
		}
	}
	cpCounter2 = cpCounter;
}

void clearScreenData() {
	// Do nothing
}

/**
 * Originally the Room file contains 8 copies of each room, one for every save plus the baseline (which is 0).
 * To put this into memory we need to get the baseline of each room and then put them continuously in a byte stream.addr
 * Whenever the game access a room instead of accessing the room for the autosave or the current save,
 * we assume only one room register is there.
 *
 * To save a game we merely copy the entire stream into the save.
 */
void initializeScreenFile() {

	Common::File roomFile;
	if (!roomFile.open(Common::Path("PANTALLA.DAT"))) {
		showError(320);
	}

	int64 fileSize = roomFile.size();
	delete (rooms);

	byte *roomData = (byte *)malloc(roomRegSize * 32);
	int roomCount = 0;

	while (!roomFile.eos()) {
		if (fileSize - roomFile.pos() >= roomRegSize) {
			roomFile.read(roomData + roomRegSize * roomCount, roomRegSize);
			// This one doesnt work for some reason:
			// rooms->writeStream(roomFile.readStream(roomRegSize), roomRegSize);
			roomFile.skip(roomRegSize * 7);
			roomCount++;
		} else {
			break;
		}
	}
	rooms = new Common::MemorySeekableReadWriteStream(roomData, roomRegSize * roomCount, DisposeAfterUse::NO);
	roomFile.close();
}

void resetGameState() {

	characterPosX = 160;
	characterPosY = 80;
	iframe = 0;
	trajectory[0].x = characterPosX;
	trajectory[0].y = characterPosY;
	xframe2 = 0;
	yframe2 = 1;
	currentZone = 1;
	targetZone = 1;
	oldTargetZone = 0;
	charFacingDirection = 1;
	firstTimeTopicA[0] = true;
	firstTimeTopicA[1] = true;
	firstTimeTopicA[2] = true;
	firstTimeTopicA[3] = true;
	firstTimeTopicA[4] = true;
	firstTimeTopicA[5] = true;
	firstTimeTopicA[6] = true;
	firstTimeTopicA[7] = true;
	firstTimeTopicA[8] = true;
	firstTimeTopicB[0] = false;
	firstTimeTopicB[1] = false;
	firstTimeTopicB[2] = false;
	firstTimeTopicB[3] = false;
	firstTimeTopicB[4] = false;
	firstTimeTopicB[5] = false;
	firstTimeTopicB[6] = false;
	firstTimeTopicB[7] = false;
	firstTimeTopicB[8] = false;

	firstTimeTopicC[0] = false;
	firstTimeTopicC[1] = false;
	firstTimeTopicC[2] = false;
	firstTimeTopicC[3] = false;
	firstTimeTopicC[4] = false;
	firstTimeTopicC[5] = false;
	firstTimeTopicC[6] = false;
	firstTimeTopicC[7] = false;
	firstTimeTopicC[8] = false;

	bookTopic[0] = false;
	bookTopic[1] = false;
	bookTopic[2] = false;
	bookTopic[3] = false;
	bookTopic[4] = false;
	bookTopic[5] = false;
	bookTopic[6] = false;
	bookTopic[7] = false;
	bookTopic[8] = false;

	mintTopic[0] = false;
	mintTopic[1] = false;
	mintTopic[2] = false;
	mintTopic[3] = false;
	mintTopic[4] = false;
	mintTopic[5] = false;
	mintTopic[6] = false;
	mintTopic[7] = false;
	mintTopic[8] = false;

	caves[0] = false;
	caves[1] = false;
	caves[2] = false;
	caves[3] = false;
	caves[4] = false;

	isSecondaryAnimationEnabled = false;
	mainCharAnimation.depth = 0;
	rightSfxVol = 6;
	leftSfxVol = 6;
	musicVolRight = 3;
	musicVolLeft = 3;

	isDrawingEnabled = true;
	isSavingDisabled = false;
	startNewGame = false;
	shouldQuitGame = false;
	obtainedList1 = false;
	obtainedList2 = false;

	list1Complete = false;
	list2Complete = false;

	isPaletteAnimEnabled = 0;
	gamePart = 1;

	isVasePlaced = false;
	isScytheTaken = false;
	isTridentTaken = false;
	isPottersWheelDelivered = false;
	isMudDelivered = false;
	isSealRemoved = false;

	isGreenDevilDelivered = false;
	isRedDevilCaptured = false;
	isCupboardOpen = false;
	isChestOpen = false;

	isTVOn = false;
	isTrapSet = false;
	palAnimStep = 0;

	niche[0][0] = 563;
	niche[0][1] = 561;
	niche[0][2] = 0;
	niche[0][3] = 2;

	niche[1][0] = 615;
	niche[1][1] = 622;
	niche[1][2] = 623;
	niche[1][3] = 0;

	currentTrajectoryIndex = 0;
	inventoryPosition = 0;
}

void initPlayAnim() {
	debug("initplayanim!");
	isLoadingFromLauncher = false;
	decryptionKey = "23313212133122121312132132312312122132322131221322222112121"
					"32121121212112111212112333131232323213222132123211213221231"
					"32132213232333333213132132132322113212132121322123121232332"
					"23123221322213233221112312231221233232122332211112233122321"
					"222312211322312223";

	rooms = nullptr;
	conversationData = nullptr;
	invItemData = nullptr;
	// encriptado[0]  = encripcod1;
	// encriptado[1]  = '\63';
	// encriptado[2]  = '\63';
	// encriptado[3]  = encripcod1 - 1;
	// encriptado[4]  = '\63';
	// encriptado[5]  = encripcod1;
	// encriptado[6]  = encripcod1 - 1;
	// encriptado[7]  = encripcod1;
	// encriptado[8]  = encripcod1 - 1;
	// encriptado[9]  = '\63';
	// encriptado[10] = '\63';
	// encriptado[11] = encripcod1 - 1;
	// encriptado[12] = encripcod1;
	// encriptado[13] = encripcod1;
	// encriptado[14] = encripcod1 - 1;
	// encriptado[15] = encripcod1;
	// encriptado[16] = encripcod1 - 1;
	// encriptado[17] = '\63';
	// encriptado[18] = encripcod1 - 1;
	// encriptado[19] = encripcod1;
	// encriptado[20] = encripcod1 - 1;
	// encriptado[21] = '\63';
	// encriptado[22] = encripcod1;
	// encriptado[23] = encripcod1 - 1;
	// encriptado[24] = '\63';
	// encriptado[25] = encripcod1;
	// encriptado[26] = '\63';
	// encriptado[27] = encripcod1 - 1;
	// encriptado[28] = encripcod1;
	// encriptado[29] = '\63';
	// encriptado[30] = encripcod1 - 1;
	// encriptado[31] = encripcod1;
	// encriptado[32] = encripcod1 - 1;
	// encriptado[33] = encripcod1;
	// encriptado[34] = encripcod1;
	// encriptado[35] = encripcod1 - 1;
	// encriptado[36] = '\63';
	// encriptado[37] = encripcod1;
	// encriptado[38] = '\63';
	// encriptado[39] = encripcod1;
	// encriptado[40] = encripcod1;
	// encriptado[41] = encripcod1 - 1;
	// encriptado[42] = '\63';
	// encriptado[43] = encripcod1 - 1;
	// encriptado[44] = encripcod1;
	// encriptado[45] = encripcod1;
	// encriptado[46] = encripcod1 - 1;
	// encriptado[47] = '\63';
	// encriptado[48] = encripcod1;
	// encriptado[49] = encripcod1;
	// encriptado[50] = encripcod1;
	// encriptado[51] = encripcod1;
	// encriptado[52] = encripcod1;
	// encriptado[53] = encripcod1 - 1;
	// encriptado[54] = encripcod1 - 1;
	// encriptado[55] = encripcod1;
	// encriptado[56] = encripcod1 - 1;
	// encriptado[57] = encripcod1;
	// encriptado[58] = encripcod1 - 1;
	// encriptado[59] = '\63';
	// encriptado[60] = encripcod1;
	// encriptado[61] = encripcod1 - 1;
	// encriptado[62] = encripcod1;
	// encriptado[63] = encripcod1 - 1;
	// encriptado[64] = encripcod1 - 1;
	// encriptado[65] = encripcod1;
	// encriptado[66] = encripcod1 - 1;
	// encriptado[67] = encripcod1;
	// encriptado[68] = encripcod1 - 1;
	// encriptado[69] = encripcod1;
	// encriptado[70] = encripcod1 - 1;
	// encriptado[71] = encripcod1 - 1;
	// encriptado[72] = encripcod1;
	// encriptado[73] = encripcod1 - 1;
	// encriptado[74] = encripcod1 - 1;
	// encriptado[75] = encripcod1 - 1;
	// encriptado[76] = encripcod1;
	// encriptado[77] = encripcod1 - 1;
	// encriptado[78] = encripcod1;
	// encriptado[79] = encripcod1 - 1;
	// encriptado[80] = encripcod1 - 1;
	// encriptado[81] = encripcod1;
	// encriptado[82] = '\63';
	// encriptado[83] = '\63';
	// encriptado[84] = '\63';
	// encriptado[85] = encripcod1 - 1;
	// encriptado[86] = '\63';
	// encriptado[87] = encripcod1 - 1;
	// encriptado[88] = encripcod1;
	// encriptado[89] = '\63';
	// encriptado[90] = encripcod1;
	// encriptado[91] = '\63';
	// encriptado[92] = encripcod1;
	// encriptado[93] = '\63';
	// encriptado[94] = encripcod1;
	// encriptado[95] = encripcod1 - 1;
	// encriptado[96] = '\63';
	// encriptado[97] = encripcod1;
	// encriptado[98] = encripcod1;
	// encriptado[99] = encripcod1;
	// encriptado[100] = encripcod1 - 1;
	// encriptado[101] = '\63';
	// encriptado[102] = encripcod1;
	// encriptado[103] = encripcod1 - 1;
	// encriptado[104] = encripcod1;
	// encriptado[105] = '\63';
	// encriptado[106] = encripcod1;
	// encriptado[107] = encripcod1 - 1;
	// encriptado[108] = encripcod1 - 1;
	// encriptado[109] = encripcod1;
	// encriptado[110] = encripcod1 - 1;
	// encriptado[111] = '\63';
	// encriptado[112] = encripcod1;
	// encriptado[113] = encripcod1;
	// encriptado[114] = encripcod1 - 1;
	// encriptado[115] = encripcod1;
	// encriptado[116] = '\63';
	// encriptado[117] = encripcod1 - 1;
	// encriptado[118] = '\63';
	// encriptado[119] = encripcod1;
	// encriptado[120] = encripcod1 - 1;
	// encriptado[121] = '\63';
	// encriptado[122] = encripcod1;
	// encriptado[123] = encripcod1;
	// encriptado[124] = encripcod1 - 1;
	// encriptado[125] = '\63';
	// encriptado[126] = encripcod1;
	// encriptado[127] = '\63';
	// encriptado[128] = encripcod1;
	// encriptado[129] = '\63';
	// encriptado[130] = '\63';
	// encriptado[131] = '\63';
	// encriptado[132] = '\63';
	// encriptado[133] = '\63';
	// encriptado[134] = '\63';
	// encriptado[135] = encripcod1;
	// encriptado[136] = encripcod1 - 1;
	// encriptado[137] = '\63';
	// encriptado[138] = encripcod1 - 1;
	// encriptado[139] = '\63';
	// encriptado[140] = encripcod1;
	// encriptado[141] = encripcod1 - 1;
	// encriptado[142] = '\63';
	// encriptado[143] = encripcod1;
	// encriptado[144] = encripcod1 - 1;
	// encriptado[145] = '\63';
	// encriptado[146] = encripcod1;
	// encriptado[147] = '\63';
	// encriptado[148] = encripcod1;
	// encriptado[149] = encripcod1;
	// encriptado[150] = encripcod1 - 1;
	// encriptado[151] = encripcod1 - 1;
	// encriptado[152] = '\63';
	// encriptado[153] = encripcod1;
	// encriptado[154] = encripcod1 - 1;
	// encriptado[155] = encripcod1;
	// encriptado[156] = encripcod1 - 1;
	// encriptado[157] = '\63';
	// encriptado[158] = encripcod1;
	// encriptado[159] = encripcod1 - 1;
	// encriptado[160] = encripcod1;
	// encriptado[161] = encripcod1 - 1;
	// encriptado[162] = '\63';
	// encriptado[163] = encripcod1;
	// encriptado[164] = encripcod1;
	// encriptado[165] = encripcod1 - 1;
	// encriptado[166] = encripcod1;
	// encriptado[167] = '\63';
	// encriptado[168] = encripcod1 - 1;
	// encriptado[169] = encripcod1;
	// encriptado[170] = encripcod1 - 1;
	// encriptado[171] = encripcod1;
	// encriptado[172] = '\63';
	// encriptado[173] = encripcod1;
	// encriptado[174] = '\63';
	// encriptado[175] = '\63';
	// encriptado[176] = encripcod1;
	// encriptado[177] = encripcod1;
	// encriptado[178] = '\63';
	// encriptado[179] = encripcod1 - 1;
	// encriptado[180] = encripcod1;
	// encriptado[181] = '\63';
	// encriptado[182] = encripcod1;
	// encriptado[183] = encripcod1;
	// encriptado[184] = encripcod1 - 1;
	// encriptado[185] = '\63';
	// encriptado[186] = encripcod1;
	// encriptado[187] = encripcod1;
	// encriptado[188] = encripcod1;
	// encriptado[189] = encripcod1 - 1;
	// encriptado[190] = '\63';
	// encriptado[191] = encripcod1;
	// encriptado[192] = '\63';
	// encriptado[193] = '\63';
	// encriptado[194] = encripcod1;
	// encriptado[195] = encripcod1;
	// encriptado[196] = encripcod1 - 1;
	// encriptado[197] = encripcod1 - 1;
	// encriptado[198] = encripcod1 - 1;
	// encriptado[199] = encripcod1;
	// encriptado[200] = '\63';
	// encriptado[201] = encripcod1 - 1;
	// encriptado[202] = encripcod1;
	// encriptado[203] = encripcod1;
	// encriptado[204] = '\63';
	// encriptado[205] = encripcod1 - 1;
	// encriptado[206] = encripcod1;
	// encriptado[207] = encripcod1;
	// encriptado[208] = encripcod1 - 1;
	// encriptado[209] = encripcod1;
	// encriptado[210] = '\63';
	// encriptado[211] = '\63';
	// encriptado[212] = encripcod1;
	// encriptado[213] = '\63';
	// encriptado[214] = encripcod1;
	// encriptado[215] = encripcod1 - 1;
	// encriptado[216] = encripcod1;
	// encriptado[217] = encripcod1;
	// encriptado[218] = '\63';
	// encriptado[219] = '\63';
	// encriptado[220] = encripcod1;
	// encriptado[221] = encripcod1;
	// encriptado[222] = encripcod1 - 1;
	// encriptado[223] = encripcod1 - 1;
	// encriptado[224] = encripcod1 - 1;
	// encriptado[225] = encripcod1 - 1;
	// encriptado[226] = encripcod1;
	// encriptado[227] = encripcod1;
	// encriptado[228] = '\63';
	// encriptado[229] = '\63';
	// encriptado[230] = encripcod1 - 1;
	// encriptado[231] = encripcod1;
	// encriptado[232] = encripcod1;
	// encriptado[233] = '\63';
	// encriptado[234] = encripcod1;
	// encriptado[235] = encripcod1 - 1;
	// encriptado[236] = encripcod1;
	// encriptado[237] = encripcod1;
	// encriptado[238] = encripcod1;
	// encriptado[239] = '\63';
	// encriptado[240] = encripcod1 - 1;
	// encriptado[241] = encripcod1;
	// encriptado[242] = encripcod1;
	// encriptado[243] = encripcod1 - 1;
	// encriptado[244] = encripcod1 - 1;
	// encriptado[245] = '\63';
	// encriptado[246] = encripcod1;
	// encriptado[247] = encripcod1;
	// encriptado[248] = '\63';
	// encriptado[249] = encripcod1 - 1;
	// encriptado[250] = encripcod1;
	// encriptado[251] = encripcod1;
	// encriptado[252] = encripcod1;
	// encriptado[253] = '\63';
	timeToDraw = false;
	for (int i = 0; i < numScreenOverlays; i++) {
		screenLayers[i] = NULL;
	}
	mouseX = 160;
	mouseY = 100;
	mouseClickX = mouseX;
	mouseClickY = mouseY;
	mouseMaskIndex = 1;

	resetGameState();
	firstList[0] = 222;
	firstList[1] = 295;
	firstList[2] = 402;
	firstList[3] = 223;
	firstList[4] = 521;

	secondList[0] = 221;
	secondList[1] = 423;
	secondList[2] = 308;
	secondList[3] = 362;
	secondList[4] = 537;
	cpCounter = 0;
	cpCounter2 = 0;
	continueGame = true;
	firstTimeDone = false;
	isIntroSeen = false;
	inGame = false;
}

void clearVars() {
	if(sceneBackground != NULL) {
		free(sceneBackground);
	}
	if(backgroundCopy != NULL) {
		free(backgroundCopy);
	}
	if(conversationData != NULL) {
		free(conversationData);
	}
	if(rooms != NULL) {
		free(rooms);
	}
	if(invItemData != NULL) {
		free(invItemData);
	}
	for(int i = 0; i < numScreenOverlays; i++) {
		if(screenLayers[i] != NULL) {
			free(screenLayers[i]);
		}
	}
	for(int i = 0; i < inventoryIconCount; i++) {
		if(inventoryIconBitmaps[i] != NULL) {
			free(inventoryIconBitmaps[i]);
		}
	}
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < walkFrameCount + 30; j++) {
			if(mainCharAnimation.bitmap[i][j] != NULL) {
				free(mainCharAnimation.bitmap[i][j]);
			}
		}
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < secAnimationFrameCount; j++) {
			if(secondaryAnimation.bitmap[i][j] != NULL){
				free(secondaryAnimation.bitmap[i][j]);
			}
		}
	}
}
} // End of namespace Tot
