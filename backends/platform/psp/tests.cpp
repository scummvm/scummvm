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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/psp/osys_psp.cpp $
 * $Id: osys_psp.cpp 46126 2009-11-24 14:18:46Z fingolfin $
 *
 */

#include "common/scummsys.h"
#include <pspiofilemgr_fcntl.h>
#include <pspiofilemgr_stat.h>
#include <pspiofilemgr.h>
#include <pspthreadman.h>
#include <pspsdk.h>
#include <psprtc.h>
#include <stdlib.h>
#include <stdio.h>
#include <psputils.h>
#include "backends/platform/psp/rtc.h"
#include "backends/platform/psp/thread.h"
#include "backends/platform/psp/memory.h"


#define UNCACHED(x)		((byte *)(((uint32)(x)) | 0x40000000))	/* make an uncached access */
#define CACHED(x)		((byte *)(((uint32)(x)) & 0xBFFFFFFF))	/* make an uncached access into a cached one */
 
//#define __PSP_DEBUG_FUNCS__
//#define __PSP_DEBUG_PRINT__
 
// Results: (333Mhz/222Mhz)
// Getting a tick: 1-2 us
// Getting a time structure: 9/14us
// ie. using a tick and just dividing by 1000 saves us time.
  
#include "backends/platform/psp/trace.h" 

void test_ticks() {
	uint32 ticksPerSecond = sceRtcGetTickResolution();
	PSP_INFO_PRINT("ticksPerSecond[%d]\n", ticksPerSecond);

	uint32 currentTicks1[2];
	uint32 currentTicks2[2];
	
	sceRtcGetCurrentTick((u64 *)currentTicks1);
	sceRtcGetCurrentTick((u64 *)currentTicks2);
	PSP_INFO_PRINT("current tick[%x %x][%u %u]\n", currentTicks1[0], currentTicks1[1], currentTicks1[0], currentTicks1[1]);
	PSP_INFO_PRINT("current tick[%x %x][%u %u]\n", currentTicks2[0], currentTicks2[1], currentTicks2[0], currentTicks2[1]);
	
	pspTime time;
	sceRtcSetTick(&time, (u64 *)currentTicks2);
	PSP_INFO_PRINT("current tick in time, year[%d] month[%d] day[%d] hour[%d] minutes[%d] seconds[%d] us[%d]\n", time.year, time.month, time.day, time.hour, time.minutes, time.seconds, time.microseconds);	
	
	pspTime time1;
	pspTime time2;
	sceRtcGetCurrentClockLocalTime(&time1);
	sceRtcGetCurrentClockLocalTime(&time2);
	PSP_INFO_PRINT("time1, year[%d] month[%d] day[%d] hour[%d] minutes[%d] seconds[%d] us[%d]\n", time1.year, time1.month, time1.day, time1.hour, time1.minutes, time1.seconds, time1.microseconds);
	PSP_INFO_PRINT("time2, year[%d] month[%d] day[%d] hour[%d] minutes[%d] seconds[%d] us[%d]\n", time2.year, time2.month, time2.day, time2.hour, time2.minutes, time2.seconds, time2.microseconds);
}

void test_getMicros() {
	uint32 time1, time2, time3, time4;
	time1 = PspRtc::instance().getMicros();
	time2 = PspRtc::instance().getMicros();
	time3 = PspRtc::instance().getMicros();
	time4 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("getMicros() times: %d, %d, %d\n", time4-time3, time3-time2, time2-time1);
}

void read_and_time(uint32 bytes, char *buffer, FILE *file) {
	uint32 time1 = PspRtc::instance().getMicros();
	// test minimal read
	fread(buffer, bytes, 1, file);
	uint32 time2 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("Reading %d byte takes %dus\n", bytes, time2-time1);	
}

