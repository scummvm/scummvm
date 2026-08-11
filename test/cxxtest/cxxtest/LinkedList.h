/*
-------------------------------------------------------------------------
 CxxTest: A lightweight C++ unit testing library.
 Copyright (c) 2008 Sandia Corporation.
 This software is distributed under the LGPL License v3
 For more information, see the COPYING file in the top CxxTest directory.
 Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------
*/

#ifndef __cxxtest__LinkedList_h__
#define __cxxtest__LinkedList_h__

#include <cxxtest/Flags.h>

namespace CxxTest
{
struct List;
class Link;

struct List
{
    Link *_head;
    Link *_tail;

    void initialize();

    Link *head();
    const Link *head() const;
    Link *tail();
    const Link *tail() const;

    bool empty() const;
    unsigned size() const;
    Link *nth(unsigned n);

    void activateAll();
    void leaveOnly(const Link &link);
};

class Link
{
public:
    Link();
    virtual ~Link();

    bool active() const;
    void setActive(bool value = true);

    Link *justNext();
    Link *justPrev();

    Link *next();
    Link *prev();
    const Link *next() const;
    const Link *prev() const;

    void attach(List &l);
    void detach(List &l);

private:
    Link *_next;
    Link *_prev;
    bool _active;

    Link(const Link &);
    Link &operator=(const Link &);
};
}

#endif // __cxxtest__LinkedList_h__
