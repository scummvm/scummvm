/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_RESOURCE_H
#define GRIM_RESOURCE_H

#include "common/archive.h"

#include "engines/grim/lab.h"

namespace Grim {

class Bitmap;
class CMap;
class Costume;
class Font;
class KeyframeAnim;
class Material;
class Model;
class LipSync;

class ResourceLoader {
public:
	bool fileExists(const char *filename) const;
	Block *getFileBlock(const char *filename) const;
	Common::File *openNewStreamFile(const char *filename) const;
	LuaFile *openNewStreamLuaFile(const char *filename) const;
	int fileLength(const char *filename) const;

	Bitmap *loadBitmap(const char *fname);
	CMap *loadColormap(const char *fname);
	Costume *loadCostume(const char *fname, Costume *prevCost);
	Font *loadFont(const char *fname);
	KeyframeAnim *loadKeyframe(const char *fname);
	Material *loadMaterial(const char *fname, const CMap *c);
	Model *loadModel(const char *fname, const CMap *c);
	LipSync *loadLipSync(const char *fname);
	void uncache(const char *fname);

	struct ResourceCache {
		char *fname;
		void *resPtr;
	};

	ResourceLoader();
	ResourceLoader(const ResourceLoader &);
	~ResourceLoader();
	const Lab *getLab(const char *filename) const;
	void *getFileFromCache(const char *filename);
	ResourceLoader::ResourceCache *getEntryFromCache(const char *filename);
	void putIntoCache(Common::String fname, void *res);

private:

	typedef Common::List<Lab *> LabList;
	LabList _labs;
	Common::SearchSet _files;

	Common::Array<ResourceCache> _cache;
	bool _cacheDirty;

	// Shut up pointless g++ warning
	friend class dummy;
};

extern ResourceLoader *g_resourceloader;

} // end of namespace Grim

#endif