/*
	333MHz/222MHz
	Reading 1 byte takes 2590us / 3167
	Reading 10 byte takes 8us / 9
	Reading 50 byte takes 8us / 11
	Reading 100 byte takes 8us / 11
	Reading 1000 byte takes 915us / 1131
	Reading 1000 byte takes 911us
	Reading 1000 byte takes 914us
	Reading 1000 byte takes 943us
	Reading 1000 byte takes 915us
	Reading 1000 byte takes 923us
	Reading 1000 byte takes 22us
	Reading 1000 byte takes 916us 
	Reading 1000 byte takes 913us / 1,120
	Reading 1000 byte takes 909us / 1,122
	Reading 2000 byte takes 1806us / 2,284
	Reading 3000 byte takes 2697us / 3,374
	Reading 4000 byte takes 3652us / 4,592
	Reading 5000 byte takes 4551us / 5,544
	Reading 6000 byte takes 5356us / 6,676
	Reading 7000 byte takes 6800us / 8,358
	Reading 8000 byte takes 6794us / 8,454
	Reading 9000 byte takes 6782us / 8,563
	Reading 10000 byte takes 8497us / 10,631
	Reading 30000 byte takes 25995us / 32,473
	Reading 50000 byte takes 42467us / 52,893
	Reading 80000 byte takes 68457us / 85,291
	Reading 100000 byte takes 85103us / 106,163
	Reading 500000 byte takes 427337us / 531,679
	Reading 1000000 byte takes 854831us / 1,063,107
*/
// Function to test the impact of MS reads
// These tests can't be done from shell - the cache screws them up
void test_ms_reads() {
	FILE *file;
	file = fopen("ms0:/psp/music/track1.mp3", "r");

	char *buffer = (char *)malloc(2 * 1024 * 1024);
	
	read_and_time(1, buffer, file);
	read_and_time(10, buffer, file);
	read_and_time(50, buffer, file);
	read_and_time(100, buffer, file);
	for (int i = 0; i< 10; i++)
		read_and_time(1000, buffer, file);
	read_and_time(2000, buffer, file);
	read_and_time(3000, buffer, file);
	read_and_time(4000, buffer, file);
	read_and_time(5000, buffer, file);
	read_and_time(6000, buffer, file);
	read_and_time(7000, buffer, file);
	read_and_time(8000, buffer, file);
	read_and_time(9000, buffer, file);	
	read_and_time(10000, buffer, file);
	read_and_time(30000, buffer, file);
	read_and_time(50000, buffer, file);
	read_and_time(80000, buffer, file);
	read_and_time(100000, buffer, file);
	read_and_time(500000, buffer, file);
	read_and_time(1000000, buffer, file);	
	
	fclose(file);
	free(buffer);	
}
 
void seek_and_time(int bytes, int origin, FILE *file) {
	char buffer[1000];
	
	uint32 time1 = PspRtc::instance().getMicros();
	// test minimal read
	fseek(file, bytes, origin);
	uint32 time2 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("Seeking %d byte from %d took %dus\n", bytes, origin, time2-time1);

	time1 = PspRtc::instance().getMicros();
	// test minimal read
	fread(buffer, 1000, 1, file);
	time2 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("Reading 1000 bytes took %dus\n", time2-time1);
}

/*
333MHz
Seeking 0 byte from 0 took 946us
Reading 1000 bytes took 1781us
Seeking 5 byte from 0 took 6us
Reading 1000 bytes took 19us
Seeking 1000 byte from 0 took 5us
Reading 1000 bytes took 913us
Seeking 100 byte from 0 took 955us
Reading 1000 bytes took 906us
Seeking 10000 byte from 0 took 963us
Reading 1000 bytes took 905us
Seeking -5 byte from 1 took 1022us
Reading 1000 bytes took 949us
Seeking -100 byte from 1 took 1040us
Reading 1000 bytes took 907us
Seeking 100 byte from 1 took 1044us
Reading 1000 bytes took 930us
Seeking 0 byte from 2 took 7211us
Reading 1000 bytes took 80us
Seeking 10000 byte from 2 took 3636us
Reading 1000 bytes took 110us
*/

