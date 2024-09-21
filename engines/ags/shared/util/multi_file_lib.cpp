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

#include "ags/shared/util/bbop.h"
#include "ags/shared/util/multi_file_lib.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

namespace MFLUtil {
const char *HeadSig = "CLIB\x1a";
const char *TailSig = "CLIB\x1\x2\x3\x4SIGE";

static const size_t SingleFilePswLen = 13;

static const size_t MaxAssetFileLen = 100;
static const size_t MaxDataFileLen = 50;
static const size_t V10LibFileLen = 20;
static const size_t V10AssetFileLen = 25;

static const int EncryptionRandSeed = 9338638;
static const char *EncryptionString = "My\x1\xde\x4Jibzle";

MFLError ReadSigsAndVersion(Stream *in, MFLVersion *p_lib_version, soff_t *p_abs_offset);
MFLError ReadSingleFileLib(AssetLibInfo &lib, Stream *in);
MFLError ReadMultiFileLib(AssetLibInfo &lib, Stream *in, MFLVersion lib_version);
MFLError ReadV10(AssetLibInfo &lib, Stream *in, MFLVersion lib_version);
MFLError ReadV20(AssetLibInfo &lib, Stream *in);
MFLError ReadV21(AssetLibInfo &lib, Stream *in);
MFLError ReadV30(AssetLibInfo &lib, Stream *in, MFLVersion lib_version);

void     WriteV30(const AssetLibInfo &lib, Stream *out);

// Encryption / decryption
int      GetNextPseudoRand(int &rand_val);
void     DecryptText(char *text);
void     ReadEncArray(void *data, size_t size, size_t count, Stream *in, int &rand_val);
int8_t   ReadEncInt8(Stream *in, int &rand_val);
int32_t  ReadEncInt32(Stream *in, int &rand_val);
void     ReadEncString(char *buffer, size_t max_len, Stream *in, int &rand_val);
} // namespace MFLUtil


MFLUtil::MFLError MFLUtil::TestIsMFL(Stream *in, bool test_is_main) {
	MFLVersion lib_version;
	MFLError err = ReadSigsAndVersion(in, &lib_version, nullptr);
	if (err == kMFLNoError) {
		if (lib_version >= kMFLVersion_MultiV10 && test_is_main) {
			// this version supports multiple data files, check if it is the first one
			if (in->ReadInt8() != 0)
				return kMFLErrNoLibBase; // not first datafile in chain
		}
	}
	return err;
}

MFLUtil::MFLError MFLUtil::ReadHeader(AssetLibInfo &lib, Stream *in) {
	MFLVersion lib_version;
	soff_t abs_offset;
	MFLError err = ReadSigsAndVersion(in, &lib_version, &abs_offset);
	if (err != kMFLNoError)
		return err;

	if (lib_version >= kMFLVersion_MultiV10) {
		// read newer clib versions (versions 10+)
		err = ReadMultiFileLib(lib, in, lib_version);
	} else {
		// read older clib versions (versions 1 to 9)
		err = ReadSingleFileLib(lib, in);
	}

	// apply absolute offset for the assets contained in base data file
	// (since only base data file may be EXE file, other clib parts are always on their own)
	if (abs_offset > 0) {
		for (auto &asset : lib.AssetInfos) {
			if (asset.LibUid == 0)
				asset.Offset += abs_offset;
		}
	}
	return err;
}

