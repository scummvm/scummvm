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
// IniFile class defines contents of the configuration file.
// It serves as a INI parser and plain enumerator of all the sections and items
// found in file, or, oppositely, as INI file constructor.
// But is not much suitable for regular key/value lookup. It is suggested to
// create a proper map to store items, from IniFile contents.
//
//=============================================================================
#ifndef __AGS_CN_UTIL__INIFILE_H
#define __AGS_CN_UTIL__INIFILE_H

#include <list>
#include "util/string.h"

namespace AGS
{
namespace Common
{

class IniFile
{
public:
    // Position of a string in the line of text:
    // is defined by a pair of first and next-after-last character indices
    typedef std::pair<int, int> StrPos;
    // Location of section in the array of text lines:
    // is defined by a pair of first and next-after-last line indices
    typedef std::pair<int, int> SectionPos;

    // Item definition
    // Valid key indicates a key-value line; no key means unparsed
    // line of text, e.g. comment or incorrectly formatted item.
    class ItemDef
    {
    public:
        ItemDef(const String &key, const String &value);
        ItemDef(const String &line, const StrPos &key, const StrPos &value, int sep_at);
        String GetLine()  const { return Line; }
        String GetKey()   const { return SubString(Line, Key); }
        String GetValue() const { return SubString(Line, Value); }
        bool   IsKeyValue() const { return Key.second - Key.first > 0; }
        void SetKey(const String &key);
        void SetValue(const String &value);

    private:
        String  Line;  // actual text
        StrPos  Key;   // position of item key
        int     SepAt; // position of the separator (assignment) symbol
        StrPos  Value; // position of item value
    };
    // Linked list of items
    typedef std::list<ItemDef> LItems;
    typedef LItems::iterator          ItemIterator;
    typedef LItems::const_iterator    ConstItemIterator;

    // Section definition
    class SectionDef
    {
    public:
        SectionDef(const String &name);
        SectionDef(const String &line, const StrPos &name);
        String GetLine() const { return Header; }
        String GetName() const { return SubString(Header, Name); }
        size_t GetItemCount() const { return Items.size(); }
        bool   IsGlobal() const { return Name.second - Name.first <= 0; }
        ItemIterator Begin() { return Items.begin(); }
        ItemIterator End()   { return Items.end(); }
        ConstItemIterator CBegin() const { return Items.begin(); }
        ConstItemIterator CEnd()   const { return Items.end(); }
        void SetName(const String &sec_name);
        void Clear();
        ItemIterator InsertItem(ItemIterator item, const ItemDef &itemdef);
        void EraseItem(ItemIterator item);

    private:
        String      Header;// section's heading line
        StrPos      Name;  // location of section name in the header line
        LItems      Items; // linked list of items belonging to the section
    };
    // Linked list of sections
    typedef std::list<SectionDef>     LSections;
    typedef LSections::iterator       SectionIterator;
    typedef LSections::const_iterator ConstSectionIterator;

private:
    inline static String SubString(const String &line, const StrPos &pos)
    {
        return line.Mid(pos.first, pos.second - pos.first);
    }

public:
    IniFile();

    SectionIterator Begin() { return _sections.begin(); }
    SectionIterator End()   { return _sections.end(); }
    ConstSectionIterator CBegin() const { return _sections.begin(); }
    ConstSectionIterator CEnd()   const { return _sections.end(); }

    void Read(Stream *in);
    void Write(Stream *out) const;

    // Return number of sections
    size_t GetSectionCount() const { return _sections.size(); }
    // Insert new item *before* existing item
    ItemIterator InsertItem(SectionIterator sec, ItemIterator item, const String &key, const String &value);
    // Insert new section *before* existing section
    SectionIterator InsertSection(SectionIterator sec, const String &name);
    // Remove a single item
    void RemoveItem(SectionIterator sec, ItemIterator item);
    // Completely remove whole section; this removes all lines between section
    // header and the last item found in that section (inclusive).
    void RemoveSection(SectionIterator sec);

private:
    LSections _sections;
};

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__INIFILE_H