void test_seeks() {
	FILE *file;
	file = fopen("ms0:/psp/music/track1.mp3", "r");

	seek_and_time(0, SEEK_SET, file);
	seek_and_time(5, SEEK_SET, file);
	seek_and_time(1000, SEEK_SET, file);
	seek_and_time(100, SEEK_SET, file);
	seek_and_time(10000, SEEK_SET, file);
	seek_and_time(-5, SEEK_CUR, file);
	seek_and_time(-100, SEEK_CUR, file);
	seek_and_time(100, SEEK_CUR, file);
	seek_and_time(0, SEEK_END, file);
	seek_and_time(-10000, SEEK_END, file);

	fclose(file);
}

// 222: 5-7us
int testGetThreadId() {
	uint32 time1 = PspRtc::instance().getMicros();
	int threadId = sceKernelGetThreadId();
	uint32 time2 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("Getting thread ID %d took %dus\n", threadId, time2-time1);
	
	return threadId;
}

// 222: 4-5us
void testGetPriority() {
	uint32 time1 = PspRtc::instance().getMicros();
	int priority = sceKernelGetThreadCurrentPriority();
	uint32 time2 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("Getting thread priority %d took %dus\n", priority, time2-time1);
}

// 222: 9-10us
void testChangePriority(int id, int priority) {
	uint32 time1 = PspRtc::instance().getMicros();
	sceKernelChangeThreadPriority(id, priority);
	uint32 time2 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("Changing thread priority to %d for id %d took %dus\n", priority, id, time2-time1);
}

void test_thread() {
	int id;
	id = testGetThreadId();
	testGetThreadId();
	testGetPriority();
	testGetPriority();
	testChangePriority(id, 30);
	testChangePriority(id, 35);
	testChangePriority(id, 25);
	
	// test context switch time
	for (int i=0; i<10; i++) {
		uint time1 = PspRtc::instance().getMicros();
		PspThread::delayMicros(0);
		uint time2 = PspRtc::instance().getMicros();
		PSP_INFO_PRINT("poll %d. context switch Time = %dus\n", i, time2-time1);	// 10-15us	
	}	
}
 
int test_mem_read(uint32 words) {
	uint32 __attribute__((aligned(64))) buffer[1024];
	register uint32 r = 0;
	
	// uncached
	uint32 *pBuffer = (uint32 *)UNCACHED(buffer);

	uint32 time1 = PspRtc::instance().getMicros();
	
	for (int i=0; i<words; i++) {
		r += *pBuffer;
		pBuffer++;
	}
	uint32 time2 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("read of %d words from uncached took %dus\n", words, time2-time1);
	
	// flush cache
	sceKernelDcacheWritebackInvalidateAll();
	
	// cached
	pBuffer = buffer;

	time1 = PspRtc::instance().getMicros();
	
	for (int i=0; i<words; i++) {
		r += *pBuffer;
		pBuffer++;
	}
	time2 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("read of %d words from cached took %dus\n", words, time2-time1);

	return r;
} 

void test_mem_write(uint32 words) {
	uint32 __attribute__((aligned(64))) buffer[1024];
	register uint32 r = 0;
	
	// uncached
	uint32 *pBuffer = (uint32 *)UNCACHED(buffer);

	uint32 time1 = PspRtc::instance().getMicros();
	
	for (int i=0; i<words; i++) {
		*pBuffer = r;
		pBuffer++;
	}
	uint32 time2 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("write of %d words to uncached took %dus\n", words, time2-time1);
	
	// flush cache
	sceKernelDcacheWritebackInvalidateAll();

	// cached
	pBuffer = buffer;
	
	time1 = PspRtc::instance().getMicros();
	
	for (int i=0; i<words; i++) {
		*pBuffer = r;
		pBuffer++;
	}
	time2 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("writeof %d words to cached took %dus\n", words, time2-time1);	
}