MFLUtil::MFLError MFLUtil::ReadSigsAndVersion(Stream *in, MFLVersion *p_lib_version, soff_t *p_abs_offset) {
	soff_t abs_offset = 0; // library offset in this file
	String sig;
	// check multifile lib signature at the beginning of file
	sig.ReadCount(in, strlen(HeadSig));
	if (sig.Compare(HeadSig) != 0) {
		// signature not found, check signature at the end of file
		in->Seek(-(soff_t)strlen(TailSig), kSeekEnd);
		// by definition, tail marks the max absolute offset value
		auto tail_abs_offset = in->GetPosition();
		sig.ReadCount(in, strlen(TailSig));
		// signature not found, return error code
		if (sig.Compare(TailSig) != 0)
			return kMFLErrNoLibSig;

		// it's an appended-to-end-of-exe thing;
		// now we need to read multifile lib offset value, but we do not know
		// if its 32-bit or 64-bit yet, so we'll have to test both
		in->Seek(-(soff_t)strlen(TailSig) - sizeof(int64_t), kSeekEnd);
		abs_offset = in->ReadInt64();
		in->Seek(-(soff_t)sizeof(int32_t), kSeekCurrent);
		soff_t abs_offset_32 = in->ReadInt32();

		// test for header signature again, with 64-bit and 32-bit offsets if necessary
		if (abs_offset > 0 && abs_offset < (soff_t)(tail_abs_offset - strlen(HeadSig))) {
			in->Seek(abs_offset, kSeekBegin);
			sig.ReadCount(in, strlen(HeadSig));
		}

		// try again with 32-bit offset
		if (sig.Compare(HeadSig) != 0) {
			abs_offset = abs_offset_32;
			if (abs_offset > 0 && abs_offset < (soff_t)(tail_abs_offset - strlen(HeadSig))) {
				in->Seek(abs_offset, kSeekBegin);
				sig.ReadCount(in, strlen(HeadSig));
			}
			if (sig.Compare(HeadSig) != 0) {
				// nope, no luck, bad / unknown format
				return kMFLErrNoLibSig;
			}
		}
	}
	// if we've reached this point we must be right behind the header signature

	// read library header
	MFLVersion lib_version = static_cast<MFLVersion>(in->ReadInt8());
	if ((lib_version != kMFLVersion_SingleLib) && (lib_version != kMFLVersion_MultiV10) &&
	        (lib_version != kMFLVersion_MultiV11) && (lib_version != kMFLVersion_MultiV15) &&
	        (lib_version != kMFLVersion_MultiV20) && (lib_version != kMFLVersion_MultiV21) &&
	        lib_version != kMFLVersion_MultiV30)
		return kMFLErrLibVersion; // unsupported version

	if (p_lib_version)
		*p_lib_version = lib_version;
	if (p_abs_offset)
		*p_abs_offset = abs_offset;
	return kMFLNoError;
}

MFLUtil::MFLError MFLUtil::ReadSingleFileLib(AssetLibInfo &lib, Stream *in) {
	char passwmodifier = in->ReadInt8();
	in->ReadInt8(); // unused byte
	lib.LibFileNames.resize(1); // only one library part
	size_t asset_count = (uint16)in->ReadInt16();
	lib.AssetInfos.resize(asset_count);

	in->Seek(SingleFilePswLen, kSeekCurrent); // skip password dooberry
	char fn_buf[SingleFilePswLen + 1];
	// read information on contents
	for (size_t i = 0; i < asset_count; ++i) {
		in->Read(fn_buf, SingleFilePswLen);
		fn_buf[SingleFilePswLen] = 0;
		for (char *c = fn_buf; *c; ++c)
			*c -= passwmodifier;
		lib.AssetInfos[i].FileName = fn_buf;
		lib.AssetInfos[i].LibUid = 0;
	}
	for (size_t i = 0; i < asset_count; ++i) {
		lib.AssetInfos[i].Size = (uint32)in->ReadInt32();
	}
	in->Seek(2 * asset_count, kSeekCurrent); // skip flags & ratio
	lib.AssetInfos[0].Offset = in->GetPosition();
	// set offsets (assets are positioned in sequence)
	for (size_t i = 1; i < asset_count; ++i) {
		lib.AssetInfos[i].Offset = lib.AssetInfos[i - 1].Offset + lib.AssetInfos[i - 1].Size;
	}
	// return success
	return kMFLNoError;
}

MFLUtil::MFLError MFLUtil::ReadMultiFileLib(AssetLibInfo &lib, Stream *in, MFLVersion lib_version) {
	if (in->ReadInt8() != 0)
		return kMFLErrNoLibBase; // not first datafile in chain

	if (lib_version >= kMFLVersion_MultiV30) {
		// read new clib format with 64-bit files support
		return ReadV30(lib, in, lib_version);
	}
	if (lib_version >= kMFLVersion_MultiV21) {
		// read new clib format with encoding support (versions 21+)
		return ReadV21(lib, in);
	} else if (lib_version == kMFLVersion_MultiV20) {
		// read new clib format without encoding support (version 20)
		return ReadV20(lib, in);
	}
	// read older clib format (versions 10 to 19), the ones with shorter filenames
	return ReadV10(lib, in, lib_version);
}

