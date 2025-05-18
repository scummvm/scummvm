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

#include "common/debug.h"
#include "common/file.h"

#include "tot/forest.h"

namespace Tot {

void initTree(Tree &a, nodeElement dato) {
	a = new treeDef;
	a->element = dato;
	a->parent = NULL;
	a->sibling = NULL;
	a->child = NULL;
}

bool root(Tree nodo) {
	bool raiz_result;
	if (nodo->parent == NULL)
		raiz_result = true;
	else
		raiz_result = false;
	return raiz_result;
}

Tree rightSibling(Tree nodo) {
	Tree hermanoder_result;
	hermanoder_result = nodo->sibling;
	return hermanoder_result;
}

Tree parent(Tree nodo) {
	Tree padre_result;
	padre_result = nodo->parent;
	return padre_result;
}

Tree leftChild(Tree nodo) {
	Tree hijoizq_result;
	hijoizq_result = nodo->child;
	return hijoizq_result;
}

int depth(Tree nodo) {
	Tree auxiliar;
	int contador;

	int profundidad_result;
	contador = 0;
	auxiliar = nodo;
	while (auxiliar->parent != NULL) {
		contador += 1;
		auxiliar = parent(auxiliar);
	}
	profundidad_result = contador;
	return profundidad_result;
}

void expandNode(Tree &nodo, nodeElement dato) {
	Tree auxiliar;

	auxiliar = nodo;
	if (auxiliar->child != NULL) {

		auxiliar = leftChild(auxiliar);
		while (auxiliar->sibling != NULL)
			auxiliar = rightSibling(auxiliar);
		;
		auxiliar->sibling = new treeDef;
		auxiliar = auxiliar->sibling;
		auxiliar->element = dato;
		auxiliar->sibling = NULL;
		auxiliar->child = NULL;
		auxiliar->parent = nodo;
	} else {

		auxiliar->child = new treeDef;
		auxiliar = auxiliar->child;
		auxiliar->element = dato;
		auxiliar->sibling = NULL;
		auxiliar->child = NULL;
		auxiliar->parent = nodo;
	}
}

void preOrder(Tree a, Common::String &cadena) {
	if (a != NULL) {
		cadena = Common::String::format("%s%d%cN%d@", cadena.c_str(), a->element.index, a->element.dicho, depth(a));
		preOrder(leftChild(a), cadena);
		preOrder(rightSibling(a), cadena);
	}
}

void saveExpression(Common::SeekableWriteStream *s, Common::String expression) {
	s->writeByte(expression.size());
	s->writeString(expression);
	int paddingSize = 255 - expression.size();
	if (paddingSize > 0) {
		debug("Writing padding of %d", paddingSize);
		char *padding = (char *)malloc(paddingSize);
		for (int i = 0; i < paddingSize; i++) {
			padding[i] = '\0';
		}
		// 8 max char name
		s->write(padding, paddingSize);
		free(padding);
	}
}

const int chatRegSize = 256;

void saveConversations(Common::SeekableWriteStream *s, Tree a, uint sitio) {

	Common::String expression = "";
	preOrder(a, expression);
	debug("Pos-Expression=%s", expression.c_str());
	debug("saving in position = %d", sitio);
	s->seek(sitio * chatRegSize, SEEK_SET);
	saveExpression(s, expression);
}

void readTree(Common::SeekableReadStream &stream, Tree &a, uint lugar) {

	const nodeElement vacio = {'0', 0};
	nodeElement dato;

	Common::String strInd, exppaso;
	Tree arbolaux;
	byte posicion,

		nivel,
		nivelact;

	Common::String nivelstr;

	stream.seek(chatRegSize * lugar);

	Common::String expresion;
	expresion = stream.readPascalString();
	debug("Pre-Expression=%s", expresion.c_str());

	initTree(a, vacio);
	arbolaux = a;
	posicion = 0;
	nivelact = 0;
	do {

		exppaso = "";
		do {
			exppaso = exppaso + expresion[posicion];
		} while (expresion[posicion++] != '@');
		dato.dicho = '0';
		dato.index = 0;

		int nIndex = exppaso.find('N');
		strInd = exppaso.substr(0, nIndex - 1);

		dato.dicho = exppaso[nIndex - 1];
		dato.index = atoi(strInd.c_str());

		nivelstr = "";
		nivelstr = exppaso.substr(nIndex + 1, exppaso.size() - nIndex - 2);
		nivel = atoi(nivelstr.c_str());

		if (nivel == 0)
			arbolaux->element = dato;
		else if (nivel == (nivelact + 1))
			expandNode(arbolaux, dato);
		else if (nivel > (nivelact + 1)) {
			arbolaux = leftChild(arbolaux);
			nivelact += 1;
			while (rightSibling(arbolaux) != NULL)
				arbolaux = rightSibling(arbolaux);
			expandNode(arbolaux, dato);
		} else {
			do {
				nivelact -= 1;
				arbolaux = parent(arbolaux);
			} while (!(nivelact < nivel));
			expandNode(arbolaux, dato);
		}

	} while (posicion != expresion.size());
}

void readTree(Common::String f, Tree &a, uint lugar) {

	Common::File fichero;
	debug("Filename = %s", f.c_str());
	if (!fichero.open(Common::Path(f))) {
		showError(314);
	}
	readTree(fichero, a, lugar);
	fichero.close();
}

} // End of namespace Tot
