/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * rec.h
 *
 * HISTORY
 *
 *      1.00      04/19/94     Jos     Creation
 *
 * MODULE DESCRIPTION:
 *
 *      [Describe the function of the module]
 *
 * RELEVANT DOCUMENTATION:
 *
 *      [Specifications, documents, test plans, etc.]
 *
 ****************************************************************/


#include <lzexpand.h>

#ifndef __game_rec_H__
#define __game_rec_H__

#define CRYPT_TXT_FILE		"Crypt.lz"
#define CRYPT_RECS			200

#define MAX_GRAM_LEN		512
#define MAX_SOURCE_LEN		128
#define RECORD_LEN			392


class CCryptRecord {
private:
	HFILE	m_hfCryptFile;
	int 	m_nID;
	char 	m_lpszGram[MAX_GRAM_LEN];
	char 	m_lpszSource[MAX_SOURCE_LEN];

public:

	CCryptRecord();		// constructor
	~CCryptRecord();    // destructor

	BOOL 	GetRecord(int nID);
	int 	GetID() { return m_nID; };
	char*	GetGram()	{ return m_lpszGram; };
	char* 	GetSource() { return m_lpszSource; };
};
#endif // __game_rec_H__
