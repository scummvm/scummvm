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

/* unzip.c -- IO on .zip files using zlib
   Version 0.15 beta, Mar 19th, 1998,

   Read unzip.h for more info
*/

/* unzip.h -- IO for uncompress .zip files using zlib
   Version 0.15 beta, Mar 19th, 1998,

   Copyright (C) 1998 Gilles Vollant

   This unzip package allow extract file from .ZIP file, compatible with PKZip 2.04g
     WinZip, InfoZip tools and compatible.
   Encryption and multi volume ZipFile (span) are not supported.
   Old compressions used by old PKZip 1.x are not supported

   THIS IS AN ALPHA VERSION. AT THIS STAGE OF DEVELOPPEMENT, SOMES API OR STRUCTURE
   CAN CHANGE IN FUTURE VERSION !!
   I WAIT FEEDBACK at mail info@winimage.com
   Visit also http://www.winimage.com/zLibDll/unzip.htm for evolution

   Condition of use and distribution are the same than zlib :

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.


*/
/* for more info about .ZIP format, see
      ftp://ftp.cdrom.com/pub/infozip/doc/appnote-970311-iz.zip
   PkWare has also a specification at :
      ftp://ftp.pkware.com/probdesc.zip */

#include "common/scummsys.h"

// Even when zlib is not linked in, we can still open ZIP archives and read
// uncompressed files from them.  Attempted decompression of compressed files
// will result in an error.
//
// Define the constants and types used by zlib.
#define Z_ERRNO -1
#define Z_OK 0
#define Z_DEFLATED 8
typedef void *voidp;
typedef unsigned int uInt;
typedef unsigned long uLong;
typedef long z_off_t;
typedef unsigned char Byte;
typedef Byte Bytef;

#include "common/crc.h"
#include "common/fs.h"
#include "common/compression/gzio.h"
#include "common/compression/unzip.h"
#include "common/memstream.h"

#include "common/hashmap.h"
#include "common/hash-str.h"

#if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void *) without cast */
typedef struct TagunzFile__ { int unused; } unzFile__;
typedef unzFile__ *unzFile;
#else
typedef voidp unzFile;
#endif

#define UNZ_OK                                  (0)
#define UNZ_END_OF_LIST_OF_FILE (-100)
#define UNZ_ERRNO               (Z_ERRNO)
#define UNZ_EOF                 (0)
#define UNZ_PARAMERROR                  (-102)
#define UNZ_BADZIPFILE                  (-103)
#define UNZ_INTERNALERROR               (-104)
#define UNZ_CRCERROR                    (-105)

/* tm_unz contain date/time info */
typedef struct {
       uInt tm_sec;            /* seconds after the minute - [0,59] */
       uInt tm_min;            /* minutes after the hour - [0,59] */
       uInt tm_hour;           /* hours since midnight - [0,23] */
       uInt tm_mday;           /* day of the month - [1,31] */
       uInt tm_mon;            /* months since January - [0,11] */
       uInt tm_year;           /* years - [1980..2044] */
} tm_unz;

/* unz_global_info structure contain global data about the ZIPfile
   These data comes from the end of central dir */
typedef struct {
	uLong number_entry;         /* total number of entries in
				       the central dir on this disk */
	uLong size_comment;         /* size of the global comment of the zipfile */
} unz_global_info;


/* unz_file_info contain information about a file in the zipfile */
typedef struct {
	uLong version;              /* version made by                 2 bytes */
	uLong version_needed;       /* version needed to extract       2 bytes */
	uLong flag;                 /* general purpose bit flag        2 bytes */
	uLong compression_method;   /* compression method              2 bytes */
	uLong dosDate;              /* last mod file date in Dos fmt   4 bytes */
	uLong crc;                  /* crc-32                          4 bytes */
	uLong compressed_size;      /* compressed size                 4 bytes */
	uLong uncompressed_size;    /* uncompressed size               4 bytes */
	uLong size_filename;        /* filename length                 2 bytes */
	uLong size_file_extra;      /* extra field length              2 bytes */
	uLong size_file_comment;    /* file comment length             2 bytes */

	uLong disk_num_start;       /* disk number start               2 bytes */
	uLong internal_fa;          /* internal file attributes        2 bytes */
	uLong external_fa;          /* external file attributes        4 bytes */
} unz_file_info;


