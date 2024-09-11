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

#ifndef QDENGINE_DT_INTERNAL_H
#define QDENGINE_DT_INTERNAL_H

namespace QDEngine {

typedef struct ImGuiImage {
	ImTextureID id;
	int width;
	int height;
} ImGuiImage;

enum {
	kDisplayQDA,
	kDisplayTGA,
};

struct FileTree {
	Common::Path path;
	Common::String name;
	Common::Array<FileTree *> children;
	int id;

	FileTree(Common::Path *p, Common::String n, bool node, int i);
	FileTree() { id = 0; }
};

typedef struct ImGuiState {
	bool _showArchives = false;

	Common::HashMap<Common::String, ImGuiImage> _frames;

	Common::Path _fileToDisplay;

	int _qdaToDisplayFrame = -1;
	int _qdaToDisplayFrameCount = 0;
	bool _qdaIsPlaying = false;
	int _qdaNextFrameTimestamp = 0;

	ImGuiTextFilter _nameFilter;

	FileTree _files;

	int _displayMode = -1;
} ImGuiState;

extern ImGuiState *_state;

}

#endif // QDENGINE_DT_INTERNAL_H