MFLUtil::MFLError MFLUtil::ReadV10(AssetLibInfo &lib, Stream *in, MFLVersion lib_version) {
	// number of clib parts
	size_t mf_count = (uint32)in->ReadInt32();
	lib.LibFileNames.resize(mf_count);
	// filenames for all clib parts; filenames are only 20 chars long in this format version
	for (size_t i = 0; i < mf_count; ++i) {
		lib.LibFileNames[i].ReadCount(in, V10LibFileLen);
	}

	// number of files in clib
	size_t asset_count = (uint32)in->ReadInt32();
	// read information on clib contents
	lib.AssetInfos.resize(asset_count);
	// filename array is only 25 chars long in this format version
	char fn_buf[V10AssetFileLen];
	for (size_t i = 0; i < asset_count; ++i) {
		in->Read(fn_buf, V10AssetFileLen);
		if (lib_version >= kMFLVersion_MultiV11)
			DecryptText(fn_buf);
		lib.AssetInfos[i].FileName = fn_buf;
	}
	for (size_t i = 0; i < asset_count; ++i)
		lib.AssetInfos[i].Offset = (uint32)in->ReadInt32();
	for (size_t i = 0; i < asset_count; ++i)
		lib.AssetInfos[i].Size = (uint32)in->ReadInt32();
	for (size_t i = 0; i < asset_count; ++i)
		lib.AssetInfos[i].LibUid = (uint32)in->ReadInt8();
	return kMFLNoError;
}

MFLUtil::MFLError MFLUtil::ReadV20(AssetLibInfo &lib, Stream *in) {
	// number of clib parts
	size_t mf_count = (uint32)in->ReadInt32();
	lib.LibFileNames.resize(mf_count);
	// filenames for all clib parts
	for (size_t i = 0; i < mf_count; ++i) {
		lib.LibFileNames[i].Read(in, MaxDataFileLen);
	}

	// number of files in clib
	size_t asset_count = (uint32)in->ReadInt32();
	// read information on clib contents
	lib.AssetInfos.resize(asset_count);
	char fn_buf[MaxAssetFileLen];
	for (size_t i = 0; i < asset_count; ++i) {
		size_t len = in->ReadInt16();
		len /= 5; // CHECKME: why 5?
		if (len > MaxAssetFileLen)
			return kMFLErrAssetNameLong;
		in->Read(fn_buf, len);
		// decrypt filenames
		DecryptText(fn_buf);
		lib.AssetInfos[i].FileName = fn_buf;
	}
	for (size_t i = 0; i < asset_count; ++i)
		lib.AssetInfos[i].Offset = (uint32)in->ReadInt32();
	for (size_t i = 0; i < asset_count; ++i)
		lib.AssetInfos[i].Size = (uint32)in->ReadInt32();
	for (size_t i = 0; i < asset_count; ++i)
		lib.AssetInfos[i].LibUid = (uint32)in->ReadInt8();
	return kMFLNoError;
}

MFLUtil::MFLError MFLUtil::ReadV21(AssetLibInfo &lib, Stream *in) {
	// init randomizer
	int rand_val = in->ReadInt32() + EncryptionRandSeed;
	// number of clib parts
	size_t mf_count = (uint32)ReadEncInt32(in, rand_val);
	lib.LibFileNames.resize(mf_count);
	// filenames for all clib parts
	char fn_buf[MaxDataFileLen > MaxAssetFileLen ? MaxDataFileLen : MaxAssetFileLen];
	for (size_t i = 0; i < mf_count; ++i) {
		ReadEncString(fn_buf, MaxDataFileLen, in, rand_val);
		lib.LibFileNames[i] = fn_buf;
	}

	// number of files in clib
	size_t asset_count = (uint32)ReadEncInt32(in, rand_val);
	// read information on clib contents
	lib.AssetInfos.resize(asset_count);
	for (size_t i = 0; i < asset_count; ++i) {
		ReadEncString(fn_buf, MaxAssetFileLen, in, rand_val);
		lib.AssetInfos[i].FileName = fn_buf;
	}
	for (size_t i = 0; i < asset_count; ++i)
		lib.AssetInfos[i].Offset = (uint32)ReadEncInt32(in, rand_val);
	for (size_t i = 0; i < asset_count; ++i)
		lib.AssetInfos[i].Size = (uint32)ReadEncInt32(in, rand_val);
	for (size_t i = 0; i < asset_count; ++i)
		lib.AssetInfos[i].LibUid = (uint32)ReadEncInt8(in, rand_val);
	return kMFLNoError;
}

