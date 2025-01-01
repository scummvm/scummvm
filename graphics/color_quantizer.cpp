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

#include "common/stack.h"

#include "graphics/color_quantizer.h"
#include "graphics/palette.h"

namespace Graphics {

// This code is heavily based on "Color Quantization using Octrees" by Dean
// Clark, published in - I think - the January 1996 issue of Dr. Dobb's Journal.

#define kOctreeDepth 6

struct OctreeNode {
	byte level;
	bool isLeaf;
	uint32 numPixels;
	uint32 sumRed;
	uint32 sumGreen;
	uint32 sumBlue;
	OctreeNode *child[8];
	OctreeNode *nextNode;
};

// An octree is a tree where each node has up to eight children. Colors are
// inserted into it by looking at the bits of the R, G, and B components one
// bit at a time, starting at the most significant bit. These three bits form
// a value from 0 to 7, indicating which child node to enter.
//
// This means that adjacent leaves in the tree will represent colors that are
// close together. Once the tree has more leaves than we want, we take all
// leaves under one node, combine them, and make their parent a new leaf with
// their average color. The old leaves are then discarded.
//
// The depth of the tree is the number of bits we look at. Technically this
// would be eight, but six should be enough.

class Octree {
private:
	uint _leafLevel = kOctreeDepth - 1;

	OctreeNode *_root = nullptr;
	uint _numLeaves = 0;
	uint _maxLeaves = 0;

	OctreeNode *_reduceList[kOctreeDepth - 1];
	Common::Stack<OctreeNode *> _nodePool;

	OctreeNode *allocateNode(byte level) {
		OctreeNode *node;

		if (!_nodePool.empty())
			node = _nodePool.pop();
		else
			node = new OctreeNode();

		if (level == _leafLevel) {
			node->isLeaf = true;
			_numLeaves++;
		} else
			node->isLeaf = false;

		node->level = level;
		node->numPixels = 0;
		node->sumRed = 0;
		node->sumGreen = 0;
		node->sumBlue = 0;
		node->nextNode = nullptr;

		for (int i = 0; i < 8; i++)
			node->child[i] = nullptr;

		return node;
	}

	void releaseNode(OctreeNode *node) {
		_nodePool.push(node);
	}

	void deleteNodeRecursively(OctreeNode *node) {
		if (!node)
			return;

		for (int i = 0; i < 8; i++)
			deleteNodeRecursively(node->child[i]);

		delete node;
	}

	void insert(OctreeNode **node, byte r, byte g, byte b, uint level) {
		if (*node == nullptr) {
			*node = allocateNode(level);
			if (level != _leafLevel) {
				(*node)->nextNode = _reduceList[level];
				_reduceList[level] = *node;
			}
		}

		// Once we encounter a leaf, add the color there. This is not
		// necessarily at the bottom of the tree, so I guess it would
		// not be out of the question to transform the leaf into a
		// regular node. But I saw no mention of this in the article.

		if ((*node)->isLeaf) {
			(*node)->numPixels++;
			(*node)->sumRed += r;
			(*node)->sumGreen += g;
			(*node)->sumBlue += b;
		} else {
			byte bit = (0x80 >> level);
			byte rbit = (r & bit) >> (5 - level);
			byte gbit = (g & bit) >> (6 - level);
			byte bbit = (b & bit) >> (7 - level);
			int idx = rbit | gbit | bbit;

			insert(&((*node)->child[idx]), r, g, b, level + 1);
		}

		// Usually one reduction would be enough, but it's possible
		// that the reduction will not actually remove any leaves.

		while (_numLeaves > _maxLeaves)
			reduceTree();
	}

	void reduceTree() {
		// In the original article, once a reduce list has been emptied
		// the leaf level was decreased, meaning that the tree could
		// never again grow beyond that height. I don't understand why,
		// so I have made this a local variable instead.

		int level = _leafLevel - 1;

		while (!_reduceList[level])
			level--;

		// There are several possible approaches to picking the node to
		// reduce. Picking the one with the largest number of pixels
		// may leave more color for fine details. Picking the one with
		// the smallest number may sacrifice detail, but preserve subtle
		// gradations in large areas. This just picks the first one,
		// i.e. the most recently inserted one, so it's pretty random
		// which may be good on average.
		//
		// Once a subtree has been pruned, it will no longer grow back.
		// It seems to me it should be possible to allow it to, but the
		// article doesn't mention it. On the contrary, it states that
		// "any new colors whose path through the tree take them
		// through [a node that was turned into a leaf] now stop here".

		OctreeNode *node = _reduceList[level];
		_reduceList[level] = _reduceList[level]->nextNode;

		// Combine all the leaves into their parent, and make the
		// parent a leaf.

		uint32 sumRed = 0;
		uint32 sumGreen = 0;
		uint32 sumBlue = 0;

		byte numChildren = 0;

		for (int i = 0; i < 8; i++) {
			OctreeNode *child = node->child[i];

			if (child) {
				numChildren++;
				sumRed += child->sumRed;
				sumGreen += child->sumGreen;
				sumBlue += child->sumBlue;
				node->numPixels += child->numPixels;
				releaseNode(child);
				node->child[i] = nullptr;
			}
		}

		node->isLeaf = true;
		node->sumRed = sumRed;
		node->sumGreen = sumGreen;
		node->sumBlue = sumBlue;

		_numLeaves -= (numChildren - 1);
	}

	Palette *_palette;
	uint _colorIndex;

	void getPalette(OctreeNode *node) {
		if (node->isLeaf) {
			byte r = node->sumRed / node->numPixels;
			byte g = node->sumGreen / node->numPixels;
			byte b = node->sumBlue / node->numPixels;
			_palette->set(_colorIndex, r, g, b);
			_colorIndex++;
		} else {
			for (uint i = 0; i < 8; i++) {
				if (node->child[i])
					getPalette(node->child[i]);
			}
		}
	}

public:
	Octree(int maxLeaves) : _maxLeaves(maxLeaves) {
		for (uint i = 0; i < kOctreeDepth; i++)
			_reduceList[i] = nullptr;
	}

	~Octree() {
		while (!_nodePool.empty())
			delete _nodePool.pop();
		deleteNodeRecursively(_root);
	}

	void insert(byte r, byte g, byte b) {
		insert(&_root, r, g, b, 0);
	}

	Palette *getPalette() {
		_palette = new Graphics::Palette(_maxLeaves);
		_colorIndex = 0;

		getPalette(_root);

		return _palette;
	}
};

ColorQuantizer::ColorQuantizer(int maxColors) {
	_octree = new Octree(maxColors);
}

ColorQuantizer::~ColorQuantizer() {
	delete _octree;
}

void ColorQuantizer::addColor(byte r, byte g, byte b) {
	_octree->insert(r, g, b);
}

Graphics::Palette *ColorQuantizer::getPalette() {
	return _octree->getPalette();
}

} // End of namespace Graphics
