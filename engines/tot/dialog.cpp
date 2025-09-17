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

#include "tot/dialog.h"
#include "tot/tot.h"

namespace Tot {

Common::String decrypt(Common::String encryptedText) {
	for (uint i = 0; i < encryptedText.size(); i++) {
		encryptedText.setChar(g_engine->_decryptionKey[i] ^ (char)encryptedText[i], i);
	}
	return encryptedText;
}

Common::List<uint>* findDialogue(Tree tree, byte characterIndex);

static void findDownwards(Tree curTree, bool &descend) {
	if (curTree != nullptr) {
		if (curTree->element.spoken != '1') {
			descend = true;
			return;
		} else {
			findDownwards(leftChild(curTree), descend);
			findDownwards(rightSibling(curTree), descend);
		}
	}
}

Common::List<uint>* findDialogue(Tree tree, byte characterIndex) {
	bool speak = false, ascend = false, descend = false, border = false, forward = false, done = false;

	Tree auxTree = tree->child;
	Common::List<uint> *linkedList = new Common::List<uint>();
	Tree step;
	do {
		switch (auxTree->element.spoken) {
		case '0':
		case '2':
		case 'H':
			speak = true;
			break;
		case '1': {
			speak = false;
			step = auxTree->child;
			descend = false;
			findDownwards(step, descend);
			if (!(descend))
				ascend = true;
			step = nullptr;
		} break;
		case '3':
			if (g_engine->_bookTopic[0] && (characterIndex == 3)) {
				speak = false;
				ascend = true;
			} else
				speak = true;
			break;
		case '4':
			if (g_engine->_firstTimeTopicA[characterIndex - 1]) {
				speak = false;
				ascend = true;
			} else
				speak = true;
			break;
		case '5':
			if (g_engine->_bookTopic[characterIndex - 1] && g_engine->_firstTimeTopicB[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case '6':
			if (g_engine->_bookTopic[characterIndex - 1] && !g_engine->_firstTimeTopicB[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case '7':
			if (g_engine->_bookTopic[characterIndex - 1]) {
				speak = false;
				ascend = true;
			} else if (!g_engine->_firstTimeTopicA[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case '8':
			if (g_engine->_mintTopic[characterIndex - 1] && g_engine->_firstTimeTopicC[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case '9':
			if (g_engine->_mintTopic[characterIndex - 1] && !g_engine->_firstTimeTopicC[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case 'A':
			if (!g_engine->_mintTopic[characterIndex - 1] && !g_engine->_firstTimeTopicA[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case 'B':
			if (g_engine->_caves[0] && !g_engine->_firstTimeTopicA[8])
				speak = true;
			else
				speak = false;
			break;
		case 'C':
			if (g_engine->_caves[1] && !g_engine->_firstTimeTopicA[8])
				speak = true;
			else
				speak = false;
			break;
		case 'D':
			if ((g_engine->_caves[0] && g_engine->_caves[1]) && !g_engine->_firstTimeTopicA[8])
				speak = true;
			else
				speak = false;
			break;
		case 'E':
			if ((g_engine->_caves[0] && !g_engine->_caves[2]) && !g_engine->_firstTimeTopicA[8])
				speak = true;
			else
				speak = false;
			break;
		case 'F':
			if (!g_engine->_caves[3])
				speak = true;
			else
				speak = false;
			break;
		case 'G':
			if (!g_engine->_caves[4])
				speak = true;
			else
				speak = false;
			break;
		case 'I':
			if (!g_engine->_isSealRemoved)
				speak = true;
			else
				speak = false;
			break;
		case 'Z':
			speak = false;
			break;
		}
		if (speak) {
			if (auxTree->element.spoken == '2') {
				if (!border) {
					border = true;
					switch (characterIndex) {
					case 1:
						if (g_engine->_firstTimeTopicA[characterIndex - 1]) {
							linkedList->push_back(12);
							forward = true;
						} else if (g_engine->_bookTopic[characterIndex - 1]) {
							forward = true;
							linkedList->push_back(33);
						} else {
							linkedList->push_back(21);
							forward = true;
						}
						break;
					case 3:
						if (g_engine->_firstTimeTopicA[characterIndex - 1]) {
							linkedList->push_back(103);
							forward = true;
						} else {
							linkedList->push_back(112);
							forward = true;
						}
						break;
					default: {
						linkedList->push_back(auxTree->element.index);
						forward = true;
					}
					}
				}
			} else {
				linkedList->push_back(auxTree->element.index);
				forward = true;
			}
			if (forward) {
				forward = false;
			}
			if (rightSibling(auxTree) != nullptr)
				auxTree = rightSibling(auxTree);
			else {
				do {
					if (!isRoot(parent(auxTree)))
						auxTree = parent(auxTree);
					else
						break;
				} while (!(auxTree->element.spoken == '1' && rightSibling(auxTree) != nullptr));
				if (rightSibling(auxTree) != nullptr)
					auxTree = rightSibling(auxTree);
				else
					done = true;
			}
		} else if (ascend) {
			ascend = false;
			if (rightSibling(auxTree) != nullptr)
				auxTree = rightSibling(auxTree);
			else {
				do {
					if (!isRoot(parent(auxTree)))
						auxTree = parent(auxTree);
					else
						break;
				} while (!((auxTree->element.spoken == '1') && (rightSibling(auxTree) != nullptr)));
				if (rightSibling(auxTree) != nullptr)
					auxTree = rightSibling(auxTree);
				else
					done = true;
			}
		} else if (leftChild(auxTree) != nullptr)
			auxTree = leftChild(auxTree);
		else if (rightSibling(auxTree) != nullptr)
			auxTree = rightSibling(auxTree);
		else {
			auxTree = parent(auxTree);
			if (rightSibling(auxTree) != nullptr)
				auxTree = rightSibling(auxTree);
			else {
				do {
					auxTree = parent(auxTree);
				} while (!(isRoot(auxTree) || rightSibling(auxTree) != nullptr));
				if (isRoot(auxTree))
					done = true;
				else
					auxTree = rightSibling(auxTree);
			}
		}
	} while (!done);
	auxTree = nullptr;
	step = nullptr;
	return linkedList;
}

void modifyTree(Tree tree, uint node) {

	bool found = false;
	Tree auxTree = tree->child;
	do {
		if (auxTree->element.index == node) {

			if ((auxTree->element.spoken != '2') && (auxTree->element.spoken != 'H'))
				auxTree->element.spoken = '1';
			else if (auxTree->element.spoken != 'H')
				auxTree->element.spoken = 'Z';
			found = true;
		} else {
			if (leftChild(auxTree) != nullptr)
				auxTree = leftChild(auxTree);
			else if (rightSibling(auxTree) != nullptr)
				auxTree = rightSibling(auxTree);
			else {
				do {
					auxTree = parent(auxTree);
				} while (!(rightSibling(auxTree) != nullptr));
				auxTree = rightSibling(auxTree);
			}
		}
	} while (!found);
}

void drawTalkMenu() {
	byte auxTextY;
	g_engine->_mouse->hide();
	for (auxTextY = 25; auxTextY >= 1; auxTextY--)
		rectangle(0, 175 - auxTextY, 319, 174 + auxTextY, 0);
	for (auxTextY = 1; auxTextY <= 25; auxTextY++)
		g_engine->buttonBorder(0, 175 - auxTextY, 319, 174 + auxTextY, 253, 253, 253, 253, 0);
	g_engine->drawMenu(5);
	g_engine->_mouse->show();
}

void fixTree(Tree tree) {
	if (tree != nullptr) {
		if (tree->element.spoken == 'Z')
			tree->element.spoken = '2';
		else {
			fixTree(leftChild(tree));
			fixTree(rightSibling(tree));
		}
	}
}

void showDialogueLine(
	Common::String conversationMatrix[16],
	uint &chosenTopic,
	byte conversationIndex,
	Common::ListInternal::Iterator<uint> l1,
	bool &endOfConversation) {

	byte firstChat = 1;
	byte selectedConv = 0;
	g_engine->_mouse->hide();

	g_engine->drawMenu(5);
	euroText(6, 151, conversationMatrix[1], 255);
	euroText(6, 162, conversationMatrix[2], 255);
	euroText(6, 173, conversationMatrix[3], 255);
	euroText(6, 184, conversationMatrix[4], 255);
	g_engine->_mouse->show();
	Common::Event e;
	do {
		bool lMouseClicked = false;
		bool rMouseClicked = false;
		do {
			g_engine->_chrono->updateChrono();
			g_engine->_mouse->animateMouseIfNeeded();

			while (g_system->getEventManager()->pollEvent(e)) {
				if (e.type == Common::EVENT_LBUTTONUP) {
					lMouseClicked = true;
					g_engine->_mouse->mouseClickX = e.mouse.x;
					g_engine->_mouse->mouseClickY = e.mouse.y;
				} else if (e.type == Common::EVENT_RBUTTONUP) {
					rMouseClicked = true;
					g_engine->_mouse->mouseClickX = e.mouse.x;
					g_engine->_mouse->mouseClickY = e.mouse.y;
				}
			}

			if (g_engine->_chrono->_gameTick) {
				g_engine->_graphics->advancePaletteAnim();
			}
			g_system->delayMillis(10);
			g_engine->_screen->update();
		} while (!lMouseClicked && !rMouseClicked && !g_engine->shouldQuit());

		if (lMouseClicked) {
			if (g_engine->_mouse->mouseClickY < 143)
				selectedConv = 0;
			else {
				if (g_engine->_mouse->mouseClickX <= 280) {
					if (g_engine->_mouse->mouseClickY >= 143 && g_engine->_mouse->mouseClickY <= 155) {
						selectedConv = firstChat;
					} else if (g_engine->_mouse->mouseClickY >= 156 && g_engine->_mouse->mouseClickY <= 166) {
						selectedConv = firstChat + 1;
					} else if (g_engine->_mouse->mouseClickY >= 167 && g_engine->_mouse->mouseClickY <= 177) {
						selectedConv = firstChat + 2;
					} else if (g_engine->_mouse->mouseClickY >= 178 && g_engine->_mouse->mouseClickY <= 186) {
						selectedConv = firstChat + 3;
					}
				} else if (g_engine->_mouse->mouseClickX >= 281 && g_engine->_mouse->mouseClickX <= 319) {
					if (g_engine->_mouse->mouseClickY >= 143 && g_engine->_mouse->mouseClickY <= 165) {
						if (firstChat > 1) {
							selectedConv = 0;
							firstChat -= 1;
							g_engine->_mouse->hide();
							g_engine->drawMenu(5);
							euroText(6, 151, conversationMatrix[firstChat], 255);
							euroText(6, 162, conversationMatrix[firstChat + 1], 255);
							euroText(6, 173, conversationMatrix[firstChat + 2], 255);
							euroText(6, 184, conversationMatrix[firstChat + 3], 255);
							g_engine->_mouse->show();
						}
					} else if (g_engine->_mouse->mouseClickY >= 167 && g_engine->_mouse->mouseClickY <= 186) {
						if (firstChat < 12) {
							selectedConv = 0;
							firstChat += 1;
							g_engine->_mouse->hide();
							g_engine->drawMenu(5);
							euroText(6, 151, conversationMatrix[firstChat], 255);
							euroText(6, 162, conversationMatrix[firstChat + 1], 255);
							euroText(6, 173, conversationMatrix[firstChat + 2], 255);
							euroText(6, 184, conversationMatrix[firstChat + 3], 255);
							g_engine->_mouse->show();
						}
					}
				}
			}
		} else if (rMouseClicked)
			selectedConv = conversationIndex;
	} while (!((selectedConv > 0) && (selectedConv <= conversationIndex)) && !g_engine->shouldQuit());

	if (selectedConv == conversationIndex)
		endOfConversation = true;
	for (int i = 1; i <= (selectedConv - 1); i++)
		l1++;
	chosenTopic = *l1;
}

void talk(byte characterIndex) {
	debug("Talking to person: %d", characterIndex);
	uint response, newNode;
	TextEntry text;
	byte stringAux, insertName, invIndex;

	Common::String conversationMatrix[16];
	drawTalkMenu();
	bool endOfConversation = false;
	g_engine->readTextFile();
	// The original game makes a copy of the file upon starting a new game. .007 is the current game (the game
	// that resumes when clicking "continue game" in the main menu. Part of the savegame data is this 007
	// conversation file which marks conversatino topics as already gone through or not.
	Tree tree;
	readTree(*g_engine->_conversationData, tree, characterIndex - 1);
	loadTalkAnimations();
	Common::ListInternal::Iterator<uint> l1;
	do {

		for (int i = 0; i < 16; i++) {
			conversationMatrix[i] = "";
		}

		Common::List<uint> *linkedList = findDialogue(tree, characterIndex);
		byte conversationIndex = 0;
		l1 = linkedList->begin();
		while (l1 != linkedList->end() && !g_engine->shouldQuit()) {
			g_engine->_verbFile.seek(kVerbRegSize * (*l1));
			conversationIndex += 1;
			text = g_engine->readTextRegister();
			insertName = 0;
			conversationMatrix[conversationIndex] = decrypt(text.text);

			for (uint i = 0; i < conversationMatrix[conversationIndex].size(); i++) {
				if (conversationMatrix[conversationIndex][i] == '@')
					insertName = i;
			}
			if (insertName > 0) {
				conversationMatrix[conversationIndex].deleteChar(insertName);
				conversationMatrix[conversationIndex].insertString(g_engine->_characterName, insertName);
			}
			if (conversationMatrix[conversationIndex].size() > 45) {
				stringAux = 45;
				do {
					stringAux -= 1;
				} while (conversationMatrix[conversationIndex][stringAux] != ' ');
				conversationMatrix[conversationIndex] = conversationMatrix[conversationIndex].substr(0, stringAux);
				conversationMatrix[conversationIndex].insertString(" ...", stringAux);
			}
			l1++;
		};
		l1 = linkedList->begin();
		showDialogueLine(conversationMatrix, newNode, conversationIndex, l1, endOfConversation);
		delete linkedList;
		g_engine->sayLine(newNode, 255, 0, response, true);
		stringAux = 0;
		modifyTree(tree, newNode);
		while (response > 0 && !g_engine->shouldQuit()) {
			newNode = response;
			stringAux += 1;
			if (odd(stringAux))
				g_engine->sayLine(newNode, 253, 249, response, true);
			else
				g_engine->sayLine(newNode, 255, 0, response, true);
			switch (newNode) {
			case 9: {
				g_engine->_obtainedList1 = true;
				invIndex = 0;
				while (g_engine->_inventory[invIndex].code != 0) {
					invIndex += 1;
				}
				g_engine->_inventory[invIndex].bitmapIndex = kList1Index;
				g_engine->_inventory[invIndex].code = kList1code;
				g_engine->_inventory[invIndex].objectName = getObjectName(0);
			} break;
			case 25: {
				g_engine->_obtainedList2 = true;
				invIndex = 0;
				while (g_engine->_inventory[invIndex].code != 0) {
					invIndex += 1;
				}
				g_engine->_inventory[invIndex].bitmapIndex = kList2Index;
				g_engine->_inventory[invIndex].code = kList2code;
				g_engine->_inventory[invIndex].objectName = getObjectName(1);
			} break;
			}
		}
		g_system->delayMillis(10);
	} while (!endOfConversation && !g_engine->shouldQuit());

	unloadTalkAnimations();
	Tree step = tree;
	fixTree(step);
	saveConversations(g_engine->_conversationData, tree, characterIndex - 1);

	g_engine->_verbFile.close();
	if (g_engine->shouldQuit()) {
		return;
	}
	delete tree;

	g_engine->_mouse->hide();

	for (int i = 25; i >= 1; i--)
		rectangle(0, 175 - i, 319, 174 + i, 0);
	g_engine->drawInventoryMask();
	g_engine->drawInventory();
	g_engine->_mouse->show();

	if (characterIndex < 5) {
		if (g_engine->_firstTimeTopicA[characterIndex - 1])
			g_engine->_firstTimeTopicA[characterIndex - 1] = false;
		if (g_engine->_firstTimeTopicB[characterIndex - 1])
			g_engine->_firstTimeTopicB[characterIndex - 1] = false;
		if (g_engine->_firstTimeTopicC[characterIndex - 1])
			g_engine->_firstTimeTopicC[characterIndex - 1] = false;
	} else if (characterIndex == 8)
		g_engine->_firstTimeTopicA[8] = false;
}

void talkToSceneObject() {
	Common::Point p = g_engine->_mouse->getClickCoordsWithinGrid();
	int correctedMouseX = p.x;
	int correctedMouseY = p.y;
	uint sceneObject = g_engine->_currentRoomData->screenObjectIndex[g_engine->_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex;
	if (sceneObject == 0)
		return;

	g_engine->readObject(sceneObject);
	g_engine->goToObject(g_engine->_currentRoomData->walkAreasGrid[(g_engine->_characterPosX + kCharacterCorrectionX) / kXGridCount][(g_engine->_characterPosY + kCharacerCorrectionY) / kYGridCount],
			   g_engine->_currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);

	if (g_engine->_curObject.speaking > 0) {
		talk(g_engine->_curObject.speaking);
	} else {
		g_engine->readTextFile();
		uint foo = 0;
		g_engine->sayLine((getRandom(10) + 1039), 255, 0, foo, false);
		g_engine->_verbFile.close();
		if (g_engine->_cpCounter > 198)
			showError(274);
	}
}

/**
 * Loads talking animation of main adn secondary character
 */
void loadTalkAnimations() {
	Common::File animFile;

	if (!animFile.open("TIOHABLA.SEC")) {
		showError(265);
	}
	g_engine->_mainCharFrameSize = animFile.readUint16LE();

	int32 offset = g_engine->_mainCharFrameSize * 16;
	offset = (offset * g_engine->_charFacingDirection) + 2;
	animFile.seek(offset);
	// Will load talking anim always in the upwards direction of the walk cycle array
	for (int i = 0; i < 16; i++) {
		g_engine->_mainCharAnimation.bitmap[0][i] = (byte *)malloc(g_engine->_mainCharFrameSize);
		animFile.read(g_engine->_mainCharAnimation.bitmap[0][i], g_engine->_mainCharFrameSize);
	}
	animFile.close();

	if ((g_engine->_currentRoomData->animationName != "PETER") && (g_engine->_currentRoomData->animationName != "ARZCAEL")) {
		g_engine->_iframe2 = 0;
		free(g_engine->_curSecondaryAnimationFrame);
		bool result;
		switch (g_engine->_curObject.speaking) {
		case 1:
			result = animFile.open("JOHN.SEC");
			break;
		case 5:
			result = animFile.open("ALFRED.SEC");
			break;
		default:
			result = animFile.open(Common::Path(g_engine->_currentRoomData->animationName + Common::String(".SEC")));
		}

		if (!result)
			showError(265);
		g_engine->_secondaryAnimFrameSize = animFile.readUint16LE();
		g_engine->_secondaryAnimationFrameCount = animFile.readByte();
		g_engine->_secondaryAnimDirCount = animFile.readByte();

		g_engine->_curSecondaryAnimationFrame = (byte *)malloc(g_engine->_secondaryAnimFrameSize);
		if (g_engine->_secondaryAnimDirCount != 0) {
			g_engine->_secondaryAnimationFrameCount = g_engine->_secondaryAnimationFrameCount / 4;
			for (int i = 0; i <= 3; i++) {
				g_engine->loadAnimationForDirection(&animFile, i);
			}
		} else {
			g_engine->loadAnimationForDirection(&animFile, 0);
		}
		animFile.close();
	}
}

void unloadTalkAnimations() {

	Common::File animFile;
	if (!animFile.open("PERSONAJ.SPT")) {
		showError(265);
	}
	g_engine->_mainCharFrameSize = animFile.readUint16LE();

	for (int i = 0; i < kWalkFrameCount; i++) {
		g_engine->_mainCharAnimation.bitmap[0][i] = (byte *)malloc(g_engine->_mainCharFrameSize);
		animFile.read(g_engine->_mainCharAnimation.bitmap[0][i], g_engine->_mainCharFrameSize);
	}
	animFile.close();

	if ((g_engine->_currentRoomData->animationName != "PETER") && (g_engine->_currentRoomData->animationName != "ARZCAEL")) {
		if (!animFile.open(Common::Path(g_engine->_currentRoomData->animationName + ".DAT"))) {
			showError(265);
		}
		g_engine->_secondaryAnimFrameSize = animFile.readUint16LE();
		g_engine->_secondaryAnimationFrameCount = animFile.readByte();
		g_engine->_secondaryAnimDirCount = animFile.readByte();
		g_engine->_curSecondaryAnimationFrame = (byte *)malloc(g_engine->_secondaryAnimFrameSize);
		if (g_engine->_secondaryAnimDirCount != 0) {

			g_engine->_secondaryAnimationFrameCount = g_engine->_secondaryAnimationFrameCount / 4;
			for (int i = 0; i <= 3; i++) {
				g_engine->loadAnimationForDirection(&animFile, i);
			}
		} else {
			g_engine->loadAnimationForDirection(&animFile, 0);
		}
		animFile.close();
	}
}


} // End of namespace Tot
