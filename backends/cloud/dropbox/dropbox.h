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

#ifndef DROPBOX_H
#define DROPBOX_H

#include "common/noncopyable.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/str-array.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/config-manager.h"
#include "backends/cloud/cloud.h"

namespace Cloud {
  class Dropbox:public CloudAbstract {
    Common::String token;
    Common::String getSavePath();
    Common::String getToken();
    Common::String getTokenFromCode(Common::String code);

    public:
    Dropbox();
    virtual bool checkAuth();
    virtual void Auth(Common::String code);

    /* Until the object is authorized, the following functions are unusable*/
    virtual int Sync(const Common::String &pattern);
    virtual int Download(const Common::String &FileName);
    virtual int Upload(const Common::String &FileName);
    virtual int Copy(const Common::String &srcFilename, const Common::String &destFilename);
    virtual int Remove(const Common::String &filename);
  };
}
#endif
