#include "stdafx.h"
//#include <windows.h>
#include "ParamParse.h"
//#include <stdio.h>
using namespace std;

static char error[2048];
const offset_enum=1000;
const MAXSTRLEN=1024;

void ParseParam::SkipSpace()
{
Retry:
	while(IsSpace(*cur))cur++;
	if(*cur=='/' && cur[1]=='/')
	{
		while(*cur && *cur!=0xD && *cur!=0xA)
			cur++;

		while(*cur && *cur==0xD && *cur==0xA)
		{
			if(*cur==10)
				line++;
			cur++;
		}

		if(*cur)goto Retry;
	}

	if(*cur=='/' && cur[1]=='*')
	{
		cur+=2;
		while(*cur && !(*cur=='*' && cur[1]=='/'))
		{
			if(*cur==10)
				line++;
			cur++;
		}

		if(*cur)
		{
			cur+=2;
			goto Retry;
		}
	}

}


ParseParam::ParseParam(char* _text)
:text(_text),line(1)
{
	b_char.Set('a','z');
	b_char.Set('A','Z');
	b_char.Set('_');

	space.Set((unsigned char)32);
	space.Set((unsigned char)9);
	space.Set((unsigned char)10);
	space.Set((unsigned char)13);

	cur=text;
}

void ParseParam::ReadName(char* buf)
{
	char* p=buf;
	SkipSpace();

	if(IsString())
	{
		ReadString(buf);
		return;
	}

	if(b_char.In(*cur))
		*p++=*cur++;

	while(b_char.In(*cur) || (*cur>='0' && *cur<='9'))
		*p++=*cur++;

	*p=0;

	if(*buf==0)
		throw "Cannot read object name";
}

bool ParseParam::IsInt()
{
	char* p=cur;

	if(*p=='+' || *p=='-')
		p++;

	if(!(*p>='0' && *p<='9'))
		return false;

	while(*p>='0' && *p<='9')
		p++;

	return true;

}

bool ParseParam::IsFloat()
{
	char* p=cur;
	if(*p=='+' || *p=='-')
		p++;

	if(!(*p>='0' && *p<='9'))
		return false;

	while(*p>='0' && *p<='9')
		p++;

	if(*p=='.')
		return true;

	return *p=='e' || *p=='E';
}

bool ParseParam::IsString()
{
	return *cur=='"';
}

int ParseParam::ReadInt()
{
	char buf[MAXSTRLEN];
	char* p=buf;
	if(*cur=='+' || *cur=='-') *p++=*cur++;
	while(*cur>='0' && *cur<='9')
	{
		*p++=*cur++;
	}
	*p=0;

	return atoi(buf);
}

float ParseParam::ReadFloat()
{
	char buf[MAXSTRLEN];
	char* p=buf;

	if(*cur=='+' || *cur=='-') *p++=*cur++;
	while(*cur>='0' && *cur<='9')*p++=*cur++;
	
	if(*cur=='.')
	{
		*p++=*cur++;
		while(*cur>='0' && *cur<='9')*p++=*cur++;
	}

	if(*cur=='e' || *cur=='E')
	{
		*p++=*cur++;
		if(*cur=='+' || *cur=='-') *p++=*cur++;
		while(*cur>='0' && *cur<='9')*p++=*cur++;
	}

	*p=0;

	return (float)atof(buf);
}

void ParseParam::ReadString(char* buf)
{
	if(*cur!='"')
		throw "Must be \"";
	cur++;

	for(;*cur && *cur!='"';cur++)
		if(*cur!=10)
			*buf++=*cur;
		else
		{
			line++;
		}

	if(*cur!='"')
		throw "Must be \"";
	cur++;
	*buf=0;
}

//////////////////OneBlock////////////////////
OneBlock::OneBlock(ParseParam& _pp,OneBlock* _parent)
:pp(_pp),parent(_parent)
{
	bconst_read=false;
}

OneBlock::~OneBlock()
{
	for(vector<OneBlock*>::iterator it=child.begin();it!=child.end();it++)
		delete *it;
	child.clear();
}

void OneBlock::Process()
{
	if(parent)
	{
		Read();
	}else
	{
		pp.SkipSpace();
		ReadBlock();
	}
}

void OneBlock::Read()
{
	char word[MAXSTRLEN];
	pp.ReadName(word);
	name=word;

	pp.SkipSpace();

	if(*pp.cur=='{')
	{
		pp.cur++;
		ReadBlock();
	}else
	if(*pp.cur=='=')
	{
		pp.cur++;
		pp.SkipSpace();

		if(pp.IsString())
		{
			pp.ReadString(word);
			svalue=word;
		}else
		if(pp.IsFloat())
		{
			fvalue=pp.ReadFloat();
		}else
		if(pp.IsInt())
		{
			ivalue=pp.ReadInt();
		}else
			throw "Непонятное значение";
	
		pp.SkipSpace();

	}else
	if(*pp.cur==';')
	{
	}else
		throw "Требуется = или {";

	if(*pp.cur!=';')
		throw "Здесь необходима ;";
	*pp.cur++;
}

void OneBlock::ReadBlock()
{

	while(*pp.cur)
	{
		pp.SkipSpace();
		if(*pp.cur==0)break;
		if(*pp.cur=='}')
		{
			pp.cur++;
			return;
		}

		OneBlock* p=new OneBlock(pp,this);
		p->Process();
		child.push_back(p);
	}
	
	if(parent!=0)
		throw "Здесь необходима }";
}

//////////////////////////////////////
/*
char* ParseText(const char* text,ParamBlock** paramblock,int* size,ParamBlock* pRules,
				NonStaticByConst pGetIntByConstNonstatic)
{
	if(paramblock)
		*paramblock=0;
	if(size)
		*size=0;

	ParseParam pp((char*)text,pRules,pGetIntByConstNonstatic);

	try 
	{
		OneBlock ob(pp,0);
		ob.Process();
		//ob.Print("");

		MemBlock m;
		ob.Save(m);
		if(paramblock)
		{
			*paramblock=(ParamBlock*)m.p;
			m.p=0;
		}

		if(size)
			*size=m.size;
	}catch(char* err)
	{
		static char word[MAXSTRLEN];
		sprintf(word,"line=%i %s\n",pp.line,err);
		return word;
	}

	return 0;
}
*/
