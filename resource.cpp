// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "resource.h"
#include "registry.h"
#include "bitmap.h"
#include "colormap.h"
#include "costume.h"
#include "keyframe.h"
#include "material.h"
#include "model.h"
#include "sound.h"
#include "debug.h"
#include <cstring>
#include <cctype>
#include <string>
#include <algorithm>

static void makeLower(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), tolower);
}

ResourceLoader *ResourceLoader::instance_ = NULL;

ResourceLoader::ResourceLoader() {
	const char *directory = Registry::instance()->get("DataDir");
	std::string dir_str = (directory != NULL ? directory : ".");
	dir_str += '/';
	int lab_counter = 0;

#ifdef _MSC_VER
	WIN32_FIND_DATAA find_file_data;
	HANDLE d = FindFirstFile(dir_str.c_str(), &find_file_data);
#else
	DIR *d = opendir(dir_str.c_str());
#endif

	if (directory == NULL)
		error("Cannot find DataDir registry entry - check configuration file");

	if (d == NULL)
		error("Cannot open DataDir (%s)- check configuration file", dir_str.c_str());

	printf("dir open\n");
#ifdef _MSC_VER
	do {
		int namelen = strlen(find_file_data.cFileName);
		if ((namelen > 4) && (stricmp(find_file_data.cFileName + namelen - 4, ".lab") == 0)) {
			std::string fullname = dir_str + find_file_data.cFileName;
			Lab *l = new Lab(fullname.c_str());
			lab_counter++;
			if (l->isOpen())
				labs_.push_back(l);
			else
				delete l;
		}
	} while (FindNextFile(d, &find_file_data) != NULL);
	FindClose(d);
#else
	dirent *de;
	while ((de = readdir(d)) != NULL) {
		int namelen = strlen(de->d_name);
		if (namelen > 4 && strcasecmp(de->d_name + namelen - 4, ".lab") == 0) {
			std::string fullname = dir_str + de->d_name;
			Lab *l = new Lab(fullname.c_str());
			lab_counter++;
			if (l->isOpen())
				labs_.push_back(l);
			else
				delete l;
		}
	}
	closedir(d);
#endif

	if (lab_counter == 0)
	error("Cannot find any resource files in %s - check configuration file", dir_str.c_str());
}

const Lab *ResourceLoader::findFile(const char *filename) const {
  for (LabList::const_iterator i = labs_.begin(); i != labs_.end(); i++)
    if ((*i)->fileExists(filename))
      return *i;
  return NULL;
}

bool ResourceLoader::fileExists(const char *filename) const {
  return findFile(filename) != NULL;
}

Block *ResourceLoader::getFileBlock(const char *filename) const {
  const Lab *l = findFile(filename);
  if (l == NULL)
    return NULL;
  else
    return l->getFileBlock(filename);
}

//std::FILE *ResourceLoader::openNewStream(const char *filename) const {
//  const Lab *l = findFile(filename);
//  if (l == NULL)
//    return NULL;
//  else
//    return l->openNewStream(filename);
//}

int ResourceLoader::fileLength(const char *filename) const {
  const Lab *l = findFile(filename);
  if (l == NULL)
    return 0;
  else
    return l->fileLength(filename);
}

Bitmap *ResourceLoader::loadBitmap(const char *filename) {
  std::string fname = filename;
  makeLower(fname);
  cache_type::iterator i = cache_.find(fname);
  if (i != cache_.end()) {
    return dynamic_cast<Bitmap *>(i->second);
  }

  Block *b = getFileBlock(filename);
  if (b == NULL) {	// Grim sometimes asks for non-existant bitmaps (eg, ha_overhead)
    warning("Could not find bitmap %s\n", filename);
    return NULL;
  }

  Bitmap *result = new Bitmap(filename, b->data(), b->len());
  delete b;
  cache_[fname] = result;
  return result;
}

Colormap *ResourceLoader::loadColormap(const char *filename) {
  std::string fname = filename;
  makeLower(fname);
  cache_type::iterator i = cache_.find(fname);
  if (i != cache_.end()) {
    return dynamic_cast<Colormap *>(i->second);
  }

  Block *b = getFileBlock(filename);
  if (b == NULL)
    error("Could not find colormap %s\n", filename);
  Colormap *result = new Colormap(filename, b->data(), b->len());
  delete b;
  cache_[fname] = result;
  return result;
}

Costume *ResourceLoader::loadCostume(const char *filename, Costume *prevCost) {
  std::string fname = filename;
  makeLower(fname);
  Block *b = getFileBlock(filename);
  if (b == NULL)
    error("Could not find costume %s\n", filename);
  Costume *result = new Costume(filename, b->data(), b->len(), prevCost);
  delete b;
  return result;
}

KeyframeAnim *ResourceLoader::loadKeyframe(const char *filename) {
  std::string fname = filename;
  makeLower(fname);
  cache_type::iterator i = cache_.find(fname);
  if (i != cache_.end()) {
    return dynamic_cast<KeyframeAnim *>(i->second);
  }

  Block *b = getFileBlock(filename);
  if (b == NULL)
    error("Could not find keyframe file %s\n", filename);
  KeyframeAnim *result = new KeyframeAnim(filename, b->data(), b->len());
  delete b;
  cache_[fname] = result;
  return result;
}

Material *ResourceLoader::loadMaterial(const char *filename,
				       const Colormap &c) {
  std::string fname = filename;
  makeLower(fname);
  cache_type::iterator i = cache_.find(fname);
  if (i != cache_.end()) {
    return dynamic_cast<Material *>(i->second);
  }

  Block *b = getFileBlock(filename);
  if (b == NULL)
    error("Could not find material %s\n", filename);
  Material *result = new Material(filename, b->data(), b->len(), c);
  delete b;
  cache_[fname] = result;
  return result;
}

Model *ResourceLoader::loadModel(const char *filename, const Colormap &c) {
  std::string fname = filename;
  makeLower(fname);
  cache_type::iterator i = cache_.find(fname);
  if (i != cache_.end()) {
    return dynamic_cast<Model *>(i->second);
  }

  Block *b = getFileBlock(filename);
  if (b == NULL)
    error("Could not find model %s\n", filename);
  Model *result = new Model(filename, b->data(), b->len(), c);
  delete b;
  cache_[fname] = result;
  return result;
}

Sound *ResourceLoader::loadSound(const char *filename) {
  std::string fname = filename;
  makeLower(fname);
  cache_type::iterator i = cache_.find(fname);
  if (i != cache_.end()) {
    return dynamic_cast<Sound *>(i->second);
  }

  Block *b = getFileBlock(filename);
  if (b == NULL)
    return NULL;
  Sound *result = new Sound(filename, b->data(), b->len());
  delete b;
  cache_[fname] = result;
  return result;
}

void ResourceLoader::uncache(const char *filename) {
  std::string fname = filename;
  makeLower(fname);
  cache_type::iterator i = cache_.find(fname);
  if (i != cache_.end())
    cache_.erase(i);
}
