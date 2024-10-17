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

#include "sci/sci.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/system.h"

#include "audio/mididrv.h"
#include "audio/mt32gm.h"

#include "sci/resource/resource.h"
#include "sci/engine/features.h"
#include "sci/sound/drivers/gm_names.h"
#include "sci/sound/drivers/mididriver.h"
#include "sci/sound/drivers/map-mt32-to-gm.h"

namespace Sci {

#ifdef ENABLE_SCI32
static const byte defaultSci32GMPatch[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
	0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E,
	0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E,
	0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E,
	0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E,
	0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E,
	0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E,
	0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E,
	0x7F, 0x00, 0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23, 0x24, 0x24, 0x25, 0x25, 0x26, 0x26,
	0x27, 0x27, 0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B, 0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E,
	0x2F, 0x2F, 0x30, 0x30, 0x31, 0x31, 0x32, 0x32, 0x33, 0x33, 0x34, 0x34, 0x35, 0x35, 0x36, 0x36,
	0x37, 0x37, 0x38, 0x38, 0x39, 0x39, 0x3A, 0x3A, 0x3B, 0x3B, 0x3C, 0x3C, 0x3D, 0x3D, 0x3E, 0x3E,
	0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E,
	0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E,
	0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E,
	0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E,
	0x7F, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
	0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E,
	0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E,
	0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E,
	0x3F, 0x40, 0x41, 0x42, 0x42, 0x43, 0x43, 0x44, 0x44, 0x45, 0x45, 0x46, 0x46, 0x47, 0x47, 0x48,
	0x48, 0x49, 0x49, 0x4A, 0x4A, 0x4B, 0x4B, 0x4C, 0x4C, 0x4D, 0x4D, 0x4E, 0x4E, 0x4F, 0x4F, 0x50,
	0x50, 0x51, 0x51, 0x52, 0x52, 0x53, 0x53, 0x54, 0x54, 0x55, 0x55, 0x56, 0x56, 0x57, 0x57, 0x58,
	0x58, 0x59, 0x59, 0x5A, 0x5A, 0x5B, 0x5B, 0x5C, 0x5C, 0x5D, 0x5D, 0x5E, 0x5E, 0x5F, 0x5F, 0x60,
	0x60, 0x00, 0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23, 0x24, 0x24, 0x25, 0x25, 0x26, 0x26,
	0x27, 0x27, 0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B, 0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E,
	0x2F, 0x2F, 0x30, 0x30, 0x31, 0x31, 0x32, 0x32, 0x33, 0x33, 0x34, 0x34, 0x35, 0x35, 0x36, 0x36,
	0x37, 0x37, 0x38, 0x38, 0x39, 0x39, 0x3A, 0x3A, 0x3B, 0x3B, 0x3C, 0x3C, 0x3D, 0x3D, 0x3E, 0x3E,
	0x3F, 0x40, 0x41, 0x42, 0x42, 0x43, 0x43, 0x44, 0x44, 0x45, 0x45, 0x46, 0x46, 0x47, 0x47, 0x48,
	0x48, 0x49, 0x49, 0x4A, 0x4A, 0x4B, 0x4B, 0x4C, 0x4C, 0x4D, 0x4D, 0x4E, 0x4E, 0x4F, 0x4F, 0x50,
	0x50, 0x51, 0x51, 0x52, 0x52, 0x53, 0x53, 0x54, 0x54, 0x55, 0x55, 0x56, 0x56, 0x57, 0x57, 0x58,
	0x58, 0x59, 0x59, 0x5A, 0x5A, 0x5B, 0x5B, 0x5C, 0x5C, 0x5D, 0x5D, 0x5E, 0x5E, 0x5F, 0x5F, 0x60,
	0x60, 0x9B, 0x00, 0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7, 0xB0, 0x65,
	0x00, 0x64, 0x00, 0x06, 0x0C, 0x26, 0x00, 0xB1, 0x65, 0x00, 0x64, 0x00, 0x06, 0x0C, 0x26, 0x00,
	0xB2, 0x65, 0x00, 0x64, 0x00, 0x06, 0x0C, 0x26, 0x00, 0xB3, 0x65, 0x00, 0x64, 0x00, 0x06, 0x0C,
	0x26, 0x00, 0xB4, 0x65, 0x00, 0x64, 0x00, 0x06, 0x0C, 0x26, 0x00, 0xB5, 0x65, 0x00, 0x64, 0x00,
	0x06, 0x0C, 0x26, 0x00, 0xB6, 0x65, 0x00, 0x64, 0x00, 0x06, 0x0C, 0x26, 0x00, 0xB7, 0x65, 0x00,
	0x64, 0x00, 0x06, 0x0C, 0x26, 0x00, 0xB8, 0x65, 0x00, 0x64, 0x00, 0x06, 0x0C, 0x26, 0x00, 0xB9,
	0x65, 0x00, 0x64, 0x00, 0x06, 0x0C, 0x26, 0x00, 0xBA, 0x65, 0x00, 0x64, 0x00, 0x06, 0x0C, 0x26,
	0x00, 0xBB, 0x65, 0x00, 0x64, 0x00, 0x06, 0x0C, 0x26, 0x00, 0xBC, 0x65, 0x00, 0x64, 0x00, 0x06,
	0x0C, 0x26, 0x00, 0xBD, 0x65, 0x00, 0x64, 0x00, 0x06, 0x0C, 0x26, 0x00, 0xBE, 0x65, 0x00, 0x64,
	0x00, 0x06, 0x0C, 0x26, 0x00, 0xBF, 0x65, 0x00, 0x64, 0x00, 0x06, 0x0C, 0x26, 0x00
};
#endif

Mt32ToGmMapList *Mt32dynamicMappings = nullptr;

class MidiPlayer_Midi : public MidiPlayer {
public:
	enum {
		kVoices = 32,
		kReverbConfigNr = 11,
		kMaxSysExSize = 264
	};

	enum Mt32Type {
		kMt32TypeNone,
		kMt32TypeReal,
		kMt32TypeEmulated,
		kMt32TypeD110
	};

	MidiPlayer_Midi(SciVersion version);
	~MidiPlayer_Midi() override;

