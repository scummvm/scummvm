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

#ifndef GLK_BLORB_H
#define GLK_BLORB_H

#include "glk/glk_types.h"
#include "glk/streams.h"
#include "common/archive.h"
#include "common/array.h"

namespace Glk {

/**
 * Describes one chunk of the Blorb file.
 */
struct ChunkEntry {
	uint _type;
	uint _number;
	uint _id;
	size_t _offset;
	size_t _size;
	Common::String _filename;
};

enum {
	ID_FORM = MKTAG('F', 'O', 'R', 'M'),
	ID_IFRS = MKTAG('I', 'F', 'R', 'S'),
	ID_RIdx = MKTAG('R', 'I', 'd', 'x'),
	ID_APal = MKTAG('A', 'P', 'a', 'l'),

	ID_Snd = MKTAG('S', 'n', 'd', ' '),
	ID_Exec = MKTAG('E', 'x', 'e', 'c'),
	ID_Pict = MKTAG('P', 'i', 'c', 't'),
	ID_Data = MKTAG('D', 'a', 't', 'a'),

	ID_Copyright = MKTAG('(', 'c', ')', ' '),
	ID_AUTH = MKTAG('A', 'U', 'T', 'H'),
	ID_ANNO = MKTAG('A', 'N', 'N', 'O'),

	ID_ADRI = MKTAG('A', 'D', 'R', 'I'),
	ID_ZCOD = MKTAG('Z', 'C', 'O', 'D'),
	ID_GLUL = MKTAG('G', 'L', 'U', 'L'),
	ID_TAD2 = MKTAG('T', 'A', 'D', '2'),
	ID_TAD3 = MKTAG('T', 'A', 'D', '3'),
	ID_HUGO = MKTAG('H', 'U', 'G', 'O'),
	ID_SAAI = MKTAG('S', 'A', 'A', 'I'),

	ID_JPEG = MKTAG('J', 'P', 'E', 'G'),
	ID_PNG  = MKTAG('P', 'N', 'G', ' '),
	ID_Rect = MKTAG('R', 'e', 'c', 't'),

	ID_MIDI = MKTAG('M', 'I', 'D', 'I'),
	ID_MP3 = MKTAG('M', 'P', '3', ' '),
	ID_WAVE = MKTAG('W', 'A', 'V', 'E'),
	ID_AIFF = MKTAG('A', 'I', 'F', 'F'),
	ID_OGG = MKTAG('O', 'G', 'G', ' '),
	ID_MOD = MKTAG('M', 'O', 'D', ' ')
};

/**
 * Blorb file manager
 */
class Blorb : public Common::Archive {
private:
	Common::String _filename;
	Common::FSNode _fileNode;
	InterpreterType _interpType;
	Common::Array<ChunkEntry> _chunks;	///< list of chunk descriptors
private:
	/**
	 * Parses the Blorb file index to load in a list of the chunks
	 */
	Common::ErrorCode load();

	/**
	 * Add possible Blorb filenames for Infocom games
	 */
	static void getInfocomBlorbFilenames(Common::StringArray &filenames, const Common::String &gameId);
public:
	/**
	 * Constructor
	 */
	Blorb(const Common::String &filename, InterpreterType interpType);

	/**
	 * Constructor
	 */
	Blorb(const Common::FSNode &fileNode, InterpreterType interpType);

	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	bool hasFile(const Common::String &name) const override;

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
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;

	/**
	 * Read the RIdx section from the stream.
	 */
	static bool readRIdx(Common::SeekableReadStream &stream, Common::Array<ChunkEntry> &chunks);

	/**
	 * Returns true if a given file is a Blorb file
	 */
	static bool isBlorb(Common::SeekableReadStream &stream, uint32 type = 0);

	/**
	 * Returns true if a given filename specifies a Blorb file
	 */
	static bool isBlorb(const Common::String &filename, uint32 type = 0);

	/**
	 * Returns true if a given filename has a Blorb file extension
	 */
	static bool hasBlorbExt(const Common::String &filename);

	/**
	 * Return a list of possible filenames for blorb files
	 */
	static void getBlorbFilenames(const Common::String &srcFilename, Common::StringArray &filenames,
		InterpreterType interpType, const Common::String &gameId);
};

} // End of namespace Glk

#endif
