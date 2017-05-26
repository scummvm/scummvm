// Copyright (c) 2010 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

// Modified by Rikard Peterson 2011 to fit in the SLUDGE engine.

#include "mkvreader.hpp"
#include "../newfatal.h"
#include "../fileset.h"

#include <cassert>
#include <stdio.h>

MkvReader::MkvReader() :
    m_file(0)
{
}

MkvReader::~MkvReader()
{
    Close();
}

int MkvReader::Open(int fileNumber)
{
    if (! fileNumber)
        return -1;

    if (m_file)
        return -1;

	m_file = fileNumber;

	setResourceForFatal (fileNumber);
	m_length = openFileFromNum (fileNumber);
	if (m_length == 0) {
		finishAccess();
		setResourceForFatal (-1);
		return -1;
	}
	/*
#ifdef WIN32
	m_start = _ftelli64(bigDataFile);
#else*/
    m_start = ftell(bigDataFile);
/*#endif
*/
	finishAccess();
    return 0;
}

void MkvReader::Close()
{
    if (m_file)
    {
		finishAccess();
		setResourceForFatal (-1);
        m_file = 0;
    }
}

int MkvReader::Length(long long* total, long long* available)
{
    if (! m_file)
        return -1;

    if (total)
        *total = m_length;

    if (available)
        *available = m_length;

    return 0;
}

int MkvReader::Read(long long offset, long len, unsigned char* buffer)
{
    if (! m_file)
        return -1;

    if (offset < 0)
        return -1;

    if (len < 0)
        return -1;

    if (len == 0)
        return 0;


    if (offset >= m_length)
        return -1;

	if (startAccess())
		fprintf(stderr, "Warning: Datafile already in use when playing movie!\n");
/*
#ifdef WIN32
    const int status = _fseeki64(bigDataFile, m_start+offset, SEEK_SET);

    if (status)
        return -1;  //error
#else*/
    fseek(bigDataFile, m_start+offset, SEEK_SET);
//#endif

    const size_t size = fread(buffer, 1, len, bigDataFile);

	finishAccess();

    if (size < size_t(len))
        return -1;  //error

    return 0;  //success
}
