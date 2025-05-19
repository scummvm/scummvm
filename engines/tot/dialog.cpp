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
#include "tot/playanim.h"
#include "tot/routines.h"
#include "tot/routines2.h"
#include "tot/texts.h"
#include "tot/tot.h"

namespace Tot {

byte conversationIndex;

plista l1, l;
Tree ar, auxTree, step;

bool endOfConversation;

Common::String decrypt(Common::String tEncriptado) {
	for (int i = 0; i < tEncriptado.size(); i++) {
		tEncriptado.setChar(decryptionKey[i] ^ (char)tEncriptado[i], i);
	}
	return tEncriptado;
}

void findDialogLine(byte persona);

static void findDownwards(Tree paso, bool &desciende) {
	if (paso != NULL) {
		if (paso->element.dicho != '1') {
			desciende = true;
			return;
		} else {
			findDownwards(leftChild(paso), desciende);
			findDownwards(rightSibling(paso), desciende);
		}
	}
}

void findDialogLine(byte persona) {
	bool hecho, decir, subida, desciende, borde, adelanta;

	auxTree = ar;
	auxTree = auxTree->child;
	hecho = false;
	l = new lista;
	l->siguiente = NULL;
	l1 = l;
	borde = false;
	subida = false;
	do {
		switch (auxTree->element.dicho) {
		case '0':
		case '2':
		case 'H':
			decir = true;
			break;
		case '1': {
			decir = false;
			step = auxTree->child;
			desciende = false;
			findDownwards(step, desciende);
			if (!(desciende))
				subida = true;
			step = NULL;
		} break;
		case '3':
			if (libro[0] && (persona == 3)) {
				decir = false;
				subida = true;
			} else
				decir = true;
			break;
		case '4':
			if (primera[persona - 1]) {
				decir = false;
				subida = true;
			} else
				decir = true;
			break;
		case '5':
			if (libro[persona - 1] && lprimera[persona - 1])
				decir = true;
			else {
				decir = false;
				subida = true;
			}
			break;
		case '6':
			if (libro[persona - 1] && !lprimera[persona - 1])
				decir = true;
			else {
				decir = false;
				subida = true;
			}
			break;
		case '7':
			if (libro[persona - 1]) {
				decir = false;
				subida = true;
			} else if (!primera[persona - 1])
				decir = true;
			else {
				decir = false;
				subida = true;
			}
			break;
		case '8':
			if (caramelos[persona - 1] && cprimera[persona - 1])
				decir = true;
			else {
				decir = false;
				subida = true;
			}
			break;
		case '9':
			if (caramelos[persona - 1] && !cprimera[persona - 1])
				decir = true;
			else {
				decir = false;
				subida = true;
			}
			break;
		case 'A':
			if (!caramelos[persona - 1] && !primera[persona - 1])
				decir = true;
			else {
				decir = false;
				subida = true;
			}
			break;
		case 'B':
			if (cavernas[0] && !primera[8])
				decir = true;
			else
				decir = false;
			break;
		case 'C':
			if (cavernas[1] && !primera[8])
				decir = true;
			else
				decir = false;
			break;
		case 'D':
			if ((cavernas[0] && cavernas[1]) && !primera[8])
				decir = true;
			else
				decir = false;
			break;
		case 'E':
			if ((cavernas[0] && !cavernas[2]) && !primera[8])
				decir = true;
			else
				decir = false;
			break;
		case 'F':
			if (!cavernas[3])
				decir = true;
			else
				decir = false;
			break;
		case 'G':
			if (!cavernas[4])
				decir = true;
			else
				decir = false;
			break;
		case 'I':
			if (!sello_quitado)
				decir = true;
			else
				decir = false;
			break;
		case 'Z':
			decir = false;
			break;
		}
		if (decir) {
			if (auxTree->element.dicho == '2')
				if (!borde) {
					borde = true;
					switch (persona) {
					case 1:
						if (primera[persona - 1]) {
							l1->elemento = 12;
							adelanta = true;
						} else if (libro[persona - 1]) {
							adelanta = true;
							l1->elemento = 33;
						} else {
							l1->elemento = 21;
							adelanta = true;
						}
						break;
					case 3:
						if (primera[persona - 1]) {
							l1->elemento = 103;
							adelanta = true;
						} else {
							l1->elemento = 112;
							adelanta = true;
						}
						break;
					default: {
						l1->elemento = auxTree->element.index;
						adelanta = true;
					}
					}
				} else {
					;
				}
			else {
				l1->elemento = auxTree->element.index;
				adelanta = true;
			}
			if (adelanta) {
				adelanta = false;
				l1->siguiente = new lista;
				l1 = l1->siguiente;
				l1->siguiente = NULL;
			}
			if (rightSibling(auxTree) != NULL)
				auxTree = rightSibling(auxTree);
			else {
				do {
					if (!root(parent(auxTree)))
						auxTree = parent(auxTree);
					else
						break;
				} while (!(auxTree->element.dicho == '1' && rightSibling(auxTree) != NULL));
				if (rightSibling(auxTree) != NULL)
					auxTree = rightSibling(auxTree);
				else
					hecho = true;
			}
		} else if (subida) {
			subida = false;
			if (rightSibling(auxTree) != NULL)
				auxTree = rightSibling(auxTree);
			else {
				do {
					if (!root(parent(auxTree)))
						auxTree = parent(auxTree);
					else
						break;
				} while (!((auxTree->element.dicho == '1') &&
						   (rightSibling(auxTree) != NULL)));
				if (rightSibling(auxTree) != NULL)
					auxTree = rightSibling(auxTree);
				else
					hecho = true;
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
				} while (!(root(auxTree) || rightSibling(auxTree) != NULL));
				if (root(auxTree))
					hecho = true;
				else
					auxTree = rightSibling(auxTree);
			}
		}
	} while (!hecho);
	auxTree = NULL;
	step = NULL;
	l1 = NULL;
}

