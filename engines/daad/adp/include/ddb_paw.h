#ifndef INC_DDB_PAW_H
#define INC_DDB_PAW_H

#include <ddb.h>

#if HAS_PAWS

enum
{
	DDB_PAWS_FLAG_BYTES = 256,
	DDB_PAWS_OBJECT_BYTES = 256,
	DDB_PAWS_GRAPHIC_BYTES = 32,
	DDB_PAWS_STATE_SIZE = DDB_PAWS_FLAG_BYTES + DDB_PAWS_OBJECT_BYTES + DDB_PAWS_GRAPHIC_BYTES,
	DDB_PAWS_TICK_MS = 20,
	DDB_PAWS_TIME_TICKS = 64,
	DDB_SDB_MAX_SEGMENTS = 16,
};

enum DDB_SDBMemoryModel
{
	DDB_SDB_48K = 48,
	DDB_SDB_128K = 128,
};

enum DDB_SDBSegmentRole
{
	DDB_SDB_SEGMENT_LOW = 1,
	DDB_SDB_SEGMENT_HIGH = 2,
};

struct DDB_SDBSegment
{
	uint8_t bank;
	uint8_t role;
	uint16_t loadAddress;
	uint16_t length;
	const uint8_t* data;
};

extern bool DDB_PAWSIsSDB(File* file);
extern bool DDB_PAWSLoadSDB(File* file, uint8_t** memory, size_t* memorySize,
	uint8_t* memoryModel, uint8_t* segmentCount);
extern bool DDB_PAWSBuildSDB(uint8_t memoryModel, const DDB_SDBSegment* segments,
	uint8_t segmentCount, uint8_t** data, size_t* size);
extern bool DDB_PAWSWriteSDB(const char* filename, uint8_t memoryModel,
	const DDB_SDBSegment* segments, uint8_t segmentCount);
extern uint32_t DDB_PAWSSDBCRC32(const uint8_t* data, size_t size);

extern void DDB_PAWSResetState(DDB_Interpreter* i);
extern void DDB_PAWSSetMode(DDB_Interpreter* i, uint8_t mode, uint8_t options);
extern void DDB_PAWSSetLine(DDB_Interpreter* i, uint8_t line);
extern void DDB_PAWSProtect(DDB_Interpreter* i);
extern void DDB_PAWSSetGraphic(DDB_Interpreter* i, uint8_t option);
extern void DDB_PAWSSetTime(DDB_Interpreter* i, uint8_t duration, uint8_t options);
extern void DDB_PAWSSetInput(DDB_Interpreter* i, uint8_t options);
extern uint8_t DDB_PAWSRandom(DDB_Interpreter* i);
extern void DDB_PAWSClear(DDB_Interpreter* i);
extern void DDB_PAWSOutputAnyKeyMessage(DDB_Interpreter* i);
extern bool DDB_PAWSPrepareDescription(DDB_Interpreter* i, uint8_t location);
extern void DDB_PAWSFinishDescription(DDB_Interpreter* i, bool automaticPictureDrawn);
extern void DDB_PAWSReloadScrollCounter(DDB_Interpreter* i, DDB_Window* w);
extern void DDB_PAWSRefreshWindow(DDB_Interpreter* i, DDB_Window* w);
extern int DDB_PAWSValidatedInternalRow(uint8_t row);
extern int DDB_PAWSUserRow(uint8_t internalRow);
extern void DDB_PAWSStartPause(DDB_Interpreter* i, uint8_t ticks);
extern bool DDB_PAWSAdvancePause(DDB_Interpreter* i, uint32_t elapsedMs);
extern void DDB_PAWSStartTimeout(DDB_Interpreter* i);
extern bool DDB_PAWSAdvanceTimeout(DDB_Interpreter* i, uint32_t elapsedMs);

#endif

#endif
