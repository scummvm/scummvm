/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_XRC_READER_H
#define STARK_XRC_READER_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/types.h"

#include "math/vector3d.h"
#include "math/vector4d.h"

#include "engines/stark/resources/object.h"
#include "engines/stark/resourcereference.h"

namespace Stark {
namespace Formats {

class XARCArchive;

/**
 * A read stream with helper functions to read usual XRC data types
 */
class XRCReadStream : public Common::SeekableSubReadStream {
public:
	XRCReadStream(const Common::String &archiveName, Common::SeekableReadStream *parentStream, DisposeAfterUse::Flag disposeParentStream = DisposeAfterUse::YES);
	virtual ~XRCReadStream();

	/** Obtain the file name of the archive containing the XRC tree */
	Common::String getArchiveName() const;

	Common::String readString();
	Resources::Type readResourceType();
	ResourceReference readResourceReference();
	Math::Vector3d readVector3();
	Common::Rect readRect();
	Common::Point readPoint();
	bool readBool();
	bool isDataLeft();

private:
	Common::String _archiveName;
};

/**
 * An XRC stream parser, used to build resource trees.
 */
class XRCReader {
public:
	/**
	 * Build a resource tree from a stream
	 */
	static Resources::Object *importTree(XARCArchive *archive);

protected:
	static Resources::Object *importResource(XRCReadStream *stream, Resources::Object *parent);
	static Resources::Object *createResource(XRCReadStream *stream, Resources::Object *parent);
	static void importResourceChildren(XRCReadStream *stream, Resources::Object *resource);
	static void importResourceData(XRCReadStream* stream, Resources::Object* resource);
};

} // End of namespace Formats
} // End of namespace Stark

#endif // STARK_XRC_READER_H
