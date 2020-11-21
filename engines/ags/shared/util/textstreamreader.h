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
// Class for reading plain text from the stream
//
//=============================================================================
#ifndef __AGS_CN_UTIL__TEXTSTREAMREADER_H
#define __AGS_CN_UTIL__TEXTSTREAMREADER_H

#include "util/textreader.h"

namespace AGS
{
namespace Common
{

class Stream;

class TextStreamReader : public TextReader
{
public:
    // TODO: use shared ptr
    TextStreamReader(Stream *stream);
    ~TextStreamReader() override;

    bool    IsValid() const override;
    const Stream   *GetStream() const;
    // TODO: use shared ptr instead
    void            ReleaseStream();

    bool            EOS() const;

    // Read single character
    char    ReadChar() override;
    // Read defined number of characters
    String  ReadString(size_t length) override;
    // Read till line break
    String  ReadLine() override;
    // Read till end of available data
    String  ReadAll() override;

private:
    Stream *_stream;
};

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__TEXTSTREAM_H