/*
  Open a Zip file. path contain the full pathname (by example,
	 on a Windows NT computer "c:\\zlib\\zlib111.zip" or on an Unix computer
	 "zlib/zlib111.zip".
	 If the zipfile cannot be opened (file don't exist or in not valid), the
	   return value is NULL.
	 Else, the return value is a unzFile Handle, usable with other function
	   of this unzip package.
*/

int unzClose(unzFile file);
/*
  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
	these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
  return UNZ_OK if there is no problem. */


/***************************************************************************/
/* Unzip package allow you browse the directory of the zipfile */

int unzGoToFirstFile(unzFile file);
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/

int unzGoToNextFile(unzFile file);
/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/

int unzLocateFile(unzFile file, const char *szFileName, int iCaseSensitivity);
/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/


int unzGetCurrentFileInfo(unzFile file,
					     unz_file_info *pfile_info,
					     char *szFileName,
					     uLong fileNameBufferSize,
					     void *extraField,
					     uLong extraFieldBufferSize,
					     char *szComment,
					     uLong commentBufferSize);
/*
  Get Info about the current file
  if pfile_info!=NULL, the *pfile_info structure will contain somes info about
	    the current file
  if szFileName!=NULL, the filemane string will be copied in szFileName
			(fileNameBufferSize is the size of the buffer)
  if extraField!=NULL, the extra field information will be copied in extraField
			(extraFieldBufferSize is the size of the buffer).
			This is the Central-header version of the extra field
  if szComment!=NULL, the comment string of the file will be copied in szComment
			(commentBufferSize is the size of the buffer)
*/

/***************************************************************************/
/* for reading the content of the current zipfile, you can open it, read data
   from it, and close it (you can close it before reading all the file)
   */

Common::SharedArchiveContents unzOpenCurrentFile(unzFile file, const Common::CRC32& crc);
/*
  Open for reading data the current file in the zipfile.
  If there is no error, the return value is UNZ_OK.
*/

int unzCloseCurrentFile(unzFile file);
/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/


int unzReadCurrentFile(unzFile file, voidp buf, unsigned len);
/*
  Read bytes from the current file (opened by unzOpenCurrentFile)
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
	(UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/

int unzGetLocalExtrafield(unzFile file, voidp buf, unsigned len);
/*
  Read extra field from the current file (opened by unzOpenCurrentFile)
  This is the local-header version of the extra field (sometimes, there is
	more info in the local-header version than in the central-header)

  if buf==NULL, it return the size of the local extra field

  if buf!=NULL, len is the size of the buffer, the extra header is copied in
	buf.
  the return value is the number of bytes copied in buf, or (if <0)
	the error code
*/

#if !defined(unix) && !defined(CASESENSITIVITYDEFAULT_YES) && \
					  !defined(CASESENSITIVITYDEFAULT_NO)
#define CASESENSITIVITYDEFAULT_NO
#endif


#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#define UNZ_MAXFILENAMEINZIP (256)
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)


#if 0
const char unz_copyright[] =
   " unzip 0.15 Copyright 1998 Gilles Vollant ";
#endif

/* unz_file_info_interntal contain internal info about a file in zipfile*/
typedef struct {
	uLong offset_curfile;/* relative offset of local header 4 bytes */
} unz_file_info_internal;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
	when reading and decompress it */
typedef struct {
	uLong offset_local_extrafield;/* offset of the local extra field */
	uInt  size_local_extrafield;/* size of the local extra field */
	uLong pos_local_extrafield;   /* position in the local extra field in read*/

	Common::SeekableReadStream *_stream;                 /* io structore of the zipfile */
	uLong byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
} file_in_zip_read_info_s;

typedef struct {
	uLong num_file;					/* number of the current file in the zipfile*/
	uLong pos_in_central_dir;		/* pos of the current file in the central dir*/
	uLong current_file_ok;			/* flag about the usability of the current file*/
	unz_file_info cur_file_info;					/* public info about the current file in zip*/
	unz_file_info_internal cur_file_info_internal;	/* private info about it*/
} cached_file_in_zip;

typedef Common::HashMap<Common::String, cached_file_in_zip, Common::IgnoreCase_Hash,
	Common::IgnoreCase_EqualTo> ZipHash;

