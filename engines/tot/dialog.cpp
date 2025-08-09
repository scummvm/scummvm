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

#define __conversa_implementation__

#include "common/scummsys.h"
#include "common/textconsole.h"

#include "tot/chrono.h"
#include "tot/dialog.h"
#include "tot/mouse.h"
#include "tot/routines.h"
#include "tot/routines2.h"
#include "tot/texts.h"
#include "tot/tot.h"

namespace Tot {

byte conversationIndex;

listP l1, l;
Tree ar, auxTree, step;

bool endOfConversation;

Common::String decrypt(Common::String encryptedText) {
	for (int i = 0; i < encryptedText.size(); i++) {
		encryptedText.setChar(decryptionKey[i] ^ (char)encryptedText[i], i);
	}
	return encryptedText;
}

void findDialogLine(byte characterIndex);

static void findDownwards(Tree curTree, bool &descend) {
	if (curTree != NULL) {
		if (curTree->element.spoken != '1') {
			descend = true;
			return;
		} else {
			findDownwards(leftChild(curTree), descend);
			findDownwards(rightSibling(curTree), descend);
		}
	}
}

void findDialogLine(byte characterIndex) {
	bool speak, ascend, descend, border, forward;

	auxTree = ar;
	auxTree = auxTree->child;
	bool done = false;
	l = new list;
	l->next = NULL;
	l1 = l;
	border = false;
	ascend = false;
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
			step = NULL;
		} break;
		case '3':
			if (bookTopic[0] && (characterIndex == 3)) {
				speak = false;
				ascend = true;
			} else
				speak = true;
			break;
		case '4':
			if (firstTimeTopicA[characterIndex - 1]) {
				speak = false;
				ascend = true;
			} else
				speak = true;
			break;
		case '5':
			if (bookTopic[characterIndex - 1] && firstTimeTopicB[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case '6':
			if (bookTopic[characterIndex - 1] && !firstTimeTopicB[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case '7':
			if (bookTopic[characterIndex - 1]) {
				speak = false;
				ascend = true;
			} else if (!firstTimeTopicA[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case '8':
			if (mintTopic[characterIndex - 1] && firstTimeTopicC[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case '9':
			if (mintTopic[characterIndex - 1] && !firstTimeTopicC[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case 'A':
			if (!mintTopic[characterIndex - 1] && !firstTimeTopicA[characterIndex - 1])
				speak = true;
			else {
				speak = false;
				ascend = true;
			}
			break;
		case 'B':
			if (caves[0] && !firstTimeTopicA[8])
				speak = true;
			else
				speak = false;
			break;
		case 'C':
			if (caves[1] && !firstTimeTopicA[8])
				speak = true;
			else
				speak = false;
			break;
		case 'D':
			if ((caves[0] && caves[1]) && !firstTimeTopicA[8])
				speak = true;
			else
				speak = false;
			break;
		case 'E':
			if ((caves[0] && !caves[2]) && !firstTimeTopicA[8])
				speak = true;
			else
				speak = false;
			break;
		case 'F':
			if (!caves[3])
				speak = true;
			else
				speak = false;
			break;
		case 'G':
			if (!caves[4])
				speak = true;
			else
				speak = false;
			break;
		case 'I':
			if (!isSealRemoved)
				speak = true;
			else
				speak = false;
			break;
		case 'Z':
			speak = false;
			break;
		}
		if (speak) {
			if (auxTree->element.spoken == '2')
				if (!border) {
					border = true;
					switch (characterIndex) {
					case 1:
						if (firstTimeTopicA[characterIndex - 1]) {
							l1->item = 12;
							forward = true;
						} else if (bookTopic[characterIndex - 1]) {
							forward = true;
							l1->item = 33;
						} else {
							l1->item = 21;
							forward = true;
						}
						break;
					case 3:
						if (firstTimeTopicA[characterIndex - 1]) {
							l1->item = 103;
							forward = true;
						} else {
							l1->item = 112;
							forward = true;
						}
						break;
					default: {
						l1->item = auxTree->element.index;
						forward = true;
					}
					}
				} else {
					;
				}
			else {
				l1->item = auxTree->element.index;
				forward = true;
			}
			if (forward) {
				forward = false;
				l1->next = new list;
				l1 = l1->next;
				l1->next = NULL;
			}
			if (rightSibling(auxTree) != NULL)
				auxTree = rightSibling(auxTree);
			else {
				do {
					if (!isRoot(parent(auxTree)))
						auxTree = parent(auxTree);
					else
						break;
				} while (!(auxTree->element.spoken == '1' && rightSibling(auxTree) != NULL));
				if (rightSibling(auxTree) != NULL)
					auxTree = rightSibling(auxTree);
				else
					done = true;
			}
		} else if (ascend) {
			ascend = false;
			if (rightSibling(auxTree) != NULL)
				auxTree = rightSibling(auxTree);
			else {
				do {
					if (!isRoot(parent(auxTree)))
						auxTree = parent(auxTree);
					else
						break;
				} while (!((auxTree->element.spoken == '1') && (rightSibling(auxTree) != NULL)));
				if (rightSibling(auxTree) != NULL)
					auxTree = rightSibling(auxTree);
				else
					done = true;
			}
		} else if (leftChild(auxTree) != NULL)
			auxTree = leftChild(auxTree);
		else if (rightSibling(auxTree) != NULL)
			auxTree = rightSibling(auxTree);
		else {
			auxTree = parent(auxTree);
			if (rightSibling(auxTree) != NULL)
				auxTree = rightSibling(auxTree);
			else {
				do {
					auxTree = parent(auxTree);
				} while (!(isRoot(auxTree) || rightSibling(auxTree) != NULL));
				if (isRoot(auxTree))
					done = true;
				else
					auxTree = rightSibling(auxTree);
			}
		}
	} while (!done);
	auxTree = NULL;
	step = NULL;
	l1 = NULL;
}

void modifyTree(uint node) {

	bool found = false;
	auxTree = ar->child;
	do {
		if (auxTree->element.index == node) {

			if ((auxTree->element.spoken != '2') && (auxTree->element.spoken != 'H'))
				auxTree->element.spoken = '1';
			else if (auxTree->element.spoken != 'H')
				auxTree->element.spoken = 'Z';
			found = true;
		} else {
			if (leftChild(auxTree) != NULL)
				auxTree = leftChild(auxTree);
			else if (rightSibling(auxTree) != NULL)
				auxTree = rightSibling(auxTree);
			else {
				do {
					auxTree = parent(auxTree);
				} while (!(rightSibling(auxTree) != NULL));
				auxTree = rightSibling(auxTree);
			}
		}
	} while (!found);
}

void drawTalkMenu() {
	byte auxTextY;
	g_engine->_mouseManager->hide();
	for (auxTextY = 25; auxTextY >= 1; auxTextY--)
		rectangle(0, 175 - auxTextY, 319, 174 + auxTextY, 0);
	for (auxTextY = 1; auxTextY <= 25; auxTextY++)
		buttonBorder(0, 175 - auxTextY, 319, 174 + auxTextY, 253, 253, 253, 253, 0);
	drawMenu(5);
	g_engine->_mouseManager->show();
}

void fixTree(Tree tree) {
	if (tree != NULL) {
		if (tree->element.spoken == 'Z')
			tree->element.spoken = '2';
		else {
			fixTree(leftChild(tree));
			fixTree(rightSibling(tree));
		}
	}
}

void showDialogLine(Common::String conversationMatrix[16], uint &choosenTopic) {
	byte firstChat, convselec;

	firstChat = 1;
	convselec = 0;
	g_engine->_mouseManager->hide();

	drawMenu(5);
	euroText(6, 151, conversationMatrix[1], 255);
	euroText(6, 162, conversationMatrix[2], 255);
	euroText(6, 173, conversationMatrix[3], 255);
	euroText(6, 184, conversationMatrix[4], 255);
	g_engine->_mouseManager->show();
	Common::Event e;
	do {
		bool lMouseClicked = false;
		bool rMouseClicked = false;
		do {
			g_engine->_chrono->updateChrono();
			g_engine->_mouseManager->animateMouseIfNeeded();

			while (g_system->getEventManager()->pollEvent(e)) {
				if (e.type == Common::EVENT_LBUTTONUP) {
					lMouseClicked = true;
					mouseClickX = e.mouse.x;
					mouseClickY = e.mouse.y;
				} else if (e.type == Common::EVENT_RBUTTONUP) {
					rMouseClicked = true;
					mouseClickX = e.mouse.x;
					mouseClickY = e.mouse.y;
				}
			}

			if (timeToDraw) {
				if (currentRoomData->paletteAnimationFlag && palAnimStep >= 4) {
					palAnimStep = 0;
					if (isPaletteAnimEnabled > 6)
						isPaletteAnimEnabled = 0;
					else
						isPaletteAnimEnabled += 1;
					updatePalette(isPaletteAnimEnabled);
				} else
					palAnimStep += 1;
			}
			g_system->delayMillis(10);
			g_engine->_screen->update();
		} while (!lMouseClicked && !rMouseClicked && !g_engine->shouldQuit());

		if (lMouseClicked) {
			if (mouseClickY < 143)
				convselec = 0;
			else {
				if (mouseClickX >= 0 && mouseClickX <= 280) {
					if (mouseClickY >= 143 && mouseClickY <= 155) {
						convselec = firstChat;
					} else if (mouseClickY >= 156 && mouseClickY <= 166) {
						convselec = firstChat + 1;
					} else if (mouseClickY >= 167 && mouseClickY <= 177) {
						convselec = firstChat + 2;
					} else if (mouseClickY >= 178 && mouseClickY <= 186) {
						convselec = firstChat + 3;
					}
				} else if (mouseClickX >= 281 && mouseClickX <= 319) {
					if (mouseClickY >= 143 && mouseClickY <= 165) {
						if (firstChat > 1) {
							convselec = 0;
							firstChat -= 1;
							g_engine->_mouseManager->hide();
							drawMenu(5);
							euroText(6, 151, conversationMatrix[firstChat], 255);
							euroText(6, 162, conversationMatrix[firstChat + 1], 255);
							euroText(6, 173, conversationMatrix[firstChat + 2], 255);
							euroText(6, 184, conversationMatrix[firstChat + 3], 255);
							g_engine->_mouseManager->show();
						}
					} else if (mouseClickY >= 167 && mouseClickY <= 186) {
						if (firstChat < 12) {
							convselec = 0;
							firstChat += 1;
							g_engine->_mouseManager->hide();
							drawMenu(5);
							euroText(6, 151, conversationMatrix[firstChat], 255);
							euroText(6, 162, conversationMatrix[firstChat + 1], 255);
							euroText(6, 173, conversationMatrix[firstChat + 2], 255);
							euroText(6, 184, conversationMatrix[firstChat + 3], 255);
							g_engine->_mouseManager->show();
						}
					}
				}
			}
		} else if (rMouseClicked)
			convselec = conversationIndex;
	} while (!((convselec > 0) && (convselec <= conversationIndex)) && !g_engine->shouldQuit());

	if (convselec == conversationIndex)
		endOfConversation = true;
	for (int i = 1; i <= (convselec - 1); i++)
		l1 = l1->next;
	choosenTopic = l1->item;
}

void talk(byte characterIndex) {
	debug("Talking to person: %d", characterIndex);
	uint response, newNode;
	TextEntry text;
	byte stringAux, insertName, invIndex;

	Common::String conversationMatrix[16];
	drawTalkMenu();
	endOfConversation = false;
	assignText();
	// The original game makes a copy of the file upon starting a new game. .007 is the current game (the game
	// that resumes when clicking "continue game" in the main menu. Part of the savegame data is this 007
	// conversation file which marks conversatino topics as already gone through or not.
	readTree(*conversationData, ar, characterIndex - 1);
	loadTalkAnimations();
	do {

		for (int i = 0; i < 16; i++) {
			conversationMatrix[i] = "";
		}

		findDialogLine(characterIndex);
		conversationIndex = 0;
		l1 = l;
		do {
			verb.seek(verbRegSize * l1->item);
			conversationIndex += 1;
			text = readVerbRegister();
			insertName = 0;
			conversationMatrix[conversationIndex] = decrypt(text.text);

			for (int i = 0; i < conversationMatrix[conversationIndex].size(); i++) {
				if (conversationMatrix[conversationIndex][i] == '@')
					insertName = i;
			}
			if (insertName > 0) {
				conversationMatrix[conversationIndex].deleteChar(insertName);
				conversationMatrix[conversationIndex].insertString(characterName, insertName);
			}
			if (conversationMatrix[conversationIndex].size() > 45) {
				stringAux = 45;
				do {
					stringAux -= 1;
				} while (conversationMatrix[conversationIndex][stringAux] != ' ');
				conversationMatrix[conversationIndex] = conversationMatrix[conversationIndex].substr(0, stringAux);
				conversationMatrix[conversationIndex].insertString(" ...", stringAux);
			}
			l1 = l1->next;
		} while ((l1->next != NULL) && (l1 != NULL) && !g_engine->shouldQuit());
		l1 = l;
		showDialogLine(conversationMatrix, newNode);
		delete l;
		hypertext(newNode, 255, 0, response, true);
		stringAux = 0;
		modifyTree(newNode);
		// 	verifyCopyProtection();
		while (response > 0 && !g_engine->shouldQuit()) {
			newNode = response;
			stringAux += 1;
			if (odd(stringAux))
				hypertext(newNode, 253, 249, response, true);
			else
				hypertext(newNode, 255, 0, response, true);
			switch (newNode) {
			case 9: {
				obtainedList1 = true;
				invIndex = 0;
				while (mobj[invIndex].code != 0) {
					invIndex += 1;
				}
				mobj[invIndex].bitmapIndex = list1Index;
				mobj[invIndex].code = list1code;
				mobj[invIndex].objectName = getObjectName(0);
			} break;
			case 25: {
				obtainedList2 = true;
				invIndex = 0;
				while (mobj[invIndex].code != 0) {
					invIndex += 1;
				}
				mobj[invIndex].bitmapIndex = list2Index;
				mobj[invIndex].code = list2code;
				mobj[invIndex].objectName = getObjectName(1);
			} break;
			}
		}
		g_system->delayMillis(10);
	} while (!endOfConversation && !g_engine->shouldQuit());

	unloadTalkAnimations();
	step = ar;
	fixTree(step);
	saveConversations(conversationData, ar, characterIndex - 1);

	verb.close();
	if (g_engine->shouldQuit()) {
		return;
	}
	delete ar;
	l1 = NULL;
	g_engine->_mouseManager->hide();

	for (int i = 25; i >= 1; i--)
		rectangle(0, 175 - i, 319, 174 + i, 0);
	mask();
	drawBackpack();
	g_engine->_mouseManager->show();

	if (characterIndex < 5) {
		if (firstTimeTopicA[characterIndex - 1])
			firstTimeTopicA[characterIndex - 1] = false;
		if (firstTimeTopicB[characterIndex - 1])
			firstTimeTopicB[characterIndex - 1] = false;
		if (firstTimeTopicC[characterIndex - 1])
			firstTimeTopicC[characterIndex - 1] = false;
	} else if (characterIndex == 8)
		firstTimeTopicA[8] = false;
}

void talkToSceneObject() {
	int mouseX = (mouseClickX + 7) / xGridCount;
	int mouseY = (mouseClickY + 7) / yGridCount;
	uint sceneObject = currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[mouseX][mouseY]]->fileIndex;
	if (sceneObject == 0)
		return;

	// verifyCopyProtection2();
	readItemRegister(sceneObject);
	goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount],
			   currentRoomData->walkAreasGrid[mouseX][mouseY]);

	if (regobj.speaking > 0) {
		talk(regobj.speaking);
	} else {
		assignText();
		hypertext((Random(10) + 1039), 255, 0, foo, false);
		verb.close();
		if (cpCounter > 198)
			showError(274);
	}
}

} // End of namespace Tot
