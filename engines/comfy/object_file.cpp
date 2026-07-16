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
 */

#include "comfy/comfy.h"

namespace Comfy {

ComfyEngine::ObjFileCacheRow *ComfyEngine::tileListEvictTail(ObjFile *objectFile) {
	if (!objectFile || !objectFile->cacheRows || objectFile->tileLruTail >= objectFile->cacheRowCount)
		return nullptr;

	ObjFileCacheRow *row = &objectFile->cacheRows[objectFile->tileLruTail];
	objectFile->tileLruTail = row->previousIndex;
	objectFile->tileLruTailValid = row->previousValid;

	if (objectFile->tileStatus && row->frameIndex < objectFile->tileStatusCount)
		objectFile->tileStatus[row->frameIndex] = 0xFFFF;

	return row;
}

ComfyEngine::ObjFileCacheRow *ComfyEngine::tileListAlloc(ObjFile *objectFile) {
	if (!objectFile || !objectFile->cacheRows)
		return nullptr;

	ObjFileCacheRow *row = nullptr;
	uint16 rowIndex = objectFile->currentBlock;

	if (objectFile->currentBlock < objectFile->cacheRowCount) {
		row = &objectFile->cacheRows[rowIndex];

		if (!objectFile->currentBlock) {
			objectFile->tileLruTail = rowIndex;
			objectFile->tileLruTailValid = 1;
			row->nextIndex = rowIndex;
			row->nextValid = 1;
			row->previousIndex = rowIndex;
			row->previousValid = 1;
		}

		objectFile->currentBlock++;
		if (objectFile->tileLruTailValid && objectFile->tileLruTail < objectFile->cacheRowCount) {
			ObjFileCacheRow *tail = &objectFile->cacheRows[objectFile->tileLruTail];
			row->nextIndex = tail->nextIndex;
			row->nextValid = tail->nextValid;
			row->previousIndex = objectFile->tileLruTail;
			row->previousValid = 1;

			if (row->nextIndex < objectFile->cacheRowCount) {
				objectFile->cacheRows[row->nextIndex].previousIndex = rowIndex;
				objectFile->cacheRows[row->nextIndex].previousValid = 1;
			}

			tail->nextIndex = rowIndex;
			tail->nextValid = 1;
		}
	} else {
		row = tileListEvictTail(objectFile);
	}

	return row;
}

void ComfyEngine::tileListTouch(ObjFile *objectFile, uint16 index) {
	if (!objectFile || !objectFile->cacheRows || index >= objectFile->cacheRowCount)
		return;

	ObjFileCacheRow *row = &objectFile->cacheRows[index];
	if (objectFile->tileLruTailValid && objectFile->tileLruTail == index) {
		objectFile->tileLruTail = row->previousIndex;
		objectFile->tileLruTailValid = row->previousValid;
		return;
	}

	if (row->nextIndex < objectFile->cacheRowCount) {
		objectFile->cacheRows[row->nextIndex].previousIndex = row->previousIndex;
		objectFile->cacheRows[row->nextIndex].previousValid = row->previousValid;
	}

	if (row->previousIndex < objectFile->cacheRowCount) {
		objectFile->cacheRows[row->previousIndex].nextIndex = row->nextIndex;
		objectFile->cacheRows[row->previousIndex].nextValid = row->nextValid;
	}

	if (objectFile->tileLruTailValid && objectFile->tileLruTail < objectFile->cacheRowCount) {
		ObjFileCacheRow *tail = &objectFile->cacheRows[objectFile->tileLruTail];
		row->nextIndex = tail->nextIndex;
		row->nextValid = tail->nextValid;
		row->previousIndex = objectFile->tileLruTail;
		row->previousValid = 1;

		if (row->nextIndex < objectFile->cacheRowCount) {
			objectFile->cacheRows[row->nextIndex].previousIndex = index;
			objectFile->cacheRows[row->nextIndex].previousValid = 1;
		}

		tail->nextIndex = index;
		tail->nextValid = 1;
	}
}

uint16 ComfyEngine::tileListTouchRange(ObjFile *objectFile, uint16 first, uint16 last) {
	if (!objectFile || !objectFile->tileStatus)
		return 0;

	uint16 removed = 0;
	uint16 row = first;

	while (row <= last && row < objectFile->tileStatusCount && objectFile->tileStatus[row] != 0xFFFF) {
		removed++;
		row++;
	}

	row++;

	while (row <= last && row < objectFile->tileStatusCount) {
		uint16 tile = objectFile->tileStatus[row];
		if (tile != 0xFFFF) {
			tileListTouch(objectFile, tile);
			removed++;
		}

		row++;
	}

	return removed;
}

void ComfyEngine::tileUploadToXms(ObjFile *objectFile, const byte *source, uint16 size, uint32 destinationOffset) {
	if (!objectFile || !source)
		return;

	XmsMove move;
	move.length = size;
	move.destinationHandle = objectFile->cacheXmsHandle;
	move.destinationOffset = destinationOffset;
	move.sourceMemory = source;
	if (xmsTransfer(move) < 0)
		error("Unable to upload object-file tile to XMS");
}

void ComfyEngine::tileCopyXmsPair(ObjFile *objectFile, byte *destination, uint32 sourceOffset, uint32 size) {
	if (!objectFile || !destination)
		return;

	if (size == 1)
		size = 2;

	if (size & 1) {
		tileCopyXmsPair(objectFile, destination + 1, sourceOffset + 1, size - 1);
		tileCopyXmsPair(objectFile, destination, sourceOffset, 2);
		return;
	}

	XmsMove move;
	move.length = size;
	move.sourceHandle = objectFile->cacheXmsHandle;
	move.sourceOffset = sourceOffset;
	move.destinationMemory = destination;

	if (xmsTransfer(move) < 0)
		error("Unable to read object-file tile from XMS");
}

uint16 ComfyEngine::tileDrawStrip(ObjFile *objectFile, byte *destination, uint16 cacheRow, uint16 sourceColumn, uint32 width) {
	if (!objectFile || !destination || !objectFile->blockSize)
		return 0;

	uint16 drawWidth = (uint16)width;
	uint16 remainingColumns = (uint16)(objectFile->blockSize - sourceColumn);

	if (remainingColumns < drawWidth)
		drawWidth = remainingColumns;

	uint32 sourceOffset = (uint32)cacheRow * objectFile->blockSize + sourceColumn;
	tileCopyXmsPair(objectFile, destination, sourceOffset, drawWidth);
	tileListTouch(objectFile, cacheRow);
	_stripsDrawn++;

	return drawWidth;
}

void ComfyEngine::tileLoadAndCache(ObjFile *objectFile, const byte *source, uint16 tileId) {
	ObjFileCacheRow *row = tileListAlloc(objectFile);
	if (!objectFile || !row)
		return;

	uint16 frameIndex = row->tileIndex;
	row->frameIndex = tileId;
	if (objectFile->tileStatus && tileId < objectFile->tileStatusCount)
		objectFile->tileStatus[tileId] = frameIndex;

	tileUploadToXms(objectFile, source, objectFile->blockSize, (uint32)frameIndex * objectFile->blockSize);
	tileListTouch(objectFile, frameIndex);
}

void ComfyEngine::frameLoaderLoadTileCore(ObjFile *objectFile, const byte *source, uint16 tileId) {
	tileLoadAndCache(objectFile, source, tileId);
}

void ComfyEngine::frameLoaderLoadTile(ObjFile *objectFile, uint16 tileId) {
	if (!objectFile || !objectFile->tileReadBuffer)
		return;

	objFileReadRow(objectFile->tileReadBuffer, tileId, 1, objectFile->blockSize, objectFile);
	frameLoaderLoadTileCore(objectFile, objectFile->tileReadBuffer, tileId);
}

void ComfyEngine::objFileReadRow(byte *destination, uint16 row, uint16 count, uint16 size, ObjFile *objectFile) {
	if (!destination || !objectFile || !objectFile->stream || !size)
		return;

	uint32 offset = (uint32)row * size;
	if (!objectFile->stream->seek(offset))
		return;

	uint32 total = (uint32)size * count;

	if (total == 0x10000) {
		objectFile->stream->read(destination, size);
		objectFile->stream->read(destination + size, (uint32)size * (count - 1));
	} else {
		objectFile->stream->read(destination, total);
	}

	_tilesRead += count;
}

void ComfyEngine::objFileReadFieldCore(byte *destination, uint32 sourceOffset, uint32 byteCount, ObjFile *objectFile) {
	if (!destination || !objectFile || !objectFile->blockSize || !byteCount)
		return;

	uint16 firstRow = (uint16)(sourceOffset / objectFile->blockSize);
	uint16 lastRow = (uint16)((sourceOffset + byteCount - 1) / objectFile->blockSize);
	uint16 row = firstRow;
	uint16 column = (uint16)(sourceOffset % objectFile->blockSize);
	byte *output = destination;
	uint32 remaining = byteCount;

	tileListTouchRange(objectFile, firstRow, lastRow);

	while (row <= lastRow && remaining) {
		uint16 copied = 0;

		if (row < objectFile->tileStatusCount && objectFile->tileStatus[row] == 0xFFFF && (_isPanther || _engineVersion == 3))
			frameLoaderLoadTile(objectFile, row);

		if (row < objectFile->tileStatusCount && objectFile->tileStatus[row] != 0xFFFF) {
			copied = tileDrawStrip(objectFile, output, objectFile->tileStatus[row], column, remaining);
		} else {
			uint16 missingRows = 0;
			uint16 probe = row;

			while (probe <= lastRow && probe < objectFile->tileStatusCount && objectFile->tileStatus[probe] == 0xFFFF) {
				if ((uint32)(missingRows + 1) * objectFile->blockSize > 0x10000)
					break;

				missingRows++;
				probe++;
			}

			if (!missingRows)
				return;

			bool bounce = column || (uint32)missingRows * objectFile->blockSize > remaining;
			byte *readDestination = bounce ? objectFile->readBuffer : output;
			if (!readDestination)
				return;

			objFileReadRow(readDestination, row, missingRows, objectFile->blockSize, objectFile);

			for (uint16 i = 0; i < missingRows; i++) {
				byte *tileSource = readDestination + (uint32)i * objectFile->blockSize;
				uint16 size = objectFile->blockSize;

				if (bounce) {
					size = objectFile->blockSize - column;
					if (size > remaining)
						size = (uint16)remaining;

					if (size)
						memcpy(output, tileSource + column, size);
				}

				tileLoadAndCache(objectFile, tileSource, (uint16)(row + i));
				column = 0;
				remaining -= size;
				output += size;
			}

			row += missingRows;
			continue;
		}

		if (!copied)
			return;

		remaining -= copied;
		output += copied;
		row++;
		column = 0;
	}
}

byte *ComfyEngine::objFileReadTiledCore(uint32 sourceOffset, uint32 byteCount, ObjFile *objectFile) {
	if (!objectFile || !objectFile->stream || !objectFile->blockSize || sourceOffset >= objectFile->fileSize || !byteCount)
		return nullptr;

	if (byteCount > objectFile->fileSize - sourceOffset)
		byteCount = objectFile->fileSize - sourceOffset;

	if (!objectFile->readBuffer)
		return nullptr;

	if (_isPanther || _engineVersion == 3) {
		if (byteCount > objectFile->readBufferSize)
			return nullptr;

		objFileReadFieldCore(objectFile->readBuffer, sourceOffset, byteCount, objectFile);
		return objectFile->readBuffer;
	}

	uint16 blockSize = objectFile->blockSize;
	uint16 firstRow = (uint16)(sourceOffset / blockSize);
	uint16 lastRow = (uint16)((sourceOffset + byteCount - 1) / blockSize);
	uint16 firstColumn = (uint16)(sourceOffset % blockSize);
	bool firstCached = firstRow < objectFile->tileStatusCount && objectFile->tileStatus[firstRow] != 0xFFFF;
	bool lastCached = lastRow < objectFile->tileStatusCount && objectFile->tileStatus[lastRow] != 0xFFFF;
	uint32 windowSize;

	if (firstRow == lastRow) {
		windowSize = firstCached ? byteCount : blockSize;
	} else {
		uint32 headSize = blockSize;
		uint32 tailSize = blockSize;
		if (firstCached)
			headSize -= firstColumn;

		if (lastCached)
			tailSize = (sourceOffset + byteCount) % blockSize;

		windowSize = (uint32)(lastRow - firstRow - 1) * blockSize + headSize + tailSize;
	}

	if (windowSize <= 0x10000) {
		uint32 windowOffset = sourceOffset;
		if (!firstCached)
			windowOffset -= firstColumn;

		objFileReadFieldCore(objectFile->readBuffer, windowOffset, windowSize, objectFile);
		return objectFile->readBuffer + (firstCached ? 0 : firstColumn);
	}

	tileListTouchRange(objectFile, firstRow, lastRow);
	uint16 missing = 0;
	uint16 row = firstRow;

	while (row <= lastRow && row < objectFile->tileStatusCount && objectFile->tileStatus[row] == 0xFFFF) {
		missing++;
		row++;
	}

	if ((uint32)missing * blockSize > 0x10000)
		missing = 0x10000 / blockSize;

	if (missing) {
		objFileReadRow(objectFile->readBuffer, firstRow, missing, blockSize, objectFile);

		for (uint16 i = 0; i < missing; i++)
			tileLoadAndCache(objectFile, objectFile->readBuffer + (uint32)i * blockSize, (uint16)(firstRow + i));
	}

	missing = 0;
	row = lastRow;

	while (row >= firstRow && row < objectFile->tileStatusCount && objectFile->tileStatus[row] == 0xFFFF) {
		missing++;
		if (row == firstRow)
			break;

		row--;
	}

	if ((uint32)missing * blockSize > 0x10000)
		missing = 0x10000 / blockSize;

	if (missing) {
		uint16 readRow = (uint16)(lastRow - missing + 1);
		objFileReadRow(objectFile->readBuffer, readRow, missing, blockSize, objectFile);
		for (uint16 i = missing; i != 0; i--) {
			uint16 tileId = (uint16)(lastRow - i + 1);
			uint16 bufferRow = (uint16)(missing - i);
			tileLoadAndCache(objectFile, objectFile->readBuffer + (uint32)bufferRow * blockSize, tileId);
		}
	}

	objFileReadFieldCore(objectFile->readBuffer, sourceOffset, byteCount, objectFile);
	return objectFile->readBuffer;
}

void ComfyEngine::objFileReadField(byte *destination, uint32 sourceOffset, uint32 byteCount, ObjFile *objectFile) {
	objFileReadFieldCore(destination, sourceOffset, byteCount, objectFile);
}

byte *ComfyEngine::objFileReadTiled(uint32 sourceOffset, uint32 byteCount, ObjFile *objectFile) {
	return objFileReadTiledCore(sourceOffset, byteCount, objectFile);
}

byte *ComfyEngine::frameLoaderReadFrameData(uint32 sourceOffset, uint32 byteCount, ObjFile *objectFile) {
	if (_engineVersion != 3 || byteCount <= 0x10000)
		return objFileReadTiled(sourceOffset, byteCount, objectFile);

	if (!objectFile || !objectFile->readBuffer || byteCount > objectFile->readBufferSize)
		return nullptr;

	byte *data = objFileReadTiled(sourceOffset, 0x10000, objectFile);
	if (!data)
		return nullptr;

	objFileReadFieldCore(objectFile->readBuffer + 0x10000, sourceOffset + 0x10000, byteCount - 0x10000, objectFile);
	return data;
}

ComfyEngine::ObjFile *ComfyEngine::objFileOpen(const Common::Path &path, uint16 blockSize, uint32 maximumBytes) {
	Common::SeekableReadStream *stream = pathFOpen(path, true);
	if (!stream || stream->size() < 0 || (uint64)stream->size() > UINT32_MAX || !blockSize) {
		delete stream;
		return nullptr;
	}

	if (blockSize & 0x07FF)
		blockSize += 0x0800 - (blockSize & 0x07FF);

	uint32 fileSize = (uint32)stream->size();
	uint16 fileBlocks = (uint16)((fileSize + blockSize - 1) / blockSize);
	uint16 maximumBlocks = (uint16)(maximumBytes / blockSize);
	uint16 cacheRows = MAX<uint16>(MIN<uint16>(maximumBlocks, fileBlocks), 4);
	uint32 cacheBytes = (uint32)cacheRows * blockSize;
	uint16 cachePages = (uint16)((cacheBytes + 0x03FF) / 0x0400);
	uint16 controlSize = (_isPanther || _engineVersion == 3) ? 0x56 : 0x54;

	ObjFile *objectFile = new ObjFile();
	objectFile->stream = stream;
	objectFile->path = path;
	objectFile->fileSize = fileSize;
	objectFile->blockSize = blockSize;
	objectFile->cacheRowCount = cacheRows;
	objectFile->tileStatusCount = (uint16)fileBlocks;
	objectFile->trackedSize = (uint32)((uint32)fileBlocks * sizeof(uint16) + (uint32)cacheRows * 0x0C + controlSize + 4);
	_memAllocTotal += objectFile->trackedSize;
	objectFile->readBufferSize = _isPanther || _engineVersion == 3 ? 0x20010 : 0x10010;
	objectFile->readBuffer = (byte *)malloc(objectFile->readBufferSize);
	objectFile->tileReadBuffer = (byte *)malloc(blockSize);
	objectFile->tileStatus = (uint16 *)malloc((fileBlocks ? fileBlocks : 1) * sizeof(uint16));
	objectFile->cacheRows = (ObjFileCacheRow *)malloc((cacheRows ? cacheRows : 1) * sizeof(ObjFileCacheRow));

	int32 cacheHandle = memAllocTrack((uint16)(cachePages + 1));
	if (!objectFile->readBuffer || !objectFile->tileReadBuffer || !objectFile->tileStatus || !objectFile->cacheRows || cacheHandle < 0) {
		if (cacheHandle >= 0)
			memFreeTrack((uint16)cacheHandle);

		objFileClose(objectFile);
		return nullptr;
	}

	objectFile->cacheXmsHandle = (uint16)cacheHandle;
	for (uint32 i = 0; i < fileBlocks; i++)
		objectFile->tileStatus[i] = 0xFFFF;

	for (uint16 i = 0; i < cacheRows; i++) {
		objectFile->cacheRows[i].tileIndex = i;
		objectFile->cacheRows[i].frameIndex = 0;
		objectFile->cacheRows[i].previousIndex = 0;
		objectFile->cacheRows[i].previousValid = 0;
		objectFile->cacheRows[i].nextIndex = 0;
		objectFile->cacheRows[i].nextValid = 0;
	}

	return objectFile;
}

void ComfyEngine::objFileClose(ObjFile *&objectFile) {
	if (!objectFile)
		return;

	if (objectFile->cacheXmsHandle)
		memFreeTrack(objectFile->cacheXmsHandle);

	_memAllocTotal -= objectFile->trackedSize;

	delete objectFile->stream;
	free(objectFile->readBuffer);
	free(objectFile->tileReadBuffer);
	free(objectFile->tileStatus);
	free(objectFile->cacheRows);
	delete objectFile;
	objectFile = nullptr;
}

} // End of namespace Comfy
