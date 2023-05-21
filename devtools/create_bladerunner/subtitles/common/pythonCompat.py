#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

def makeUnicode(inp):
        if sys.version_info[0] <= 2:
                return unicode(inp, 'utf-8')
        else:
                return inp

def makeAscii(inp):
        if sys.version_info[0] <= 2:
                return inp
        else:
                return inp.encode('ascii')

def unicodeEncode(inp, enc):
        if sys.version_info[0] <= 2:
                return unicode.encode(inp, enc)
        else:
                return inp.encode(enc)

def encodedSplit(s, spl):
        if sys.version_info[0] <= 2:
                return s.split(spl)
        elif isinstance(spl, bytes):
                return s.split(spl)
        else:
                return s.split(spl.encode('ascii'))

def getUnicodeSym(s):
        if sys.version_info[0] <= 2:
                return unichr(ord(s))
        return s

def makeToBytes(s):
	if sys.version_info[0] <= 2:
		return [ord(i) for i in s]
	return bytes(i for i in s)
