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
//
// LogFile, the IOutputHandler implementation that writes to file.
//
// When created LogFile may open file right away or delay doing this.
// In the latter case it will buffer output up to certain size limit.
// When told to open the file, it will first flush its buffer. This allows to
// log events even before the log path is decided (for example, before or
// during reading configuration and/or parsing command line).
//
//=============================================================================
#ifndef __AGS_EE_DEBUG__LOGFILE_H
#define __AGS_EE_DEBUG__LOGFILE_H

#include <memory>
#include "debug/outputhandler.h"

namespace AGS
{

namespace Common { class Stream; }

namespace Engine
{

using Common::DebugMessage;
using Common::Stream;
using Common::String;

class LogFile : public AGS::Common::IOutputHandler
{
public:
    enum OpenMode
    {
        kLogFile_Overwrite,
        kLogFile_OverwriteAtFirstMessage,
        kLogFile_Append
    };

public:
        LogFile();

    void PrintMessage(const Common::DebugMessage &msg) override;

    // Open file using given file path, optionally appending if one exists
    //
    // TODO: filepath parameter here may be actually used as a pattern
    // or prefix, while the actual filename could be made by combining
    // this prefix with current date, game name, and similar additional
    // useful information. Whether this is to be determined here or on
    // high-level side remains a question.
    //
    bool         OpenFile(const String &file_path, OpenMode open_mode = kLogFile_Overwrite);
        // Close file
    void         CloseFile();

private:
        std::unique_ptr<Stream> _file;
        String                _filePath;
        OpenMode              _openMode;
};

}   // namespace Engine
}   // namespace AGS

#endif // __AGS_EE_DEBUG__LOGFILE_H
