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

#include "tot/forest.h"

namespace Tot {

void initTree(Tree &a, nodeElement data) {
	a = new TreeDef;
	a->element = data;
	a->parent = nullptr;
	a->sibling = nullptr;
	a->child = nullptr;
}

bool isRoot(Tree node) {
	bool root;
	if (node->parent == nullptr)
		root = true;
	else
		root = false;
	return root;
}

Tree rightSibling(Tree node) {
	Tree rightSibling;
	rightSibling = node->sibling;
	return rightSibling;
}

Tree parent(Tree node) {
	Tree parent;
	parent = node->parent;
	return parent;
}

Tree leftChild(Tree node) {
	Tree leftChild;
	leftChild = node->child;
	return leftChild;
}

int depth(Tree node) {
	Tree aux;
	int depthCount = 0;
	aux = node;
	while (aux->parent != nullptr) {
		depthCount += 1;
		aux = parent(aux);
	}
	return depthCount;
}

void expandNode(Tree &node, nodeElement data) {
	Tree aux = node;
	if (aux->child != nullptr) {

		aux = leftChild(aux);
		while (aux->sibling != nullptr)
			aux = rightSibling(aux);
		;
		aux->sibling = new TreeDef;
		aux = aux->sibling;
		aux->element = data;
		aux->sibling = nullptr;
		aux->child = nullptr;
		aux->parent = node;
	} else {

		aux->child = new TreeDef;
		aux = aux->child;
		aux->element = data;
		aux->sibling = nullptr;
		aux->child = nullptr;
		aux->parent = node;
	}
}

void preOrder(Tree a, Common::String &encodedString) {
	if (a != nullptr) {
		encodedString = Common::String::format("%s%d%cN%d@", encodedString.c_str(), a->element.index, a->element.spoken, depth(a));
		preOrder(leftChild(a), encodedString);
		preOrder(rightSibling(a), encodedString);
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

void saveConversations(Common::SeekableWriteStream *s, Tree a, uint offset) {
	Common::String expression = "";
	preOrder(a, expression);
	s->seek(offset * chatRegSize, SEEK_SET);
	saveExpression(s, expression);
}

void readTree(Common::SeekableReadStream &stream, Tree &a, uint position) {

	const nodeElement empty = {'0', 0};
	nodeElement data;

	Common::String strInd, tmpExpression;
	byte level;
	Common::String levelAsString;

	stream.seek(chatRegSize * position);

	Common::String expresion = stream.readPascalString();
	initTree(a, empty);
	Tree aux = a;
	byte pos = 0;
	byte currentLevel = 0;
	do {

		tmpExpression = "";
		do {
			tmpExpression = tmpExpression + expresion[pos];
		} while (expresion[pos++] != '@');
		data.spoken = '0';
		data.index = 0;

		int nIndex = tmpExpression.find('N');
		strInd = tmpExpression.substr(0, nIndex - 1);

		data.spoken = tmpExpression[nIndex - 1];
		data.index = atoi(strInd.c_str());

		levelAsString = "";
		levelAsString = tmpExpression.substr(nIndex + 1, tmpExpression.size() - nIndex - 2);
		level = atoi(levelAsString.c_str());

		if (level == 0)
			aux->element = data;
		else if (level == (currentLevel + 1))
			expandNode(aux, data);
		else if (level > (currentLevel + 1)) {
			aux = leftChild(aux);
			currentLevel += 1;
			while (rightSibling(aux) != nullptr)
				aux = rightSibling(aux);
			expandNode(aux, data);
		} else {
			do {
				currentLevel -= 1;
				aux = parent(aux);
			} while (!(currentLevel < level));
			expandNode(aux, data);
		}

	} while (pos != expresion.size());
}

void readTree(Common::String f, Tree &a, uint offset) {

	Common::File treeFile;
	if (!treeFile.open(Common::Path(f))) {
		showError(314);
	}
	readTree(treeFile, a, offset);
	treeFile.close();
}

} // End of namespace Tot