	int open(ResourceManager *resMan) override;
	void close() override;
	void send(uint32 b) override;
	void sysEx(const byte *msg, uint16 length) override;
	uint16 sysExNoDelay(const byte *msg, uint16 length) override;
	bool hasRhythmChannel() const override { return true; }
	byte getPlayId() const override;
	int getPolyphony() const override {
		if (g_sci && g_sci->_features->useAltWinGMSound())
			return 16;
		else
			return kVoices;
	}
	int getFirstChannel() const override;
	int getLastChannel() const override;
	void setVolume(byte volume) override;
	int getVolume() override;
	void setReverb(int8 reverb) override;
	void playSwitch(bool play) override;
	void initTrack(SciSpan<const byte> &) override;
	const char *reportMissingFiles() override { return _missingFiles; }

private:
	bool isMt32GmPatch(const SciSpan<const byte> &data);
	void readMt32GmPatch(const SciSpan<const byte> &data);
	void readMt32Patch(const SciSpan<const byte> &data);
	void readMt32DrvData();
	bool readD110DrvData();
	bool readD110SysEx();

	void mapMt32ToGm(const SciSpan<const byte> &data);
	uint8 lookupGmInstrument(const char *iname);
	uint8 lookupGmRhythmKey(const char *iname);
	uint8 getGmInstrument(const Mt32ToGmMap &Mt32Ins);

	void sendMt32SysEx(const uint32 addr, Common::SeekableReadStream &data, const int len, bool noDelay, bool mainThread);
	void sendMt32SysEx(const uint32 addr, const SciSpan<const byte> &data, bool noDelay, bool mainThread);
	void setMt32Volume(byte volume);
	void resetMt32();

	void noteOn(int channel, int note, int velocity);
	void setPatch(int channel, int patch);
	void controlChange(int channel, int control, int value);

	struct Channel {
		byte mappedPatch;
		byte patch;
		int velocityMapIdx;
		bool playing;
		int8 keyShift;
		int8 volAdjust;
		uint8 pan;
		uint8 hold;
		uint8 volume;

		Channel() : mappedPatch(MIDI_UNMAPPED), patch(MIDI_UNMAPPED), velocityMapIdx(0), playing(false),
			keyShift(0), volAdjust(0), pan(0x40), hold(0), volume(0x7f) { }
	};

	Mt32Type _mt32Type;
	uint _mt32LCDSize;
	bool _useMT32Track;
	bool _hasReverb;
	bool _playSwitch;
	int _masterVolume;

	byte _reverbConfig[kReverbConfigNr][3];
	int8 _defaultReverb;
	Channel _channels[16];
	uint8 _percussionMap[128];
	int8 _keyShift[128];
	int8 _volAdjust[128];
	uint8 _patchMap[128];
	uint8 _velocityMapIdx[128];
	uint8 _velocityMap[4][128];

	// These are extensions used for our own MT-32 to GM mapping
	uint8 _pitchBendRange[128];
	uint8 _percussionVelocityScale[128];

	byte _goodbyeMsg[32];
	byte _sysExBuf[kMaxSysExSize];

	const char *_missingFiles;
};

MidiPlayer_Midi::MidiPlayer_Midi(SciVersion version) : 
	MidiPlayer(version),
	_playSwitch(true),
	_masterVolume(15),
	_mt32Type(kMt32TypeNone),
	_mt32LCDSize(20),
	_hasReverb(false),
	_defaultReverb(-1),
	_useMT32Track(true),
	_missingFiles(nullptr) {

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI);
	_driver = MidiDriver::createMidi(dev);

	if (ConfMan.getInt("midi_mode") == kMidiModeD110) {
		_mt32Type = kMt32TypeD110;
		_mt32LCDSize = 32;
	} else if (MidiDriver::getMusicType(dev) == MT_MT32 || ConfMan.getBool("native_mt32")) {
		if (MidiDriver::getDeviceString(dev, MidiDriver::kDriverId) == "mt32") {
			_mt32Type = kMt32TypeEmulated;
		} else {
			_mt32Type = kMt32TypeReal;
		}
	}

	_sysExBuf[0] = 0x41;
	_sysExBuf[1] = 0x10;
	_sysExBuf[2] = 0x16;
	_sysExBuf[3] = 0x12;

	Mt32dynamicMappings = new Mt32ToGmMapList();
}

MidiPlayer_Midi::~MidiPlayer_Midi() {
	delete _driver;

	const Mt32ToGmMapList::iterator end = Mt32dynamicMappings->end();
	for (Mt32ToGmMapList::iterator it = Mt32dynamicMappings->begin(); it != end; ++it) {
		delete[] (*it).name;
		(*it).name = nullptr;
	}

	Mt32dynamicMappings->clear();
	delete Mt32dynamicMappings;
}

void MidiPlayer_Midi::noteOn(int channel, int note, int velocity) {
	uint8 patch = _channels[channel].mappedPatch;

	assert(channel <= 15);
	assert(note <= 127);
	assert(velocity <= 127);

	if (channel == MIDI_RHYTHM_CHANNEL) {
		if (_percussionMap[note] == MIDI_UNMAPPED) {
			debugC(kDebugLevelSound, "[Midi] Percussion instrument %i is unmapped", note);
			return;
		}

		note = _percussionMap[note];
		// Scale velocity;
		velocity = velocity * _percussionVelocityScale[note] / 127;
	} else if (patch >= 128) {
		if (patch == MIDI_UNMAPPED)
			return;

		// Map to rhythm
		channel = MIDI_RHYTHM_CHANNEL;
		note = patch - 128;

		// Scale velocity;
		velocity = velocity * _percussionVelocityScale[note] / 127;
	} else {
		int8 keyshift = _channels[channel].keyShift;

		int shiftNote = note + keyshift;

		if (keyshift > 0) {
			while (shiftNote > 127)
				shiftNote -= 12;
		} else {
			while (shiftNote < 0)
				shiftNote += 12;
		}

		note = shiftNote;

		// We assume that velocity 0 maps to 0 (for note off)
		int mapIndex = _channels[channel].velocityMapIdx;
		assert(velocity <= 127);
		velocity = _velocityMap[mapIndex][velocity];
	}

	_channels[channel].playing = true;
	_driver->send(0x90 | channel, note, velocity);
}

void MidiPlayer_Midi::controlChange(int channel, int control, int value) {
	assert(channel <= 15);
	bool standard_midi_controller = true;

	switch (control) {
	case 0x07:
		_channels[channel].volume = value;

		if (!_playSwitch)
			return;

		value += _channels[channel].volAdjust;

		if (value > 0x7f)
			value = 0x7f;

		if (value < 0)
			value = 1;

		value *= _masterVolume;

		if (value != 0) {
			value /= 15;

			if (value == 0)
				value = 1;
		}
		break;
	case 0x0a:
		_channels[channel].pan = value;
		break;
	case 0x40:
		_channels[channel].hold = value;
		break;
	case 0x4b:	// voice mapping
		// this is an internal Sierra command, and shouldn't be sent to the real MIDI driver - fixing #11409
		standard_midi_controller = false;
		break;
	case 0x4e:	// velocity
		break;
	case 0x7b:
		_channels[channel].playing = false;
	default:
		break;
	}

	if (standard_midi_controller)
		_driver->send(0xb0 | channel, control, value);
}

