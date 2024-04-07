#pragma once
#include "ShaderData.h"
#include "ParamParse.h"

struct Definition
{
	string name;
	bool static_definition;
	vector<string> value;
};

struct ExcludeProfile
{
	string target_profile;
	vector<Definition> def;//Имена должны быть обязательно в ShaderLine::def 
};

struct ShaderLine
{
	string filename;
	string out_filename;
	string target_profile;
	vector<Definition> def;
	vector<ExcludeProfile> exclude_profile;
	string profile_script;
};

class ParseParamH:public ParseParam
{
public:
	ParseParamH(char* _text);
	string GetCommaString();
	bool IsEnd(char c);
	void ProcessRange(vector<string>& def);
};

class Process
{
public:
	Process(char* str);

	vector<ShaderLine>& Get(){return shader_line;};
protected:
	ParseParamH pp;

	vector<ShaderLine> shader_line;
	void OpenBracket();
	void CloseBracket();

	void ReadOneDefinition();
	void ReadDefine(ShaderLine& line,bool is_static);
	void ReadProfile(ShaderLine& line);
	void ReadOutput(ShaderLine& line);


	void List();
};
