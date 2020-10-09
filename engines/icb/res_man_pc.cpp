/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

Common::File *openDiskFileForBinaryRead(const char *filename) {
	pxString path = filename;
	path.ConvertPath();
	Common::File *result = new Common::File();
	if (result->open(path.c_str())) {
		return result;
	} else {
		delete result;
		warning("openDiskFileForBinaryRead(%s) - FAILED", path.c_str());
		return NULL;
	}
}

Common::SeekableReadStream *openDiskFileForBinaryStreamRead(const Common::String &filename) {
	// Quick-fix to start replacing FILE with Stream.
	Common::File *f = openDiskFileForBinaryRead(filename.c_str());
	if (!f) {
		return nullptr;
	}
	int size = f->size();
	byte *data = new byte[size];
	f->read(data, size);
	f->close();
	delete f;
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
	return stream;
}

Common::WriteStream *openDiskWriteStream(const Common::String &filename) { error("TODO: Connect up the savegame-handler and friends"); }

bool checkFileExists(const char *fullpath) {
	Common::File file;

	return file.exists(fullpath);
}

#if 0

FILE *openDiskFileForBinaryWrite(const char *filename) {
	pxString path = filename;
	path.ConvertPath();
	warning("openDiskFileForBinaryWrite(%s)", path.c_str());
	FILE *result = fopen(path, "wb");
	if (result != NULL) {
		warning(" - SUCCESS");
		return result;
	} else {
		warning(" - FAILED");
		return NULL;
	}
}

#endif

// Thread procedure take files and load them
int async_loadThread(void *v) {
	Common::File *in;
	res_man *rm = (res_man *)v;

	int8 *fn = NULL;
	uint8 *p;
	int32 z;
	int32 size;
	int32 newSize;

	Zdebug(100, "ASYNC: Thread active...\n");
	for (;;) {
		g_system->delayMillis(10); // Sleep(10);
		rm->hResManMutex->lock(); // if ( WaitForSingleObject(rm->hResManMutex,INFINITE)!=WAIT_TIMEOUT )
		{
			if (rm->async_loading == 1) {
				z = rm->async_data.zipped;
				fn = new int8[1 + strlen(rm->async_data.fn)];
				strcpy((char *)fn, rm->async_data.fn);
				p = rm->async_data.p;
				size = rm->async_data.size;

				rm->hResManMutex->unlock(); // ReleaseMutex(rm->hResManMutex);

				if (z)
					memUncompress(p, (const char *)fn); // TODO: Use wrapCompressedStream to solve?
				else {
					uint32 timer = g_system->getMillis();
					in = openDiskFileForBinaryRead((char *)fn);
					newSize = size;
					while (newSize != 0) {
						if (newSize > 1024) {
							in->read(p, 1024);
							p += 1024;
							newSize -= 1024;
						} else {
							in->read(p, newSize);
							p += newSize;
							newSize = 0;
						}
					}
					in->close();
					delete in;

					timer = g_system->getMillis() - timer;
					Zdebug(100, "%d", timer);
				}

				rm->hResManMutex->lock(); // WaitForSingleObject(rm->hResManMutex,INFINITE);
				rm->async_loading = 0;
				rm->async_done = 1;
				rm->hResManMutex->unlock(); // ReleaseMutex(rm->hResManMutex);

			} else {
				rm->hResManMutex->unlock(); // ReleaseMutex(rm->hResManMutex);

				//				if (SDL_TryLockMutex(rm->hRunMutex) != SDL_MUTEX_TIMEDOUT) { //if ( WaitForSingleObject(rm->hRunMutex,0)!=WAIT_TIMEOUT
				//)
				rm->hRunMutex->lock(); // TODO: Fix this.
				{
					if (fn)
						delete[] fn;

					Zdebug("ASYNC: thread ending...\n");
					return 0;
				}
			}
		}
	}

	return 0;
}

