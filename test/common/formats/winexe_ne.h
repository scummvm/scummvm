#include <cxxtest/TestSuite.h>

#include "common/endian.h"
#include "common/formats/winexe_ne.h"
#include "common/memstream.h"
#include "common/ptr.h"

class NEResourcesTestSuite : public CxxTest::TestSuite {
private:
	struct NEResourceBuilder {
		Common::Array<byte> bytes;
		uint tablePos = 128;
		uint dataPos = 8192;

		NEResourceBuilder() : bytes(16384) {
			memset(bytes.data(), 0, bytes.size());
			WRITE_BE_UINT16(bytes.data(), 0x4d5a);
			WRITE_LE_UINT32(bytes.data() + 60, 64);
			WRITE_BE_UINT16(bytes.data() + 64, 0x4e45);
			WRITE_LE_UINT16(bytes.data() + 64 + 36, 64);
			word(0); // Resource alignment shift.
		}

		void word(uint16 value) {
			WRITE_LE_UINT16(bytes.data() + tablePos, value);
			tablePos += 2;
		}

		void type(uint16 id, uint16 count) {
			word(id);
			word(count);
			word(0);
			word(0);
		}

		void resource(uint16 id, const byte *data, uint16 size) {
			word(dataPos);
			word(size);
			word(0);
			word(id);
			word(0);
			word(0);
			memcpy(bytes.data() + dataPos, data, size);
			dataPos += size;
		}

		void resource(uint16 id, byte value) {
			resource(id, &value, 1);
		}

		uint16 name(const char *value) {
			uint16 offset = tablePos - 128;
			uint length = strlen(value);
			bytes[tablePos] = length;
			tablePos += 1;
			memcpy(bytes.data() + tablePos, value, length);
			tablePos += length;
			return offset;
		}

		bool load(Common::NEResources &reader) {
			return reader.loadFromEXE(new Common::MemoryReadStream(bytes.data(), bytes.size()));
		}
	};

	static void checkResource(Common::NEResources &reader, const Common::WinResourceID &type, const Common::WinResourceID &id, byte expected) {
		Common::ScopedPtr<Common::SeekableReadStream> data(reader.getResource(type, id));
		TS_ASSERT(data);
		if (data) {
			TS_ASSERT_EQUALS(data->size(), 1);
			TS_ASSERT_EQUALS(data->readByte(), expected);
		}
	}

public:
	void test_rr1cloud9_r1map_named_types() {
		// Type-name entries for ASEQ and RRGB with numeric ID 1852 (from rr1cloud9's r1map.rsc).
		static constexpr byte names[] = {
			0x0c, 0x00, 0x01, 0xff, 0x3c, 0x07, 'A', 'S', 'E', 'Q', 0x00, 0x00,
			0x0c, 0x00, 0x02, 0xff, 0x3c, 0x07, 'R', 'R', 'G', 'B', 0x00, 0x00,
			0x00, 0x00};
		NEResourceBuilder builder;
		builder.type(0x8000 | Common::kWinNameTable, 1);
		builder.resource(0x8001, names, sizeof(names));
		builder.type(0x8000 | 32513, 1);
		builder.resource(0x8000 | 1852, 11);
		builder.type(0x8000 | 32514, 1);
		builder.resource(0x8000 | 1852, 22);
		builder.word(0);
		Common::NEResources reader;
		TS_ASSERT(builder.load(reader));
		checkResource(reader, Common::String("ASEQ"), 1852, 11);
		checkResource(reader, Common::String("RRGB"), 1852, 22);
		checkResource(reader, 32513, 1852, 11);
		const auto ids = reader.getIDList(Common::String("ASEQ"));
		TS_ASSERT_EQUALS(ids.size(), 1U);
		if (!ids.empty())
			TS_ASSERT_EQUALS(ids[0].getID(), 1852U);
	}

