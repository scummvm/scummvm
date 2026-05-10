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

#include "common/formats/json.h"
#include "common/str.h"

#include "backends/imgui/imgui.h"
#include "backends/imgui/components/imgui_memory_editor.h"

#include "scumm/debugger/editor.h"
#include "scumm/debugger/explorer.h"
#include "scumm/debugger/file.h"

namespace Scumm {

namespace Editor {

static const char *explorerColorNames[] = {"Data", "Zero"};

Explorer::Explorer(Resource *resource, Common::Array<ImVec4> &colors)
	: _resource(resource),
	  _colors(colors) {
	// Specify default colors
	_explorerColors.resize(kColorCount);
	_explorerColors[kColorData] = ImVec4(0.165f, 0.631f, 0.596f, 1.0f); // SOL_CYAN
	_explorerColors[kColorZero] = ImVec4(0.345f, 0.431f, 0.459f, 1.0f); // SOL_BASE01
}

void Explorer::renderTab(int fileIndex) {
	// Ensure resource contains valid blocks
	const Common::Array<Block> &blocks = _resource->getBlocks(fileIndex);
	if (blocks.empty()) {
		ImGui::TextColored(_colors[kColorError], "No blocks!");
		return;
	}

	// Initialize block selection
	_selectedBlocks.resize(fileIndex + 1, -1);
	if (_selectedBlocks[fileIndex] >= (int)blocks.size())
		_selectedBlocks[fileIndex] = -1;

	// Render block tree and data
	renderTree(blocks, _selectedBlocks[fileIndex]);
	ImGui::SameLine();
	renderBlock(fileIndex);
}

void Explorer::renderTree(const Common::Array<Block> &blocks, int &selectedBlock) {
	ImGui::BeginChild("BlockTree", ImVec2(350, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);

	// Find and render root blocks
	for (uint i = 0; i < blocks.size(); ++i)
		if (blocks[i].parent == -1)
			renderTreeNode(blocks, i, selectedBlock);

	ImGui::EndChild();
}

void Explorer::renderTreeNode(const Common::Array<Block> &blocks, int blockIndex, int &selectedBlock) {
	const Block &block = blocks[blockIndex];

	// Setup flags
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (block.children.empty())
		flags |= ImGuiTreeNodeFlags_Leaf;
	if (selectedBlock == blockIndex)
		flags |= ImGuiTreeNodeFlags_Selected;

	// Render node
	Common::String label = Common::String::format("%s [0x%08X]", tag2str(block.tag), block.offset);
	bool nodeOpen = ImGui::TreeNodeEx((void *)(intptr_t)blockIndex, flags, label.c_str());
	if (ImGui::IsItemClicked())
		selectedBlock = blockIndex;

	// Render children
	if (nodeOpen) {
		for (uint i = 0; i < block.children.size(); ++i)
			renderTreeNode(blocks, block.children[i], selectedBlock);
		ImGui::TreePop();
	}
}

void Explorer::renderBlock(int fileIndex) {
	const Common::Array<Block> &blocks = _resource->getBlocks(fileIndex);

	ImGui::BeginChild("BlockContents", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);

	// Render selected block
	if (_selectedBlocks[fileIndex] != -1) {
		const Block &block = blocks[_selectedBlocks[fileIndex]];

		// Info section
		ImGui::SeparatorText("Block Info");
		if (ImGui::BeginTable("BlockInfo", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
			ImGui::TableSetupColumn("Property");
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 0.0f);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextColored(_colors[kColorLabel], "Tag");
			ImGui::TableNextColumn();
			ImGui::TextColored(_colors[kColorProperty], "%s", tag2str(block.tag));
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextColored(_colors[kColorLabel], "Size");
			ImGui::TableNextColumn();
			ImGui::TextColored(_colors[kColorProperty], "%u", block.size);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextColored(_colors[kColorLabel], "Offset");
			ImGui::TableNextColumn();
			ImGui::TextColored(_colors[kColorProperty], "0x%08X", block.offset);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextColored(_colors[kColorLabel], "Children");
			ImGui::TableNextColumn();
			ImGui::TextColored(_colors[kColorProperty], "%u", block.children.size());
			ImGui::EndTable();
		}

		// Prepare memory editor
		struct ReadFnData { File *file; uint32 baseOffset; };
		ReadFnData readFnData = {_resource->getFile(fileIndex), block.offset};
		MemoryEditor memEdit;
		memEdit.ReadOnly = true;
		memEdit.OptShowOptions = false;
		memEdit.UserData = &readFnData;
		memEdit.ReadFn = [](const ImU8 *, size_t off, void *userData) -> ImU8 {
			ReadFnData *data = static_cast<ReadFnData *>(userData);
			data->file->seek(data->baseOffset + off);
			return data->file->readByte();
		};

		// Data section
		ImGui::SeparatorText("Block Data");
		ImGui::PushStyleColor(ImGuiCol_Text, _explorerColors[kColorData]);
		ImGui::PushStyleColor(ImGuiCol_TextDisabled, _explorerColors[kColorZero]);
		memEdit.DrawContents(nullptr, block.size, block.offset);
		ImGui::PopStyleColor(2);
	} else {
		// No block selected
		ImGui::TextDisabled("Select a block.");
	}

	ImGui::EndChild();
}

void Explorer::loadState(const Common::JSONObject &json) {
	// Load colors
	if (json.contains("Colors")) {
		const Common::JSONObject &colors = json["Colors"]->asObject();
		for (uint i = 0; i < _explorerColors.size(); ++i)
			if (colors.contains(explorerColorNames[i])) {
				const Common::JSONArray &arr = colors[explorerColorNames[i]]->asArray();
				_explorerColors[i].x = (float)arr[0]->asNumber();
				_explorerColors[i].y = (float)arr[1]->asNumber();
				_explorerColors[i].z = (float)arr[2]->asNumber();
				_explorerColors[i].w = (float)arr[3]->asNumber();
			}
	}
}

Common::JSONObject Explorer::saveState() const {
	Common::JSONObject obj;

	// Save colors
	Common::JSONObject colors;
	for (uint i = 0; i < _explorerColors.size(); ++i) {
		Common::JSONArray arr;
		arr.push_back(new Common::JSONValue((float)_explorerColors[i].x));
		arr.push_back(new Common::JSONValue((float)_explorerColors[i].y));
		arr.push_back(new Common::JSONValue((float)_explorerColors[i].z));
		arr.push_back(new Common::JSONValue((float)_explorerColors[i].w));
		colors[explorerColorNames[i]] = new Common::JSONValue(arr);
	}
	obj["Colors"] = new Common::JSONValue(colors);

	return obj;
}

void Explorer::showSettings() {
	if (ImGui::CollapsingHeader("Explorer", ImGuiTreeNodeFlags_DefaultOpen))
		for (uint i = 0; i < _explorerColors.size(); ++i)
			ImGui::ColorEdit4(explorerColorNames[i], &_explorerColors[i].x);
}

void Explorer::render(const char *icon, ImGuiID dockSpaceId, bool *open) {
	ImGui::SetNextWindowDockID(dockSpaceId, ImGuiCond_FirstUseEver);
	Common::String title = Common::String::format("%s Explorer", icon);
	if (ImGui::Begin(title.c_str(), open))
		if (ImGui::BeginTabBar("ExplorerTabs")) {
			for (int i = 0; i < _resource->getFileCount(); ++i)
				if (ImGui::BeginTabItem(_resource->getFile(i)->getPath().baseName().c_str())) {
					ImGui::PushID(i);
					renderTab(i);
					ImGui::PopID();
					ImGui::EndTabItem();
				}
			ImGui::EndTabBar();
		}
	ImGui::End();
}

} // End of namespace Editor

} // End of namespace Scumm