void MidiPlayer_Midi::setPatch(int channel, int patch) {
	assert(channel <= 15);

	// No need to do anything if a patch change is sent on the rhythm channel of an MT-32
	// or if the requested patch is the same as the current patch.
	if ((_mt32Type != kMt32TypeNone && channel == MIDI_RHYTHM_CHANNEL) || (_channels[channel].patch == patch))
		return;

	int patchToSend;
	if (channel != MIDI_RHYTHM_CHANNEL) {
		_channels[channel].patch = patch;
		_channels[channel].velocityMapIdx = _velocityMapIdx[patch];

		bool resetVol = (_channels[channel].mappedPatch == MIDI_UNMAPPED);

		_channels[channel].mappedPatch = patchToSend = _patchMap[patch];

		if (_patchMap[patch] == MIDI_UNMAPPED) {
			debugC(kDebugLevelSound, "[Midi] Channel %i set to unmapped patch %i", channel, patch);
			_driver->send(0xb0 | channel, 0x7b, 0);
			_driver->send(0xb0 | channel, 0x40, 0);
			return;
		}

		if (_patchMap[patch] >= 128) {
			// Mapped to rhythm, don't send channel commands
			return;
		}

		if (_channels[channel].keyShift != _keyShift[patch]) {
			_channels[channel].keyShift = _keyShift[patch];
			_driver->send(0xb0 | channel, 0x7b, 0);
			_driver->send(0xb0 | channel, 0x40, 0);
			resetVol = true;
		}

		if (resetVol || (_channels[channel].volAdjust != _volAdjust[patch])) {
			_channels[channel].volAdjust = _volAdjust[patch];
			controlChange(channel, 0x07, _channels[channel].volume);
		}

		uint8 bendRange = _pitchBendRange[patch];
		if (bendRange != MIDI_UNMAPPED)
			_driver->setPitchBendRange(channel, bendRange);
	} else {
		// A patch change on the rhythm channel of a Roland GS device indicates a drumkit change.
		// Some GM devices support the GS drumkits as well.

		// Apply drumkit fallback to correct invalid drumkit numbers.
		patchToSend = patch < 128 ? MidiDriver_MT32GM::GS_DRUMKIT_FALLBACK_MAP[patch] : 0;
		_channels[channel].patch = patchToSend;
		debugC(kDebugLevelSound, "[Midi] Selected drumkit %i (requested %i)", patchToSend, patch);
	}

	_driver->send(0xc0 | channel, patchToSend, 0);

	// Send a pointless command to work around a firmware bug in common
	// USB-MIDI cables. If the first MIDI command in a USB packet is a
	// Cx or Dx command, the second command in the packet is dropped
	// somewhere.
	// FIXME: consider putting a workaround in the MIDI backend drivers
	// instead.
	// Known to be affected: alsa, coremidi
	// Known *not* to be affected: windows (only seems to send one MIDI
	// command per USB packet even if the device allows larger packets).
	_driver->send(0xb0 | channel, 0x0a, _channels[channel].pan);
}

void MidiPlayer_Midi::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0x7f;
	byte op2 = (b >> 16) & 0x7f;

	// In early SCI0, we may also get events for AdLib rhythm channels.
	// While an MT-32 would ignore those with the default channel mapping,
	// we filter these out for the benefit of other MIDI devices.
	if (_version == SCI_VERSION_0_EARLY) {
		if (channel < 1 || channel > 9)
			return;
	}

	switch (command) {
	case 0x80:
		noteOn(channel, op1, 0);
		break;
	case 0x90:
		noteOn(channel, op1, op2);
		break;
	case 0xb0:
		controlChange(channel, op1, op2);
		break;
	case 0xc0:
		setPatch(channel, op1);
		break;
	// The original MIDI driver from sierra ignores aftertouch completely, so should we
	case 0xa0: // Polyphonic key pressure (aftertouch)
	case 0xd0: // Channel pressure (aftertouch)
		break;
	case 0xe0:
		_driver->send(b);
		break;
	default:
		warning("Ignoring MIDI event %02x", command);
	}
}

// We return 1 for mt32, because if we remap channels to 0 for mt32, those won't get played at all
// NOTE: SSCI uses channels 1 through 8 for General MIDI as well, in the drivers I checked
int MidiPlayer_Midi::getFirstChannel() const {
	return 1;
}

int MidiPlayer_Midi::getLastChannel() const {
	return 8;
}

void MidiPlayer_Midi::setVolume(byte volume) {
	_masterVolume = volume;

	if (!_playSwitch)
		return;

	for (uint i = 1; i < 10; i++) {
		if (_channels[i].volume != 0xff)
			controlChange(i, 0x07, _channels[i].volume & 0x7f);
	}
}

int MidiPlayer_Midi::getVolume() {
	return _masterVolume;
}

void MidiPlayer_Midi::setReverb(int8 reverb) {
	assert(reverb < kReverbConfigNr);

	if (_hasReverb && _reverb != reverb) {
		sendMt32SysEx(0x100001, SciSpan<const byte>(_reverbConfig[reverb], 3), true, true);
	}

	_reverb = reverb;
}

void MidiPlayer_Midi::playSwitch(bool play) {
	_playSwitch = play;
	if (play)
		setVolume(_masterVolume);
	else {
		for (uint i = 1; i < 10; i++)
			_driver->send(0xb0 | i, 7, 0);
	}
}

