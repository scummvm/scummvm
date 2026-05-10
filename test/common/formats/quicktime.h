#include <cxxtest/TestSuite.h>
#include "common/util.h"
#include "common/formats/quicktime.h"

static const byte VALID_MOOV_DATA[] = { // a minimally 'correct' quicktime file.
	// size				'moov'					size				'mdat'
	0x0, 0x0, 0x0, 0x8, 0x6d, 0x6f, 0x6f, 0x76, 0x0, 0x0, 0x0, 0x8, 0x6d, 0x64, 0x61, 0x74
};

static const byte VALID_MHDR_DATA[] = { // a 'correct' quicktime file with a header
	// size	(incl mvhd)	'moov'
	0x0, 0x0, 0x0, 0x74, 0x6d, 0x6f, 0x6f, 0x76,
	//size (27*4)		'mvhd'					vers  3bytes flags
	0x0, 0x0, 0x0, 0x6c, 0x6d, 0x76, 0x68, 0x64, 0x00, 0xff, 0xff, 0xff,
	// creation				modification			timescale (60?)		length (999 * 60)+ 1
	0x65, 0x52, 0xef, 0x5b, 0x65, 0x52, 0xef, 0x5b, 0x0, 0x0, 0x0, 0x3c, 0x0, 0x0, 0xea, 0x25,
	// preferred scale, vol, 		[10 bytes reserved]
	0x0, 0x0, 0x0, 0x1, 0x0, 0x10, 0,0,0,0,0,0,0,0,0,0,
	// display matrix, mostly ignored by parser except xMod (0x8000) and yMod (0xa000)
	0x0, 0x0, 0x80, 0x0, 0,0,0,0,0,0,0,0,0,0,0,0, 0x0, 0x0, 0xa0, 0x0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	// 7 more 32-bit values
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	// size				'mdat'
	0x0, 0x0, 0x0, 0x8, 0x6d, 0x64, 0x61, 0x74
};


class QuickTimeTestParser : public Common::QuickTimeParser {
public:
	uint32 getDuration() const { return _duration; }
	const Common::Rational &getScaleFactorX() const { return _scaleFactorX; }
	const Common::Rational &getScaleFactorY() const { return _scaleFactorY; }
	const Common::Array<Track *> &getTracks() const { return _tracks; }

	SampleDesc *readSampleDesc(Track *track, uint32 format, uint32 descSize) override {
		return nullptr;
	}
};

class QuicktimeParserTestSuite : public CxxTest::TestSuite {
public:
	void test_streamAtEOS() {
		QuickTimeTestParser parser;
		const byte data[] = "";
		Common::MemoryReadStream stream(data, sizeof(data));
		stream.readByte(); // read the null char
		bool result = parser.parseStream(&stream, DisposeAfterUse::NO);
		TS_ASSERT(!result);
	}

	void test_streamInvalid() {
		QuickTimeTestParser parser;
		const byte data[] = "not a moov";
		Common::MemoryReadStream stream(data, sizeof(data));
		bool result = parser.parseStream(&stream, DisposeAfterUse::NO);
		TS_ASSERT(!result);
	}

	void test_moov() {
		QuickTimeTestParser parser;
		Common::MemoryReadStream stream(VALID_MOOV_DATA, sizeof(VALID_MOOV_DATA));
		bool result = parser.parseStream(&stream, DisposeAfterUse::NO);
		TS_ASSERT(result);
	}

	void test_mhdr() {
		QuickTimeTestParser parser;
		Common::MemoryReadStream stream(VALID_MHDR_DATA, sizeof(VALID_MHDR_DATA));
		bool result = parser.parseStream(&stream, DisposeAfterUse::NO);
		TS_ASSERT(result);
		TS_ASSERT_EQUALS(parser.getDuration(), 999u*60 + 1);
		TS_ASSERT_EQUALS(parser.getScaleFactorX(), Common::Rational(0x10000, 0x8000));
		TS_ASSERT_EQUALS(parser.getScaleFactorY(), Common::Rational(0x10000, 0xa000));
	}

	void test_mhdrEarlyEOF() {
		QuickTimeTestParser parser;
		Common::MemoryReadStream stream(VALID_MHDR_DATA, sizeof(VALID_MHDR_DATA) - 10);
		bool result = parser.parseStream(&stream, DisposeAfterUse::NO);
		TS_ASSERT(!result);
	}

};