void res_man::OpenAsync() {
	Zdebug("starting ASYNC");
	hasThread = 1;
	// hRunMutex=CreateMutex(NULL,TRUE,NULL);
	hRunMutex = new Common::Mutex();
	hRunMutex->lock();

	// hResManMutex=CreateMutex(NULL,FALSE,NULL);
	hResManMutex = new Common::Mutex();

	//_beginthread(async_loadThread,0,this);
	warning("TODO: Fix threading");
#if 0
	hThread = NULL;
	//hThread = SDL_CreateThread(async_loadThread, "async_loadThread", (void *)this);
	if (!hThread) {
		printf("ERROR: res_man::OpenAsync() Couldn't create thread");
	}
#endif
	async_loading = 0;
	async_done = 0;
}

void res_man::CloseAsync() {
	// close async thread.
	if (hasThread) {
		Zdebug("ASYNC: shutting down\n");

		hRunMutex->unlock(); // ReleaseMutex(hRunMutex);
		// SDL_Delay(50);    //Sleep(50);

		warning("TODO: Fix threading");
#if 0
		//wait for thread to finish
		SDL_WaitThread(hThread, NULL);
		hThread = NULL;
#endif
		delete hRunMutex; // CloseHandle(hRunMutex);
		delete hResManMutex; // CloseHandle(hResManMutex);
	}
}

// Shift array (might not be needed soon)
async_PacketType res_man::async_shiftArray() {
	async_PacketType top;
	rcActArray<async_PacketType> newArray;

	if (async_fnArray.GetNoItems() == 0)
		return top;

	top = async_fnArray[0];

	uint i;

	for (i = 1; i < async_fnArray.GetNoItems(); i++) {
		newArray.Add(async_fnArray[i]);
	}

	async_fnArray.Reset();

	for (i = 0; i < newArray.GetNoItems(); i++) {
		async_fnArray.Add(newArray[i]);
	}

	return top;
}

// Add item to list
void res_man::async_addFile(const int8 *fn, uint8 *p, int32 size, int32 zipped, int32 memListNo) {
	async_PacketType a;
	a.fn = (char *)const_cast<int8 *>(fn);
	a.p = p;
	a.size = size;
	a.zipped = zipped;
	a.memListNo = memListNo;
	async_fnArray.Add(a);
}

// Pass information to thread to start loading
void res_man::async_setLoading(async_PacketType s) {
	hResManMutex->lock(); // WaitForSingleObject(hResManMutex,INFINITE);
	async_loading = 1;
	async_data = s;
	hResManMutex->unlock(); // ReleaseMutex(hResManMutex);
}

// Check for new files
int32 res_man::async_checkArray() {
	int32 i;

	i = async_fnArray.GetNoItems();

	if (i == 0)
		return i;

	//if (SDL_TryLockMutex(hResManMutex) != SDL_MUTEX_TIMEDOUT) { //if ( WaitForSingleObject(hResManMutex,0)!=WAIT_TIMEOUT )
	// TODO: Fix this
	hResManMutex->lock();
	{
		if (async_done == 1) {
			async_done = 0;
			hResManMutex->unlock(); // ReleaseMutex(hResManMutex);
			async_shiftArray();
			i--;
			RegisterAsync(async_data.memListNo);
			return i;
		} else if ((i > 0) && (async_loading == 0)) {
			hResManMutex->unlock(); // ReleaseMutex(hResManMutex);
			async_setLoading(async_fnArray[0]);
		} else
			hResManMutex->unlock(); // ReleaseMutex(hResManMutex);
	}
	return i;
}

