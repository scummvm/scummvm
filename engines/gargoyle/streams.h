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

#ifndef GARGOYLE_STREAMS_H
#define GARGOYLE_STREAMS_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/str.h"
#include "gargoyle/glk_types.h"

namespace Gargoyle {

#define SAVEGAME_VERSION 1

class Window;
class Streams;

enum FileUsage {
	fileusage_Data = 0x00,
	fileusage_SavedGame = 0x01,
	fileusage_Transcript = 0x02,
	fileusage_InputRecord = 0x03,
	fileusage_TypeMask = 0x0f,

	fileusage_TextMode = 0x100,
	fileusage_BinaryMode = 0x000,
};

enum FileMode {
	filemode_Write = 0x01,
	filemode_Read = 0x02,
	filemode_ReadWrite = 0x03,
	filemode_WriteAppend = 0x05,
};

enum SeekMode {
	seekmode_Start = 0,
	seekmode_Current = 1,
	seekmode_End = 2,
};

struct StreamResult {
	uint32 _readCount;
	uint32 _writeCount;
};
typedef StreamResult stream_result_t;

struct SavegameHeader {
	uint8 _version;
	byte _interpType;
	byte _language;
	Common::String _md5;
	Common::String _saveName;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;

	/**
	 * Constructor
	 */
	SavegameHeader() : _version(0), _interpType(0), _language(0), _year(0), _month(0), _day(0),
		_hour(0), _minute(0), _totalFrames(0) {}
};

/**
 * File details
 */
struct FileReference {
	glui32 _rock;
	int _slotNumber;
	Common::String _description;
	Common::String _filename;
	FileUsage _fileType;
	bool _textMode;
	gidispatch_rock_t _dispRock;

	/**
	 * Constructor
	 */
	FileReference() : _rock(0), _slotNumber(-1), _fileType(fileusage_Data), _textMode(false) {}

	/**
	 * Constructor
	 */
	FileReference(int slot, const Common::String &desc, glui32 usage, glui32 rock = 0) :
		_rock(rock), _slotNumber(slot), _description(desc),
		_fileType((FileUsage)(usage & fileusage_TypeMask)), _textMode(usage & fileusage_TextMode) {}

	/**
	 * Get savegame filename
	 */
	const Common::String getSaveName() const;

	/**
	 * Returns true if the given file exists
	 */
	bool exists() const;

	/**
	 * Delete the given file
	 */
	void deleteFile();
};

typedef FileReference *frefid_t;
typedef Common::Array< Common::SharedPtr<FileReference> > FileRefArray;


/**
 * Base class for streams
 */
class Stream {
public:
	Streams *_streams;
	Stream *_prev;
	Stream *_next;
	uint32 _rock;
	bool _unicode;
	uint32 _readCount;
	uint32 _writeCount;
	bool _readable, _writable;
public:
	/**
	 * Constructor
	 */
	Stream(Streams *streams, bool readable, bool writable, uint32 rock, bool unicode);

	/**
	 * Destructor
	 */
	virtual ~Stream();

	/**
	 * Get the next stream
	 */
	Stream *getNext(uint32 *rock) const;

	/**
	 * Get the rock value for the stream
	 */
	uint32 getRock() const { return _rock; }

	/**
	 * Fill out the total amount read and/or written
	 */
	void fillResult(StreamResult *result);

	/**
	 * Close and delete the stream
	 */
	void close(StreamResult *result = nullptr);

	/**
	 * Write a character
	 */
	virtual void putChar(unsigned char ch) = 0;

	/**
	 * Write a unicode character
	 */
	virtual void putCharUni(uint32 ch) = 0;

	/**
	 * Write a buffer
	 */
	virtual void putBuffer(const char *buf, size_t len) = 0;

	/**
	 * Write a unicode character
	 */
	virtual void putBufferUni(const uint32 *buf, size_t len) = 0;

	/**
	 * Send a line to the stream with a trailing newline
	 */
	void echoLine(char *buf, glui32 len) {
		putBuffer(buf, len);
		putChar('\n');
	};

	/**
	 * Send a line to the stream with a trailing newline
	 */
	void echoLineUni(glui32 *buf, glui32 len) {
		putBufferUni(buf, len);
		putCharUni('\n');
	}

	virtual glui32 getPosition() const { return 0; }

