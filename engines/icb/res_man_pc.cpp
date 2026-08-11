/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_array.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/res_man.h"
#include "engines/icb/debug.h"
#include "engines/icb/protocol.h"
#include "engines/icb/keyboard.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/zsupport.h"

#include "common/archive.h"
#include "common/mutex.h"
#include "common/textconsole.h"
#include "common/config-manager.h"
#include "common/memstream.h"

namespace ICB {

uint32 MAX_MEM_BLOCKS = 0;

Common::SeekableReadStream *openDiskFileForBinaryStreamRead(const Common::String &filename) {
	pxString path = filename.c_str();
	path.ConvertPath();
	Common::SeekableReadStream *stream = nullptr;
	if (SearchMan.hasFile(path.c_str()))
		stream = SearchMan.createReadStreamForMember(path.c_str());
	else
		error("File not found: %s", path.c_str());

	return stream;
}

Common::WriteStream *openDiskWriteStream(const Common::String &filename) { error("TODO: Connect up the savegame-handler and friends"); }

bool checkFileExists(const char *fullpath) {
	Common::File file;

	return file.exists(fullpath);
}

void Memory_stats() {
	uint32 one_meg = 1024 * 1024;

	// Find ram specs and decide how much we'll later give to resource manager(s)
	int32 m;

	// HACK: set to 256MB to ensure most generous caching
	m = 256;

	if (m < 16) { // less than 16mb
		Fatal_error("not enough memory - requires 16mb or better");
	} else if (m < 20) { // 16Mb to 20Mb - take 12Mb
		memory_available = 12 * one_meg;
		BACKGROUND_BUFFER_SIZE = 6 * one_meg;
		ANIMATION_BUFFER_SIZE = 4 * one_meg;
		BITMAP_BUFFER_SIZE = 1 * one_meg;
		SONICS_BUFFER_SIZE = 1 * one_meg;
		MAX_MEM_BLOCKS = (1024); // 4K of table space
	}

	else if (m <= 32) { // 20-32MB - take 18Mb
		memory_available = 18 * one_meg;
		BACKGROUND_BUFFER_SIZE = 9 * one_meg;
		ANIMATION_BUFFER_SIZE = 6 * one_meg;
		BITMAP_BUFFER_SIZE = 2 * one_meg;
		SONICS_BUFFER_SIZE = 1 * one_meg;
		MAX_MEM_BLOCKS = (1024); // 4K of table space
	}

	else if (m <= 64) { // 33 - 64MB - take 32mb
		memory_available = 30 * one_meg;
		BACKGROUND_BUFFER_SIZE = 15 * one_meg;
		ANIMATION_BUFFER_SIZE = 8 * one_meg;
		BITMAP_BUFFER_SIZE = 5 * one_meg;
		SONICS_BUFFER_SIZE = 2 * one_meg;
		MAX_MEM_BLOCKS = (2048); // 8K of table space
	}

	else if (m <= 128) { // 65 - 128MB - take 64 mb
		memory_available = 60 * one_meg;
		BACKGROUND_BUFFER_SIZE = 36 * one_meg;
		ANIMATION_BUFFER_SIZE = 10 * one_meg;
		BITMAP_BUFFER_SIZE = 10 * one_meg;
		SONICS_BUFFER_SIZE = 4 * one_meg;
		MAX_MEM_BLOCKS = (4096); // 16K of table space
	}

	else { // Lots and lots and lots of memory - take 128Mb
		memory_available = 120 * one_meg;
		BACKGROUND_BUFFER_SIZE = 80 * one_meg;
		ANIMATION_BUFFER_SIZE = 15 * one_meg;
		BITMAP_BUFFER_SIZE = 15 * one_meg;
		SONICS_BUFFER_SIZE = 10 * one_meg;
		MAX_MEM_BLOCKS = (8192); // 32K of table space
	}
}

uint8 *res_man::AllocMemory(uint32 &memory_tot) {
	// use new so the overloaded new gets called
	uint8 *memory_b = new uint8[memory_tot];

	if (!memory_b) { // could not grab the memory
		Zdebug("couldn't malloc %d in Init_memory_manager", total_free_memory);
		Fatal_error("Init_memory_manager() couldn't malloc %d bytes [line=%d file=%s]", total_free_memory, __LINE__, __FILE__);
	}
	return memory_b;
}

uint32 res_man::Fetch_size(const char * /*url*/, uint32 url_hash, const char *cluster, uint32 cluster_hash) {
	// try to find the cluster file
	RMParams params;
	params.url_hash = NULL_HASH;
	params.cluster = cluster;
	params.cluster_hash = cluster_hash;
	params.mode = RM_LOADNOW;
	params.not_ready_yet = 0;
	int32 cluster_search = FindFile(&params);
	params.url_hash = url_hash;

	HEADER_NORMAL *hn = GetFileHeader(cluster_search, &params);

	// return value of 0 means no such file
	if (hn == nullptr)
		return 0;

	return hn->size;
}

bool8 res_man::Test_file(const char *url, uint32 url_hash, const char *cluster, uint32 cluster_hash) {
	Tdebug("clusters.txt", "**Testing file %s in cluster %s", url, cluster);

	// try to find the cluster file
	RMParams params;
	params.url_hash = NULL_HASH;
	params.cluster = cluster;
	params.cluster_hash = cluster_hash;
	params.mode = RM_LOADNOW;
	params.not_ready_yet = 0;
	int32 cluster_search = FindFile(&params);
	params.url_hash = url_hash;

	HEADER_NORMAL *hn = GetFileHeader(cluster_search, &params);

	if (hn == nullptr)
		return 0;

	return 1;
}

bool8 res_man::Test_file(const char *url) {
	pxString path(url);
	path.ConvertPath();

	Common::File file;

	return (bool8)file.exists(path.c_str());
}

void res_man::ReadFile(const char * /*url*/, RMParams *params) {
	// params->search is mem_list slot number to load the file into

	// Normally the url in params->url is stored, but when we are loading a cluster
	// this is empty, so we should use params->cluster
	// If we are reading in a cluster

	mem_list[params->search].state = MEM_in_use; // slot now being used
	mem_list[params->search].url_hash = params->url_hash;
	mem_list[params->search].cluster_hash = params->cluster_hash;
	mem_list[params->search].total_hash = MAKE_TOTAL_HASH(params->cluster_hash, params->url_hash);

	//      now load the file
	if (params->mode == RM_LOADNOW) {
		if (params->zipped) {
			// When loading from a cluster the params->seekpos value holds the position to read from
			Tdebug("clusters.txt", "  fseek to pos %d", params->seekpos);
			if (!params->_stream->seek(params->seekpos, SEEK_SET))
				Fatal_error("Could not fseek to %d bytes in %s", params->seekpos, params->cluster);

			memUncompress(mem_list[params->search].ad, params->cluster, params->_stream);
		} else {
			// When loading from a cluster the params->seekpos value holds the position to read from
			Tdebug("clusters.txt", "  fseek to pos %d", params->seekpos);
			if (!params->_stream->seek(params->seekpos, SEEK_SET))
				Fatal_error("Could not fseek to %d bytes in %s", params->seekpos, params->cluster);

			Tdebug("clusters.txt", "  Read %d bytes", params->len);

			// hurray, load it in.
			if (params->_stream->read(mem_list[params->search].ad, sizeof(char) * params->len) != (uint)params->len)
				Fatal_error("Failed to read %d bytes from 0x%X", params->len, params->url_hash);
		}

		Tdebug("clusters.txt", "  Close handle %x", params->_stream);
		delete params->_stream; // close the cluster
		params->_stream = nullptr;

		mem_list[params->search].protect = 0;
	}
}

const char *res_man::OpenFile(int32 &cluster_search, RMParams *params) {
	pxString rootPath("");
	pxString clusterName(params->cluster);
	clusterName.ToLower();

	pxString clusterPath = rootPath + clusterName;
	clusterPath.ConvertPath();

	// Are we are trying to open a cluster
	if (params->url_hash == NULL_HASH) {
		Tdebug("clusters.txt", "  Read in cluster header");
		// We are trying to open a cluster.
		// The name of the cluster is, in theory, in params->cluster

		// Open up the cluster
		params->_stream = openDiskFileForBinaryStreamRead(clusterPath.c_str());
		Tdebug("clusters.txt", "  open cluster file %s handle %x", clusterPath.c_str(), params->_stream);

		if (params->_stream == nullptr)
			Fatal_error("Res_open cannot *OPEN* cluster file %s", clusterPath.c_str());

		if (params->_stream->size() < 16) {
			Fatal_error("res_man::OpenFile cannot read 16 bytes from cluster %s %d", clusterPath.c_str(), params->cluster_hash);
		}

		params->seekpos = 0;
		params->_stream->skip(8);
		params->len = params->_stream->readSint32LE();
		return params->cluster;
	}

	HEADER_NORMAL *hn = GetFileHeader(cluster_search, params);
	if (hn == nullptr) {
		// Big error the file wasn't found in the cluster
		Fatal_error("res_man::OpenFile couldn't find url %X in cluster %s %X", params->url_hash, params->cluster, params->cluster_hash);
	}

	// This has to be done here because GetFileHeader can read in data which closes the file
	// whose handle is stored in params->fh

	// Open up the cluster
	params->_stream = openDiskFileForBinaryStreamRead(clusterPath.c_str());
	Tdebug("clusters.txt", "  open cluster file %s handle %x", clusterPath.c_str(), params->_stream);

	if (params->_stream == nullptr)
		Fatal_error("Res_open cannot *OPEN* cluster file %s", clusterPath.c_str());

	params->seekpos = FROM_LE_32(hn->offset);

	// Set the length of the data
	if (params->zipped) {
		params->_stream->seek(params->seekpos, SEEK_SET);
		params->len = fileGetZipLength2(params->_stream); // TODO: Use wrapCompressedStream to solve?
	} else
		params->len = FROM_LE_32(hn->size);

	return nullptr;
}

// Get the header information for a particular file from a cluster
// Will load the cluster in if need be
HEADER_NORMAL *res_man::GetFileHeader(int32 &cluster_search, RMParams *params) {
	Cluster_API *clu;

	// If cluster_search = -1 then the cluster was not found so load the cluster in
	if (cluster_search == -1) {
		uint32 url_hash = params->url_hash;
		params->url_hash = NULL_HASH;
		uint32 compression = params->compressed; // Cluster headers are not compressed
		params->compressed = params->zipped = FALSE8;
		clu = (Cluster_API *)LoadFile(cluster_search, params);
		cluster_search = params->search;
		params->url_hash = url_hash;
		params->compressed = params->zipped = compression; // Restore compression
	} else {
		// The cluster is in the memory pool at position cluster_search
		clu = (Cluster_API *)mem_list[cluster_search].ad;
	}

	// So we are trying to read in a file from a cluster
	// and the cluster is now in memory in clu variable

	// So just need to search the cluster to find the url_hash value
	// and get the position and length to get the data from

	// Check the schema value and the ID
	if ((FROM_LE_32(clu->schema) != CLUSTER_API_SCHEMA) || (READ_LE_U32((uint32 *)clu->ID) != *(uint32 *)const_cast<char *>(CLUSTER_API_ID))) {
		// Big error unknown cluster filetype
		Fatal_error("res_man::GetFileHeader unknown cluster schema or ID %d %s for %s::0x%X", clu->schema, clu->ID, params->cluster, params->url_hash);
	}

	if (FROM_LE_32(clu->ho.cluster_hash) != params->cluster_hash) {
		// Big error this cluster has a different internal hash value to the one
		// we are looking for
//		Fatal_error("res_man::GetFileHeader different internal cluster_hash value %x file %x for %s::0x%X", params->cluster_hash, clu->ho.cluster_hash, params->cluster,
//		            params->url_hash);
	}

	HEADER_NORMAL *hn = clu->hn;
	uint32 i;
	for (i = 0; i < FROM_LE_32(clu->ho.noFiles); hn++, i++) {
		// Hey it has been found
		if (FROM_LE_32(hn->hash) == params->url_hash)
			break;
	}

	// Check that the file was actually found
	if (i == FROM_LE_32(clu->ho.noFiles)) {
		return nullptr;
	}

	return hn;
}

void res_man::Res_open_cluster(const char * /* cluster_url */, uint32 & /* cluster_hash */, int32 /* size */) { Fatal_error("Res_open_cluster not supported on the pc"); }

void res_man::Res_open_mini_cluster(const char *cluster_url, uint32 &cluster_hash, const char *fake_cluster_url, uint32 &fake_cluster_hash) {
	// open the mini-cluster

	uint32 zeroHash = 0;
	Cluster_API *clu = (Cluster_API *)Res_open(nullptr, zeroHash, cluster_url, cluster_hash);

	int32 numFiles = FROM_LE_32(clu->ho.noFiles);

	// check none of the fake files exist
	// also find total size required

	int32 mem_needed = 0;
	int32 i;

	for (i = 0; i < numFiles; i++) {
		HEADER_NORMAL *hn = clu->hn + i;
		uint32 check_hash = FROM_LE_32(hn->hash);

		if (FindFile(check_hash, fake_cluster_hash, MAKE_TOTAL_HASH(fake_cluster_hash, check_hash)) != -1) {
			warning("File %s::%08x exists in res_man so can't load my mini-cluster!", fake_cluster_url, check_hash);
			return;
		}

		int32 fileSize = (FROM_LE_32(hn->size) + 7) & (~7);
		mem_needed += fileSize;
	}

	// check mem_needed is multiple of 8

	mem_needed = (mem_needed + 7) & (~7);

	// now grab enough memory..

	uint16 mem_block;
	RMParams params; // the params are only used for debugging

	params.url_hash = NULL_HASH;
	params.cluster = fake_cluster_url;
	params.cluster_hash = fake_cluster_hash;

	mem_block = (uint16)FindMemBlock(mem_needed, &params);

	// ensure the header is still in memory

	clu = (Cluster_API *)Res_open(nullptr, zeroHash, cluster_url, cluster_hash);

	// now load in the body...
	// from first file upwards

	// DiscRead(mem_list[mem_block].ad,(clu->hn)->offset,mem_needed,NULL,clu->ho.cdpos);

	pxString rootPath("");
	pxString clusterName(fake_cluster_url);
	clusterName.ToLower();

	pxString clusterPath = rootPath + clusterName;
	clusterPath.ConvertPath();

	Common::SeekableReadStream *stream;

	// open
	stream = openDiskFileForBinaryStreamRead(clusterPath.c_str());

	// seek
	stream->seek(FROM_LE_32((clu->hn)->offset), SEEK_SET);

	// read
	stream->read(mem_list[mem_block].ad, mem_needed);

	// close
	delete stream;

	// now tie up the files...

	uint8 *ad = mem_list[mem_block].ad;
	uint16 current_parent = mem_list[mem_block].parent;
	uint16 current_child = mem_list[mem_block].child;

	for (i = 0; i < numFiles; i++) {
		// printf("tying up files %d\n",i);

		HEADER_NORMAL *hn = clu->hn + i;

		if (i != 0) {
			current_parent = mem_block;

			// spawn a new block for all but first...
			mem_block = Fetch_spawn(current_parent);

			// set parent...
			mem_list[current_parent].child = mem_block;

			// setup current...
			mem_list[mem_block].parent = current_parent;
			mem_list[mem_block].child = current_child;

			// setup child...
			mem_list[current_child].parent = mem_block;
		}

		mem_list[mem_block].url_hash = FROM_LE_32(hn->hash);
		mem_list[mem_block].cluster_hash = fake_cluster_hash;
		mem_list[mem_block].total_hash = MAKE_TOTAL_HASH(fake_cluster_hash, FROM_LE_32(hn->hash));

		mem_list[mem_block].ad = ad;

		mem_list[mem_block].state = MEM_in_use;
		mem_list[mem_block].protect = 0;
		mem_list[mem_block].age = current_time_frame;

		// adjusted size
		int32 fileSize = (FROM_LE_32(hn->size) + 7) & (~7);

		mem_list[mem_block].size = fileSize;

		// one more file open
		number_files_open++;

		// next file has next address along
		ad += fileSize;
	}
}

void res_man::Garbage_removal() {
	// does nothing at present on pc
}

} // End of namespace ICB
