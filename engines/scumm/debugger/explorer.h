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

#ifndef SCUMM_EDITOR_EXPLORER_H
#define SCUMM_EDITOR_EXPLORER_H

#include "common/array.h"
#include "common/formats/json.h"

#include "scumm/debugger/resource.h"

namespace Scumm {

namespace Editor {

class Explorer {
private:
	enum {
		kColorData,
		kColorZero,
		kColorCount
	};

	Resource *_resource;

	Common::Array<ImVec4> &_colors;
	Common::Array<ImVec4> _explorerColors;

	Common::Array<int> _selectedBlocks;

	void renderTab(int fileIndex);
	void renderTree(const Common::Array<Block> &blocks, int &selectedBlock);
	void renderTreeNode(const Common::Array<Block> &blocks, int blockIndex, int &selectedBlock);
	void renderBlock(int fileIndex);

public:
	Explorer(Resource *resource, Common::Array<ImVec4> &colors);

	void loadState(const Common::JSONObject &json);
	Common::JSONObject saveState() const;

	void showSettings();

	void render(const char *icon, ImGuiID dockSpaceId, bool *open);
};

} // End of namespace Editor

} // End of namespace Scumm

#endif
