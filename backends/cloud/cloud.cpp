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
    useDropBox();
}
CloudInstance::CloudInstance(CloudAbstract *absCloud) {
    _cloud = absCloud;
}
void CloudInstance::auth(Common::String code) {
    _cloud->auth(code);
}
cloudAuth CloudInstance::checkAuth() {
    return _cloud->checkAuth();
}

int CloudInstance::sync(const Common::String &pattern) {
    return _cloud->sync(pattern);
}

int CloudInstance::download(const Common::String &fileName) {
    return _cloud->download(fileName);
}

int CloudInstance::upload(const Common::String &fileName) {
    return _cloud->upload(fileName);
}

int CloudInstance::copy(const Common::String &srcFileName, const Common::String &destFileName) {
    return _cloud->copy(srcFileName, destFileName);
}

int CloudInstance::remove(const Common::String &fileName) {
    return _cloud->remove(fileName);
}

void CloudInstance::useDropBox() {
    CloudAbstract* newcloud = new DropBox();
    delete _cloud;
    _cloud = newcloud;
    _cloudName = Common::String("dropbox_");
}

void CloudInstance::useOneDrive() {
    //TODO: Implement
}
void CloudInstance::useGDrive() {
    //TODO: Implement
}

}
