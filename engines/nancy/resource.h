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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_RESOURCE_H
#define NANCY_RESOURCE_H

#include "engines/nancy/cif.h"

namespace Graphics {
struct Surface;
class ManagedSurface;
}

namespace Nancy {

class IFF;

class ResourceManager {
	friend class NancyConsole;
	friend class NancyEngine;
public:
	ResourceManager() = default;
	~ResourceManager() = default;

	// Load an image resource. Can be either external .bmp file, or raw image data embedded inside a ciftree
	// Ciftree images may have additional data dictating how they need to be blitted on screen (see ConversationCel).
	// This is accessed via the outSrc/outDest parameters.
	bool loadImage(const Common::Path &name, Graphics::ManagedSurface &surf, const Common::String &treeName = Common::String(), Common::Rect *outSrc = nullptr, Common::Rect *outDest = nullptr);

	// Loads a single IFF file. These can either be inside standalone .cif files, or embedded inside a ciftree
	IFF *loadIFF(const Common::Path &name);

	// Load a new ciftree
	bool readCifTree(const Common::String &name, const Common::String &ext, int priority);
	PatchTree *readPatchTree(Common::SeekableReadStream *stream, const Common::String &name, int priority);

private:
	// Debug functions

	// Return a human-readable description of a single CIF file.
	Common::String getCifDescription(const Common::String &treeName, const Common::Path &name) const;

	// Return a list of all resources of a certain type (does not list external files)
	void list(const Common::String &treeName, Common::Array<Common::Path> &outList, CifInfo::ResType type) const;

	// Exports a single resource as a standalone .cif file
	bool exportCif(const Common::String &treeName, const Common::Path &name);

	// Exports a collection of resources as a ciftree
	bool exportCifTree(const Common::String &treeName, const Common::Array<Common::Path> &names);

private:
	Common::Array<Common::String> _cifTreeNames;
};

} // End of namespace Nancy

#endif // NANCY_RESOURCE_H
