#pragma once

#if HAS_XMSG
#include <ddb.h>

extern bool     xmsgFilePresent;

extern bool             DDB_OpenXMessageFile        (const char* ddbFileName);
extern int              DDB_InitializeXMessageCache (uint32_t size);

extern const uint8_t*   DDB_GetXMessage             (uint32_t offset);

extern void             DDB_CloseXMessageFile       ();
extern void             DDB_FreeXMessageCache       ();

#endif