void MidiPlayer_Midi::initTrack(SciSpan<const byte> &header) {
	if (_version > SCI_VERSION_0_LATE)
		return;

	if (_defaultReverb >= 0)
		// SCI0 in combination with MT-32 requires a reset of the reverb to
		// the default value that is present in either the MT-32 patch data
		// or MT32.DRV itself.
		setReverb(_defaultReverb);

	uint8 caps = header.getInt8At(0);
	if (caps != 0 && (_version == SCI_VERSION_0_EARLY || caps != 2))
		return;

	uint8 readPos = 1;
	uint8 flags = 0;
	byte msg[9];
	memset(msg, 0x10, 9);

	if (_version == SCI_VERSION_0_EARLY) {
		uint8 writePos = 0;
		for (int i = 0; i < 16; ++i) {
			flags = header.getInt8At(readPos++);
			if (flags & 8) {
				// If both flags 1 and 8 are set this will make the driver assign that channel to MT32 part 9.
				// This suggests that any one channel could be the rhythm channel. I don't know whether this has any practical relevance.
				// A channel not flagged with 8 can also be assigned to MT-32 part 9 if it just happens to be the last channel. This is how
				// it is done in the tracks that I have seen so far. Flag 8 without flag 1 is the control channel (not handled in the driver).
				if (flags & 1) {
					if (i < 11) {
						msg[8] = i;
						writePos++;
					}
				} else {
					debugC(9, kDebugLevelSound, "MidiPlayer_Midi::initTrack(): Control channel found: 0x%.02x", i);
				}
			} else if (i < 11 && (flags & 1)) {
				assert(writePos < 9);
				msg[writePos++] = i;
			}
		}

	} else {
		readPos = 3;
		for (int i = 1; i < 9; ++i) {
			readPos++;
			flags = header.getInt8At(readPos++);
			msg[i - 1] = (flags & 1) ? i : 0x10;
		}

		flags = header.getInt8At(readPos);
		msg[8] = (flags & 0x80) ? 9 : 0x10;
	}

	// assign channels
	debugC(5, kDebugLevelSound, "MidiPlayer_Midi::initTrack(): Channels assigned to MT-32 parts: 0x%.02x 0x%.02x 0x%.02x 0x%.02x 0x%.02x 0x%.02x 0x%.02x 0x%.02x 0x%.02x", msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8]);
 	Sci::SciSpan<const byte> s(msg, 9);
	sendMt32SysEx(0x10000D, s, false, false);
}

bool MidiPlayer_Midi::isMt32GmPatch(const SciSpan<const byte> &data) {
	uint32 size = data.size();

	// WORKAROUND: Some Mac games (e.g. LSL5) may have an extra byte at the
	// end, so compensate for that here - bug #6725.
	if (size == 16890)
		size--;

	// Need at least 1153 + 2 bytes for a GM patch. Check readMt32GmPatch()
	// below for more info.
	if (size < 1153 + 2)
		return false;
	// The maximum number of bytes for an MT-32 patch is 16889. The maximum
	// number of timbres is 64, which leads us to:
	// 491 + 1 + 64 * 246 + 653 = 16889
	if (size > 16889)
		return true;

	bool isMt32 = false;
	bool isMt32Gm = false;

	// First, check for a GM patch. The presence of MIDI data after the
	// initial 1153 + 2 bytes indicates a GM patch
	if (data.getUint16LEAt(1153) + 1155U == size)
		isMt32Gm = true;

	// Now check for a regular MT-32 patch. Check readMt32Patch() below for
	// more info.
	// 491 = 20 + 20 + 20 + 2 + 1 + 11 + 3 * 11 + 256 + 128
	byte timbresNr = data[491];
	uint pos = 492 + 246 * timbresNr;

	// Patches 49-96
	if (size >= pos + 386 && data.getUint16BEAt(pos) == 0xabcd)
		pos += 386;	// 256 + 128 + 2

	// Rhythm key map + partial reserve
	if (size >= pos + 267 && data.getUint16BEAt(pos) == 0xdcba)
		pos += 267;	// 256 + 9 + 2

	if (size == pos)
		isMt32 = true;

	if (isMt32 == isMt32Gm)
		error("Failed to detect MT-32 patch format");

	return isMt32Gm;
}

void MidiPlayer_Midi::sendMt32SysEx(const uint32 addr, Common::SeekableReadStream &stream, int len, bool noDelay = false, bool mainThread = true) {
	if (len + 8 > kMaxSysExSize) {
		warning("SysEx message exceed maximum size; ignoring");
		return;
	}

	uint16 chk = 0;

	_sysExBuf[4] = (addr >> 16) & 0xff;
	_sysExBuf[5] = (addr >> 8) & 0xff;
	_sysExBuf[6] = addr & 0xff;

	stream.read(_sysExBuf + 7, len);

	for (int i = 4; i < 7 + len; i++)
		chk -= _sysExBuf[i];

	_sysExBuf[7 + len] = chk & 0x7f;

	uint16 delay = sysExNoDelay(_sysExBuf, len + 8);
	if (!noDelay && delay > 0) {
		// Use the appropriate delay technique based on the current thread.
		// On the main thread, use SciEngine::sleep() to keep the UI responsive,
		// which is important because loading patches can take several seconds.
		// On a timer thread however, SciEngine::sleep() can't be used because
		// it polls events and updates the screen, which isn't thread safe. (bug #12947)
		if (mainThread) {
			g_sci->sleep(delay);
		} else {
			g_system->delayMillis(delay);
		}
	}
}

void MidiPlayer_Midi::sendMt32SysEx(const uint32 addr, const SciSpan<const byte> &buf, bool noDelay = false, bool mainThread = true) {
	Common::MemoryReadStream stream(buf.toStream());
	sendMt32SysEx(addr, stream, buf.size(), noDelay, mainThread);
}