/* unz_s contain internal information about the zipfile
*/
typedef struct {
	Common::SeekableReadStream *_stream;				/* io structore of the zipfile */
	unz_global_info gi;				/* public global information */
	uLong byte_before_the_zipfile;	/* byte before the zipfile, (>0 for sfx)*/
	uLong num_file;					/* number of the current file in the zipfile*/
	uLong pos_in_central_dir;		/* pos of the current file in the central dir*/
	uLong current_file_ok;			/* flag about the usability of the current file*/
	uLong central_pos;				/* position of the beginning of the central dir*/

	uLong size_central_dir;			/* size of the central directory  */
	uLong offset_central_dir;		/* offset of start of central directory with
									respect to the starting disk number */

	unz_file_info cur_file_info;					/* public info about the current file in zip*/
	unz_file_info_internal cur_file_info_internal;	/* private info about it*/

	ZipHash _hash;
} unz_s;

/* ===========================================================================
	 Read a byte from a gz_stream; update next_in and avail_in. Return EOF
   for end of file.
   IN assertion: the stream s has been successfully opened for reading.
*/


/*static int unzlocal_getByte(Common::SeekableReadStream &fin, int *pi) {
	unsigned char c = fin.readByte();
	  *pi = (int)c;
		return UNZ_OK;
	} else {
		if (fin.err())
			return UNZ_ERRNO;
		else
			return UNZ_EOF;
	}
}*/


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
static int unzlocal_getShort(Common::SeekableReadStream *fin, uLong *pX) {
	*pX = fin->readUint16LE();
	return (fin->err() || fin->eos()) ? UNZ_ERRNO : UNZ_OK;
}

static int unzlocal_getLong(Common::SeekableReadStream *fin, uLong *pX) {
	*pX = fin->readUint32LE();
	return (fin->err() || fin->eos()) ? UNZ_ERRNO : UNZ_OK;
}

#define BUFREADCOMMENT (0x400)

/*
  Locate the Central directory of a zipfile (at the end, just before
	the global comment)
*/
static uLong unzlocal_SearchCentralDir(Common::SeekableReadStream &fin) {
	unsigned char *buf;
	uLong uSizeFile;
	uLong uBackRead;
	uLong uMaxBack=0xffff; /* maximum size of global comment */
	uLong uPosFound=0;

	uSizeFile = fin.size();
	if (fin.err())
		return 0;

	if (uMaxBack > uSizeFile)
		uMaxBack = uSizeFile;

	buf = (unsigned char *)malloc(BUFREADCOMMENT + 4);
	if (buf == nullptr)
		return 0;

	uBackRead = 4;
	while (uBackRead < uMaxBack) {
		uLong uReadSize,uReadPos;
		int i;
		if (uBackRead + BUFREADCOMMENT > uMaxBack)
			uBackRead = uMaxBack;
		else
			uBackRead += BUFREADCOMMENT;
		uReadPos = uSizeFile-uBackRead;

		uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
					 (BUFREADCOMMENT+4) : (uSizeFile-uReadPos);
		fin.seek(uReadPos, SEEK_SET);
		if (fin.err())
			break;

		if (fin.read(buf, (uInt)uReadSize) != uReadSize)
			break;

				for (i = (int)uReadSize - 3; (i--) > 0;)
			if (((*(buf + i)) == 0x50) && ((*(buf + i + 1)) == 0x4b) &&
				((*(buf + i + 2)) == 0x05) && ((*(buf + i + 3)) == 0x06)) {
				uPosFound = uReadPos + i;
				break;
			}

		if (uPosFound != 0)
			break;
	}
	free(buf);
	return uPosFound;
}

