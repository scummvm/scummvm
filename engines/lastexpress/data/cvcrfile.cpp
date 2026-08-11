#include "cvcrfile.h"
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

#include "lastexpress/lastexpress.h"

#include "common/file.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/stream.h"

#include "engines/metaengine.h"

namespace LastExpress {

CVCRFile::CVCRFile(LastExpressEngine *engine) {
	_engine = engine;
	_filename = "";
	_virtualFileIsOpen = false;
	_virtualFileSize = 0;
	_rwStreamSize = 0;
	memset(_rleDstBuf, 0, sizeof(_rleDstBuf));
	_rleBufferPosition = -1;
	_rleByte = 0;
	_rleCount = 0;
	_rleMode = 0;
	_bytesProcessed = 0;
	_rleStatus = 0;
	_fileMode = 0;
}

CVCRFile::~CVCRFile() {
	close();
}

bool CVCRFile::open(const char *filename, int mode) {
	_filename = filename;
	_rleBufferPosition = -1;

	switch (mode) {
	case CVCRMODE_WB:
		debug(6, "CVCRFile::open(): Opening file \'%s\' on mode 0 (\'wb\')", filename);
		break;
	case CVCRMODE_RB:
		debug(6, "CVCRFile::open(): Opening file \'%s\' on mode 0 (\'rb\')", filename);
		break;
	case CVCRMODE_RWB:
		debug(6, "CVCRFile::open(): Opening file \'%s\' on mode 0 (\'r+b\')", filename);
		break;
	default:
		error("CVCRFile::open(): Invalid file mode");
	}

	Common::OutSaveFile *outSaveFile = nullptr;
	Common::InSaveFile *inSaveFile = nullptr;

	if (mode == CVCRMODE_WB) {
		outSaveFile = g_engine->getSaveFileManager()->openForSaving(_engine->getTargetName() + "-" + Common::String(_filename), false);
		_virtualFileIsOpen = outSaveFile != nullptr;

		if (outSaveFile)
			_virtualFileSize = (uint32)outSaveFile->size();
	} else {
		inSaveFile = g_engine->getSaveFileManager()->openForLoading(_engine->getTargetName() + "-" + Common::String(_filename));
		_virtualFileIsOpen = inSaveFile != nullptr;

		if (inSaveFile)
			_virtualFileSize = (uint32)inSaveFile->size();
	}

	_fileMode = mode;

	if (!_virtualFileIsOpen) {
		error("Error opening file \"%s\".  It probably doesn\'t exist or is write protected.", _filename.c_str());
	} else {
		// If this file was opened on 'r+b' mode, allocate at least 16kb
		uint32 fileSize = ((_fileMode == CVCRMODE_RWB || _fileMode == CVCRMODE_WB) && _virtualFileSize < 16 * 1024) ? 16 * 1024 : _virtualFileSize;

		if (inSaveFile && fileSize) {
			byte *fileContent = (byte *)malloc(fileSize);
			if (!fileContent) {
				error("Out of memory");
			}

			inSaveFile->read(fileContent, fileSize);

			_rwStream = new Common::MemorySeekableReadWriteStream(fileContent, fileSize, DisposeAfterUse::YES);
			_rwStreamSize = fileSize;

			delete inSaveFile;
		}

		if (outSaveFile && fileSize) {
			byte *fileContent = (byte *)malloc(fileSize);
			if (!fileContent) {
				error("Out of memory");
			}

			memset(fileContent, 0, fileSize);

			_rwStream = new Common::MemorySeekableReadWriteStream(fileContent, fileSize, DisposeAfterUse::YES);
			_rwStreamSize = fileSize;

			outSaveFile->finalize();
			delete outSaveFile;
		}
	}

	return _virtualFileIsOpen;
}

uint32 CVCRFile::read(void *dstBuf, uint32 elementSize, uint32 elementCount, bool forceSimpleReadOnRle, bool checkSize) {
	uint32 readAmount;

	if ((forceSimpleReadOnRle || !_rleStatus) && _virtualFileIsOpen) {
		readAmount = _rwStream->read(dstBuf, elementSize * elementCount);
		if (checkSize) {
			if (readAmount != elementSize * elementCount) {
				error("Error reading from file \"%s\"", _filename.c_str());
			}
		}

		return readAmount / elementSize;
	} else {
		error("Error reading from file \"%s\"", _filename.c_str());
		return 0;
	}
}

uint32 CVCRFile::write(void *srcBuf, uint32 elementSize, uint32 elementCount, bool forceSimpleWriteOnRle) {
	uint32 wroteAmount;

	if ((forceSimpleWriteOnRle || !_rleStatus) && _virtualFileIsOpen) {
		// Check if we have to allocate a bigger stream...
		uint32 amountToWrite = elementSize * elementCount;
		int64 atWhichPos = _rwStream->pos();
		uint32 oldSize = _rwStreamSize;
		bool haveToRaiseStreamSize = amountToWrite + atWhichPos > _rwStreamSize;

		if (haveToRaiseStreamSize) {
			while (amountToWrite + atWhichPos > _rwStreamSize) {
				_rwStreamSize += 16 * 1024; // Add another 16 kb until we can fit the new data...
			}

			// Allocate the new and bigger buffer and create a new stream with it...
			byte *newBuffer = (byte *)malloc(_rwStreamSize);
			if (!newBuffer) {
				error("Out of memory");
			}

			Common::MemorySeekableReadWriteStream *newStream = new Common::MemorySeekableReadWriteStream(newBuffer, _rwStreamSize, DisposeAfterUse::YES);

			// Create a temporary buffer in which we will pour the old data...
			byte *tmp = (byte *)malloc(oldSize);
			if (!tmp) {
				error("Out of memory");
			}

			// Read the old data from the old stream to the temporary buffer...
			_rwStream->seek(0, SEEK_SET);
			_rwStream->read(tmp, oldSize);

			// Delete the old stream...
			_rwStream->finalize();
			delete _rwStream;

			// Update the pointer for _rwStream with the new one...
			_rwStream = newStream;
			_rwStream->write(tmp, oldSize);
			_rwStream->seek(atWhichPos, SEEK_SET);

			// Clean-up...
			free(tmp);
		}

		wroteAmount = _rwStream->write(srcBuf, amountToWrite);
		if (wroteAmount != amountToWrite) {
			error("Error writing to file \"%s\".  Your disk is probably full.", _filename.c_str());
		}

		// Update virtual file size...
		_virtualFileSize = _virtualFileSize < (amountToWrite + atWhichPos) ? (amountToWrite + atWhichPos) : _virtualFileSize;

		return wroteAmount / elementSize;
	} else {
		error("Error writing to file \"%s\".  Your disk is probably full.", _filename.c_str());
		return 0;
	}
}

uint32 CVCRFile::readRLE(void *dstBuf, uint32 elementSize, uint32 elementCount) {
	uint8 curRleOp;
	uint8 *value = (uint8 *)dstBuf;

	uint32 remaining = elementSize * elementCount;
	if (_rleStatus == 2) {
		error("Error reading from file \"%s\"", _filename.c_str());
		return 0;
	}

	_rleStatus = 1;

	if (remaining != 0) {
		while (true) {
			if (_rleMode >= 2) {
				if (_rleMode == 2) {
					*value = _rleByte;
					value++;
					_rleCount--;
					if (!_rleCount)
						_rleMode = 1;
				}

				if (!--remaining)
					return elementCount;

				continue;
			}

			if (_rleBufferPosition == -1 || _rleBufferPosition >= 256) {
				read(_rleDstBuf, 1, 256, true, false);
				_rleBufferPosition = 0;
			}

			curRleOp = _rleDstBuf[_rleBufferPosition];
			_rleBufferPosition++;
			switch (curRleOp) {
			case 0xFE:
				if (_rleBufferPosition == -1 || _rleBufferPosition >= 256) {
					read(_rleDstBuf, 1, 256, true, false);
					_rleBufferPosition = 0;
				}

				curRleOp = _rleDstBuf[_rleBufferPosition];
				_rleBufferPosition++;

				*value = curRleOp;
				value++;

				break;
			case 0xFF:
				if (_rleBufferPosition == -1 || _rleBufferPosition >= 256) {
					read(_rleDstBuf, 1, 256, true, false);
					_rleBufferPosition = 0;
				}

				_rleCount = _rleDstBuf[_rleBufferPosition] - 1;
				_rleBufferPosition++;

				if (_rleBufferPosition == -1 || _rleBufferPosition >= 256) {
					read(_rleDstBuf, 1, 256, true, false);
					_rleBufferPosition = 0;
				}

				_rleByte = _rleDstBuf[_rleBufferPosition];
				_rleBufferPosition++;

				*value = _rleByte;
				value++;

				_rleMode = 2;
				break;

			case 0xFB:
				_rleCount = 2;
				_rleByte = 0;

				*value = 0;
				value++;

				_rleMode = 2;
				break;

			case 0xFD:
				if (_rleBufferPosition == -1 || _rleBufferPosition >= 256) {
					read(_rleDstBuf, 1, 256, 1, 0);
					_rleBufferPosition = 0;
				}

				_rleByte = 0;
				_rleCount = _rleDstBuf[_rleBufferPosition] - 1;
				_rleBufferPosition++;

				*value = 0;
				value++;

				_rleMode = 2;
				break;

			case 0xFC:
				_rleCount = 0xFE;
				_rleByte = 0;

				*value = 0;
				value++;

				_rleMode = 2;
				break;

			default:
				*value = curRleOp;
				value++;
				break;
			}

			if (!--remaining)
				return elementCount;
		} // while
	}

	return elementCount;
}

uint32 CVCRFile::writeRLE(void *srcBuf, uint32 elementSize, uint32 elementCount) {
	uint8 *value = (uint8 *)srcBuf;
	uint32 remaining = elementSize * elementCount;

	if (_rleStatus == 1) {
		error("Error writing to file \"%s\".  Your disk is probably full.", _filename.c_str());
		return 0;
	}

	for (_rleStatus = 2; remaining; --remaining) {
		switch (_rleMode) {
		case 0:
			_rleMode = 1;
			_rleByte = *value;
			value++;
			break;

		case 1:
			if (*value == _rleByte) {
				_rleMode = 2;
				_rleCount = 2;
				value++;
			} else {
				if (_rleBufferPosition == -1)
					_rleBufferPosition = 0;

				if (_rleBufferPosition == 256) {
					_rleBufferPosition = 0;
					write(_rleDstBuf, 256, 1, true);
				}

				if (_rleByte < 0xFB) {
					_rleDstBuf[_rleBufferPosition] = _rleByte;
				} else {
					_rleDstBuf[_rleBufferPosition] = 0xFE;
				}

				_bytesProcessed++;
				_rleBufferPosition++;

				if (_rleByte >= 0xFB) {
					if (_rleBufferPosition == 256) {
						_rleBufferPosition = 0;
						write(_rleDstBuf, 256, 1, true);
					}

					_rleDstBuf[_rleBufferPosition] = _rleByte;
					_rleBufferPosition++;
					_bytesProcessed++;
				}

				_rleByte = *value;
				value++;
			}

			break;

		case 2:
			if (*value != _rleByte) {
				if (_rleCount != 3 || _rleByte) {
					if (_rleCount == 0xFF) {

						if (_rleByte) {
							writeToRLEBuffer(0xFF, 1);
							writeToRLEBuffer(_rleCount, 1);
							writeToRLEBuffer(_rleByte, 1);
							_rleMode = 1;
							_rleByte = *value;
							value++;
							continue;
						}

						writeToRLEBuffer(0xFC, 1);
					} else {
						if (_rleByte) {
							writeToRLEBuffer(0xFF, 1);
							writeToRLEBuffer(_rleCount, 1);
							writeToRLEBuffer(_rleByte, 1);
							_rleMode = 1;
							_rleByte = *value;
							value++;
							continue;
						}

						writeToRLEBuffer(0xFD, 1);
						writeToRLEBuffer(_rleCount, 1);
					}
				} else {
					if (_rleBufferPosition == -1)
						_rleBufferPosition = 0;

					if (_rleBufferPosition == 256) {
						_rleBufferPosition = 0;
						write(_rleDstBuf, 256, 1, true);
					}

					_rleDstBuf[_rleBufferPosition++] = 0xFB;
					_bytesProcessed++;
				}

				_rleMode = 1;
				_rleByte = *value;
				value++;
				continue;
			}

			if (_rleCount == 0xFF) {
				if (!_rleByte) {
					writeToRLEBuffer(0xFC, 1);
					_rleMode = 1;
					_rleByte = *value;
					value++;
					continue;
				}

				writeToRLEBuffer(0xFF, 1);
				writeToRLEBuffer(_rleCount, 1);
				writeToRLEBuffer(_rleByte, 1);
				_rleMode = 1;
				_rleByte = *value;
				value++;
				continue;
			}

			value++;
			_rleCount++;
			break;

		default:
			break;
		}
	}

	return elementCount;
}

void CVCRFile::writeToRLEBuffer(uint8 operation, uint8 flag) {
	if (_rleBufferPosition == -1)
		_rleBufferPosition = 0;

	if (_rleBufferPosition == 256) {
		_rleBufferPosition = 0;
		write(_rleDstBuf, 256, 1, true);
	}

	if (flag || operation < 0xFB) {
		_rleDstBuf[_rleBufferPosition] = operation;
	} else {
		_rleDstBuf[_rleBufferPosition] = 0xFE;
	}

	_bytesProcessed++;
	_rleBufferPosition++;

	if (!flag && operation >= 0xFB) {
		if (_rleBufferPosition == 256) {
			_rleBufferPosition = 0;
			write(_rleDstBuf, 256, 1, true);
		}

		_rleDstBuf[_rleBufferPosition] = operation;

		_rleBufferPosition++;
		_bytesProcessed++;
	}
}

#define FINISH_FLUSH_OP											\
	{															\
		if (_rleBufferPosition != -1 && _rleBufferPosition) {   \
			write(_rleDstBuf, _rleBufferPosition, 1, true);     \
																\
			_rleBufferPosition = -1;							\
			_rleStatus = 0;										\
			_rleMode = 0;										\
			result = _bytesProcessed;							\
			_bytesProcessed = 0;								\
			return result;										\
		} else {												\
			_rleStatus = 0;										\
			_rleMode = 0;										\
			result = _bytesProcessed;							\
			_bytesProcessed = 0;								\
			return result;										\
		}														\
	}															\

uint32 CVCRFile::flush() {
	uint32 result;

	if (_rleStatus == 2) {
		if (_rleMode == 1) {
			if (_rleBufferPosition == -1)
				_rleBufferPosition = 0;

			if (_rleBufferPosition == 256) {
				_rleBufferPosition = 0;
				write(_rleDstBuf, 256, 1, 1);
			}

			if (_rleByte < 0xFB) {
				_rleDstBuf[_rleBufferPosition] = _rleByte;
			} else {
				_rleDstBuf[_rleBufferPosition] = 0xFE;
			}

			_rleBufferPosition++;
			_bytesProcessed++;

			if (_rleByte >= 0xFB) {
				if (_rleBufferPosition == 256) {
					_rleBufferPosition = 0;
					write(_rleDstBuf, 256, 1, true);
				}

				_rleDstBuf[_rleBufferPosition] = _rleByte;
				_rleBufferPosition++;
				_bytesProcessed++;
			}

			FINISH_FLUSH_OP;
		}

		if (_rleMode != 2) {
			FINISH_FLUSH_OP;
		}

		if (_rleCount == 3 && !_rleByte) {
			if (_rleBufferPosition == -1)
				_rleBufferPosition = 0;

			if (_rleBufferPosition == 256) {
				_rleBufferPosition = 0;
				write(_rleDstBuf, 256, 1, true);
			}

			_rleDstBuf[_rleBufferPosition] = 0xFB;
			_rleBufferPosition++;
			_bytesProcessed++;

			FINISH_FLUSH_OP;
		}

		if (_rleCount == 0xFF) {
			if (!_rleByte) {
				if (_rleBufferPosition == -1)
					_rleBufferPosition = 0;

				if (_rleBufferPosition == 256) {
					_rleBufferPosition = 0;
					write(_rleDstBuf, 256, 1, true);
				}

				_rleDstBuf[_rleBufferPosition] = 0xFC;
				_rleBufferPosition++;
				_bytesProcessed++;

				FINISH_FLUSH_OP;
			}
		} else if (!_rleByte) {
			writeToRLEBuffer(0xFD, 1);
			writeToRLEBuffer(_rleCount, 1);

			FINISH_FLUSH_OP;
		}

		writeToRLEBuffer(0xFF, 1);
		writeToRLEBuffer(_rleCount, 1);
		writeToRLEBuffer(_rleByte, 1);

		FINISH_FLUSH_OP;
	}

	if (_rleStatus == 1) {
		_rleStatus = 0;
		if (_rleBufferPosition != -1 && _rleBufferPosition != 256) {
			seek(_rleBufferPosition - 256, SEEK_CUR);
			_rleBufferPosition = -1;
			_rleStatus = 0;
			_rleMode = 0;
			result = _bytesProcessed;
			_bytesProcessed = 0;
			return result;
		}
	}

	_rleStatus = 0;
	_rleMode = 0;
	result = _bytesProcessed;
	_bytesProcessed = 0;
	return result;
}

#undef FINISH_FLUSH_OP

uint32 CVCRFile::seek(int32 offset, int mode) {
	int result;

	if (_rleStatus) {
		error("Error seeking in file \"%s\"", _filename.c_str());
		return 0;
	} else {
		if (_virtualFileIsOpen) {
			result = !_rwStream->seek(offset, mode);
			if (result) {
				error("Error seeking in file \"%s\"", _filename.c_str());
			}
			return result;
		} else {
			error("Error seeking in file \"%s\"", _filename.c_str());
			return 0;
		}
	}
}

void CVCRFile::close() {
	if (_virtualFileIsOpen) {
		Common::OutSaveFile *dumpFile = _engine->getSaveFileManager()->openForSaving(_engine->getTargetName() + "-" + Common::String(_filename), false);

		bool dumpFileIsOpen = dumpFile != nullptr;

		if (!dumpFileIsOpen) {
			error("Error opening file \"%s\".  It probably doesn\'t exist or is write protected.", _filename.c_str());
		}

		_rwStream->seek(0, SEEK_SET);

		assert(_virtualFileSize <= _rwStream->size());

		byte *dumpFileBuf = (byte *)malloc(_virtualFileSize);
		if (!dumpFileBuf) {
			error("Out of memory");
		}

		_rwStream->read(dumpFileBuf, _virtualFileSize);
		dumpFile->write(dumpFileBuf, _virtualFileSize);

		if (!dumpFile->flush()) {
			error("Error closing file \"%s\"", _filename.c_str());
		}

		// This block is not in the original and is used to ensure portability for VCR::shuffleGames()
		if (_fileMode == CVCRMODE_WB || _fileMode == CVCRMODE_RWB) {
			Common::String tsFilename = _filename;
			tsFilename.chop(4);
			tsFilename = _engine->getTargetName() + "-" + tsFilename + ".timestamp";

			if (_engine->getSaveFileManager()->exists(tsFilename))
				_engine->getSaveFileManager()->removeSavefile(tsFilename);

			Common::OutSaveFile *tsFile = _engine->getSaveFileManager()->openForSaving(tsFilename, false);
			assert(tsFile);

			// Add the seconds to the timestamp, so that we have the same granularity as the original...
			TimeDate td;
			_engine->_system->getTimeAndDate(td, true);

			tsFile->writeSint32LE((int32)td.tm_sec);

			// Append the extended save header, which includes the file modification timestamp (date and hours/minutes)...
			_engine->getMetaEngine()->appendExtendedSave(tsFile, _engine->getTotalPlayTime(), _filename, false);

			tsFile->finalize();
			delete tsFile;
		}

		dumpFile->finalize();
		delete dumpFile;

		free(dumpFileBuf);
		_rwStream->finalize();
		delete _rwStream;
		_rwStream = nullptr;

		_rwStreamSize = 0;
		_virtualFileSize = 0;
		_virtualFileIsOpen = false;
	}
}

int32 CVCRFile::tell() {
	if (_rleStatus || !_virtualFileIsOpen) {
		error("Error telling in file \"%s\"", _filename.c_str());
		return 0;
	} else {
		return (int32)_rwStream->pos();
	}
}

} // End of namespace LastExpress
