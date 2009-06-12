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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef PARALLACTION_IFF_H
#define PARALLACTION_IFF_H

#include "common/stream.h"
#include "common/func.h"
#include "common/iff_container.h"		// for IFF chunk names
#include "graphics/iff.h"				// for BMHD


namespace Parallaction {

/**
 *  Represents a IFF chunk available to client code.
 *
 *  Client code must *not* deallocate _stream when done.
 */
struct IFFChunk {
	Common::IFF_ID			_type;
	uint32					_size;
	Common::ReadStream		*_stream;

	IFFChunk(Common::IFF_ID type, uint32 size, Common::ReadStream *stream) : _type(type), _size(size), _stream(stream) {
		assert(_stream);
	}
};

/**
 *  Parser for IFF containers.
 */
class IFFParser {

	/**
	 *  This private class implements IFF chunk navigation.
	 */
	class IFFChunkNav : public Common::ReadStream {
	protected:
		Common::ReadStream *_input;
		uint32 _bytesRead;
	public:
		Common::IFF_ID id;
		uint32 size;

		IFFChunkNav() : _input(0) {
		}
		void setInputStream(Common::ReadStream *input) {
			_input = input;
			size = _bytesRead = 0;
		}
		void incBytesRead(uint32 inc) {
			_bytesRead += inc;
			if (_bytesRead > size) {
				error("Chunk overread");
			}
		}
		void readHeader() {
			id = _input->readUint32BE();
			size = _input->readUint32BE();
			_bytesRead = 0;
		}
		bool hasReadAll() const {
			return (size - _bytesRead) == 0;
		}
		void feed() {
			if (size % 2) {
				size++;
			}
			while (!hasReadAll()) {
				readByte();
			}
		}
		// Common::ReadStream implementation
		bool eos() const { return _input->eos(); }
		bool err() const { return _input->err(); }
		void clearErr() { _input->clearErr(); }

		uint32 read(void *dataPtr, uint32 dataSize) {
			incBytesRead(dataSize);
			return _input->read(dataPtr, dataSize);
		}
	};

	IFFChunkNav _formChunk;	//!< The root chunk of the file.
	IFFChunkNav _chunk; 	//!< The current chunk.

	Common::ReadStream *_stream;
	bool _disposeStream;

	void setInputStream(Common::ReadStream *stream);

public:
	IFFParser(Common::ReadStream *stream, bool disposeStream = false) : _stream(stream), _disposeStream(stream) {
		setInputStream(stream);
	}
	~IFFParser() {
		if (_disposeStream) {
			delete _stream;
		}
		_stream = 0;
	}

	/**
	 * Returns the IFF FORM type.
	 * @return the IFF FORM type of the stream, or 0 if FORM header is not found.
	 */
	Common::IFF_ID getFORMType() const;

	/**
	 * Returns the size of the data.
	 * @return the size of the data in file, or -1 if FORM header is not found.
	 */
	uint32 getFORMSize() const;

	/**
	 * Callback type for the parser.
	 */
	typedef Common::Functor1< IFFChunk&, bool > IFFCallback;

	/**
	 * Parse the IFF container, invoking the callback on each chunk encountered.
	 * The callback can interrupt the parsing by returning 'true'.
	 */
	void parse(IFFCallback &callback);

private:
	uint32 _formSize;
	Common::IFF_ID _formType;
};




struct ILBMDecoder {
	/**
	 * ILBM header data, necessary for loadBitmap()
	 */
	Graphics::BMHD	_header;

	/**
	 * Available decoding modes for loadBitmap().
	 */
	enum {
		ILBM_UNPACK_PLANES = 0xFF,		//!< Decode all bitplanes, and map 1 pixel to 1 byte.
		ILBM_PACK_PLANES   = 0x100,		//!< Request unpacking, used as a mask with below options.

		ILBM_1_PLANES      = 1,									//!< Decode only the first bitplane, don't pack.
		ILBM_1_PACK_PLANES = ILBM_1_PLANES | ILBM_PACK_PLANES, 	//!< Decode only the first bitplane, pack 8 pixels in 1 byte.
		ILBM_2_PLANES      = 2,									//!< Decode first 2 bitplanes, don't pack.
		ILBM_2_PACK_PLANES = ILBM_2_PLANES | ILBM_PACK_PLANES,	//!< Decode first 2 bitplanes, pack 4 pixels in 1 byte.
		ILBM_3_PLANES      = 3,									//!< Decode first 3 bitplanes, don't pack.
		ILBM_4_PLANES      = 4,									//!< Decode first 4 bitplanes, don't pack.
		ILBM_4_PACK_PLANES = ILBM_4_PLANES | ILBM_PACK_PLANES,	//!< Decode first 4 bitplanes, pack 2 pixels in 1 byte.
		ILBM_5_PLANES      = 5,									//!< Decode first 5 bitplanes, don't pack.
		ILBM_8_PLANES      = 8									//!< Decode all 8 bitplanes.
	};

	/**
	 * Fills the _header member from the given stream.
	 */
	void loadHeader(Common::ReadStream *stream);

	/**
	 * Loads and unpacks the ILBM bitmap data from the stream into the buffer.
	 * The functions assumes the buffer is large enough to contain all data.
	 * The caller controls how data should be packed by choosing mode from
	 * the enum above.
	 */
	void loadBitmap(uint32 mode, byte *buffer, Common::ReadStream *stream);

	/**
	 * Converts from bitplanar to chunky representation. Intended for internal
	 * usage, but you can be (ab)use it from client code if you know what you
	 * are doing.
	 */
	void planarToChunky(byte *out, uint32 width, byte *in, uint32 planeWidth, uint32 nPlanes, bool packPlanes);
};


}

#endif