/*
  Open a Zip file. path contain the full pathname (by example,
	 on a Windows NT computer "c:\\test\\zlib109.zip" or on an Unix computer
	 "zlib/zlib109.zip".
	 If the zipfile cannot be opened (file don't exist or in not valid), the
	   return value is NULL.
	 Else, the return value is a unzFile Handle, usable with other function
	   of this unzip package.
*/
unzFile unzOpen(Common::SeekableReadStream *stream, bool flattenTree) {
	if (!stream)
		return nullptr;

	unz_s *us = new unz_s;
	uLong central_pos,uL;

	uLong number_disk;          /* number of the current dist, used for
								   spaning ZIP, unsupported, always 0*/
	uLong number_disk_with_CD;  /* number the disk with central dir, used
								   for spaning ZIP, unsupported, always 0*/
	uLong number_entry_CD;      /* total number of entries in
	                               the central dir
	                               (same than number_entry on nospan) */

	int err = UNZ_OK;

	us->_stream = stream;

	central_pos = unzlocal_SearchCentralDir(*us->_stream);
	if (central_pos == 0)
		err = UNZ_ERRNO;

	us->_stream->seek(central_pos, SEEK_SET);
	if (us->_stream->err())
		err = UNZ_ERRNO;

	/* the signature, already checked */
	if (unzlocal_getLong(us->_stream, &uL) != UNZ_OK)
		err = UNZ_ERRNO;

	/* number of this disk */
	if (unzlocal_getShort(us->_stream, &number_disk) != UNZ_OK)
		err = UNZ_ERRNO;

	/* number of the disk with the start of the central directory */
	if (unzlocal_getShort(us->_stream, &number_disk_with_CD) != UNZ_OK)
		err = UNZ_ERRNO;

	/* total number of entries in the central dir on this disk */
	if (unzlocal_getShort(us->_stream, &us->gi.number_entry) != UNZ_OK)
		err = UNZ_ERRNO;

	/* total number of entries in the central dir */
	if (unzlocal_getShort(us->_stream, &number_entry_CD) != UNZ_OK)
		err = UNZ_ERRNO;

	if ((number_entry_CD != us->gi.number_entry) ||
	    (number_disk_with_CD != 0) ||
	    (number_disk != 0))
		err = UNZ_BADZIPFILE;

	/* size of the central directory */
	if (unzlocal_getLong(us->_stream, &us->size_central_dir) != UNZ_OK)
		err = UNZ_ERRNO;

	/* offset of start of central directory with respect to the
	      starting disk number */
	if (unzlocal_getLong(us->_stream, &us->offset_central_dir) != UNZ_OK)
		err = UNZ_ERRNO;

	/* zipfile comment length */
	if (unzlocal_getShort(us->_stream, &us->gi.size_comment) != UNZ_OK)
		err = UNZ_ERRNO;

	if ((central_pos < us->offset_central_dir + us->size_central_dir) && (err == UNZ_OK))
		err = UNZ_BADZIPFILE;

	if (err != UNZ_OK) {
		delete us->_stream;
		delete us;
		return nullptr;
	}

	us->byte_before_the_zipfile = central_pos -
		                    (us->offset_central_dir + us->size_central_dir);
	us->central_pos = central_pos;

	err = unzGoToFirstFile((unzFile)us);

	while (err == UNZ_OK) {
		// Get the file details
		char szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
		unzGetCurrentFileInfo(us, nullptr, szCurrentFileName, sizeof(szCurrentFileName) - 1,
							nullptr, 0, nullptr, 0);

		// Save details into the hash
		cached_file_in_zip fe;
		fe.num_file = us->num_file;
		fe.pos_in_central_dir = us->pos_in_central_dir;
		fe.current_file_ok = us->current_file_ok;
		fe.cur_file_info = us->cur_file_info;
		fe.cur_file_info_internal = us->cur_file_info_internal;

		const char *name = szCurrentFileName;

		if (flattenTree)
			for (const char *p = szCurrentFileName; *p; p++)
				if (*p == '\\' || *p == '/')
					name = p + 1;

		us->_hash[Common::String(name)] = fe;

		// Move to the next file
		err = unzGoToNextFile((unzFile)us);
	}
	return (unzFile)us;
}


/*
  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unzipOpenCurrentFile (see later),
	these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
  return UNZ_OK if there is no problem. */