	virtual void setPosition(glui32 pos, glui32 seekMode) {}

	virtual void setStyle(glui32 val) {}

	/**
	 * Get a character from the stream
	 */
	virtual glsi32 getChar() { return -1; }

	/**
	 * Get a unicode character from the stream
	 */
	virtual glsi32 getCharUni() { return -1; }

	/**
	 * Get a buffer
	 */
	virtual glui32 getBuffer(char *buf, glui32 len) { return 0; }

	/**
	 * Get a unicode buffer
	 */
	virtual glui32 getBufferUni(glui32 *buf, glui32 len) { return 0; }

	/**
	 * Get a line
	 */
	virtual glui32 getLine(char *buf, glui32 len) { return 0; }

	/**
	 * Get a unicode line
	 */
	virtual glui32 getLineUni(glui32 *ubuf, glui32 len) { return 0; }

	/**
	 * Set a hyperlink
	 */
	virtual void setHyperlink(glui32 linkVal) {}
};
typedef Stream *strid_t;

/**
 * Implements the stream for writing text to a window
 */
class WindowStream : public Stream {
private:
	Window *_window;
public:
	/**
	 * Constructor
	 */
	WindowStream(Streams *streams, Window *window, uint32 rock = 0, bool unicode = true) :
		Stream(streams, false, true, rock, unicode), _window(window) {}

	/**
	 * Close the stream
	 */
	virtual void close(StreamResult *result = nullptr);

	/**
	 * Write a character
	 */
	virtual void putChar(unsigned char ch) override;

	/**
	 * Write a unicode character
	 */
	virtual void putCharUni(uint32 ch) override;

	/**
	 * Write a buffer
	 */
	virtual void putBuffer(const char *buf, size_t len) override;

	/**
	 * Write a unicode character
	 */
	virtual void putBufferUni(const uint32 *buf, size_t len) override;

	virtual void setStyle(glui32 val) override;

	/**
	 * Set a hyperlink
	 */
	virtual void setHyperlink(glui32 linkVal) override;
};

/**
 * Implements an in-memory stream
 */
class MemoryStream : public Stream {
private:
	void *_buf;		///< unsigned char* for latin1, glui32* for unicode
	void *_bufPtr;
	void *_bufEnd;
	void *_bufEof;
	size_t _bufLen;	///< # of bytes for latin1, # of 4-byte words for unicode
public:
	/**
	 * Constructor
	 */
	MemoryStream(Streams *streams, void *buf, size_t buflen, FileMode mode, uint32 rock = 0, bool unicode = true);

	/**
	 * Write a character
	 */
	virtual void putChar(unsigned char ch) override;

	/**
	 * Write a unicode character
	 */
	virtual void putCharUni(uint32 ch) override;

	/**
	 * Write a buffer
	 */
	virtual void putBuffer(const char *buf, size_t len) override;

	/**
	 * Write a unicode character
	 */
	virtual void putBufferUni(const uint32 *buf, size_t len) override;

	virtual glui32 getPosition() const override;

	virtual void setPosition(glui32 pos, glui32 seekMode) override;

	/**
	 * Get a character from the stream
	 */
	virtual glsi32 getChar() override;

	/**
	 * Get a unicode character from the stream
	 */
	virtual glsi32 getCharUni() override;

	/**
	 * Get a buffer
	 */
	virtual glui32 getBuffer(char *buf, glui32 len) override;

	/**
	 * Get a unicode buffer
	 */
	virtual glui32 getBufferUni(glui32 *buf, glui32 len) override;

	/**
	 * Get a line
	 */
	virtual glui32 getLine(char *buf, glui32 len) override;

	/**
	 * Get a unicode line
	 */
	virtual glui32 getLineUni(glui32 *ubuf, glui32 len) override;
};

/**
 * Implements a file stream
 */
class FileStream : public Stream {
private:
	Common::File _file;
	Common::OutSaveFile *_outFile;
	Common::InSaveFile *_inFile;
	Common::SeekableReadStream *_inStream;
	uint32 _lastOp;					///< 0, filemode_Write, or filemode_Read
	bool _textFile;
private:
	/**
	 * Ensure the stream is ready for the given operation
	 */
	void ensureOp(FileMode mode);

	/**
	 * Put a UTF8 character
	 */
	void putCharUtf8(glui32 val);

