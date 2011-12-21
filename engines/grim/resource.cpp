/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 */

#include "engines/grim/resource.h"
#include "engines/grim/colormap.h"
#include "engines/grim/costume.h"
#include "engines/grim/keyframe.h"
#include "engines/grim/material.h"
#include "engines/grim/grim.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/savegame.h"
#include "engines/grim/lab.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/font.h"
#include "engines/grim/model.h"
#include "engines/grim/modelemi.h"
#include "engines/grim/skeleton.h"
#include "engines/grim/inputdialog.h"
#include "engines/grim/debug.h"
#include "common/algorithm.h"
#include "gui/message.h"

namespace Grim {

ResourceLoader *g_resourceloader = NULL;

class LabListComperator {
	const Common::String _labName;
public:
	LabListComperator() {}
	LabListComperator(const Common::String &ln) : _labName(ln) {}

	bool operator()(const Common::ArchiveMemberPtr &l) {
		return _labName.compareToIgnoreCase(l->getName()) == 0;
	}

	bool operator()(const Common::ArchiveMemberPtr &l, const Common::ArchiveMemberPtr &r) {
		return (l->getName().compareToIgnoreCase(r->getName()) > 0);
	}
};

ResourceLoader::ResourceLoader() {
	_cacheDirty = false;
	_cacheMemorySize = 0;

	Lab *l;
	Common::ArchiveMemberList files;

	if (g_grim->getGameType() == GType_GRIM) {
		if (g_grim->getGameFlags() & ADGF_DEMO) {
			SearchMan.listMatchingMembers(files, "gfdemo01.lab");
			SearchMan.listMatchingMembers(files, "grimdemo.mus");
			SearchMan.listMatchingMembers(files, "sound001.lab");
			SearchMan.listMatchingMembers(files, "voice001.lab");
		} else {
			SearchMan.listMatchingMembers(files, "data???.lab");
			SearchMan.listMatchingMembers(files, "movie??.lab");
			SearchMan.listMatchingMembers(files, "vox????.lab");
			SearchMan.listMatchingMembers(files, "year?mus.lab");
			SearchMan.listMatchingMembers(files, "local.lab");
			SearchMan.listMatchingMembers(files, "credits.lab");

			//Sort the archives in order to ensure that they are loaded with the correct order
			Common::sort(files.begin(), files.end(), LabListComperator());

			//Check the presence of datausr.lab and ask the user if he wants to load it.
			//In this case put it in the top of the list
			Common::ArchiveMemberList::iterator datausr_it = Common::find_if(files.begin(), files.end(), LabListComperator("datausr.lab"));
			if (datausr_it != files.end()) {
				Grim::InputDialog d("User-patch detected, the Residual-team\n provides no support for using such patches.\n Click OK to load, or Cancel\n to skip the patch.", "OK", false);
				int res = d.runModal();
				if (res == GUI::kMessageOK)
					files.push_front(*datausr_it);
				files.erase(datausr_it);
			}
		}
	}

	if (g_grim->getGameType() == GType_MONKEY4)
		SearchMan.listMatchingMembers(files, "*.m4b");

	if (files.empty())
		error("Cannot find game data - check configuration file");

	int priority = files.size();
	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		Common::String filename = (*x)->getName();
		filename.toLowercase();

		l = new Lab();
		if (l->open(filename))
			_files.add(filename, l, priority--, true);
		else
			delete l;
	}

	files.clear();
}

template<typename T>
void clearList(Common::List<T> &list) {
	while (!list.empty()) {
		T p = list.front();
		list.erase(list.begin());
		delete p;
	}
}

ResourceLoader::~ResourceLoader() {
	for (Common::Array<ResourceCache>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
		ResourceCache &r = *i;
		delete[] r.fname;
		delete[] r.resPtr;
	}
	clearList(_models);
	clearList(_colormaps);
	clearList(_keyframeAnims);
	clearList(_lipsyncs);
}

static int sortCallback(const void *entry1, const void *entry2) {
	return scumm_stricmp(((ResourceLoader::ResourceCache *)entry1)->fname, ((ResourceLoader::ResourceCache *)entry2)->fname);
}