/*
read of 1 words from uncached took 2us
read of 1 words from cached took 3us
read of 64 words from uncached took 12us
read of 64 words from cached took 6us
read of 65 words from uncached took 12us
read of 65 words from cached took 6us
read of 128 words from uncached took 24us
read of 128 words from cached took 8us
read of 1024 words from uncached took 168us
read of 1024 words from cached took 50us
write of 1 words to uncached took 2us
writeof 1 words to cached took 2us
write of 64 words to uncached took 4us
writeof 64 words to cached took 5us
write of 65 words to uncached took 5us
writeof 65 words to cached took 5us
write of 128 words to uncached took 5us
writeof 128 words to cached took 8us
write of 1024 words to uncached took 32us
writeof 1024 words to cached took 45us
*/ 
void test_memory() {
	test_mem_read(1);
	test_mem_read(64);
	test_mem_read(65);
	test_mem_read(128);
	test_mem_read(1024);
	test_mem_write(1);
	test_mem_write(64);
	test_mem_write(65);
	test_mem_write(128);
	test_mem_write(1024);	
}

void test_semaphore() { 
	// create semaphore
	int sem = sceKernelCreateSema("testSemaphore", 0, 1, 1, 0);
	
	uint32 time1 = PspRtc::instance().getMicros();
	
	sceKernelWaitSema(sem, 1, 0);
	
	uint32 time2 = PspRtc::instance().getMicros();
	
	PSP_INFO_PRINT("taking semaphore took %d us\n", time2-time1);	// 10us
	
	uint32 time3 = PspRtc::instance().getMicros();
	
	sceKernelSignalSema(sem, 1);
	
	uint32 time4 = PspRtc::instance().getMicros();
	PSP_INFO_PRINT("releasing semaphore took %d us\n", time4-time3);	//10us-55us
}

int globalSem = 0;	// semaphore

int threadFunc(SceSize args, void *argp) {
	PSP_INFO_PRINT("thread %x created.\n", sceKernelGetThreadId());
	
	sceKernelWaitSema(globalSem, 1, 0);	// grab semaphore
	PSP_INFO_PRINT("grabbed semaphore. Quitting thread\n");
	
	return 0;
}

void test_semaphore_many_threads() {	
	globalSem = sceKernelCreateSema("testSemaphore2", 0, 0, 255, 0);

	// create 4 threads
	for (int i=0; i<4; i++) {
		int thid = sceKernelCreateThread("my_thread", threadFunc, 0x18, 0x10000, THREAD_ATTR_USER, NULL);
		sceKernelStartThread(thid, 0, 0);
	}
		
	PSP_INFO_PRINT("main thread. created threads\n");

	SceKernelSemaInfo info;
	int waitingThreads = 0;
	while (waitingThreads < 4) {
		sceKernelReferSemaStatus(globalSem, &info);
		waitingThreads = info.numWaitThreads;
		PSP_INFO_PRINT("main thread: waiting threads[%d]\n", waitingThreads);
	}
	
	PSP_INFO_PRINT("main: semaphore value[%d]\n", info.currentCount);
	PSP_INFO_PRINT("main thread: waiting threads[%d]\n", info.numWaitThreads);
	
	sceKernelSignalSema(globalSem, 4);
}

void test_sce_ms_reads() { 
	SceUID file;
	char *buffer = (char *)malloc(2 * 1024 * 1024);
	{
		// get file size
		SceIoStat stat;
		
		uint32 time1 = PspRtc::instance().getMicros();
		
		sceIoGetstat("ms0:/psp/music/track1.mp3", &stat);
		
		uint32 time2 = PspRtc::instance().getMicros();
		
		PSP_INFO_PRINT("getting size of %lld took %d us\n", stat.st_size, time2-time1); // 3180 us before an open
	}
	{
		// open file
		uint32 time1 = PspRtc::instance().getMicros();

		file = sceIoOpen("ms0:/psp/music/track1.mp3", PSP_O_RDONLY, 0777);
		
		uint32 time2 = PspRtc::instance().getMicros();
		
		PSP_INFO_PRINT("opening scefile took %d us\n", time2-time1);	// 3355us (or 1223us if already got size)
	}
	{
		// get file size
		SceIoStat stat;
		
		uint32 time1 = PspRtc::instance().getMicros();
		
		sceIoGetstat("ms0:/psp/music/track1.mp3", &stat);
		
		uint32 time2 = PspRtc::instance().getMicros();
		
		PSP_INFO_PRINT("getting size of %lld took %d us\n", stat.st_size, time2-time1); // 1223us after an open
	}
	{
		uint32 time1 = PspRtc::instance().getMicros();
	
		// asynchronous read
		sceIoReadAsync(file, buffer, 100000);
		
		uint32 time2 = PspRtc::instance().getMicros();
		
		PSP_INFO_PRINT("async read of 100000 took %d us\n", time2-time1);  // 146us
		
		// poll for completion
		SceInt64 result;
		for (int i=0; i<10; i++) {
			sceIoPollAsync(file, &result);
			uint time3 = PspRtc::instance().getMicros();
			PSP_INFO_PRINT("poll %d. result = %lld. Time = %d us.\n", i, result, time3-time1);
			PspThread::delayMicros(100);
		}	
	
	}
	{
		uint32 time1 = PspRtc::instance().getMicros();
		sceIoClose(file);
		uint32 time2 = PspRtc::instance().getMicros();
		PSP_INFO_PRINT("file close took %d us\n", time2-time1);	// 230us
	}
	free(buffer);
}