	/**
	 * Get a UTF8 character
	 */
	glsi32 getCharUtf8();
public:
	/**
	 * Read a savegame header from a stream
	 */
	static bool readSavegameHeader(Common::SeekableReadStream *stream, SavegameHeader &header);

	/**
	 * Write out a savegame header
	 */
	static void writeSavegameHeader(Common::WriteStream *stream, const Common::String &saveName);
public:
	/**
	 * Constructor
	 */
	FileStream(Streams *streams, frefid_t fref, glui32 fmode, glui32 rock, bool unicode);

	/**
	 * Destructor
	 */
	virtual ~FileStream();

	/**
	 * Write a character
	 */
	virtual void putChar(unsigned char ch) override;

	/**
	 * Write a unicode character
	 */
	virtual void putCharUni(uint32 ch) override;

	/**
	 * Write a buffer
	 */
	virtual void putBuffer(const char *buf, size_t len) override;

	/**
	 * Write a unicode character
	 */
	virtual void putBufferUni(const uint32 *buf, size_t len) override;

	virtual glui32 getPosition() const override;

	virtual void setPosition(glui32 pos, glui32 seekMode) override;

	/**
	 * Get a character from the stream
	 */
	virtual glsi32 getChar() override;

	/**
	 * Get a unicode character from the stream
	 */
	virtual glsi32 getCharUni() override;

	/**
	 * Get a buffer
	 */
	virtual glui32 getBuffer(char *buf, glui32 len) override;

	/**
	 * Get a unicode buffer
	 */
	virtual glui32 getBufferUni(glui32 *buf, glui32 len) override;

	/**
	 * Get a line
	 */
	virtual glui32 getLine(char *buf, glui32 len) override;

	/**
	 * Get a unicode line
	 */
	virtual glui32 getLineUni(glui32 *ubuf, glui32 len) override;
};

/**
 * Streams manager
 */
class Streams {
	friend class Stream;
private:
	Stream *_streamList;
	Stream *_currentStream;
	FileRefArray _fileReferences;
private:
	/**
	 * Adds a created stream to the list
	 */
	void addStream(Stream *stream);

	/**
	 * Remove a stream
	 */
	void removeStream(Stream *stream);
public:
	/**
	 * Constructor
	 */
	Streams();

	/**
	 * Destructor
	 */
	~Streams();

	/**
	 * Open a file stream
	 */
	FileStream *openFileStream(frefid_t fref, glui32 fmode, glui32 rock, bool unicode);

	/**
	 * Open a window stream
	 */
	WindowStream *openWindowStream(Window *window);

	/**
	 * Open a memory stream
	 */
	MemoryStream *openMemoryStream(void *buf, size_t buflen, FileMode mode, uint32 rock = 0, bool unicode = true);

	/**
	 * Delete a stream
	 */
	void deleteStream(Stream *stream) {
		delete stream;
	}

	/**
	 * Start an Iteration through streams
	 */
	Stream *getFirst(uint32 *rock);

	/**
	 * Set the current output stream
	 */
	void setCurrent(Stream *stream) {
		assert(stream->_writable);
		_currentStream = stream;
	}

	/**
	 * Gets the current output stream
	 */
	Stream *getCurrent() const { return _currentStream; }

	/**
	 * Prompt for a savegame to load or save, and populate a file reference from the result
	 */
	frefid_t createByPrompt(glui32 usage, FileMode fmode, glui32 rock);

	/**
	 * Create a new file reference
	 */
	frefid_t createRef(int slot, const Common::String &desc, glui32 usage, glui32 rock);

	/**
	 * Create a new file reference
	 */
	frefid_t createRef(const Common::String &filename, glui32 usage, glui32 rock);

	/**
	 * Create a new temporary file reference
	 */
	frefid_t createTemp(glui32 usage, glui32 rock);

	/**
	 * Create a new file reference from an old one
	 */
	frefid_t createFromRef(frefid_t fref, glui32 usage, glui32 rock);

	/**
	 * Delete a file reference
	 */
	void deleteRef(frefid_t fref);

	/**
	 * Iterates to the next file reference following the specified one,
	 * or the first if null is passed
	 */
	frefid_t iterate(frefid_t fref, glui32 *rock);
};

} // End of namespace Gargoyle

#endif
