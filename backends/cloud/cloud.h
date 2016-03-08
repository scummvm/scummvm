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

#ifndef CLOUD_H
#define CLOUD_H
#include "common/noncopyable.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/str-array.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/config-manager.h"

namespace Cloud {

  /**
   * The main cloud API class which can be instantiated to get a new cloud
   * provider. It implements the state Pattern.
   */
  class CloudInstance {
    class CloudAbstract *cloud;
    Common::String cloudName;
    public:
    CloudInstance();
    CloudInstance(CloudAbstract* absCloud);
    ~CloudInstance(){};

    /**
     * Attempt to authenticate the provider with the given code. This is a one
     * time process. Until the provider is authenticated, the services below
     * won't work.
     * @param code The Authentication Code recieved from visitng provider' URL
     */
    void Auth(Common::String code);

    /**
     * @return true if the Provider is already authenticated. False, otherwise
     */
    bool checkAuth();

    /**
     * Attempt to Sync all the files matching the given pattern
     * @param pattern The Search pattern to match files on the cloud
     * @return 0 if success, failure otherwise
     */
    int Sync(const Common::String &pattern);

    /**
     * Attempt to download the file with the given name.
     * @param FileName Name of the file to download
     * @return 0 if success, failure otherwise
     */
    int Download(const Common::String &FileName);

    /**
     * Attempt to Upload the file with the given name.
     * @param FileName Name of the file to upload
     * @return 0 if success, failure otherwise
     */
    int Upload(const Common::String &FileName);

    /**
     * Copy a file from source to destination
     * @param srcFilename Name of the file to copy
     * @param destFileName Name of the new copied file
     * @return 0 if success, failure otherwise
     */
    int Copy(const Common::String &srcFilename, const Common::String &destFilename);

    /**
     * Remove a file with given name
     * @param filename Name of the file to Delete
     * @return 0 if success, failure otherwise
     */
    int Remove(const Common::String &filename);

    /**
     * Use Dropbox service provider. After this is called, this instance
     * will provide dropbox services. This is also the default behaviour
     */
    void useDropbox();

    /**
     * Use OneDrive service provider. After this is called, this instance
     * will provide OneDrive services.
     */
    void useOneDrive();

    /**
     * Use Google service provider. After this is called, this instance
     * will provide Google Drive services.
     */
    void useGDrive();
  };

  /**
   * The Abstract Cloud Class which can be inherited into different cloud
   * providers
   */
  class CloudAbstract {
    public:
      virtual void Auth(Common::String code)=0;
      virtual bool checkAuth()=0;
      virtual int Sync(const Common::String &pattern)=0;
      virtual int Download(const Common::String &FileName)=0;
      virtual int Upload(const Common::String &filename)=0;
      virtual int Copy(const Common::String &srcFilename, const Common::String &destFilename)=0;
      virtual int Remove(const Common::String &filename)=0;
      virtual ~CloudAbstract() {}
  };  
  CloudInstance* getDefaultInstance();
}
#endif