#define MEMCPY_BUFFER_SIZE 4096

void test_fastCopy_specific(byte *dst, byte *src, uint32 bytes) {
	memset(dst, 0, bytes);
	PspMemory::fastCopy(dst, src, bytes);
	PSP_INFO_PRINT("\n");
}

void test_fastCopy_different_sizes(byte *dst, byte *src) {
	test_fastCopy_specific(dst, src, 1);
	test_fastCopy_specific(dst, src, 2);
	test_fastCopy_specific(dst, src, 4);
	test_fastCopy_specific(dst, src, 6);
	test_fastCopy_specific(dst, src, 8);
	test_fastCopy_specific(dst, src, 9);
	test_fastCopy_specific(dst, src, 16);
	test_fastCopy_specific(dst, src, 17);
	test_fastCopy_specific(dst, src, 32);
	test_fastCopy_specific(dst, src, 33);
	test_fastCopy_specific(dst, src, 34);
	test_fastCopy_specific(dst, src, 35);
	test_fastCopy_specific(dst, src, 36);
}

void test_fastcopy() {
	PSP_INFO_PRINT("running fastcopy test\n");

	uint32 *bufferSrc32 = (uint32 *)memalign(16, MEMCPY_BUFFER_SIZE);
	uint32 *bufferDst32 = (uint32 *)memalign(16, MEMCPY_BUFFER_SIZE);
	
	// fill buffer 1
	for (int i=0; i<MEMCPY_BUFFER_SIZE/4; i++)
		bufferSrc32[i] = i | (((MEMCPY_BUFFER_SIZE/4)-i)<<16);
		
	// print buffer
	for (int i=0; i<50; i++)
		PSP_INFO_PRINT("%x ", bufferSrc32[i]);
	PSP_INFO_PRINT("\n");
	
	byte *bufferSrc = ((byte *)bufferSrc32);
	byte *bufferDst = ((byte *)bufferDst32);
	
	//test_fastCopy_different_sizes(bufferDst, bufferSrc);
	//test_fastCopy_different_sizes(bufferDst+1, bufferSrc+1);
	//test_fastCopy_different_sizes(bufferDst+2, bufferSrc+2);
	//test_fastCopy_different_sizes(bufferDst+3, bufferSrc+3);
	//test_fastCopy_different_sizes(bufferDst, bufferSrc+1);
	//test_fastCopy_different_sizes(bufferDst, bufferSrc+2);
	//test_fastCopy_different_sizes(bufferDst, bufferSrc+3);
	test_fastCopy_different_sizes(bufferDst+1, bufferSrc+2);
	test_fastCopy_different_sizes(bufferDst+1, bufferSrc+3);
	test_fastCopy_different_sizes(bufferDst+2, bufferSrc+1);
	test_fastCopy_different_sizes(bufferDst+2, bufferSrc+3);
	test_fastCopy_different_sizes(bufferDst+3, bufferSrc+1);
	test_fastCopy_different_sizes(bufferDst+3, bufferSrc+2);	
	
	free(bufferSrc32);
	free(bufferDst32);
}