void modifyTree(uint nodonew) {
	bool encontrado;

	encontrado = false;
	auxTree = ar->child;
	do {
		if (auxTree->element.index == nodonew) {

			if ((auxTree->element.dicho != '2') && (auxTree->element.dicho != 'H'))
				auxTree->element.dicho = '1';
			else if (auxTree->element.dicho != 'H')
				auxTree->element.dicho = 'Z';
			encontrado = true;
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
	} while (!encontrado);
}

void drawTalkMenu() {
	byte ytextaux;
	g_engine->_mouseManager->hide();
	for (ytextaux = 25; ytextaux >= 1; ytextaux--)
		rectangle(0, 175 - ytextaux, 319, 174 + ytextaux, 0);
	for (ytextaux = 1; ytextaux <= 25; ytextaux++)
		buttonBorder(0, 175 - ytextaux, 319, 174 + ytextaux, 253, 253, 253, 253, 0, 0, "");
	drawMenu(5);
	g_engine->_mouseManager->show();
}

void fixTree(Tree paso) {
	if (paso != NULL) {
		if (paso->element.dicho == 'Z')
			paso->element.dicho = '2';
		else {
			fixTree(leftChild(paso));
			fixTree(rightSibling(paso));
		}
	}
}

void showDialogLine(Common::String matrizconversa[16], uint &charlaelegida) {
	byte primeraconv, buscanodo, convselec;

	primeraconv = 1;
	convselec = 0;
	g_engine->_mouseManager->hide();

	drawMenu(5);
	outtextxy(6, 151, matrizconversa[1], 255, true);
	outtextxy(6, 162, matrizconversa[2], 255, true);
	outtextxy(6, 173, matrizconversa[3], 255, true);
	outtextxy(6, 184, matrizconversa[4], 255, true);
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
					pulsax = e.mouse.x;
					pulsay = e.mouse.y;
				} else if (e.type == Common::EVENT_RBUTTONUP) {
					rMouseClicked = true;
					pulsax = e.mouse.x;
					pulsay = e.mouse.y;
				}
			}

			if (tocapintar) {
				if (currentRoomData->paletteAnimationFlag && saltospal >= 4) {
					saltospal = 0;
					if (movidapaleta > 6)
						movidapaleta = 0;
					else
						movidapaleta += 1;
					updatePalette(movidapaleta);
				} else
					saltospal += 1;
			}
			g_system->delayMillis(10);
			g_engine->_screen->update();
		} while (!lMouseClicked && !rMouseClicked && !g_engine->shouldQuit());

		if (lMouseClicked) {
			if (pulsay < 143)
				convselec = 0;
			else {
				if (pulsax >= 0 && pulsax <= 280) {
					if (pulsay >= 143 && pulsay <= 155) {
						convselec = primeraconv;
					} else if (pulsay >= 156 && pulsay <= 166) {
						convselec = primeraconv + 1;
					} else if (pulsay >= 167 && pulsay <= 177) {
						convselec = primeraconv + 2;
					} else if (pulsay >= 178 && pulsay <= 186) {
						convselec = primeraconv + 3;
					}
				} else if (pulsax >= 281 && pulsax <= 319) {
					if (pulsay >= 143 && pulsay <= 165) {
						if (primeraconv > 1) {
							convselec = 0;
							primeraconv -= 1;
							g_engine->_mouseManager->hide();
							drawMenu(5);
							outtextxy(6, 151, matrizconversa[primeraconv], 255, true);
							outtextxy(6, 162, matrizconversa[primeraconv + 1], 255, true);
							outtextxy(6, 173, matrizconversa[primeraconv + 2], 255, true);
							outtextxy(6, 184, matrizconversa[primeraconv + 3], 255, true);
							g_engine->_mouseManager->show();
						}
					} else if (pulsay >= 167 && pulsay <= 186) {
						if (primeraconv < 12) {
							convselec = 0;
							primeraconv += 1;
							g_engine->_mouseManager->hide();
							drawMenu(5);
							outtextxy(6, 151, matrizconversa[primeraconv], 255, true);
							outtextxy(6, 162, matrizconversa[primeraconv + 1], 255, true);
							outtextxy(6, 173, matrizconversa[primeraconv + 2], 255, true);
							outtextxy(6, 184, matrizconversa[primeraconv + 3], 255, true);
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
	for (buscanodo = 1; buscanodo <= (convselec - 1); buscanodo++)
		l1 = l1->siguiente;
	charlaelegida = l1->elemento;
}

void talk(byte person) {
	debug("Talking to persona: %d", person);
	uint pasorespuesta, nuevonodo;
	regismht pasoh;
	byte auxilcadena, insertarnombre, indicemochila;

	Common::String matrizconversa[16];
	drawTalkMenu();
	endOfConversation = false;
	assignText();
	// The original game makes a copy of the file upon starting a new game. .007 is the current game (the game
	// that resumes when clicking "continue game" in the main menu. Part of the savegame data is this 007
	// conversation file which marks conversatino topics as already gone through or not.
	readTree(*conversationData, ar, person - 1);
	loadTalkAnimations();
	do {

		for (int i = 0; i < 16; i++) {
			matrizconversa[i] = "";
		}

		findDialogLine(person);
		conversationIndex = 0;
		l1 = l;
		do {
			verb.seek(verbRegSize * l1->elemento);
			conversationIndex += 1;
			pasoh = readVerbRegister();
			insertarnombre = 0;
			matrizconversa[conversationIndex] = decrypt(pasoh.cadenatext);

			for (int i = 0; i < matrizconversa[conversationIndex].size(); i++) {
				if (matrizconversa[conversationIndex][i] == '@')
					insertarnombre = i;
			}
			if (insertarnombre > 0) {
				matrizconversa[conversationIndex].deleteChar(insertarnombre);
				matrizconversa[conversationIndex].insertString(nombrepersonaje, insertarnombre);
			}
			if (matrizconversa[conversationIndex].size() > 45) {
				auxilcadena = 45;
				do {
					auxilcadena -= 1;
				} while (matrizconversa[conversationIndex][auxilcadena] != ' ');
				matrizconversa[conversationIndex] = matrizconversa[conversationIndex].substr(0, auxilcadena);
				matrizconversa[conversationIndex].insertString(" ...", auxilcadena);
			}
			l1 = l1->siguiente;
		} while ((l1->siguiente != NULL) && (l1 != NULL) && !g_engine->shouldQuit());
		l1 = l;
		showDialogLine(matrizconversa, nuevonodo);
		delete l;
		hypertext(nuevonodo, 255, 0, pasorespuesta, true);
		auxilcadena = 0;
		modifyTree(nuevonodo);
		// 	verifyCopyProtection();
		while (pasorespuesta > 0 && !g_engine->shouldQuit()) {
			nuevonodo = pasorespuesta;
			auxilcadena += 1;
			if (odd(auxilcadena))
				hypertext(nuevonodo, 253, 249, pasorespuesta, true);
			else
				hypertext(nuevonodo, 255, 0, pasorespuesta, true);
			switch (nuevonodo) {
			case 9: {
				lista1 = true;
				indicemochila = 0;
				while (mobj[indicemochila].code != 0) {
					indicemochila += 1;
				}
				mobj[indicemochila].bitmapIndex = indicelista1;
				mobj[indicemochila].code = codigolista1;
				mobj[indicemochila].objectName = getObjectName(0);
			} break;
			case 25: {
				lista2 = true;
				indicemochila = 0;
				while (mobj[indicemochila].code != 0) {
					indicemochila += 1;
				}
				mobj[indicemochila].bitmapIndex = indicelista2;
				mobj[indicemochila].code = codigolista2;
				mobj[indicemochila].objectName = getObjectName(1);
			} break;
			}
		}
		g_system->delayMillis(10);
	} while (!endOfConversation && !g_engine->shouldQuit());

	unloadTalkAnimations();
	step = ar;
	fixTree(step);
	saveConversations(conversationData, ar, person - 1);

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

	// release(marca);
	if (person < 5) {
		if (primera[person - 1])
			primera[person - 1] = false;
		if (lprimera[person - 1])
			lprimera[person - 1] = false;
		if (cprimera[person - 1])
			cprimera[person - 1] = false;
	} else if (person == 8)
		primera[8] = false;
}

void talkScreenObject() {
	int x_del_raton = (pulsax + 7) / factorx;
	int y_del_raton = (pulsay + 7) / factory;
	uint objeto_de_la_pantalla = currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[x_del_raton][y_del_raton]]->indicefichero;
	if (objeto_de_la_pantalla == 0)
		return;

	// verifyCopyProtection2();
	readItemRegister(objeto_de_la_pantalla);
	goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory],
			   currentRoomData->rejapantalla[x_del_raton][y_del_raton]);

	if (regobj.habla > 0) {
		talk(regobj.habla);
	} else {
		assignText();
		hypertext((Random(10) + 1039), 255, 0, kaka, false);
		verb.close();
		if (contadorpc > 198)
			showError(274);
	}
}

} // End of namespace Tot
