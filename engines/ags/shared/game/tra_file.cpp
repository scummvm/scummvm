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

#include "ags/shared/game/tra_file.h"
#include "ags/shared/ac/words_dictionary.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/util/data_ext.h"
#include "ags/shared/util/string_compat.h"
#include "ags/shared/util/string_utils.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

const char *TRASignature = "AGSTranslation";


String GetTraFileErrorText(TraFileErrorType err) {
	switch (err) {
	case kTraFileErr_NoError:
		return "No error.";
	case kTraFileErr_SignatureFailed:
		return "Not an AGS translation file or an unsupported format.";
	case kTraFileErr_FormatNotSupported:
		return "Format version not supported.";
	case kTraFileErr_GameIDMismatch:
		return "Game ID does not match, translation is meant for a different game.";
	case kTraFileErr_UnexpectedEOF:
		return "Unexpected end of file.";
	case kTraFileErr_UnknownBlockType:
		return "Unknown block type.";
	case kTraFileErr_BlockDataOverlapping:
		return "Block data overlapping.";
	}
	return "Unknown error.";
}

String GetTraBlockName(TraFileBlock id) {
	switch (id) {
	case kTraFblk_Dict: return "Dictionary";
	case kTraFblk_GameID: return "GameID";
	case kTraFblk_TextOpts: return "TextOpts";
	default: break;
	}
	return "unknown";
}

HError OpenTraFile(Stream *in) {
	// Test the file signature
	char sigbuf[16] = { 0 };
	in->Read(sigbuf, 15);
	if (ags_stricmp(TRASignature, sigbuf) != 0)
		return new TraFileError(kTraFileErr_SignatureFailed);
	return HError::None();
}

HError ReadTraBlock(Translation &tra, Stream *in, TraFileBlock block, soff_t block_len) {
	switch (block) {
	case kTraFblk_Dict:
	{
		char original[1024];
		char translation[1024];
		// Read lines until we find zero-length key & value
		while (true) {
			read_string_decrypt(in, original, sizeof(original));
			read_string_decrypt(in, translation, sizeof(translation));
			if (!original[0] && !translation[0])
				break;
			tra.Dict.insert(std::make_pair(String(original), String(translation)));
		}
	}
	break;
	case kTraFblk_GameID:
	{
		char gamename[256];
		tra.GameUid = in->ReadInt32();
		read_string_decrypt(in, gamename, sizeof(gamename));
		tra.GameName = gamename;
	}
	break;
	case kTraFblk_TextOpts:
		tra.NormalFont = in->ReadInt32();
		tra.SpeechFont = in->ReadInt32();
		tra.RightToLeft = in->ReadInt32();
		break;
	default:
		return new TraFileError(kTraFileErr_UnknownBlockType,
			String::FromFormat("Type: %d, known range: %d - %d.", block, kTraFblk_Dict, kTraFblk_TextOpts));
	}
	return HError::None();
}

static Translation *reader_tra;
HError TestTraGameIDReader(Stream *in, int block_id, const String &ext_id,
		soff_t block_len, bool &read_next) {
	if (block_id == kTraFblk_GameID) {
		read_next = false;
		return ReadTraBlock(*reader_tra, in, (TraFileBlock)block_id, block_len);
	}
	in->Seek(block_len); // skip block
	return HError::None();
}

HError TestTraGameID(int game_uid, const String &game_name, Stream *in) {
	HError err = OpenTraFile(in);
	if (!err)
		return err;

	// This reader would only process kTraFblk_GameID and exit as soon as one is found
	Translation tra;
	reader_tra = &tra;

	err = ReadExtData(TestTraGameIDReader, kDataExt_NumID32 | kDataExt_File32, in);
	if (!err)
		return err;
	// Test the identifiers, if they are not present then skip the test
	if ((tra.GameUid != 0 && (game_uid != tra.GameUid)) ||
		(!tra.GameName.IsEmpty() && (game_name != tra.GameName)))
		return new TraFileError(kTraFileErr_GameIDMismatch,
			String::FromFormat("The translation is designed for '%s'", tra.GameName.GetCStr()));
	return HError::None();
}

// This reader will process all blocks inside ReadTraBlock() function,
// and read compatible data into the given Translation object
HError ReadTraDataReader(Stream *in, int block_id, const String &ext_id,
		soff_t block_len, bool &read_next) {
	return ReadTraBlock(*reader_tra, in, (TraFileBlock)block_id, block_len);
}

HError ReadTraData(Translation &tra, Stream *in) {
	reader_tra = &tra;
	HError err = OpenTraFile(in);
	if (!err)
		return err;

	return ReadExtData(ReadTraDataReader, kDataExt_NumID32 | kDataExt_File32, in);
}

// TODO: perhaps merge with encrypt/decrypt utilities
static const char *EncryptText(std::vector<char> &en_buf, const String &s) {
	if (en_buf.size() < s.GetLength() + 1)
		en_buf.resize(s.GetLength() + 1);
	strncpy(&en_buf.front(), s.GetCStr(), s.GetLength() + 1);
	encrypt_text(&en_buf.front());
	return &en_buf.front();
}

// TODO: perhaps merge with encrypt/decrypt utilities
static const char *EncryptEmptyString(std::vector<char> &en_buf) {
	en_buf[0] = 0;
	encrypt_text(&en_buf.front());
	return &en_buf.front();
}

void WriteGameID(const Translation &tra, Stream *out) {
	std::vector<char> en_buf;
	out->WriteInt32(tra.GameUid);
	StrUtil::WriteString(EncryptText(en_buf, tra.GameName), tra.GameName.GetLength() + 1, out);
}

void WriteDict(const Translation &tra, Stream *out) {
	std::vector<char> en_buf;
	for (const auto &kv : tra.Dict) {
		const String &src = kv._key;
		const String &dst = kv._value;
		if (!dst.IsNullOrSpace()) {
			String unsrc = StrUtil::Unescape(src);
			String undst = StrUtil::Unescape(dst);
			StrUtil::WriteString(EncryptText(en_buf, unsrc), unsrc.GetLength() + 1, out);
			StrUtil::WriteString(EncryptText(en_buf, undst), undst.GetLength() + 1, out);
		}
	}
	// Write a pair of empty key/values
	StrUtil::WriteString(EncryptEmptyString(en_buf), 1, out);
	StrUtil::WriteString(EncryptEmptyString(en_buf), 1, out);
}

void WriteTextOpts(const Translation &tra, Stream *out) {
	out->WriteInt32(tra.NormalFont);
	out->WriteInt32(tra.SpeechFont);
	out->WriteInt32(tra.RightToLeft);
}

static const Translation *writer_tra;
static void(*writer_writer)(const Translation &tra, Stream *out);

static void WriteTraBlockWriter(Stream *out) {
	writer_writer(*writer_tra, out);
}

inline void WriteTraBlock(const Translation &tra, TraFileBlock block,
		void(*writer)(const Translation &tra, Stream *out), Stream *out) {
	writer_tra = &tra;
	writer_writer = writer;

	WriteExtBlock(block, WriteTraBlockWriter,
		kDataExt_NumID32 | kDataExt_File32, out);
}

void WriteTraData(const Translation &tra, Stream *out) {
	// Write header
	out->Write(TRASignature, strlen(TRASignature) + 1);

	// Write all blocks
	WriteTraBlock(tra, kTraFblk_GameID, WriteGameID, out);
	WriteTraBlock(tra, kTraFblk_Dict, WriteDict, out);
	WriteTraBlock(tra, kTraFblk_TextOpts, WriteTextOpts, out);

	// Write ending
	out->WriteInt32(kTraFile_EOF);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
