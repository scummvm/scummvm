/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011, 2012, 2013, 2014 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//#include <cstring>
#include "ROMInfo.h"

namespace MT32Emu {

static const ROMInfo *getKnownROMInfoFromList(unsigned int index) {
	static const ControlROMFeatureSet MT32_COMPATIBLE(true, true);
	static const ControlROMFeatureSet CM32L_COMPATIBLE(false, false);

	// Known ROMs
	static const ROMInfo CTRL_MT32_V1_04 = {65536, "5a5cb5a77d7d55ee69657c2f870416daed52dea7", ROMInfo::Control, "ctrl_mt32_1_04", "MT-32 Control v1.04", ROMInfo::Full, NULL, &MT32_COMPATIBLE};
	static const ROMInfo CTRL_MT32_V1_05 = {65536, "e17a3a6d265bf1fa150312061134293d2b58288c", ROMInfo::Control, "ctrl_mt32_1_05", "MT-32 Control v1.05", ROMInfo::Full, NULL, &MT32_COMPATIBLE};
	static const ROMInfo CTRL_MT32_V1_06 = {65536, "a553481f4e2794c10cfe597fef154eef0d8257de", ROMInfo::Control, "ctrl_mt32_1_06", "MT-32 Control v1.06", ROMInfo::Full, NULL, &MT32_COMPATIBLE};
	static const ROMInfo CTRL_MT32_V1_07 = {65536, "b083518fffb7f66b03c23b7eb4f868e62dc5a987", ROMInfo::Control, "ctrl_mt32_1_07", "MT-32 Control v1.07", ROMInfo::Full, NULL, &MT32_COMPATIBLE};
	static const ROMInfo CTRL_MT32_BLUER = {65536, "7b8c2a5ddb42fd0732e2f22b3340dcf5360edf92", ROMInfo::Control, "ctrl_mt32_bluer", "MT-32 Control BlueRidge", ROMInfo::Full, NULL, &MT32_COMPATIBLE};

	static const ROMInfo CTRL_CM32L_V1_00 = {65536, "73683d585cd6948cc19547942ca0e14a0319456d", ROMInfo::Control, "ctrl_cm32l_1_00", "CM-32L/LAPC-I Control v1.00", ROMInfo::Full, NULL, &CM32L_COMPATIBLE};
	static const ROMInfo CTRL_CM32L_V1_02 = {65536, "a439fbb390da38cada95a7cbb1d6ca199cd66ef8", ROMInfo::Control, "ctrl_cm32l_1_02", "CM-32L/LAPC-I Control v1.02", ROMInfo::Full, NULL, &CM32L_COMPATIBLE};

	static const ROMInfo PCM_MT32 = {524288, "f6b1eebc4b2d200ec6d3d21d51325d5b48c60252", ROMInfo::PCM, "pcm_mt32", "MT-32 PCM ROM", ROMInfo::Full, NULL, NULL};
	static const ROMInfo PCM_CM32L = {1048576, "289cc298ad532b702461bfc738009d9ebe8025ea", ROMInfo::PCM, "pcm_cm32l", "CM-32L/CM-64/LAPC-I PCM ROM", ROMInfo::Full, NULL, NULL};

	static const ROMInfo * const ROM_INFOS[] = {
		&CTRL_MT32_V1_04,
		&CTRL_MT32_V1_05,
		&CTRL_MT32_V1_06,
		&CTRL_MT32_V1_07,
		&CTRL_MT32_BLUER,
		&CTRL_CM32L_V1_00,
		&CTRL_CM32L_V1_02,
		&PCM_MT32,
		&PCM_CM32L,
		NULL};

	return ROM_INFOS[index];
}

const ROMInfo* ROMInfo::getROMInfo(Common::File *file) {
	size_t fileSize = file->size();
	Common::String fileName = file->getName();
	fileName.toUppercase();
	bool isCM32LROM = fileName.hasPrefix("CM32L_");
	// We haven't added the SHA1 checksum code in ScummVM, as the file size
	// and ROM name suffices for our needs for now.
	//const char *fileDigest = file->getSHA1();
	for (int i = 0; getKnownROMInfoFromList(i) != NULL; i++) {
		const ROMInfo *romInfo = getKnownROMInfoFromList(i);
		if (fileSize == romInfo->fileSize /*&& !strcmp(fileDigest, romInfo->sha1Digest)*/) {
			if (fileSize == 65536) {
				// If we are looking for a CM-32L ROM, make sure we return the first matching
				// CM-32L ROM from the list, instead of the first matching MT-32 ROM
				if (isCM32LROM && romInfo->controlROMFeatures->isDefaultReverbMT32Compatible())
					continue;
			}
			return romInfo;
		}
	}
	return NULL;
}

void ROMInfo::freeROMInfo(const ROMInfo *romInfo) {
	(void) romInfo;
}

static int getROMCount() {
	int count;
	for(count = 0; getKnownROMInfoFromList(count) != NULL; count++) {
	}
	return count;
}

const ROMInfo** ROMInfo::getROMInfoList(unsigned int types, unsigned int pairTypes) {
	const ROMInfo **romInfoList = new const ROMInfo*[getROMCount() + 1];
	const ROMInfo **currentROMInList = romInfoList;
	for(int i = 0; getKnownROMInfoFromList(i) != NULL; i++) {
		const ROMInfo *romInfo = getKnownROMInfoFromList(i);
		if ((types & (1 << romInfo->type)) && (pairTypes & (1 << romInfo->pairType))) {
			*currentROMInList++ = romInfo;
		}
	}
	*currentROMInList = NULL;
	return romInfoList;
}

void ROMInfo::freeROMInfoList(const ROMInfo **romInfoList) {
	delete[] romInfoList;
}

const ROMImage* ROMImage::makeROMImage(Common::File *file) {
	ROMImage *romImage = new ROMImage;
	romImage->file = file;
	romImage->romInfo = ROMInfo::getROMInfo(romImage->file);
	return romImage;
}

void ROMImage::freeROMImage(const ROMImage *romImage) {
	ROMInfo::freeROMInfo(romImage->romInfo);
	delete romImage;
}

Common::File* ROMImage::getFile() const {
	return file;
}

const ROMInfo* ROMImage::getROMInfo() const {
	return romInfo;
}

ControlROMFeatureSet::ControlROMFeatureSet(bool useDefaultReverbMT32Compatible, bool useOldMT32AnalogLPF) :
	defaultReverbMT32Compatible(useDefaultReverbMT32Compatible),
	oldMT32AnalogLPF(useOldMT32AnalogLPF)
{}

bool ControlROMFeatureSet::isDefaultReverbMT32Compatible() const {
	return defaultReverbMT32Compatible;
}

bool ControlROMFeatureSet::isOldMT32AnalogLPF() const {
	return oldMT32AnalogLPF;
}

}