int unzClose(unzFile file) {
	unz_s *s;
	if (file == nullptr)
		return UNZ_PARAMERROR;
	s = (unz_s *)file;

	delete s->_stream;
	delete s;
	return UNZ_OK;
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
static int unzlocal_GetCurrentFileInfoInternal(unzFile file,
												  unz_file_info *pfile_info,
												  unz_file_info_internal
												  *pfile_info_internal,
												  char *szFileName,
												  uLong fileNameBufferSize,
												  void *extraField,
												  uLong extraFieldBufferSize,
												  char *szComment,
												  uLong commentBufferSize);

static int unzlocal_GetCurrentFileInfoInternal(unzFile file,
											  unz_file_info *pfile_info,
											  unz_file_info_internal *pfile_info_internal,
											  char *szFileName, uLong fileNameBufferSize,
											  void *extraField, uLong extraFieldBufferSize,
											  char *szComment,  uLong commentBufferSize) {
	unz_s *s;
	unz_file_info file_info;
	unz_file_info_internal file_info_internal;
	int err = UNZ_OK;
	uLong uMagic;
	long lSeek = 0;

	if (file == nullptr)
		return UNZ_PARAMERROR;
	s = (unz_s *)file;
	s->_stream->seek(s->pos_in_central_dir + s->byte_before_the_zipfile, SEEK_SET);
	if (s->_stream->err())
		err = UNZ_ERRNO;


	/* we check the magic */
	if (err == UNZ_OK) {
		if (unzlocal_getLong(s->_stream, &uMagic) != UNZ_OK)
			err = UNZ_ERRNO;
		else if (uMagic != 0x02014b50)
			err = UNZ_BADZIPFILE;
	}

	if (unzlocal_getShort(s->_stream, &file_info.version) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getShort(s->_stream, &file_info.version_needed) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getShort(s->_stream, &file_info.flag) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getShort(s->_stream, &file_info.compression_method) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getLong(s->_stream, &file_info.dosDate) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getLong(s->_stream, &file_info.crc) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getLong(s->_stream, &file_info.compressed_size) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getLong(s->_stream, &file_info.uncompressed_size) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getShort(s->_stream, &file_info.size_filename) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getShort(s->_stream, &file_info.size_file_extra) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getShort(s->_stream, &file_info.size_file_comment) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getShort(s->_stream, &file_info.disk_num_start) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getShort(s->_stream, &file_info.internal_fa) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getLong(s->_stream, &file_info.external_fa) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getLong(s->_stream, &file_info_internal.offset_curfile) != UNZ_OK)
		err = UNZ_ERRNO;

	lSeek += file_info.size_filename;
	if ((err == UNZ_OK) && (szFileName != nullptr)) {
		uLong uSizeRead;
		if (file_info.size_filename < fileNameBufferSize) {
			*(szFileName + file_info.size_filename) = '\0';
			uSizeRead = file_info.size_filename;
		} else
			uSizeRead = fileNameBufferSize;

		if ((file_info.size_filename > 0) && (fileNameBufferSize > 0))
			if (s->_stream->read(szFileName, (uInt)uSizeRead) != uSizeRead)
				err = UNZ_ERRNO;
		lSeek -= uSizeRead;
	}


	if ((err == UNZ_OK) && (extraField != nullptr)) {
		uLong uSizeRead;
		if (file_info.size_file_extra<extraFieldBufferSize)
			uSizeRead = file_info.size_file_extra;
		else
			uSizeRead = extraFieldBufferSize;

		if (lSeek != 0) {
			s->_stream->seek(lSeek, SEEK_CUR);
			if (s->_stream->err())
				lSeek=0;
			else
				err = UNZ_ERRNO;
		}
		if ((file_info.size_file_extra > 0) && (extraFieldBufferSize > 0))
			if (s->_stream->read(extraField, (uInt)uSizeRead) != uSizeRead)
				err = UNZ_ERRNO;
		lSeek += file_info.size_file_extra - uSizeRead;
	} else
		lSeek += file_info.size_file_extra;


	if ((err == UNZ_OK) && (szComment != nullptr)) {
		uLong uSizeRead;
		if (file_info.size_file_comment<commentBufferSize) {
			*(szComment + file_info.size_file_comment) = '\0';
			uSizeRead = file_info.size_file_comment;
		} else
			uSizeRead = commentBufferSize;

		if (lSeek!=0) {
			s->_stream->seek(lSeek, SEEK_CUR);
			if (s->_stream->err())
				lSeek = 0;
			else
				err = UNZ_ERRNO;
		}
		if ((file_info.size_file_comment>0) && (commentBufferSize > 0))
			if (s->_stream->read(szComment, (uInt)uSizeRead) != uSizeRead)
				err = UNZ_ERRNO;
		lSeek += file_info.size_file_comment - uSizeRead;
	} else
		lSeek += file_info.size_file_comment;

	if ((err == UNZ_OK) && (pfile_info != nullptr))
		*pfile_info = file_info;

	if ((err == UNZ_OK) && (pfile_info_internal != nullptr))
		*pfile_info_internal = file_info_internal;

	return err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
int unzGetCurrentFileInfo(unzFile file,
												  unz_file_info *pfile_info,
												  char *szFileName, uLong fileNameBufferSize,
												  void *extraField, uLong extraFieldBufferSize,
												  char *szComment,  uLong commentBufferSize) {
	return unzlocal_GetCurrentFileInfoInternal(file,pfile_info,nullptr,
												szFileName,fileNameBufferSize,
												extraField,extraFieldBufferSize,
												szComment,commentBufferSize);
}

/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
int unzGoToFirstFile(unzFile file) {
	int err = UNZ_OK;
	unz_s *s;
	if (file == nullptr)
		return UNZ_PARAMERROR;
	s = (unz_s *)file;
	s->pos_in_central_dir=s->offset_central_dir;
	s->num_file = 0;
	err = unzlocal_GetCurrentFileInfoInternal(file , &s->cur_file_info,
											 &s->cur_file_info_internal,
											 nullptr, 0, nullptr, 0, nullptr, 0);
	s->current_file_ok = (err == UNZ_OK);
	return err;
}


/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
int unzGoToNextFile(unzFile file) {
	unz_s *s;
	int err;

	if (file == nullptr)
		return UNZ_PARAMERROR;
	s = (unz_s *)file;
	if (!s->current_file_ok)
		return UNZ_END_OF_LIST_OF_FILE;
	if (s->num_file + 1 == s->gi.number_entry)
		return UNZ_END_OF_LIST_OF_FILE;

	s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename +
			s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment;
	s->num_file++;
	err = unzlocal_GetCurrentFileInfoInternal(file, &s->cur_file_info,
											   &s->cur_file_info_internal,
											   nullptr, 0, nullptr, 0, nullptr, 0);
	s->current_file_ok = (err == UNZ_OK);
	return err;
}

/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzipStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
int unzLocateFile(unzFile file, const char *szFileName, int iCaseSensitivity) {
	unz_s *s;

	if (file == nullptr)
		return UNZ_PARAMERROR;

	if (strlen(szFileName) >= UNZ_MAXFILENAMEINZIP)
		return UNZ_PARAMERROR;

	s=(unz_s *)file;
	if (!s->current_file_ok)
		return UNZ_END_OF_LIST_OF_FILE;

	// Check to see if the entry exists
	ZipHash::iterator i = s->_hash.find(Common::String(szFileName));
	if (i == s->_hash.end())
		return UNZ_END_OF_LIST_OF_FILE;

	// Found it, so reset the details in the main structure
	cached_file_in_zip &fe = i->_value;
	s->num_file = fe.num_file;
	s->pos_in_central_dir = fe.pos_in_central_dir;
	s->current_file_ok = fe.current_file_ok;
	s->cur_file_info = fe.cur_file_info;
	s->cur_file_info_internal = fe.cur_file_info_internal;

	return UNZ_OK;
}


/*
  Read the local header of the current zipfile
  Check the coherency of the local header and info in the end of central
		directory about this file
  store in *piSizeVar the size of extra info in local header
		(filename and size of extra field data)
*/
static int unzlocal_CheckCurrentFileCoherencyHeader(unz_s *s, uInt *piSizeVar,
													uLong *poffset_local_extrafield,
													uInt  *psize_local_extrafield) {
	uLong uMagic,uData,uFlags;
	uLong size_filename;
	uLong size_extra_field;
	int err = UNZ_OK;

	*piSizeVar = 0;
	*poffset_local_extrafield = 0;
	*psize_local_extrafield = 0;

	s->_stream->seek(s->cur_file_info_internal.offset_curfile +
								s->byte_before_the_zipfile, SEEK_SET);
	if (s->_stream->err())
		return UNZ_ERRNO;


	if (err == UNZ_OK) {
		if (unzlocal_getLong(s->_stream, &uMagic) != UNZ_OK)
			err = UNZ_ERRNO;
		else if (uMagic != 0x04034b50)
			err = UNZ_BADZIPFILE;
	}

	if (unzlocal_getShort(s->_stream, &uData) != UNZ_OK)
		err = UNZ_ERRNO;
/*
	else if ((err == UNZ_OK) && (uData!=s->cur_file_info.wVersion))
		err = UNZ_BADZIPFILE;
*/
	if (unzlocal_getShort(s->_stream, &uFlags) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getShort(s->_stream, &uData) != UNZ_OK)
		err = UNZ_ERRNO;
	else if ((err == UNZ_OK) && (uData != s->cur_file_info.compression_method))
		err = UNZ_BADZIPFILE;

	if ((err == UNZ_OK) && (s->cur_file_info.compression_method != 0) &&
	                     (s->cur_file_info.compression_method != Z_DEFLATED))
		err = UNZ_BADZIPFILE;

	if (unzlocal_getLong(s->_stream, &uData) != UNZ_OK) /* date/time */
		err = UNZ_ERRNO;

	if (unzlocal_getLong(s->_stream, &uData) != UNZ_OK) /* crc */
		err = UNZ_ERRNO;
	else if ((err == UNZ_OK) && (uData!=s->cur_file_info.crc) &&
		                      ((uFlags & 8) == 0))
		err = UNZ_BADZIPFILE;

	if (unzlocal_getLong(s->_stream, &uData) != UNZ_OK) /* size compr */
		err = UNZ_ERRNO;
	else if ((err == UNZ_OK) && (uData!=s->cur_file_info.compressed_size) &&
							  ((uFlags & 8) == 0))
		err = UNZ_BADZIPFILE;

	if (unzlocal_getLong(s->_stream, &uData) != UNZ_OK) /* size uncompr */
		err = UNZ_ERRNO;
	else if ((err == UNZ_OK) && (uData!=s->cur_file_info.uncompressed_size) &&
							  ((uFlags & 8) == 0))
		err = UNZ_BADZIPFILE;


	if (unzlocal_getShort(s->_stream, &size_filename) != UNZ_OK)
		err = UNZ_ERRNO;
	else if ((err == UNZ_OK) && (size_filename!=s->cur_file_info.size_filename))
		err = UNZ_BADZIPFILE;

	*piSizeVar += (uInt)size_filename;

	if (unzlocal_getShort(s->_stream, &size_extra_field) != UNZ_OK)
		err = UNZ_ERRNO;
	*poffset_local_extrafield = s->cur_file_info_internal.offset_curfile +
									SIZEZIPLOCALHEADER + size_filename;
	*psize_local_extrafield = (uInt)size_extra_field;

	*piSizeVar += (uInt)size_extra_field;

	return err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
Common::SharedArchiveContents unzOpenCurrentFile (unzFile file, const Common::CRC32 &crc) {
	uInt iSizeVar;
	unz_s *s;
	uLong offset_local_extrafield;  /* offset of the local extra field */
	uInt  size_local_extrafield;    /* size of the local extra field */

	if (file == nullptr)
		return Common::SharedArchiveContents();
	s = (unz_s *)file;
	if (!s->current_file_ok)
		return Common::SharedArchiveContents();

	if (unzlocal_CheckCurrentFileCoherencyHeader(s, &iSizeVar,
				&offset_local_extrafield, &size_local_extrafield) != UNZ_OK)
		return Common::SharedArchiveContents();

	if (s->cur_file_info.compression_method != 0 && s->cur_file_info.compression_method != Z_DEFLATED) {
		warning("Unknown compression algoritthm %d", (int)s->cur_file_info.compression_method);
		return Common::SharedArchiveContents();
	}

	uint32 crc32_wait = s->cur_file_info.crc;

	byte *compressedBuffer = new byte[s->cur_file_info.compressed_size];
	s->_stream->seek(s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + iSizeVar);
	s->_stream->read(compressedBuffer, s->cur_file_info.compressed_size);
	byte *uncompressedBuffer = nullptr;

	switch (s->cur_file_info.compression_method) {
	case 0: // Store
		uncompressedBuffer = compressedBuffer;
		break;
	case Z_DEFLATED:
		uncompressedBuffer = new byte[s->cur_file_info.uncompressed_size];
		assert(s->cur_file_info.uncompressed_size == 0 || uncompressedBuffer != nullptr);
		Common::GzioReadStream::deflateDecompress(uncompressedBuffer, s->cur_file_info.uncompressed_size, compressedBuffer, s->cur_file_info.compressed_size);
		delete[] compressedBuffer;
		compressedBuffer = nullptr;
		break;
	default:
		warning("Unknown compression algoritthm %d", (int)s->cur_file_info.compression_method);
		delete[] compressedBuffer;
		return Common::SharedArchiveContents();
	}

	uint32 crc32_data = crc.crcFast(uncompressedBuffer, s->cur_file_info.uncompressed_size);
	if (crc32_data != crc32_wait) {
		delete[] uncompressedBuffer;
		warning("CRC32 mismatch: %08x, %08x", crc32_data, crc32_wait);
		return Common::SharedArchiveContents();
	}

	return Common::SharedArchiveContents(uncompressedBuffer, s->cur_file_info.uncompressed_size);
}


namespace Common {


class ZipArchive : public MemcachingCaseInsensitiveArchive {
	unzFile _zipFile;
	Common::CRC32 _crc;
	bool _flattenTree;

public:
	ZipArchive(unzFile zipFile, bool flattenTree);


	~ZipArchive();

	bool hasFile(const Path &path) const override;
	int listMembers(ArchiveMemberList &list) const override;
	const ArchiveMemberPtr getMember(const Path &path) const override;
	Common::SharedArchiveContents readContentsForPath(const Common::String& translated) const override;
	Common::String translatePath(const Common::Path &path) const override {
		return _flattenTree ? path.getLastComponent().toString() : path.toString();
	}
};

/*
class ZipArchiveMember : public ArchiveMember {
	unzFile _zipFile;

public:
	ZipArchiveMember(FSNode &node) : _node(node) {
	}

	String getName() const {
		...
	}

	SeekableReadStream *open() {
		...
	}
};
*/

ZipArchive::ZipArchive(unzFile zipFile, bool flattenTree) : _zipFile(zipFile), _crc(), _flattenTree(flattenTree) {
	assert(_zipFile);
}

ZipArchive::~ZipArchive() {
	unzClose(_zipFile);
}

bool ZipArchive::hasFile(const Path &path) const {
	String name = path.toString();
	return (unzLocateFile(_zipFile, name.c_str(), 2) == UNZ_OK);
}

int ZipArchive::listMembers(ArchiveMemberList &list) const {
	int members = 0;

	const unz_s *const archive = (const unz_s *)_zipFile;
	for (ZipHash::const_iterator i = archive->_hash.begin(), end = archive->_hash.end();
	     i != end; ++i) {
		list.push_back(ArchiveMemberList::value_type(new GenericArchiveMember(i->_key, *this)));
		++members;
	}

	return members;
}

const ArchiveMemberPtr ZipArchive::getMember(const Path &path) const {
	String name = path.toString();
	if (!hasFile(name))
		return ArchiveMemberPtr();

	return ArchiveMemberPtr(new GenericArchiveMember(path, *this));
}

Common::SharedArchiveContents ZipArchive::readContentsForPath(const Common::String& name) const {
	if (unzLocateFile(_zipFile, name.c_str(), 2) != UNZ_OK)
		return Common::SharedArchiveContents();

	return unzOpenCurrentFile(_zipFile, _crc);
}

Archive *makeZipArchive(const String &name, bool flattenTree) {
	return makeZipArchive(SearchMan.createReadStreamForMember(name), flattenTree);
}

Archive *makeZipArchive(const FSNode &node, bool flattenTree) {
	return makeZipArchive(node.createReadStream(), flattenTree);
}

Archive *makeZipArchive(SeekableReadStream *stream, bool flattenTree) {
	if (!stream)
		return nullptr;
	unzFile zipFile = unzOpen(stream, flattenTree);
	if (!zipFile) {
		// stream gets deleted by unzOpen() call if something
		// goes wrong.
		return nullptr;
	}
	return new ZipArchive(zipFile, flattenTree);
}

} // End of namespace Common
