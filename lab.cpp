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
#include "lab.h"
#include "bits.h"
#include <algorithm>
#include <cstdlib>
#include <cctype>

bool Lab::open(const char *filename) {
  close();
  f_ = std::fopen(filename, "rb");
  if (! isOpen())
    return false;

  char header[16];
  if (std::fread(header, 1, sizeof(header), f_) < sizeof(header)) {
    close();
    return false;
  }
  if (std::memcmp(header, "LABN", 4) != 0) {
    close();
    return false;
  }

  int num_entries = READ_LE_UINT32(header + 8);
  int string_table_size = READ_LE_UINT32(header + 12);

  char *string_table = new char[string_table_size];
  std::fseek(f_, 16 * (num_entries + 1), SEEK_SET);
  std::fread(string_table, 1, string_table_size, f_);

  std::fseek(f_, 16, SEEK_SET);
  char binary_entry[16];
#ifndef _MSC_VER
  file_map_.resize(num_entries);
#endif
  for (int i = 0; i < num_entries; i++) {
    std::fread(binary_entry, 1, 16, f_);
    int fname_offset = READ_LE_UINT32(binary_entry);
    int start = READ_LE_UINT32(binary_entry + 4);
    int size = READ_LE_UINT32(binary_entry + 8);

    std::string fname = string_table + fname_offset;
    std::transform(fname.begin(), fname.end(), fname.begin(), tolower);

    file_map_.insert(std::make_pair(fname, LabEntry(start, size)));
#ifndef _MSC_VER
    file_map_.size();
#endif
  }

  delete [] string_table;
  return true;
}

bool Lab::fileExists(const char *filename) const {
  return find_filename(filename) != file_map_.end();
}

Block *Lab::getFileBlock(const char *filename) const {
  file_map_type::const_iterator i = find_filename(filename);
  if (i == file_map_.end())
    return NULL;

  std::fseek(f_, i->second.offset, SEEK_SET);

  // The sound decoder reads up to two bytes past the end of data
  // (but shouldn't actually use those bytes).  So allocate two extra bytes
  // to be safe against crashes.
  char *data = new char[i->second.len + 2];
  std::fread(data, 1, i->second.len, f_);
  data[i->second.len] = '\0';	// For valgrind cleanness
  data[i->second.len + 1] = '\0';
  return new Block(data, i->second.len);
}

std::FILE *Lab::openNewStream(const char *filename) const {
  file_map_type::const_iterator i = find_filename(filename);
  if (i == file_map_.end())
    return NULL;

  std::fseek(f_, i->second.offset, SEEK_SET);

  return f_;
}

int Lab::fileLength(const char *filename) const {
  file_map_type::const_iterator i = find_filename(filename);
  if (i == file_map_.end())
    return -1;

  return i->second.len;
}

Lab::file_map_type::const_iterator
Lab::find_filename(const char *filename) const {
  std::string s = filename;
  std::transform(s.begin(), s.end(), s.begin(), tolower);
  return file_map_.find(s);
}

void Lab::close() {
  if (f_ != NULL)
    std::fclose(f_);
  f_ = NULL;
  file_map_.clear();
}