Common::SeekableReadStream *ResourceLoader::getFileFromCache(const Common::String &filename) {
	ResourceLoader::ResourceCache *entry = getEntryFromCache(filename);
	if (!entry)
		return NULL;

	return new Common::MemoryReadStream(entry->resPtr, entry->len);

}

ResourceLoader::ResourceCache *ResourceLoader::getEntryFromCache(const Common::String &filename) {
	if (_cache.empty())
		return NULL;

	if (_cacheDirty) {
		qsort(_cache.begin(), _cache.size(), sizeof(ResourceCache), sortCallback);
		_cacheDirty = false;
	}

	ResourceCache key;
	key.fname = const_cast<char *>(filename.c_str());

	return (ResourceLoader::ResourceCache *)bsearch(&key, _cache.begin(), _cache.size(), sizeof(ResourceCache), sortCallback);
}

bool ResourceLoader::getFileExists(const Common::String &filename) {
	return _files.hasFile(filename);
}

Common::SeekableReadStream *ResourceLoader::loadFile(Common::String &filename) {
	if (_files.hasFile(filename))
		return _files.createReadStreamForMember(filename);
	else if (SearchMan.hasFile(filename))
		return SearchMan.createReadStreamForMember(filename);
	else
		return NULL;
}

Common::SeekableReadStream *ResourceLoader::openNewStreamFile(const char *filename, bool cache) {
	Common::String fname = filename;
	Common::SeekableReadStream *s;
    fname.toLowercase();

	if (cache) {
		s = getFileFromCache(fname);
		if (!s) {
			s = loadFile(fname);
			if (!s)
				return NULL;

			uint32 size = s->size();
			byte *buf = new byte[size];
			s->read(buf, size);
			putIntoCache(fname, buf, size);
			return new Common::MemoryReadStream(buf, size);
		} else
			return s;
	}

	return loadFile(fname);
}

void ResourceLoader::putIntoCache(const Common::String &fname, byte *res, uint32 len) {
	ResourceCache entry;
	entry.resPtr = res;
	entry.len = len;
	entry.fname = new char[fname.size() + 1];
	strcpy(entry.fname, fname.c_str());
	_cacheMemorySize += len;
	_cache.push_back(entry);
	_cacheDirty = true;
}

Bitmap *ResourceLoader::loadBitmap(const Common::String &filename) {
	Common::String fname = filename;
	fname.toLowercase();

	Common::SeekableReadStream *stream = openNewStreamFile(fname.c_str(), true);
	if (!stream) {	// Grim sometimes asks for non-existant bitmaps (eg, ha_overhead)
		warning("Could not find bitmap %s", filename.c_str());
		return NULL;
	}

	Bitmap *result = new Bitmap(filename, stream);

	return result;
}

CMap *ResourceLoader::loadColormap(const Common::String &filename) {
	Common::SeekableReadStream *stream = openNewStreamFile(filename.c_str());
	if (!stream) {
		error("Could not find colormap %s", filename.c_str());
	}

	CMap *result = new CMap(filename, stream);
	_colormaps.push_back(result);

	return result;
}

static Common::String fixFilename(const Common::String filename, bool append = true) {
	Common::String fname(filename);
	if (g_grim->getGameType() == GType_MONKEY4) {
		int len = fname.size();
		for (int i = 0; i < len; i++) {
			if (fname[i] == '\\') {
				fname.setChar('/', i);
			}
		}
		// Append b to end of filename for EMI
		if (append)
			fname += "b";
	}
	return fname;
}

Costume *ResourceLoader::loadCostume(const Common::String &filename, Costume *prevCost) {
	Common::String fname = fixFilename(filename);
	fname.toLowercase();

	Common::SeekableReadStream *stream = openNewStreamFile(fname.c_str(), true);
	if (!stream) {
		error("Could not find costume \"%s\"", filename.c_str());
	}

	Costume *result = new Costume(filename, stream, prevCost);

	return result;
}

Font *ResourceLoader::loadFont(const Common::String &filename) {
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(filename.c_str(), true);
	if(!stream)
		error("Could not find font file %s", filename.c_str());

	Font *result = new Font(filename, stream);

	return result;
}

KeyframeAnim *ResourceLoader::loadKeyframe(const Common::String &filename) {
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(filename.c_str());
	if(!stream)
		error("Could not find keyframe file %s", filename.c_str());

	KeyframeAnim *result = new KeyframeAnim(filename, stream);
	_keyframeAnims.push_back(result);

	return result;
}

