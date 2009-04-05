/* Residual - Virtual machine to run LucasArts' 3D adventure games
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

#include "common/sys.h"
#include "common/debug.h"
#include "common/fs.h"

#include "engine/resource.h"
#include "engine/registry.h"
#include "engine/bitmap.h"
#include "engine/colormap.h"
#include "engine/costume.h"
#include "engine/font.h"
#include "engine/keyframe.h"
#include "engine/material.h"
#include "engine/model.h"
#include "engine/engine.h"
#include "engine/lipsynch.h"

#include <algorithm>

static void makeLower(std::string& s) {
	std::transform(s.begin(), s.end(), s.begin(), tolower);
}

ResourceLoader *g_resourceloader = NULL;

ResourceLoader::ResourceLoader() {
	int lab_counter = 0;
	Lab *l;
	Common::ArchiveMemberList files;

	SearchMan.listMatchingMembers(files, "*.lab");

	if (files.empty())
		error("Cannot find game data - check configuration file");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String filename = (*x)->getName();
		l = new Lab(filename.c_str());
		if (l->isOpen()) {
			if (filename == "005.lab")
				_labs.push_front(l);
			else {
				if (filename == "gfdemo01.lab")
					g_flags |= GF_DEMO;
				_labs.push_back(l);
			}
			lab_counter++;
		} else {
			delete l;
		}
	}

	files.clear();

	SearchMan.listMatchingMembers(files, "*.mus");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String filename = (*x)->getName();
		l = new Lab(filename.c_str());
		if (l->isOpen()) {
			_labs.push_back(l);
			lab_counter++;
		} else {
			delete l;
		}
	}
}

ResourceLoader::~ResourceLoader() {
	for (LabList::const_iterator i = _labs.begin(); i != _labs.end(); i++)
		delete (*i);
}

const Lab *ResourceLoader::findFile(const char *filename) const {
	for (LabList::const_iterator i = _labs.begin(); i != _labs.end(); i++)
		if ((*i)->fileExists(filename))
			return *i;

	return NULL;
}

bool ResourceLoader::fileExists(const char *filename) const {
	return findFile(filename) != NULL;
}

Block *ResourceLoader::getFileBlock(const char *filename) const {
	const Lab *l = findFile(filename);
	if (!l)
		return NULL;
	else
		return l->getFileBlock(filename);
}

LuaFile *ResourceLoader::openNewStreamLua(const char *filename) const {
	const Lab *l = findFile(filename);

	if (!l)
		return NULL;
	else
		return l->openNewStreamLua(filename);
}

Common::File *ResourceLoader::openNewStreamFile(const char *filename) const {
	const Lab *l = findFile(filename);

	if (!l)
		return NULL;
	else
		return l->openNewStreamFile(filename);
}

int ResourceLoader::fileLength(const char *filename) const {
	const Lab *l = findFile(filename);
	if (l)
		return l->fileLength(filename);
	else
		return 0;
}

Bitmap *ResourceLoader::loadBitmap(const char *filename) {
	std::string fname = filename;
	makeLower(fname);
	CacheType::iterator i = _cache.find(fname);
	if (i != _cache.end()) {
		return dynamic_cast<Bitmap *>(i->second);
	}

	Block *b = getFileBlock(filename);
	if (!b) {	// Grim sometimes asks for non-existant bitmaps (eg, ha_overhead)
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Could not find bitmap %s", filename);
		return NULL;
	}

	Bitmap *result = new Bitmap(filename, b->data(), b->len());
	delete b;
	_cache[fname] = result;
	return result;
}

CMap *ResourceLoader::loadColormap(const char *filename) {
	std::string fname = filename;
	makeLower(fname);
	CacheType::iterator i = _cache.find(fname);

	if (i != _cache.end()) {
		return dynamic_cast<CMap *>(i->second);
	}

	Block *b = getFileBlock(filename);
	if (!b)
		error("Could not find colormap %s", filename);
	CMap *result = new CMap(filename, b->data(), b->len());
	delete b;
	_cache[fname] = result;
	return result;
}

Costume *ResourceLoader::loadCostume(const char *filename, Costume *prevCost) {
	std::string fname = filename;
	makeLower(fname);
	Block *b = getFileBlock(filename);
	if (!b)
		error("Could not find costume %s", filename);
	Costume *result = new Costume(filename, b->data(), b->len(), prevCost);
	delete b;
	return result;
}

Font *ResourceLoader::loadFont(const char *filename) {
	std::string fname = filename;
	makeLower(fname);
	CacheType::iterator i = _cache.find(fname);
	if (i != _cache.end()) {
		return dynamic_cast<Font *>(i->second);
	}

	Block *b = getFileBlock(filename);
	if (!b)
		error("Could not find font file %s", filename);
	Font *result = new Font(filename, b->data(), b->len());
	delete b;
	_cache[fname] = result;
	return result;
}

KeyframeAnim *ResourceLoader::loadKeyframe(const char *filename) {
	std::string fname = filename;
	makeLower(fname);
	CacheType::iterator i = _cache.find(fname);
	if (i != _cache.end()) {
		return dynamic_cast<KeyframeAnim *>(i->second);
	}

	Block *b = getFileBlock(filename);
	if (!b)
		error("Could not find keyframe file %s", filename);
	KeyframeAnim *result = new KeyframeAnim(filename, b->data(), b->len());
	delete b;
	_cache[fname] = result;
	return result;
}

LipSynch *ResourceLoader::loadLipSynch(const char *filename) {
	std::string fname = filename;
	LipSynch *result;

	makeLower(fname);
	CacheType::iterator i = _cache.find(fname);
	if (i != _cache.end()) {
		return dynamic_cast<LipSynch *>(i->second);
	}

	Block *b = getFileBlock(filename);
	if (!b) {
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Could not find lipsynch file %s", filename);
		result = NULL;
	} else {
		result = new LipSynch(filename, b->data(), b->len());
		
		// Some lipsynch files have no data
		if (result->isValid()) {
			delete b;
			_cache[fname] = result;
		} else {
			delete result;
			result = NULL;
		}
	}	

	return result;
}

Material *ResourceLoader::loadMaterial(const char *filename, const CMap &c) {
	std::string fname = std::string(filename) + "@" + c.filename();
	makeLower(fname);
	CacheType::iterator i = _cache.find(fname);
	if (i != _cache.end()) {
		return dynamic_cast<Material *>(i->second);
	}

	Block *b = getFileBlock(filename);
	if (!b)
		error("Could not find material %s", filename);
	Material *result = new Material(fname.c_str(), b->data(), b->len(), c);
	delete b;
	_cache[fname] = result;
	return result;
}

Model *ResourceLoader::loadModel(const char *filename, const CMap &c) {
	std::string fname = filename;
	makeLower(fname);
	CacheType::iterator i = _cache.find(fname);
	if (i != _cache.end()) {
		return dynamic_cast<Model *>(i->second);
	}

	Block *b = getFileBlock(filename);
	if (!b)
		error("Could not find model %s", filename);
	Model *result = new Model(filename, b->data(), b->len(), c);
	delete b;
	_cache[fname] = result;
	return result;
}

bool ResourceLoader::exportResource(const char *filename) {
	FILE *myFile = fopen(filename, "w");
	Block *b = getFileBlock(filename);
	
	if (!b)
		return false;
	fwrite(b->data(), b->len(), 1, myFile);
	fclose(myFile);
	delete b;
	return true;
}

void ResourceLoader::uncache(const char *filename) {
	std::string fname = filename;
	makeLower(fname);
	CacheType::iterator i = _cache.find(fname);
	if (i != _cache.end())
		_cache.erase(i);
}
