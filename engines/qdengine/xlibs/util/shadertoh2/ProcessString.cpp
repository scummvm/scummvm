#include "StdAfx.h"
#include "ProcessString.h"
#include <algorithm>
#include "ParamParse.h"

const MAXSTRLEN=1024;
/*
	Новый синтаксис.

"имя_директории\имя_файла" {
 defines {
   имя_дефайна=значения;//off,1,2,5
 }
 static {
  имя_дефайна=значения;
 }
 //дальше идет самое интересное - под какую версию шейдера компилировать.
 profile=ps_2_0;
 profile="ps_1_1" {
  имя_дефайна=значения; //При этих значениях дефайна версия будет другая. 
  //Если дефайна нет в этом списке, то возможно любое значение.
 }
 если блоков  profile несколько, и результат подходит под каждый блок, то profile будет взято из последнего.

 profile теперь пусть будет функцией на lua.
      возвращает версию шейдера. 
	  Если возвращает пустую строчку - такой вариант компилировать не надо.

 profile=$$
    if(SECOND_OPACITY_TEXTURE=="" && SECOND_UVTRANS=="1")
		return "";
    if(ZREFLECTION==1)
	  return "ps_2_0";
	return "ps_1_1";
 $$
}
	
*/
ParseParamH::ParseParamH(char* _text)
	:ParseParam(_text)
{
}

string ParseParamH::GetCommaString()
{
	SkipSpace();
	char* begin=cur;

	while(*cur)
	{
		if(space.In(*cur) || *cur==',' || *cur==';')
			break;
		cur++;
	}

	if(cur==begin)
	{
		throw "String not found";
	}

	string s;
	s.assign(begin,cur);
	return s;
}

bool ParseParamH::IsEnd(char c)
{
	return c==0 || space.In(c);
}

void ParseParamH::ProcessRange(vector<string>& value)
{
	//format off,define1,define2,define3

	while(!IsEnd(*cur))
	{
		SkipSpace();
		if(cur[0]=='o' && cur[1]=='f' && cur[2]=='f')
		{
			string r;
			value.push_back(r);
			cur+=3;
		}else
		{
			string r;
			r=GetCommaString();
			value.push_back(r);
		}

		SkipSpace();
		if(*cur==';')
			break;
		if(*cur!=',')
			throw ", not found";
		cur++;
	}
}

bool IsSpace(char c)
{
	if(c==' ' || c==8)
		return true;
	return false;
}

bool IsCRLF(char c)
{
	return c==10 || c==13;
}

bool IsEnd(char c)
{
	return IsSpace(c) || c==0;
}

Process::Process(char* begin_str)
:pp(begin_str)
{
	try {
		while(*pp.cur)
		{
			ReadOneDefinition();
			pp.SkipSpace();
		}
	}catch(char* err)
	{
		printf("line=%i %s\n",pp.line,err);
		exit(1);
	}
}

void SkipSpace(char*& cur)
{
	while(*cur && IsSpace(*cur))
		cur++;
}

void SkipNoSpace(char*& cur)
{
	while(*cur && !IsSpace(*cur))
		cur++;
}

void SkipNoSpaceEq(char*& cur)
{
	while(*cur && !IsSpace(*cur) && *cur!='=')
		cur++;
}

bool GetString(string& s,char*& cur)
{
	SkipSpace(cur);
	char* begin=cur;
	SkipNoSpace(cur);
	if(cur<=begin)
	{
		s.clear();
		return false;
	}
	s.assign(begin,cur);
	return true;
}

bool GetStringEq(string& s,char*& cur)
{
	SkipSpace(cur);
	char* begin=cur;
	SkipNoSpaceEq(cur);
	if(cur<=begin)
	{
		s.clear();
		return false;
	}
	s.assign(begin,cur);
	return true;
}

struct SortByStaticDefinition
{
	bool operator()(const Definition& p1,const Definition& p2)
	{
		return p1.static_definition<p2.static_definition;
	}
};

void Process::OpenBracket()
{
	pp.SkipSpace();
	if(*pp.cur!='{')
		throw "Must be {";
	else
		pp.cur++;
}

void Process::CloseBracket()
{
	pp.SkipSpace();
	if(*pp.cur!='}')
		throw "Must be }";
	else
		pp.cur++;
}


