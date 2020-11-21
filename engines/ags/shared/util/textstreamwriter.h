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
// Class for writing plain text to the stream
//
//=============================================================================
#ifndef __AGS_CN_UTIL__TEXTSTREAMWRITER_H
#define __AGS_CN_UTIL__TEXTSTREAMWRITER_H

#include "util/textwriter.h"

namespace AGS
{
namespace Common
{

class Stream;

class TextStreamWriter : public TextWriter
{
public:
    // TODO: use shared ptr
    TextStreamWriter(Stream *stream);
    ~TextStreamWriter() override;

    bool    IsValid() const override;
    const Stream   *GetStream() const;
    // TODO: use shared ptr instead
    void            ReleaseStream();

    bool            EOS() const;

    // Write single character
    void    WriteChar(char c) override;
    // Write string as a plain text (without null-terminator)
    void    WriteString(const String &str) override;
    // Write string and add line break at the end
    void    WriteLine(const String &str) override;
    // Write formatted string (see *printf)
    void    WriteFormat(const char *fmt, ...) override;
    void    WriteLineBreak() override;

private:
    Stream *_stream;
};

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__TEXTSTREAMWRITER_H