void MidiPlayer_Midi::readMt32Patch(const SciSpan<const byte> &data) {
	// MT-32 patch contents:
	// - 0-19        after-SysEx message   (KQ4/LSL2: before)
	// - 20-39       before-SysEx message  (KQ4/LSL2: after)
	// - 40-59       goodbye SysEx message
	// - 60-61       volume
	// - 62          reverb
	// - 63-73       reverb Sysex message
	// - 74-106      [3 * 11] reverb data
	// - 107-490     [256 + 128] patches 1-48
	// --> total: 491 bytes
	// - 491         number of timbres (64 max)
	// - 492..n      [246 * number of timbres] timbre data
	// - n-n+1       flag (0xabcd)
	// - n+2-n+385   [256 + 128] patches 49-96
	// - n+386-n+387 flag (0xdcba)
	// - n+388-n+643 rhythm key map
	// - n+644-n+652 partial reserve

	Common::MemoryReadStream stream(data.toStream());

	// before-SysEx and after-SysEx texts swapped positions after KQ4 and LSL2.
	uint beforeTextPos;
	uint afterTextPos;
	switch (g_sci->getGameId()) {
	case GID_KQ4:
	case GID_LSL2:
		beforeTextPos = 0;
		afterTextPos = _mt32LCDSize;
		break;
	default:
		beforeTextPos = _mt32LCDSize;
		afterTextPos = 0;
		break;
	}

	// Send before-SysEx text
	stream.seek(beforeTextPos);
	sendMt32SysEx(0x200000, stream, _mt32LCDSize);

	// Save goodbye message
	assert(sizeof(_goodbyeMsg) >= _mt32LCDSize);
	stream.seek(_mt32LCDSize * 2);
	stream.read(_goodbyeMsg, _mt32LCDSize);

	const uint8 volume = MIN<uint16>(stream.readUint16LE(), 100);
	setMt32Volume(volume);

	// Reverb default only used in (roughly) SCI0/SCI01
	_defaultReverb = stream.readSByte();

	_hasReverb = true;

	// Skip reverb SysEx message
	stream.seek(11, SEEK_CUR);

	// Read reverb data (stored vertically - trac #9261)
	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < kReverbConfigNr; i++) {
			_reverbConfig[i][j] = stream.readByte();
		}
	}

	// Patches 1-48
	sendMt32SysEx(0x50000, stream, 256);
	sendMt32SysEx(0x50200, stream, 128);

	// Timbres
	const uint8 timbresNr = stream.readByte();
	for (int i = 0; i < timbresNr; i++)
		sendMt32SysEx(0x80000 + (i << 9), stream, 246);

	uint16 flag = stream.readUint16BE();

	if (!stream.eos() && flag == 0xabcd) {
		// Patches 49-96
		sendMt32SysEx(0x50300, stream, 256);
		sendMt32SysEx(0x50500, stream, 128);
		flag = stream.readUint16BE();
	}

	if (!stream.eos() && flag == 0xdcba) {
		// Rhythm key map
		sendMt32SysEx(0x30110, stream, 256);
		// Partial reserve
		sendMt32SysEx(0x100004, stream, 9);
	}

	// Send after-SysEx text
	stream.seek(afterTextPos);
	sendMt32SysEx(0x200000, stream, _mt32LCDSize);

	if (_mt32Type != kMt32TypeD110) {
		// Send the mystery SysEx
		Common::MemoryReadStream mystery((const byte *)"\x16\x16\x16\x16\x16\x16", 6);
		sendMt32SysEx(0x52000a, mystery, 6);
	}
}

void MidiPlayer_Midi::readMt32GmPatch(const SciSpan<const byte> &data) {
	// GM patch contents:
	// - 128 bytes patch map
	// - 128 bytes key shift
	// - 128 bytes volume adjustment
	// - 128 bytes percussion map
	// - 1 byte volume adjust for the rhythm channel
	// - 128 bytes velocity map IDs
	// - 512 bytes velocity map
	// --> total: 1153 bytes

	data.subspan(0, sizeof(_patchMap)).unsafeCopyDataTo(_patchMap);
	data.subspan(128, sizeof(_keyShift)).unsafeCopyDataTo(_keyShift);
	data.subspan(256, sizeof(_volAdjust)).unsafeCopyDataTo(_volAdjust);
	data.subspan(384, sizeof(_percussionMap)).unsafeCopyDataTo(_percussionMap);
	_channels[MIDI_RHYTHM_CHANNEL].volAdjust = data[512];
	data.subspan(513, sizeof(_velocityMapIdx)).unsafeCopyDataTo(_velocityMapIdx);
	data.subspan(641, sizeof(_velocityMap)).unsafeCopyDataTo(_velocityMap);

	uint16 midiSize = data.getUint16LEAt(1153);

	if (midiSize > 0) {
		if (data.size() < midiSize + 1155U)
			error("Failed to read MIDI data");

		const SciSpan<const byte> midi = data.subspan(1155, midiSize);
		byte command = 0;
		uint i = 0;

		while (i < midiSize) {
			byte op1, op2;

			if (midi[i] & 0x80)
				command = midi[i++];

			switch (command & 0xf0) {
			case 0xf0: {
				const byte *sysExStart = midi.getUnsafeDataAt(i, midiSize - i);
				const byte *sysExEnd = (const byte *)memchr(sysExStart, 0xf7, midiSize - i);

				if (!sysExEnd)
					error("Failed to find end of sysEx");

				int len = sysExEnd - sysExStart;
				sysEx(sysExStart, len);

				i += len + 1; // One more for the 0xf7
				break;
			}
			case 0x80:
			case 0x90:
			case 0xa0:
			case 0xb0:
			case 0xe0:
				if (i + 1 >= midiSize)
					error("MIDI command exceeds data size");

				op1 = midi[i++];
				op2 = midi[i++];
				_driver->send(command, op1, op2);
				break;
			case 0xc0:
			case 0xd0:
				if (i >= midiSize)
					error("MIDI command exceeds data size");

				op1 = midi[i++];
				_driver->send(command, op1, 0);
				break;
			default:
				error("Failed to find MIDI command byte");
			}
		}
	}
}

void MidiPlayer_Midi::readMt32DrvData() {
	Common::File f;

	if (f.open("MT32.DRV")) {
		int size = f.size();

		// Skip before-SysEx text
		if (size == 1773 || size == 1759 || size == 1747)	// XMAS88 / KQ4 early (0.000.253 / 0.000.274)
			f.seek(0x59);
		else if (size == 2771)				// LSL2 early
			f.seek(0x29);
		else
			error("Unknown MT32.DRV size (%d)", size);

		// Skip 2 extra 0 bytes in some drivers
		if (f.readUint16LE() != 0)
			f.seek(-2, SEEK_CUR);

		// Send before-SysEx text
		sendMt32SysEx(0x200000, f, 20);

		if (size != 2771) {
			// Send after-SysEx text (SSCI sends this before every song).
			// There aren't any SysEx calls in old drivers, so this can
			// be sent right after the before-SysEx text.
			sendMt32SysEx(0x200000, f, 20);
		} else { // LSL2 early
			// Skip the after-SysEx text in the newer patch version, we'll send
			// it after the SysEx messages are sent.
			f.skip(20);
		}

		// Save goodbye message. This isn't a C string, so it may not be
		// nul-terminated.
		f.read(_goodbyeMsg, 20);

		// Set volume
		byte volume = CLIP<uint16>(f.readUint16LE(), 0, 100);
		setMt32Volume(volume);

		if (size == 2771) {
			// MT32.DRV in LSL2 early contains more data, like a normal patch
			_defaultReverb = f.readByte();

			_hasReverb = true;

			// Skip reverb SysEx message
			f.skip(11);

			// Read reverb data (stored vertically - trac #9261)
			for (int j = 0; j < 3; ++j) {
				for (int i = 0; i < kReverbConfigNr; i++) {
					_reverbConfig[i][j] = f.readByte();
				}
			}

			f.skip(2235);	// skip driver code

			// Patches 1-48
			sendMt32SysEx(0x50000, f, 256);
			sendMt32SysEx(0x50200, f, 128);

			// Send the after-SysEx text
			f.seek(0x3d);
			sendMt32SysEx(0x200000, f, 20);
		} else {
			byte reverbSysEx[13];
			// This old driver should have a full reverb SysEx
			if ((f.read(reverbSysEx, 13) != 13) || (reverbSysEx[0] != 0xf0) || (reverbSysEx[12] != 0xf7))
				error("Error reading MT32.DRV");

			// Send reverb SysEx
			sysEx(reverbSysEx + 1, 11);
			_hasReverb = false;

			f.seek(0x29);

			// Read AdLib->MT-32 patch map
			for (int i = 0; i < 48; i++) {
				_patchMap[i] = f.readByte();
			}
		}

		f.close();
	} else {
		error("Failed to open MT32.DRV");
	}
}