void Process::ReadOneDefinition()
{
	char word[MAXSTRLEN];
	pp.ReadName(word);

	ShaderLine line;
	line.filename=word;
	line.out_filename=line.filename;
	line.target_profile="vs_1_1";

	OpenBracket();

	while(*pp.cur)
	{
		pp.SkipSpace();
		if(*pp.cur=='}')
			break;
		pp.ReadName(word);
		if(strcmp(word,"define")==0)
			ReadDefine(line,false);
		else
		if(strcmp(word,"static")==0)
			ReadDefine(line,true);
		else
		if(strcmp(word,"profile")==0)
			ReadProfile(line);
		else
		if(strcmp(word,"output")==0)
			ReadOutput(line);
		else{
			throw "Unknown word";
		}
	}

	CloseBracket();
/*
	while(*cur)
	{
		char*& c=cur;
		SkipSpace(c);
		if(!*c)
			break;

		if(*c=='/' && c[1]=='/')  
			break;

		if(c[0]=='/' && (c[1]=='D' || c[1]=='S'))
		{
			Definition d;
			d.static_definition=c[1]=='S';
			c+=2;
			GetStringEq(d.name,c);
			SkipSpace(c);

			if(*c=='=')
			{
				c++;
				ProcessRange(c,d);
			}

			line.def.push_back(d);
		}else
		if(c[0]=='/' && c[1]=='O')
		{
			c+=2;
			string out_filename;
			if(GetString(out_filename,c))
			{
				char out[_MAX_PATH];
				char drive[_MAX_DRIVE];
				char dir[_MAX_DIR];
				char fname[_MAX_FNAME];
				char ext[_MAX_EXT];
				_splitpath(line.filename.c_str(),drive,dir,fname,ext);
				_makepath(out,drive,dir,out_filename.c_str(),ext);
				line.out_filename=out;
			}else
				Err("Cannot parse out_filename\n");
		}else
		if(c[0]=='/' && c[1]=='T')
		{
			c+=2;
			string target;
			if(GetString(target,c))
			{
				line.target_profile=target;
			}else
			{
				Err("Cannot parse target profile\n");
			}
		}else
		{
			Err("Unformat\n");
		}
	}
*/
	sort(line.def.begin(),line.def.end(),SortByStaticDefinition());
	shader_line.push_back(line);
}

void Process::List()
{
	for(vector<ShaderLine>::iterator ith=shader_line.begin();ith!=shader_line.end();ith++)
	{
		printf("%s ",ith->filename.c_str());
		vector<Definition>& def=ith->def;
		for(vector<Definition>::iterator it=def.begin();it!=def.end();it++)
		{
			Definition& d=*it;
			printf("%s ",d.name.c_str());
			for(DWORD irange=0;irange<d.value.size();irange++)
			{
				string& r=d.value[irange];
				printf("%s ",r.c_str());
			}
		}
		printf("\n");
	}
}

void Process::ReadDefine(ShaderLine& line,bool is_static)
{
	OpenBracket();

	while(*pp.cur)
	{
		Definition d;
		d.static_definition=is_static;
		pp.SkipSpace();
		if(*pp.cur=='}')
			break;
		char word[MAXSTRLEN];
		pp.ReadName(word);
		d.name=word;
		pp.SkipSpace();
		if(*pp.cur!='=')
			throw "Must be =";
		else
			pp.cur++;

		pp.ProcessRange(d.value);

		if(*pp.cur!=';')
			throw "Must be ;";
		*pp.cur++;

		line.def.push_back(d);
	}

	CloseBracket();
}

void Process::ReadProfile(ShaderLine& line)
{
	char word[MAXSTRLEN];
	pp.SkipSpace();
	if(*pp.cur!='=')
		throw "Must be =";
	*pp.cur++;

	pp.SkipSpace();
	if(pp.cur[0]=='$' && pp.cur[1]=='$')
	{
		pp.cur+=2;
		const char* begin=pp.cur;

		while(*pp.cur && !(pp.cur[0]=='$' && pp.cur[1]=='$'))
			pp.cur++;
		if(!*pp.cur)
			throw "Must be $$";
		line.profile_script.assign(begin,pp.cur-begin);
		pp.cur+=2;
		return;
	}

	pp.ReadName(word);

	pp.SkipSpace();
	char cend=*pp.cur;
	if(cend!=';' && cend!='{')
		throw "Must be ; or {";
	*pp.cur++;

	if(cend!='{')
	{
		line.target_profile=word;
		return;
	}

	ExcludeProfile e;
	e.target_profile=word;

	while(*pp.cur)
	{
		Definition d;
		d.static_definition=false;

		pp.SkipSpace();
		if(*pp.cur=='}')
			break;
		pp.ReadName(word);
		d.name=word;

		bool found=false;
		for(int i=0;i<(int)line.def.size();i++)
		{
			string& name=line.def[i].name;
			if(name==word)
			{
				found=true;
				break;
			}
		}

		if(!found)
			throw "Name not found in define or static";

		pp.SkipSpace();
		if(*pp.cur!='=')
			throw "Must be =";
		pp.cur++;
		pp.ProcessRange(d.value);

		if(*pp.cur!=';')
			throw "Must be ;";
		*pp.cur++;

		e.def.push_back(d);
	}

	CloseBracket();
	line.exclude_profile.push_back(e);
}

void Process::ReadOutput(ShaderLine& line)
{
	char word[MAXSTRLEN];
	if(*pp.cur!='=')
		throw "Must be =";
	pp.cur++;
	pp.ReadName(word);

	char out[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath(line.filename.c_str(),drive,dir,fname,ext);
	_makepath(out,drive,dir,word,ext);
	line.out_filename=out;

	if(*pp.cur!=';')
		throw "Must be ;";
	*pp.cur++;
}