// Flush out everything
void res_man::async_flush() {
	Zdebug("ASYNC: flushing (%d items)\n", async_fnArray.GetNoItems());
	while (async_checkArray() != 0)
		;
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

void res_man::RegisterAsync(const int32) {
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
	if (hn == NULL)
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

	if (hn == NULL)
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
		params->_stream = NULL;

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

		if (params->_stream == NULL)
			Fatal_error("Res_open cannot *OPEN* cluster file %s", clusterPath.c_str());

		// Read in 16 bytes, part of which is the cluster header length

		uint32 data[4];

		if (params->_stream->read(&data, 16) != 16) {
			Fatal_error("res_man::OpenFile cannot read 16 bytes from cluster %s %d", clusterPath.c_str(), params->cluster_hash);
		}

		params->seekpos = 0;
		params->len = data[2];
		return params->cluster;
	}

	HEADER_NORMAL *hn = GetFileHeader(cluster_search, params);
	if (hn == NULL) {
		// Big error the file wasn't found in the cluster
		Fatal_error("res_man::OpenFile couldn't find url %X in cluster %s %X", params->url_hash, params->cluster, params->cluster_hash);
	}

	// This has to be done here because GetFileHeader can read in data which closes the file
	// whose handle is stored in params->fh

	// Open up the cluster
	params->_stream = openDiskFileForBinaryStreamRead(clusterPath.c_str());
	Tdebug("clusters.txt", "  open cluster file %s handle %x", clusterPath.c_str(), params->_stream);

	if (params->_stream == NULL)
		Fatal_error("Res_open cannot *OPEN* cluster file %s", clusterPath.c_str());

	params->seekpos = hn->offset;

	// Set the length of the data
	if (params->zipped) {
		params->_stream->seek(params->seekpos, SEEK_SET);
		params->len = fileGetZipLength2(params->_stream); // TODO: Use wrapCompressedStream to solve?
	} else
		params->len = hn->size;

	return NULL;
}

// Get the header infomation for a particular file from a cluster
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
	if ((clu->schema != CLUSTER_API_SCHEMA) || (*(int *)clu->ID != *(int *)const_cast<char *>(CLUSTER_API_ID))) {
		// Big error unknown cluster filetype
		Fatal_error("res_man::GetFileHeader unknown cluster schema or ID %d %s for %s::0x%X", clu->schema, clu->ID, params->cluster, params->url_hash);
	}

	if (clu->ho.cluster_hash != params->cluster_hash) {
		// Big error this cluster has a different internal hash value to the one
		// we are looking for
//		Fatal_error("res_man::GetFileHeader different internal cluster_hash value %x file %x for %s::0x%X", params->cluster_hash, clu->ho.cluster_hash, params->cluster,
//		            params->url_hash);
	}

	HEADER_NORMAL *hn = clu->hn;
	uint32 i;
	for (i = 0; i < clu->ho.noFiles; hn++, i++) {
		// Hey it has been found
		if (hn->hash == params->url_hash)
			break;
	}

	// Check that the file was actually found
	if (i == clu->ho.noFiles) {
		return NULL;
	}

	return hn;
}

void res_man::Res_open_cluster(const char * /* cluster_url */, uint32 & /* cluster_hash */, int32 /* size */) { Fatal_error("Res_open_cluster not supported on the pc"); }

void res_man::Res_open_mini_cluster(const char *cluster_url, uint32 &cluster_hash, const char *fake_cluster_url, uint32 &fake_cluster_hash) {
	// open the mini-cluster

	uint32 zeroHash = 0;
	Cluster_API *clu = (Cluster_API *)Res_open(NULL, zeroHash, cluster_url, cluster_hash);

	int numFiles = clu->ho.noFiles;

	// check none of the fake files exist
	// also find total size required

	int32 mem_needed = 0;
	int i;

	for (i = 0; i < numFiles; i++) {
		HEADER_NORMAL *hn = clu->hn + i;
		uint32 check_hash = hn->hash;

		if (FindFile(check_hash, fake_cluster_hash, MAKE_TOTAL_HASH(fake_cluster_hash, check_hash)) != -1) {
			warning("File %s::%08x exists in res_man so can't load my mini-cluster!", fake_cluster_url, check_hash);
			return;
		}

		int32 fileSize = (hn->size + 7) & (~7);
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

	clu = (Cluster_API *)Res_open(NULL, zeroHash, cluster_url, cluster_hash);

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
	stream->seek((clu->hn)->offset, SEEK_SET);

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

		mem_list[mem_block].url_hash = hn->hash;
		mem_list[mem_block].cluster_hash = fake_cluster_hash;
		mem_list[mem_block].total_hash = MAKE_TOTAL_HASH(fake_cluster_hash, hn->hash);

		mem_list[mem_block].ad = ad;

		mem_list[mem_block].state = MEM_in_use;
		mem_list[mem_block].protect = 0;
		mem_list[mem_block].age = current_time_frame;

		// adjusted size
		int32 fileSize = (hn->size + 7) & (~7);

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