bool MidiPlayer_Midi::readD110DrvData() {
	const char *fileName;

	// Only one driver is known to exist
	switch (g_sci->getGameId()) {
	case GID_KQ4:
		fileName = "DKQ4.DRV";
		break;
	default:
		error("No D-110 driver is known to exist for this game");
	}

	Common::File f;
	if (!f.open(fileName)) {
		_missingFiles = fileName;
		return false;
	}

	if (f.size() != 3500)
		error("Unknown '%s' size (%d)", fileName, (int)f.size());

	f.seek(42);

	// Send before-SysEx text
	sendMt32SysEx(0x200000, f, 32);

	// Timbres
	f.seek(2761);
	sendMt32SysEx(0x50000, f, 256);
	sendMt32SysEx(0x50200, f, 128);

	// Rhythm
	sendMt32SysEx(0x30110, f, 256);
	sendMt32SysEx(0x30310, f, 84);

	f.seek(75);

	// Send after-SysEx text
	sendMt32SysEx(0x200000, f, 32);

	f.read(_goodbyeMsg, 32);

	byte reverbSysEx[13];
	f.read(reverbSysEx, 13);
	sysEx(reverbSysEx + 1, 11);

	_hasReverb = false;

	if (f.err() || f.eos())
		error("Error reading '%s'", fileName);

	f.close();

	return true;
}

bool MidiPlayer_Midi::readD110SysEx() {
	// These patches contain SysEx messages that were meant to be sent to the
	// device with a 3rd party tool before starting the game with MT-32 music.
	// In order to prevent the MT-32 patches from interfering with the
	// D-110/D-10/D-20 patches, these SysEx use unit #18. The user would be
	// required to change the unit number on their device. Since we can avoid
	// sending the MT-32 patch, we override the unit number back to 17 here.

	// The D-110 versions of these patches use Patch Memory at 0x060000,
	// which is not available on the D-10/D-20. Additionally, this method
	// requires user interaction on the device between SysEx upload and
	// starting the game. We therefore use the D-20 patches instead.

	// Patches for later games (using patch 4 format with GENMIDI.DRV) appear
	// to have been distributed on the Sierra BBS in file GEND110.EXE. So far
	// this file has not been recovered.

	// Note: there was also aftermarket support for E-mu Proteus 1/2, but those
	// files appear to have been lost in the mists of time as well.

	const char *fileName;

	switch (g_sci->getGameId()) {
	case GID_KQ5:
		fileName = "KQ5D20";
		break;
	case GID_QFG2:
		fileName = "QFG2D20";
		break;
	default:
		error("No aftermarket D-110 patch is known to exist for this game");
	}

	Common::File sysExFile;

	if (!sysExFile.open(fileName)) {
		_missingFiles = fileName;
		return false;
	}

	byte sysExBuf[kMaxSysExSize + 2];

	while (true) {
		byte b = sysExFile.readByte();

		if (sysExFile.err())
			error("Error reading '%s'", fileName);

		if (sysExFile.eos())
			break;

		if (b != 0xf0)
			error("Unexpected data found in SysEx file '%s'", fileName);

		uint sysExLen = 0;
		sysExBuf[sysExLen++] = b;

		while (sysExLen < ARRAYSIZE(sysExBuf) && b != 0xf7) {
			b = sysExFile.readByte();
			sysExBuf[sysExLen++] = b;
		}

		if (b != 0xf7 || sysExLen < 10)
			error("SysEx has invalid size in SysEx file '%s'", fileName);

		// Use unit #17
		sysExBuf[2] = 0x10;
		sysEx(sysExBuf + 1, sysExLen - 2);
	}

	// The D-10/D-20 have fixed MIDI channel assignments, so we need to set the D-110
	// manually here
	Common::MemoryReadStream s((const byte *)"\x01\x02\x03\x04\x05\x06\x07\x08\x09", 9);
	sendMt32SysEx(0x10000d, s, 9);

	memcpy(_goodbyeMsg, "    ScummVM                     ", 32);

	return true;
}

byte MidiPlayer_Midi::lookupGmInstrument(const char *iname) {
	int i = 0;

	if (Mt32dynamicMappings != nullptr) {
		const Mt32ToGmMapList::iterator end = Mt32dynamicMappings->end();
		for (Mt32ToGmMapList::iterator it = Mt32dynamicMappings->begin(); it != end; ++it) {
			if (scumm_strnicmp(iname, (*it).name, 10) == 0)
				return getGmInstrument((*it));
		}
	}

	while (Mt32MemoryTimbreMaps[i].name) {
		if (scumm_strnicmp(iname, Mt32MemoryTimbreMaps[i].name, 10) == 0)
			return getGmInstrument(Mt32MemoryTimbreMaps[i]);
		i++;
	}

	return MIDI_UNMAPPED;
}

byte MidiPlayer_Midi::lookupGmRhythmKey(const char *iname) {
	int i = 0;

	if (Mt32dynamicMappings != nullptr) {
		const Mt32ToGmMapList::iterator end = Mt32dynamicMappings->end();
		for (Mt32ToGmMapList::iterator it = Mt32dynamicMappings->begin(); it != end; ++it) {
			if (scumm_strnicmp(iname, (*it).name, 10) == 0)
				return (*it).gmRhythmKey;
		}
	}

	while (Mt32MemoryTimbreMaps[i].name) {
		if (scumm_strnicmp(iname, Mt32MemoryTimbreMaps[i].name, 10) == 0)
			return Mt32MemoryTimbreMaps[i].gmRhythmKey;
		i++;
	}

	return MIDI_UNMAPPED;
}