void test_fastcopy_speed_specific(byte *dst, byte *src, uint32 bytes) {
	byte *dstUncached = UNCACHED(dst);
	byte *srcUncached = UNCACHED(src);
//	byte *dstUncached = dst;
//	byte *srcUncached = src;
	
	uint32 time1, time2;
	const int iterations = 2000;
	int intc;
	
	intc = pspSdkDisableInterrupts();
	
	time1 = PspRtc::instance().getMicros();
	for (int i=0; i<iterations; i++) {
		PspMemory::fastCopy(dstUncached, srcUncached, bytes);
	}	
	time2 = PspRtc::instance().getMicros();
	
	pspSdkEnableInterrupts(intc);
	
	PSP_INFO_PRINT("fastcppy() results: bytes[%d], iters[%d], time[%d]us\n",
		bytes, iterations, time2-time1);
	
	intc = pspSdkDisableInterrupts();
	
	time1 = PspRtc::instance().getMicros();
	for (int i=0; i<iterations; i++) {
		memcpy(dstUncached, srcUncached, bytes);
	}	
	time2 = PspRtc::instance().getMicros();
	
	pspSdkEnableInterrupts(intc);
	
	PSP_INFO_PRINT("memcpy() results: bytes[%d], iters[%d], time[%d]us\n\n",
		bytes, iterations, time2-time1);

}

void test_fastcopy_speed_bunch(byte *dst, byte *src) {
	PSP_INFO_PRINT("new test bunch ******************\n");
	PSP_INFO_PRINT("src[%p], dst[%p]\n", src, dst);
	test_fastcopy_speed_specific(dst, src, 1);
	test_fastcopy_speed_specific(dst, src, 2);
	test_fastcopy_speed_specific(dst, src, 3);
	test_fastcopy_speed_specific(dst, src, 4);
	test_fastcopy_speed_specific(dst, src, 5);
	test_fastcopy_speed_specific(dst, src, 8);
	test_fastcopy_speed_specific(dst, src, 10);
	test_fastcopy_speed_specific(dst, src, 16);
	test_fastcopy_speed_specific(dst, src, 32);
	test_fastcopy_speed_specific(dst, src, 50);
	test_fastcopy_speed_specific(dst, src, 100);
	test_fastcopy_speed_specific(dst, src, 500);
	test_fastcopy_speed_specific(dst, src, 1024);
	test_fastcopy_speed_specific(dst, src, 2048);
}

void test_fastcopy_speed() {
	PSP_INFO_PRINT("running fastcopy speed test\n");

	uint32 *bufferSrc32 = (uint32 *)memalign(16, MEMCPY_BUFFER_SIZE);
	uint32 *bufferDst32 = (uint32 *)memalign(16, MEMCPY_BUFFER_SIZE);
	
	// fill buffer 1
	for (int i=0; i<MEMCPY_BUFFER_SIZE/4; i++)
		bufferSrc32[i] = i | (((MEMCPY_BUFFER_SIZE/4)-i)<<16);
		
	// print buffer
	for (int i=0; i<50; i++)
		PSP_INFO_PRINT("%x ", bufferSrc32[i]);
	PSP_INFO_PRINT("\n");
	
	byte *bufferSrc = ((byte *)bufferSrc32);
	byte *bufferDst = ((byte *)bufferDst32);
	
	test_fastcopy_speed_bunch(bufferDst, bufferSrc);
	test_fastcopy_speed_bunch(bufferDst+1, bufferSrc+1);
	test_fastcopy_speed_bunch(bufferDst, bufferSrc+1);
	test_fastcopy_speed_bunch(bufferDst+1, bufferSrc);
	
	free(bufferSrc32);
	free(bufferDst32);
}


 void tests() {
	PSP_INFO_PRINT("in tests\n");
	//test_ticks();
	//test_getMicros();
	//test_ms_reads();
	//test_seeks();
	//test_thread();
	//test_memory();
	//test_sce_ms_reads();
	//test_semaphore();	
	//test_semaphore_many_threads();
	//test_fastcopy();
	test_fastcopy_speed();
}	
