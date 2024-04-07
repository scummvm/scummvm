#ifndef _PARAMPARSE_H_
#define _PARAMPARSE_H_

#include "BitSet.h"
#include <vector>
#include <string>

class ParseParam
{
protected:
	char* text;
	BitSet b_char,space;

public:
	int line;
	char* cur;

	ParseParam(char* _text);
	inline bool IsSpace(char c)
	{
		if(c==10)line++;
		return space.In(c);
	}

	void SkipSpace();

	void ReadName(char* buf);

	bool IsInt();
	bool IsFloat();
	bool IsString();

	int ReadInt();
	float ReadFloat();
	void ReadString(char* buf);
};

class OneBlock
{
public:
	std::string name;

	int ivalue;
	float fvalue;
	std::string svalue;

	std::vector<OneBlock*> child;
	OneBlock* parent;

	ParseParam& pp;

	OneBlock(ParseParam& _pp,OneBlock* _parent);
	~OneBlock();

	void Process();
protected:
	void Read();
	void ReadBlock();

	bool bconst_read;
};

#endif  _PARAMPARSE_H_
