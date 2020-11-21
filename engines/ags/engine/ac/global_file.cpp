//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include <string.h>
#include "ac/global_file.h"
#include "ac/common.h"
#include "ac/file.h"
#include "ac/path_helper.h"
#include "ac/runtime_defines.h"
#include "ac/string.h"
#include "debug/debug_log.h"
#include "util/directory.h"
#include "util/path.h"
#include "util/stream.h"

using namespace AGS::Common;

int32_t FileOpenCMode(const char*fnmm, const char* cmode)
{
  Common::FileOpenMode open_mode;
  Common::FileWorkMode work_mode;
  // NOTE: here we ignore the text-mode flag. AGS 2.62 did not let
  // game devs to open files in text mode. The file reading and
  // writing logic in AGS makes extra control characters added for
  // security reasons, and FileWriteRawLine adds CR/LF to the end
  // of string on its own.
  if (!Common::File::GetFileModesFromCMode(cmode, open_mode, work_mode))
  {
      return 0;
  }
  return FileOpen(fnmm, open_mode, work_mode);
}

// Find a free file slot to use
int32_t FindFreeFileSlot()
{
  int useindx = 0;
  for (; useindx < num_open_script_files; useindx++) 
  {
    if (valid_handles[useindx].stream == nullptr)
      break;
  }

  if (useindx >= num_open_script_files &&
      num_open_script_files >= MAX_OPEN_SCRIPT_FILES)
  {
    quit("!FileOpen: tried to open more than 10 files simultaneously - close some first");
    return -1;
  }
  return useindx;
}

int32_t FileOpen(const char*fnmm, Common::FileOpenMode open_mode, Common::FileWorkMode work_mode)
{
  int32_t useindx = FindFreeFileSlot();
  if (useindx < 0)
    return 0;

  ResolvedPath rp;
  if (open_mode == kFile_Open && work_mode == kFile_Read)
  {
    if (!ResolveScriptPath(fnmm, true, rp))
      return 0;
  }
  else
  {
    if (!ResolveWritePathAndCreateDirs(fnmm, rp))
      return 0;
  }

  Stream *s = File::OpenFile(rp.FullPath, open_mode, work_mode);
  if (!s && !rp.AltPath.IsEmpty() && rp.AltPath.Compare(rp.FullPath) != 0)
    s = File::OpenFile(rp.AltPath, open_mode, work_mode);

  valid_handles[useindx].stream = s;
  if (valid_handles[useindx].stream == nullptr)
    return 0;
  valid_handles[useindx].handle = useindx + 1; // make handle indexes 1-based

  if (useindx >= num_open_script_files)
    num_open_script_files++;
  return valid_handles[useindx].handle;
}

void FileClose(int32_t handle) {
  ScriptFileHandle *sc_handle = check_valid_file_handle_int32(handle,"FileClose");
  delete sc_handle->stream;
  sc_handle->stream = nullptr;
  sc_handle->handle = 0;
  }
void FileWrite(int32_t handle, const char *towrite) {
  Stream *out = get_valid_file_stream_from_handle(handle,"FileWrite");
  out->WriteInt32(strlen(towrite)+1);
  out->Write(towrite,strlen(towrite)+1);
  }
void FileWriteRawLine(int32_t handle, const char*towrite) {
  Stream *out = get_valid_file_stream_from_handle(handle,"FileWriteRawLine");
  out->Write(towrite,strlen(towrite));
  out->WriteInt8 (13);
  out->WriteInt8 (10);
  }
void FileRead(int32_t handle,char*toread) {
  VALIDATE_STRING(toread);
  Stream *in = get_valid_file_stream_from_handle(handle,"FileRead");
  if (in->EOS()) {
    toread[0] = 0;
    return;
  }
  int lle=in->ReadInt32();
  if ((lle>=200) | (lle<1)) quit("!FileRead: file was not written by FileWrite");
  in->Read(toread,lle);
  }
int FileIsEOF (int32_t handle) {
  Stream *stream = get_valid_file_stream_from_handle(handle,"FileIsEOF");
  if (stream->EOS())
    return 1;

  // TODO: stream errors
  if (stream->HasErrors())
    return 1;

  if (stream->GetPosition () >= stream->GetLength())
    return 1;
  return 0;
}
int FileIsError(int32_t handle) {
  Stream *stream = get_valid_file_stream_from_handle(handle,"FileIsError");

  // TODO: stream errors
  if (stream->HasErrors())
    return 1;

  return 0;
}
void FileWriteInt(int32_t handle,int into) {
  Stream *out = get_valid_file_stream_from_handle(handle,"FileWriteInt");
  out->WriteInt8('I');
  out->WriteInt32(into);
  }
int FileReadInt(int32_t handle) {
  Stream *in = get_valid_file_stream_from_handle(handle,"FileReadInt");
  if (in->EOS())
    return -1;
  if (in->ReadInt8()!='I')
    quit("!FileReadInt: File read back in wrong order");
  return in->ReadInt32();
  }
char FileReadRawChar(int32_t handle) {
  Stream *in = get_valid_file_stream_from_handle(handle,"FileReadRawChar");
  if (in->EOS())
    return -1;
  return in->ReadInt8();
  }
int FileReadRawInt(int32_t handle) {
  Stream *in = get_valid_file_stream_from_handle(handle,"FileReadRawInt");
  if (in->EOS())
    return -1;
  return in->ReadInt32();
}
void FileWriteRawChar(int32_t handle, int chartoWrite) {
  Stream *out = get_valid_file_stream_from_handle(handle,"FileWriteRawChar");
  if ((chartoWrite < 0) || (chartoWrite > 255))
    quit("!FileWriteRawChar: can only write values 0-255");

  out->WriteInt8(chartoWrite);
}