MFLUtil::MFLError MFLUtil::ReadV30(AssetLibInfo &lib, Stream *in, MFLVersion /* lib_version */) {
	// NOTE: removed encryption like in v21, because it makes little sense
	// with open-source program. But if really wanted it may be restored
	// as one of the options here.
	/* int flags = */ in->ReadInt32(); // reserved options
	// number of clib parts
	size_t mf_count = (uint32)in->ReadInt32();
	lib.LibFileNames.resize(mf_count);
	// filenames for all clib parts
	for (size_t i = 0; i < mf_count; ++i)
		lib.LibFileNames[i] = String::FromStream(in);

	// number of files in clib
	size_t asset_count = (uint32)in->ReadInt32();
	// read information on clib contents
	lib.AssetInfos.resize(asset_count);
	for (auto &asset : lib.AssetInfos) {
		asset.FileName = String::FromStream(in);
		asset.LibUid = (uint8)in->ReadInt8();
		asset.Offset = in->ReadInt64();
		asset.Size = in->ReadInt64();
	}
	return kMFLNoError;
}

void MFLUtil::WriteHeader(const AssetLibInfo &lib, MFLVersion lib_version, int lib_index, Stream *out) {
	out->Write(HeadSig, strlen(HeadSig));
	out->WriteInt8(static_cast<uint8_t>(lib_version));
	out->WriteInt8(static_cast<uint8_t>(lib_index));   // file number

	// First datafile in chain: write the table of contents
	if (lib_index == 0) {
		WriteV30(lib, out);
	}
}

void MFLUtil::WriteV30(const AssetLibInfo &lib, Stream *out) {
	out->WriteInt32(0); // reserved options
	// filenames for all library parts
	out->WriteInt32(lib.LibFileNames.size());
	for (size_t i = 0; i < lib.LibFileNames.size(); ++i) {
		StrUtil::WriteCStr(lib.LibFileNames[i], out);
	}

	// table of contents for all assets in library
	out->WriteInt32(lib.AssetInfos.size());
	for (const auto &asset : lib.AssetInfos) {
		StrUtil::WriteCStr(asset.FileName, out);
		out->WriteInt8(static_cast<uint8_t>(asset.LibUid));
		out->WriteInt64(asset.Offset);
		out->WriteInt64(asset.Size);
	}
}

void MFLUtil::WriteEnder(soff_t lib_offset, MFLVersion lib_index, Stream *out) {
	if (lib_index < kMFLVersion_MultiV30)
		out->WriteInt32((int32_t)lib_offset);
	else
		out->WriteInt64(lib_offset);
	out->Write(TailSig, strlen(TailSig));
}

void MFLUtil::DecryptText(char *text) {
	size_t adx = 0;
	while (true) {
		text[0] -= EncryptionString[adx];
		if (text[0] == 0)
			break;

		adx++;
		text++;

		if (adx > 10) // CHECKME: why 10?
			adx = 0;
	}
}

int MFLUtil::GetNextPseudoRand(int &rand_val) {
	return (((rand_val = rand_val * 214013L
	                     + 2531011L) >> 16) & 0x7fff);
}

void MFLUtil::ReadEncArray(void *data, size_t size, size_t count, Stream *in, int &rand_val) {
	in->ReadArray(data, size, count);
	uint8_t *ch = (uint8_t *)data;
	const size_t len = size * count;
	for (size_t i = 0; i < len; ++i) {
		ch[i] = static_cast<uint8_t>(ch[i] - GetNextPseudoRand(rand_val));
	}
}

int8_t MFLUtil::ReadEncInt8(Stream *in, int &rand_val) {
	return static_cast<int8_t>(in->ReadInt8() - GetNextPseudoRand(rand_val));
}

int32_t MFLUtil::ReadEncInt32(Stream *in, int &rand_val) {
	int val;
	ReadEncArray(&val, sizeof(int32_t), 1, in, rand_val);
	return BBOp::Int32FromLE(val);
}

void MFLUtil::ReadEncString(char *buffer, size_t max_len, Stream *in, int &rand_val) {
	size_t i = 0;
	while ((i == 0) || (buffer[i - 1] != 0)) {
		buffer[i] = static_cast<int8_t>(in->ReadInt8() - GetNextPseudoRand(rand_val));
		if (i < max_len - 1)
			i++;
		else
			break; // avoid an endless loop
	}
}

} // namespace AGS
} // namespace Shared
} // namespace AGS3