uint8 MidiPlayer_Midi::getGmInstrument(const Mt32ToGmMap &Mt32Ins) {
	if (Mt32Ins.gmInstr == MIDI_MAPPED_TO_RHYTHM)
		return Mt32Ins.gmRhythmKey + 0x80;
	else
		return Mt32Ins.gmInstr;
}

void MidiPlayer_Midi::mapMt32ToGm(const SciSpan<const byte> &data) {
	// FIXME: Clean this up
	int memtimbres, patches;
	uint8 group, number, keyshift, /*finetune,*/ bender_range;
	SciSpan<const byte> patchpointer;
	uint32 pos;
	int i;

	for (i = 0; i < 128; i++) {
		_patchMap[i] = getGmInstrument(Mt32PresetTimbreMaps[i]);
		_pitchBendRange[i] = 12;
	}

	for (i = 0; i < 128; i++)
		_percussionMap[i] = Mt32PresetRhythmKeymap[i];

	memtimbres = data[0x1eb];
	pos = 0x1ec + memtimbres * 0xf6;

	if (data.size() > pos && data.getUint16BEAt(pos) == 0xabcd) {
		patches = 96;
		pos += 2 + 8 * 48;
	} else {
		patches = 48;
	}

	debugC(kDebugLevelSound, "[MT32-to-GM] %d MT-32 Patches detected", patches);
	debugC(kDebugLevelSound, "[MT32-to-GM] %d MT-32 Memory Timbres", memtimbres);

	debugC(kDebugLevelSound, "\n[MT32-to-GM] Mapping patches..");

	for (i = 0; i < patches; i++) {
		Common::String name;

		if (i < 48)
			patchpointer = data.subspan(0x6b + 8 * i);
		else
			patchpointer = data.subspan(0x1ec + 8 * (i - 48) + memtimbres * 0xf6 + 2);

		group = patchpointer[0];
		number = patchpointer[1];
		keyshift = patchpointer[2];
		//finetune = patchpointer[3];
		bender_range = patchpointer[4];

		debugCN(kDebugLevelSound, "  [%03d] ", i);

		switch (group) {
		case 1:
			number += 64;
			// Fall through
		case 0:
			_patchMap[i] = getGmInstrument(Mt32PresetTimbreMaps[number]);
			debugCN(kDebugLevelSound, "%s -> ", Mt32PresetTimbreMaps[number].name);
			break;
		case 2:
			if (number < memtimbres) {
				name = data.getStringAt(0x1ec + number * 0xf6, 10);
				_patchMap[i] = lookupGmInstrument(name.c_str());
				debugCN(kDebugLevelSound, "%s -> ", name.c_str());
			} else {
				_patchMap[i] = 0xff;
				debugCN(kDebugLevelSound, "[Invalid]  -> ");
			}
			break;
		case 3:
			_patchMap[i] = getGmInstrument(Mt32RhythmTimbreMaps[number]);
			debugCN(kDebugLevelSound, "%s -> ", Mt32RhythmTimbreMaps[number].name);
			break;
		default:
			break;
		}

		if (_patchMap[i] == MIDI_UNMAPPED) {
			debugC(kDebugLevelSound, "[Unmapped]");
		} else {
#ifndef REDUCE_MEMORY_USAGE
			if (_patchMap[i] >= 128) {
				debugC(kDebugLevelSound, "%s [Rhythm]", GmPercussionNames[_patchMap[i] - 128]);
			} else {
				debugC(kDebugLevelSound, "%s", GmInstrumentNames[_patchMap[i]]);
			}
#endif
		}

		_keyShift[i] = CLIP<uint8>(keyshift, 0, 48) - 24;
		_pitchBendRange[i] = CLIP<uint8>(bender_range, 0, 24);
	}

	if (data.size() > pos && data.getUint16BEAt(pos) == 0xdcba) {
		debugC(kDebugLevelSound, "\n[MT32-to-GM] Mapping percussion..");

		for (i = 0; i < 64; i++) {
			number = data[pos + 4 * i + 2];
			byte ins = i + 24;

			debugCN(kDebugLevelSound, "  [%03d] ", ins);

			if (number < 64) {
				Common::String name = data.getStringAt(0x1ec + number * 0xf6, 10);
				debugCN(kDebugLevelSound, "%s -> ", name.c_str());
				_percussionMap[ins] = lookupGmRhythmKey(name.c_str());
			} else {
				if (number < 94) {
					debugCN(kDebugLevelSound, "%s -> ", Mt32RhythmTimbreMaps[number - 64].name);
					_percussionMap[ins] = Mt32RhythmTimbreMaps[number - 64].gmRhythmKey;
				} else {
					debugCN(kDebugLevelSound, "[Key  %03i] -> ", number);
					_percussionMap[ins] = MIDI_UNMAPPED;
				}
			}

#ifndef REDUCE_MEMORY_USAGE
			if (_percussionMap[ins] == MIDI_UNMAPPED)
				debugC(kDebugLevelSound, "[Unmapped]");
			else
				debugC(kDebugLevelSound, "%s", GmPercussionNames[_percussionMap[ins]]);
#endif

			_percussionVelocityScale[ins] = data[pos + 4 * i + 3] * 127 / 100;
		}
	}
}

void MidiPlayer_Midi::setMt32Volume(byte volume) {
	Common::MemoryReadStream s(&volume, 1);
	sendMt32SysEx(0x100016, s, 1);
}

void MidiPlayer_Midi::resetMt32() {
	Common::MemoryReadStream s((const byte *)"\x01\x00", 2);
	sendMt32SysEx(0x7f0000, s, 2, true);

	if (_mt32Type != kMt32TypeEmulated) {
		// This seems to require a longer delay than usual
		g_sci->sleep(150); // note that sleep() can only be called from main thread, see bug #12947
	}
}

