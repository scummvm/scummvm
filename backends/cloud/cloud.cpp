/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/cloud/cloud.h"
#include "backends/cloud/dropbox/dropbox.h"


namespace Cloud {

  CloudInstance defaultObj;
  CloudInstance* getDefaultInstance() {
    return &defaultObj;
  }

  CloudInstance::CloudInstance() {
    useDropbox();
  }
  CloudInstance::CloudInstance(CloudAbstract *absCloud) {
    cloud = absCloud;
  }
  void CloudInstance::Auth(Common::String code) {
    cloud->Auth(code);
  }
  bool CloudInstance::checkAuth() {
    return cloud->checkAuth();
  }

  int CloudInstance::Sync(const Common::String &pattern) {
    return cloud->Sync(pattern);
  }

  int CloudInstance::Download(const Common::String &FileName) {
    return cloud->Download(FileName);
  }

  int CloudInstance::Upload(const Common::String &filename) {
    return cloud->Upload(filename);
  }

  int CloudInstance::Copy(const Common::String &srcFilename, const Common::String &destFilename) {
    return cloud->Copy(srcFilename, destFilename);
  }

  int CloudInstance::Remove(const Common::String &filename) {
    return cloud->Remove(filename);
  }

  void CloudInstance::useDropbox() {
    CloudAbstract* newcloud = new Dropbox();
    delete cloud;
    cloud = newcloud;
    cloudName = Common::String("dropbox_");
  }

  void CloudInstance::useOneDrive() {
    //TODO: Implement
  }
  void CloudInstance::useGDrive() {
    //TODO: Implement
  }
}
