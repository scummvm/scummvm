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

#ifndef RESOURCE_H
#define RESOURCE_H

#include "lab.h"
#include "map"
#include <list>
#include <string>

class Bitmap;
class CMap;
class Costume;
class KeyframeAnim;
class Material;
class Model;
class Sound;

class Resource {
public:
  Resource(const char *filename) :
    fname_(filename), ref_(0), luaRef_(false) { }
  Resource(const Resource &r) : fname_(r.fname_), ref_(0), luaRef_(false)
  { }
  virtual ~Resource() { }
  void ref() { ref_++; }
  void deref();
  const char *filename() const { return fname_.c_str(); }

  void luaRef() { if (! luaRef_) { ref(); luaRef_ = true; } }
  void luaGc() { if (luaRef_) { luaRef_ = false; deref(); } }

private:
  std::string fname_;
  int ref_;
  bool luaRef_;
};

template <class T>
class ResPtr {
public:
  ResPtr() { ptr_ = NULL; }
  ResPtr(const ResPtr &p) { ptr_ = p.ptr_; if (ptr_ != NULL) ptr_->ref(); }
  ResPtr(T* ptr) { ptr_ = ptr; if (ptr_ != NULL) ptr_->ref(); }
  operator T*() { return ptr_; }
  T& operator *() { return *ptr_; }
  T* operator ->() { return ptr_; }
  ResPtr& operator =(T* ptr) {
    if (ptr_ == ptr) return *this;
    if (ptr_ != NULL) ptr_->deref();
    ptr_ = ptr;
    if (ptr_ != NULL) ptr_->ref();
    return *this;
  }
  ResPtr& operator =(const ResPtr& p) {
    if (this == &p || ptr_ == p.ptr_) return *this;
    if (ptr_ != NULL) ptr_->deref();
    ptr_ = p.ptr_;
    if (ptr_ != NULL) ptr_->ref();
    return *this;
  }
  ~ResPtr() { if (ptr_ != NULL) ptr_->deref(); }

private:
  T* ptr_;
};

class ResourceLoader {
public:
  bool fileExists(const char *filename) const;
  Block *getFileBlock(const char *filename) const;
  std::FILE *openNewStream(const char *filename) const;
  int fileLength(const char *filename) const;

  static ResourceLoader *instance() {
    if (instance_ == NULL)
      instance_ = new ResourceLoader;
    return instance_;
  }

  Bitmap *loadBitmap(const char *fname);
  CMap *loadColormap(const char *fname);
  Costume *loadCostume(const char *fname, Costume *prevCost);
  KeyframeAnim *loadKeyframe(const char *fname);
  Material *loadMaterial(const char *fname, const CMap &c);
  Model *loadModel(const char *fname, const CMap &c);
  Sound *loadSound(const char *fname);
  void uncache(const char *fname);

private:
  ResourceLoader();
  ResourceLoader(const ResourceLoader &);
  ~ResourceLoader();

  static ResourceLoader *instance_;

  typedef std::list<Lab *> LabList;
  LabList labs_;

  const Lab *findFile(const char *filename) const;

  typedef std::map<std::string, Resource *> cache_type;
  cache_type cache_;

  // Shut up pointless g++ warning
  friend class dummy;
};

inline void Resource::deref() {
  if (--ref_ == 0) {
    ResourceLoader::instance()->uncache(fname_.c_str());
    delete this;
  }
}

#endif