int MidiPlayer_Midi::open(ResourceManager *resMan) {
	assert(resMan != nullptr);

	int retval = _driver->open();
	if (retval != 0) {
		warning("Failed to open MIDI driver");
		return retval;
	}

	// By default use no mapping
	for (uint i = 0; i < 128; i++) {
		_percussionMap[i] = i;
		_patchMap[i] = i;
		_velocityMap[0][i] = i;
		_velocityMap[1][i] = i;
		_velocityMap[2][i] = i;
		_velocityMap[3][i] = i;
		_keyShift[i] = 0;
		_volAdjust[i] = 0;
		_velocityMapIdx[i] = 0;
		_pitchBendRange[i] = MIDI_UNMAPPED;
		_percussionVelocityScale[i] = 127;
	}

	Resource *res = nullptr;

	if (g_sci && g_sci->_features->useAltWinGMSound()) {
		res = resMan->findResource(ResourceId(kResourceTypePatch, 4), false);
		if (!res || !isMt32GmPatch(*res)) {
			// Don't do any mapping when a Windows alternative track is selected
			// and no MIDI patch is available
			_useMT32Track = false;
			return 0;
		}
	}

	if (_mt32Type == kMt32TypeD110) {
		// D-110, no reset SysEx exists
		for (uint i = 0; i < MIDI_CHANNELS; ++i) {
			_driver->send(0xb0 | i, 0x7b, 0); // All notes off
			_driver->send(0xb0 | i, 0x79, 0); // Reset all controllers
		}

		if (getSciVersion() == SCI_VERSION_0_EARLY) {
			if (!readD110DrvData())
				return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
		} else if (getSciVersion() == SCI_VERSION_0_LATE) {
			res = resMan->findResource(ResourceId(kResourceTypePatch, 0), false);

			if (!res) {
				_missingFiles = "PATCH.000";
				return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
			}

			readMt32Patch(*res);
		} else {
			if (!readD110SysEx())
				return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
		}
	} else if (_mt32Type != kMt32TypeNone) {
		// MT-32
		resetMt32();

		res = resMan->findResource(ResourceId(kResourceTypePatch, 1), false);

		if (res) {
			if (isMt32GmPatch(*res)) {
				readMt32GmPatch(*res);
				// Note that _goodbyeMsg is not zero-terminated
				memcpy(_goodbyeMsg, "      ScummVM       ", 20);
			} else {
				readMt32Patch(*res);
			}
		} else if (_version == SCI_VERSION_0_EARLY) {
			// Early SCI0 games have the sound bank embedded in the MT-32 driver
			readMt32DrvData();
		}
	} else {
		// General MIDI
		res = resMan->findResource(ResourceId(kResourceTypePatch, 4), false);

		if (res && isMt32GmPatch(*res)) {
			// There is a GM patch
			readMt32GmPatch(*res);

			if (g_sci && g_sci->_features->useAltWinGMSound()) {
				// Always use the GM track if an alternative GM Windows soundtrack is selected
				_useMT32Track = false;
			} else {
				// Detect the format of patch 1, so that we know what play mask to use
				res = resMan->findResource(ResourceId(kResourceTypePatch, 1), false);
				if (!res)
					_useMT32Track = false;
				else
					_useMT32Track = !isMt32GmPatch(*res);

				// Check if the songs themselves have a GM track
				if (!_useMT32Track) {
					if (!resMan->isGMTrackIncluded())
						_useMT32Track = true;
				}
			}
#ifdef ENABLE_SCI32
		} else if (getSciVersion() >= SCI_VERSION_2) {
			readMt32GmPatch(SciSpan<const byte>(defaultSci32GMPatch, sizeof(defaultSci32GMPatch)));
			_useMT32Track = false;
#endif
		} else {
			// No GM patch found, map instruments using MT-32 patch

			warning("Game has no native support for General MIDI, applying auto-mapping");

			// TODO: The MT-32 <-> GM mapping hasn't been worked on for SCI1 games. Throw
			// a warning to the user
			if (getSciVersion() >= SCI_VERSION_1_EGA_ONLY)
				warning("The automatic mapping for General MIDI hasn't been worked on for "
						"SCI1 games. Music might sound wrong or broken. Please choose another "
						"music driver for this game (e.g. AdLib or MT-32) if you are "
						"experiencing issues with music");

			// Modify velocity map to make low velocity notes a little louder
			for (uint i = 1; i < 0x40; i++) {
				_velocityMap[0][i] = 0x20 + (i - 1) / 2;
				_velocityMap[1][i] = 0x20 + (i - 1) / 2;
				_velocityMap[2][i] = 0x20 + (i - 1) / 2;
				_velocityMap[3][i] = 0x20 + (i - 1) / 2;
			}

			res = resMan->findResource(ResourceId(kResourceTypePatch, 1), false);

			if (res) {
				if (!isMt32GmPatch(*res)) {
					mapMt32ToGm(*res);
				} else {
					if (getSciVersion() < SCI_VERSION_3) {
						error("MT-32 patch has wrong type");
					} else {
						// Happens in the SCI3 interactive demo of Lighthouse
						warning("TODO: Ignoring new SCI3 type of MT-32 patch for now (size = %u)", res->size());
					}
				}
			} else {
				// Early SCI0 games have the sound bank embedded in the MT-32 driver

				// No MT-32 patch present, try to read from MT32.DRV
				Common::File f;

				if (f.open("MT32.DRV")) {
					int size = f.size();

					assert(size >= 70);

					f.seek(0x29);

					// Read AdLib->MT-32 patch map
					for (int i = 0; i < 48; i++)
						_patchMap[i] = getGmInstrument(Mt32PresetTimbreMaps[f.readByte() & 0x7f]);
				}
			}
		}
	}

	return 0;
}

void MidiPlayer_Midi::close() {
	if (_mt32Type != kMt32TypeNone) {
		// Send goodbye message
		sendMt32SysEx(0x200000, SciSpan<const byte>(_goodbyeMsg, _mt32LCDSize), true);
	}

	_driver->setTimerCallback(nullptr, nullptr);
	_driver->close();
}

void MidiPlayer_Midi::sysEx(const byte *msg, uint16 length) {
	uint16 delay = sysExNoDelay(msg, length);

	if (delay > 0)
		g_system->delayMillis(delay);
}

uint16 MidiPlayer_Midi::sysExNoDelay(const byte *msg, uint16 length) {
	_driver->sysEx(msg, length);

	uint16 delay = 0;
	if (_mt32Type != kMt32TypeEmulated) {
		// Wait the time it takes to send the SysEx data
		delay = (length + 2) * 1000 / 3125;

		// Plus an additional delay for the MT-32 rev00
		if (_mt32Type == kMt32TypeReal)
			delay += 40;
	}

	return delay;
}

byte MidiPlayer_Midi::getPlayId() const {
	switch (_version) {
	case SCI_VERSION_0_EARLY:
	case SCI_VERSION_0_LATE:
		return 0x01;
	default:
		if (_mt32Type != kMt32TypeNone)
			return 0x0c;
		else
			return _useMT32Track ? 0x0c : 0x07;
	}
}

MidiPlayer *MidiPlayer_Midi_create(SciVersion version) {
	return new MidiPlayer_Midi(version);
}

} // End of namespace Sci
