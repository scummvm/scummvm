/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GOT_UTILS_RES_ARCHIVE_H
#define GOT_UTILS_RES_ARCHIVE_H

#include "common/archive.h"
#include "common/stream.h"

namespace Got {

struct ResHeader {
    Common::String _name;
    uint32 _offset = 0;
    uint32 _size = 0;
    uint32 _originalSize = 0;
    int _key = 0;

    void load(Common::SeekableReadStream *src);
};

#define RES_MAX_ENTRIES 256  // Max # of elements
#define RES_HEADER_ENTRY_SIZE 23	// Size of a single header entry
class ResArchive : public Common::Archive {
private:
    Common::Array<ResHeader> _headers;

    /**
     * Decrypts a passed buffer
     * @param buf	Pointer to buffer
     * @param len	Buffer size
     * @param key	Starting key to use for decryption
     */
    void decrypt(byte *buf, size_t len, byte key) const;

    /**
     * Decodes a passed buffer
     */
    void lzssDecompress(const byte *src, byte *dest) const;

    /**
     * Returns the index of a header for a given filename
    */
    int indexOf(const Common::String &name) const;

public:
    /**
     * Constructor
     */
    ResArchive();

    /**
     * Check if a member with the given name is present in the Archive.
     * Patterns are not allowed, as this is meant to be a quick File::exists()
     * replacement.
     */
    bool hasFile(const Common::Path &path) const override;

    /**
     * Add all members of the Archive to list.
     * Must only append to list, and not remove elements from it.
     *
     * @return the number of names added to list
     */
    int listMembers(Common::ArchiveMemberList &list) const override;

    /**
     * Returns a ArchiveMember representation of the given file.
     */
    const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;

    /**
     * Create a stream bound to a member with the specified name in the
     * archive. If no member with this name exists, 0 is returned.
     * @return the newly created input stream
     */
    Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
};

extern void resInit();

} // namespace Got

#endif