	void test_rr1cloud9_r1ca1_resource_count() {
		// Mocks a NE file which has 484 LIPS IDs, followed by an RRGB type (from rr1cloud9's r1ca1.rsc).
		static constexpr uint16 idRanges[][2] = {
			{10104, 10105},
			{10180, 10180},
			{10182, 10185},
			{10205, 10205},
			{10210, 10211},
			{10225, 10229},
			{10235, 10235},
			{10245, 10246},
			{10260, 10262},
			{10265, 10266},
			{10315, 10316},
			{10318, 10318},
			{10326, 10328},
			{10355, 10358},
			{10380, 10382},
			{10404, 10408},
			{10415, 10416},
			{10430, 10435},
			{10500, 10934},
		};
		NEResourceBuilder builder;
		builder.type(0x8000 | 32515, 484);
		for (const auto &range : idRanges) {
			for (uint id = range[0]; id <= range[1]; id += 1)
				builder.resource(0x8000 | id, 11);
		}
		builder.type(0x8000 | 32516, 1);
		builder.resource(0x8000 | 10005, 22);
		builder.word(0);
		Common::NEResources reader;
		TS_ASSERT(builder.load(reader));
		const auto ids = reader.getIDList(32515);
		TS_ASSERT_EQUALS(ids.size(), 484U);
		checkResource(reader, 32515, 10934, 11);
		checkResource(reader, 32516, 10005, 22);
	}

	void test_type_alias_preserves_oridnal_and_case_insensitive_queries() {
		static constexpr byte names[] = {
			0x0c, 0x00, 0x01, 0xff, 0x3c, 0x07, 'A', 'S', 'E', 'Q',
			0x00, 0x00, 0x00, 0x00};
		NEResourceBuilder builder;
		builder.type(0x8000 | Common::kWinNameTable, 1);
		builder.resource(0x8001, names, sizeof(names));
		builder.type(0x8000 | 32513, 1);
		builder.resource(0x8000 | 1852, 11);
		builder.word(0);

		Common::NEResources reader;
		TS_ASSERT(builder.load(reader));
		checkResource(reader, 32513, 1852, 11);
		checkResource(reader, Common::String("aseq"), 1852, 11);
		const auto ids = reader.getIDList(Common::String("AsEq"));
		TS_ASSERT_EQUALS(ids.size(), 1U);
		if (!ids.empty())
			TS_ASSERT_EQUALS(ids[0].getID(), 1852U);
		TS_ASSERT(reader.getIDList(Common::kWinBitmap).empty());
		TS_ASSERT(reader.getIDList(Common::String()).empty());
		Common::ScopedPtr<Common::SeekableReadStream> data(reader.getResource(Common::kWinBitmap, 1852));
		TS_ASSERT(!data);

		Common::WinResourceID type(Common::String("ASEQ"));
		checkResource(reader, type, 1852, 11);
		type = 32513;
		checkResource(reader, type, 1852, 11);
		type = Common::String();
		TS_ASSERT(reader.getIDList(type).empty());
		type = Common::WinResourceID();
		TS_ASSERT(reader.getIDList(type).empty());
	}

	void test_native_named_type_and_id() {
		NEResourceBuilder builder;
		uint typePos = builder.tablePos;
		builder.type(0, 1);
		uint idPos = builder.tablePos + 6;
		builder.resource(0, 11);
		builder.word(0);
		WRITE_LE_UINT16(builder.bytes.data() + typePos, builder.name("CUSTOM_TYPE"));
		WRITE_LE_UINT16(builder.bytes.data() + idPos, builder.name("HANDCURSOR"));

		Common::NEResources reader;
		TS_ASSERT(builder.load(reader));
		checkResource(reader, Common::String("custom_type"), Common::String("handcursor"), 11);
		const auto ids = reader.getIDList(Common::String("Custom_Type"));
		TS_ASSERT_EQUALS(ids.size(), 1U);
		if (!ids.empty())
			TS_ASSERT_EQUALS(ids[0].getString(), Common::String("HANDCURSOR"));
	}

	void test_type_and_resource_aliases_keep_resource_name_case() {
		byte names[] = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 'A', 'S', 'E', 'Q', 0x00,
			'E', 'N', 'T', 'R', 'Y', 0x00, 0x00, 0x00};
		WRITE_LE_UINT16(names, sizeof(names) - 2);
		WRITE_LE_UINT16(names + 2, 0x8000 | 32513);
		WRITE_LE_UINT16(names + 4, 0x8000 | 1852);
		NEResourceBuilder builder;
		builder.type(0x8000 | Common::kWinNameTable, 1);
		builder.resource(0x8001, names, sizeof(names));
		builder.type(0x8000 | 32513, 1);
		builder.resource(0x8000 | 1852, 11);
		builder.word(0);

		Common::NEResources reader;
		TS_ASSERT(builder.load(reader));
		checkResource(reader, 32513, Common::String("ENTRY"), 11);
		checkResource(reader, Common::String("aseq"), Common::String("ENTRY"), 11);
		Common::ScopedPtr<Common::SeekableReadStream> data(reader.getResource(Common::String("aseq"), Common::String("entry")));
		TS_ASSERT(!data);
	}
};
