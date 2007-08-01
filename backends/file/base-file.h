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
 * $URL: $
 * $Id: $
 */

#ifndef BACKENDS_BASE_FILE_H
#define BACKENDS_BASE_FILE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"

using namespace Common;

/**
 * Implements several file related functions used by the Common::File wrapper.
 */
class BaseFile : public Common::SeekableReadStream {
protected:
	/** File handle to the actual file; 0 if no file is open. */
	void *_handle;

	/** Status flag which tells about recent I/O failures. */
	bool _ioFailed;

	/** The name of this file, for debugging. */
	String _name;
	
	/**
	 * The following functions are meant to be redefined by subclasses if needed. E.g. ps2-file.h or ds-file.h
	 * They behave as the C++ standard I/O methods so refer to the standard documentation for usage.
	 * 
	 * This design was inspired on the Template pattern.
	 */
	void _clearerr(FILE *stream);
	int _fclose(FILE *stream);
	int _feof(FILE *stream) const;
	FILE *_fopen(const char * filename, const char * mode);
	int _fread(void *buffer, size_t obj_size, size_t num, FILE *stream);
	int _fseek(FILE * stream, long int offset, int origin) const;
	long _ftell(FILE *stream) const;
	int _fwrite(const void * ptr, size_t obj_size, size_t count, FILE * stream);

private:
	// Disallow copying BaseFile objects. There is not strict reason for this,
	// except that so far we never had real need for such a feature, and
	// code that accidentally copied File objects tended to break in strange
	// ways.
	BaseFile(const BaseFile &f);
	BaseFile &operator =(const BaseFile &f);
	
public:
	enum AccessMode {
		kFileReadMode = 1,
		kFileWriteMode = 2
	};
	
	BaseFile();
	virtual ~BaseFile();
	
	/**
	 * Clears the flag for the last ocurred IO failure.
	 */
	void clearIOFailed();
	
	/**
	 * Closes the file handle.
	 */
	virtual void close();
	
	/**
	 * Checks for end of file.
	 *
	 * @return: true if the end of file is reached, false otherwise.
	 */
	bool eof() const;
	
	/**
	 * Checks for the end of the stream. In this case it's equivalent to eof().
	 * 
	 * @return: true if the end of the stream is reached, false otherwise.
	 */
	bool eos() const { return eof(); }
	
	/**
	 * Checks whether the last IO operation failed.
	 * 
	 * @return: true if the last IO operation failed, false otherwise.
	 */
	bool ioFailed() const;
	
	/**
	 * Obtain the filename of the opened file.
	 *
	 * @return: the filename of the opened file.
	 */
	const char *name() const { return _name.c_str(); }
	
	/**
	 * Checks if the object opened a file successfully.
	 *
	 * @return: true if any file is opened, false otherwise.
	 */
	bool isOpen() const;
	
	/**
	 * Opens a given file.
	 * 
	 * @param filename Path of the file to be opened.
	 * @param mode Mode to open to file. Read or write.
	 */
	virtual bool open(const String &filename, AccessMode mode = kFileReadMode);
	
	/**
	 * Obtain the position of the seek pointer.
	 * 
	 * @return The position of the seek pointer within the file.
	 */
	uint32 pos() const;
	
	/**
	 * Read a chunk of data from the file.
	 * 
	 * @param dataPtr Buffer to the place the read contents.
	 * @param dataSize Amount of bytes to read from the file.
	 * @return Amount of read bytes.
	 */
	uint32 read(void *dataPtr, uint32 dataSize);
	
	/**
	 * Remove a given file from the filesystem.
	 * 
	 * @param filename Path to the file to be removed.
	 * @return true if the file was removed succesfully, false otherwise.
	 */
	virtual bool remove(const String &filename);
	
	/**
	 * Move the seek pointer within the file.
	 * 
	 * @param offs Amount of bytes to move the pointer within the file.
	 * @param whence Starting point of the seek cursor.
	 */
	void seek(int32 offs, int whence = SEEK_SET);
	
	/**
	 * Obtain the size of the file.
	 * 
	 * @return The size of the file in bytes.
	 */
	uint32 size() const;
	
	//TODO: Remove the write functions? Also remove the enum then
	/**
	 * Write a chunk of data to the file.
	 */
	//uint32 write(const void *dataPtr, uint32 dataSize);	
};

#endif	//BACKENDS_BASE_FILE_H