LipSync *ResourceLoader::loadLipSync(const Common::String &filename) {
	LipSync *result;
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(filename.c_str());
	if(!stream)
		return NULL;

	result = new LipSync(filename, stream);

	// Some lipsync files have no data
	if (result->isValid())
		_lipsyncs.push_back(result);
	else {
		delete result;
		result = NULL;
	}

	return result;
}

Material *ResourceLoader::loadMaterial(const Common::String &filename, CMap *c) {
	Common::String fname = fixFilename(filename, false);
	fname.toLowercase();
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(fname.c_str(), true);
	if(!stream)
		error("Could not find material %s", filename.c_str());

	Material *result = new Material(fname, stream, c);

	return result;
}

Model *ResourceLoader::loadModel(const Common::String &filename, CMap *c, Model *parent) {
	Common::String fname = fixFilename(filename);
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(fname.c_str());
	if(!stream)
		error("Could not find model %s", filename.c_str());

	Model *result = new Model(filename, stream, c, parent);
	_models.push_back(result);

	return result;
}

EMIModel *ResourceLoader::loadModelEMI(const Common::String &filename, EMIModel *parent) {
	Common::String fname = fixFilename(filename);
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(fname.c_str());
	if(!stream) {
		warning("Could not find model %s", filename.c_str());
		return NULL;
	}

	EMIModel *result = new EMIModel(filename, stream, parent);
	_emiModels.push_back(result);

	return result;
}

Skeleton *ResourceLoader::loadSkeleton(const Common::String &filename) {
	Common::String fname = fixFilename(filename);
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(fname.c_str(), true);
	if(!stream) {
		warning("Could not find skeleton %s", filename.c_str());
		return NULL;
	}

	Skeleton *result = new Skeleton(filename, stream);

	return result;
}

void ResourceLoader::uncache(const char *filename) {
	Common::String fname = filename;
	fname.toLowercase();

	if (_cacheDirty) {
		qsort(_cache.begin(), _cache.size(), sizeof(ResourceCache), sortCallback);
		_cacheDirty = false;
	}

	for (unsigned int i = 0; i < _cache.size(); i++) {
		if (fname.compareTo(_cache[i].fname) == 0) {
			delete[] _cache[i].fname;
			_cacheMemorySize -= _cache[i].len;
			delete[] _cache[i].resPtr;
			_cache.remove_at(i);
			_cacheDirty = true;
		}
	}
}

void ResourceLoader::uncacheModel(Model *m) {
	_models.remove(m);
}

void ResourceLoader::uncacheColormap(CMap *c) {
	_colormaps.remove(c);
}

void ResourceLoader::uncacheKeyframe(KeyframeAnim *k) {
	_keyframeAnims.remove(k);
}

void ResourceLoader::uncacheLipSync(LipSync *s) {
	_lipsyncs.remove(s);
}

ModelPtr ResourceLoader::getModel(const Common::String &fname, CMap *c) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<Model *>::const_iterator i = _models.begin(); i != _models.end(); ++i) {
		Model *m = *i;
		if (filename == m->_fname && *m->_cmap == *c) {
			return m;
		}
	}

	return loadModel(fname, c);
}

CMapPtr ResourceLoader::getColormap(const Common::String &fname) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<CMap *>::const_iterator i = _colormaps.begin(); i != _colormaps.end(); ++i) {
		CMap *c = *i;
		if (filename.equals(c->_fname)) {
			return c;
		}
	}

	return loadColormap(fname);
}

KeyframeAnimPtr ResourceLoader::getKeyframe(const Common::String &fname) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<KeyframeAnim *>::const_iterator i = _keyframeAnims.begin(); i != _keyframeAnims.end(); ++i) {
		KeyframeAnim *k = *i;
		if (filename == k->getFilename()) {
			return k;
		}
	}

	return loadKeyframe(fname);
}

LipSyncPtr ResourceLoader::getLipSync(const Common::String &fname) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<LipSync *>::const_iterator i = _lipsyncs.begin(); i != _lipsyncs.end(); ++i) {
		LipSync *l = *i;
		if (filename == l->getFilename()) {
			return l;
		}
	}

	return loadLipSync(fname);
}

} // end of namespace Grim
