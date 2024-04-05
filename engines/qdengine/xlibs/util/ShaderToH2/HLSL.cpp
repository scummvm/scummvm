#include "stdafx.h"
#include "HLSL.h"
#include "ProcessString.h"
#include "saver\saver.h"
#include "ShaderData.h"
#include "LuaExec.h"

int shaders_compiled=0;

int ProcessHLSL(const char* in,const char* out,const char* fname,const char* ext,
				const char* entry_point,vector<D3DXMACRO>& define)
{
	LPD3DXBUFFER pShader=NULL;
    LPD3DXBUFFER pErrorBuf = NULL;
	LPD3DXCONSTANTTABLE pConstantTable=NULL;

	HRESULT hr=D3DXCompileShaderFromFile(in,define.empty()?NULL:&define[0],
				NULL,entry_point,"vs_1_1",
				0,&pShader,&pErrorBuf,&pConstantTable);

	shaders_compiled++;
	if(FAILED(hr))
	{
		if(pErrorBuf)
		{
			char* buf=(char*)pErrorBuf->GetBufferPointer();
			printf(buf);
		}else
		{
			printf("%s unknown error\n",in);
		}

		DeleteFile(out);
		return 1;
	}

	FILE* fin=fopen(in,"rt");
	if(!fin)
	{
		printf("Cannot read %s",in);
		return 1;
	}
	FILE* fout=fopen(out,"wt");
	if(!fout)
	{
		printf("Cannot write %s\n",out);
		return 1;
	}

	fprintf(fout,"{\n");

	{

		int size=pShader->GetBufferSize();
		DWORD* pd=(DWORD*)pShader->GetBufferPointer();
		if(size%4)
		{
			printf("%s bad align\n",in);
			return false;
		}

		OutHexBuffer(fout,pd,size);
	}

	D3DXCONSTANTTABLE_DESC all_desc;
	if(FAILED(pConstantTable->GetDesc(&all_desc)))
	{
		printf("%s cannot pConstantTable->GetDesc table",in);
		return 1;
	}

	fprintf(fout,"static ConstShaderInfo shader_info[]={\n");
	for(DWORD i=0;i<all_desc.Constants;i++)
	{
		D3DXHANDLE hConstant=pConstantTable->GetConstant(NULL,i);
		D3DXCONSTANT_DESC desc;
		UINT ndesc=1;
		hr=pConstantTable->GetConstantDesc(hConstant,&desc,&ndesc);
		if(FAILED(hr) || ndesc!=1)
		{
			printf("%s cannot pConstantTable->GetConstantDesc",in);
			return 1;
		}

		fprintf(fout,"\t\t{\"%s\",%i,%i},\n",desc.Name,desc.RegisterIndex,desc.RegisterCount);

	};
	fprintf(fout,"\n\t};\n");

	fprintf(fout,"\tpShaderInfo=shader_info;\n");
	fprintf(fout,"\tShaderInfoSize=sizeof(shader_info)/sizeof(shader_info[0]);\n");
	
	fprintf(fout,"\tCompileAndFound(\"%s%s\",shader);\n",fname,ext);

	fprintf(fout,"}\n");
	fclose(fin);
	fclose(fout);
	return 0;
}


bool OutHexBuffer(FILE* fout,DWORD* pd,int size)
{
	size=(size+3)/4;

	fprintf(fout,"static DWORD shader[]={\n");
	const int line=6;
	for(int i=0;i<size;i++)
	{
		if(i%line==0)fprintf(fout,"\t");
		fprintf(fout,"0x%08x, ",pd[i]);
		if(i%line==line-1)fprintf(fout,"\n");
	}

	fprintf(fout,"\n};\n");
	return true;
}

int ProcessASM(const char* in,const char* out,const char* fname,const char* ext,const char* entry_point,vector<D3DXMACRO>& define)
{
	printf("Converting %s -> %s\n",in,out);
	FILE* fin=fopen(in,"rt");
	if(!fin)
	{
		printf("Cannot read %s",in);
		return 1;
	}
	fclose(fin);

	LPD3DXBUFFER pShaderBuf = NULL;
    LPD3DXBUFFER pErrorBuf = NULL;

	HRESULT hr=D3DXAssembleShaderFromFile(in,define.empty()?NULL:&define[0],NULL,0,&pShaderBuf, &pErrorBuf);
	if(FAILED(hr))
	{
		if(pErrorBuf)
		{
			char* buf=(char*)pErrorBuf->GetBufferPointer();
			printf(buf);
		}else
		{
			printf("%s unknown error\n",in);
		}
		return 1;
	}

	int size=pShaderBuf->GetBufferSize();
	DWORD* pd=(DWORD*)pShaderBuf->GetBufferPointer();

	FILE* fout=fopen(out,"wt");
	if(!fout)
	{
		printf("Cannot write %s\n",out);
		return 1;
	}

	if(size%4)
	{
		printf("%s bad align\n",in);
		return 1;
	}

	if(!OutHexBuffer(fout,pd,size))
		return 1;

	if(stricmp(ext,".psh")==0 || stricmp(ext,".vsh")==0)
	{
		fprintf(fout,"\tCompile(\"%s%s\",shader);\n",fname,ext);
	}


	fclose(fout);

	if(pShaderBuf)pShaderBuf->Release();
	if(pErrorBuf)pErrorBuf->Release();
	return 0;
}

class DefIterator
{
	int num_value;
	int cur_value;
	Definition* def;
public:
	DefIterator();
	void init(Definition* def_);
	void reset(){cur_value=0;}
	string& cur();
	bool next(){cur_value++;return is_end();};
	bool is_end(){return cur_value<num_value;}

	int GetNumValue(){return num_value;}
};

DefIterator::DefIterator()
:def(NULL)
{
	cur_value=0;
	num_value=0;
}

void DefIterator::init(Definition* def_)
{
	def=def_;
	cur_value=0;
	num_value=def->value.size();
}


string& DefIterator::cur()
{
	assert(cur_value>=0 && cur_value<num_value);
	return def->value[cur_value];
}


struct ConstDesc
{
	string name;
	int index;
	int count;

	ConstDesc()
	{
		index=-1;
		count=-1;
	}
};

bool operator==(const ConstDesc& c1,const ConstDesc& c2)
{
	return c1.name==c2.name;
}

void PrintDefine(vector<D3DXMACRO>& define)
{
	for(DWORD i=0;i<define.size();i++)
	{
		D3DXMACRO& d=define[i];
		if(d.Name==NULL)
			break;
		printf("%s=%s ",d.Name,d.Definition);
	}
	printf("\n");
}

bool SaveHLSL(const char* in,vector<D3DXMACRO>& define,CSaver& saver,vector<ConstDesc>& out_desc,const char* target_profile="vs_1_1")
{
	_strlwr((char*)target_profile);

	LPD3DXBUFFER pShader=NULL;
    LPD3DXBUFFER pErrorBuf = NULL;
	LPD3DXCONSTANTTABLE pConstantTable=NULL;

	D3DXMACRO* defines=define.empty()?NULL:&define[0];
	HRESULT hr=D3DXCompileShaderFromFile(in,defines,
				NULL,"main",target_profile,
				0,&pShader,&pErrorBuf,&pConstantTable);
	shaders_compiled++;
	if(FAILED(hr))
	{
		if(pErrorBuf)
		{
			char* buf=(char*)pErrorBuf->GetBufferPointer();
			PrintDefine(define);
			printf(buf);
		}else
		{
			printf("%s unknown error\n",in);
		}

		return false;
	}else
	{
		if(pErrorBuf)
		{
			char* buf=(char*)pErrorBuf->GetBufferPointer();
			PrintDefine(define);
			printf(buf);
		}
	}

	{

		DWORD size=pShader->GetBufferSize();
		DWORD* pd=(DWORD*)pShader->GetBufferPointer();
		if(size%4)
		{
			printf("%s bad align\n",in);
			return false;
		}

		if(0)
		{
			LPD3DXBUFFER pDisassembly=0;
			HRESULT hr=D3DXDisassembleShader(
				pd,
				FALSE,
				"balmer comment",
				&pDisassembly
			);

			if(pDisassembly)
			{
				char* ptr=(char*)pDisassembly->GetBufferPointer();
				int size=pDisassembly->GetBufferSize();
				const char* prefix="// approximately ";
				char* p=strstr(ptr,prefix);
				if(p)
				{
					p+=strlen(prefix);
					printf(p);
				}

				int k=0;
				//FILE* f=fopen("disasm.txt","wb");
				//fwrite(pDisassembly->GetBufferPointer(),1,pDisassembly->GetBufferSize()-1,f);
				//fclose(f);
				pDisassembly->Release();
			}
		}

		size/=4;
		saver.push(SD_SHADER_VERSION);
		saver<<target_profile;
		saver.pop();

		saver.push(target_profile[0]=='p'?SD_SHADER_COMPILED_PIXEL:SD_SHADER_COMPILED);
		saver<<size;
		for(DWORD i=0;i<size;i++)
			saver<<pd[i];
		saver.pop();
	}

	D3DXCONSTANTTABLE_DESC all_desc;
	if(FAILED(pConstantTable->GetDesc(&all_desc)))
	{
		printf("%s cannot pConstantTable->GetDesc table",in);
		return false;
	}

	for(DWORD i=0;i<all_desc.Constants;i++)
	{
		D3DXHANDLE hConstant=pConstantTable->GetConstant(NULL,i);
		D3DXCONSTANT_DESC desc;
		UINT ndesc=1;
		hr=pConstantTable->GetConstantDesc(hConstant,&desc,&ndesc);
		if(FAILED(hr) || ndesc!=1)
		{
			printf("%s cannot pConstantTable->GetConstantDesc",in);
			return false;
		}

		if(desc.RegisterSet!=D3DXRS_FLOAT4)
			continue;

		out_desc.push_back(ConstDesc());
		ConstDesc& cd=out_desc.back();
		cd.name=desc.Name;
		cd.index=desc.RegisterIndex;
		cd.count=desc.RegisterCount;
	};

	if(pErrorBuf)pErrorBuf->Release();
	if(pConstantTable)pConstantTable->Release();
	if(pShader)pShader->Release();
	return true;
}

string GetCurProfile(ShaderLine& sl,vector<DefIterator>& iterators)
{
	string profile=sl.target_profile;
	for(int iex=0;iex<(int)sl.exclude_profile.size();iex++)
	{
		ExcludeProfile& ex=sl.exclude_profile[iex];
		bool passed=true;
		for(DWORD i=0;i<sl.def.size();i++)
		{
			Definition* d=NULL;
			for(int iname=0;iname<(int)ex.def.size();iname++)
			{
				if(ex.def[iname].name==sl.def[i].name)
				{
					d=&ex.def[iname];
					break;
				}
			}

			if(d==NULL)
				continue;

			DefIterator& di=iterators[i];
			string& value=di.cur();

			bool found=false;
			for(int ivalue=0;ivalue<(int)d->value.size();ivalue++)
			{
				if(d->value[ivalue]==value)
				{
					found=true;
					break;
				}
			}

			if(!found)
			{
				passed=false;
				break;
			}
		}

		if(passed)
			profile=ex.target_profile;
	}

	return profile;
}

string GetCurProfileLua(ShaderLine& sl,vector<DefIterator>& iterators,LuaExec& lua)
{
	for(DWORD i=0;i<sl.def.size();i++)
	{
		string& name=sl.def[i].name;
		DefIterator& di=iterators[i];
		string& value=di.cur();
		lua.set(name.c_str(),value.c_str());
	}

	string profile=lua.run();
	return profile;
}

bool BuildHLSL(ShaderLine& sl,const char* out_filename)
{
	LuaExec lua;

	if(!sl.profile_script.empty())
	{
		if(!lua.open(sl.profile_script.c_str(),sl.profile_script.size(),sl.filename.c_str()))
			return false;
	}


	{
		printf("Compiling %s ",sl.filename.c_str());
		vector<Definition>& def=sl.def;
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

	vector<DefIterator> iterators(sl.def.size());
	for(DWORD i=0;i<sl.def.size();i++)
		iterators[i].init(&sl.def[i]);

	CSaver saver;
	if(!saver.Init(out_filename))
	{
		printf("Cannot write %s\n",out_filename);
		return false;
	}


	vector<ConstDesc> global_desc;
	bool is_end=false;
	while(!is_end)
	{
		vector<D3DXMACRO> define;
		saver.push(SD_SHADER);
			saver.push(SD_SHADER_MACROS_VALUE);
			saver<<(DWORD)sl.def.size();
			for(DWORD i=0;i<sl.def.size();i++)
			{
				DefIterator& di=iterators[i];
				string& value=di.cur();
				saver<<value;
				if(value.empty())
					continue;

				D3DXMACRO d;
				d.Name=sl.def[i].name.c_str();
				d.Definition=value.c_str();
				define.push_back(d);
			}
			saver.pop();
//			PrintDefine(define);

			if(!define.empty())
			{
				D3DXMACRO d;
				d.Name=NULL;
				d.Definition=NULL;
				define.push_back(d);
			}

			string profile=GetCurProfile(sl,iterators);

			if(!sl.profile_script.empty())
				profile=GetCurProfileLua(sl,iterators,lua);

			vector<ConstDesc> cur_desc;
			if(!SaveHLSL(sl.filename.c_str(),define,saver,cur_desc,profile.c_str()))
				return false;
		saver.pop();
		//add desc
		{

			for(vector<ConstDesc>::iterator it=cur_desc.begin();it!=cur_desc.end();it++)
			{
				ConstDesc& cd=*it;
				vector<ConstDesc>::iterator itf=find(global_desc.begin(),global_desc.end(),cd);
				if(itf==global_desc.end())
				{
					global_desc.push_back(cd);
				}else
				{
					ConstDesc& gd=*itf;
					assert(gd.name==cd.name);
					if(gd.index!=cd.index || gd.count!=cd.count)
					{
						printf("Error index not equal %s(%i!=%i,%i!=%i) ",cd.name.c_str(),gd.index,cd.index,gd.count,cd.count);
						PrintDefine(define);
						return false;
					}
				}
			}

		}

		//next
		if(sl.def.empty())
			break;
		for(i=0;i<sl.def.size();i++)
		{
			if(iterators[i].next())
				break;
			if(i==sl.def.size()-1)
			{
				is_end=true;
				break;
			}
			iterators[i].reset();
		}
	}

	{//пересекающиеся константы
		for(int i=0;i<(int)global_desc.size();i++)
		{
			ConstDesc& di=global_desc[i];
			int min_i=di.index;
			int max_i=di.index+di.count-1;

			for(int j=i+1;j<(int)global_desc.size();j++)
			{
				ConstDesc& dj=global_desc[j];
				int min_j=dj.index;
				int max_j=dj.index+dj.count-1;
				int minx=max(min_i,min_j);
				int maxx=min(max_i,max_j);
				if(minx<=maxx)
				{
					printf("Constant %s and %s have intersect range\n",di.name.c_str(),dj.name.c_str());
					return false;
				}
			}
		}
	}

	{
		saver.push(SD_CONSTANT_BIND);
		saver<<(DWORD)global_desc.size();
		for(DWORD i=0;i<global_desc.size();i++)
		{
			ConstDesc& gd=global_desc[i];
			saver<<gd.name;
			saver<<gd.index;
			saver<<gd.count;
		}
		saver.pop();
	}

	{
		saver.push(SD_MACROS);
		for(DWORD i=0;i<sl.def.size();i++)
		{
			Definition& d=sl.def[i];
			DefIterator& di=iterators[i];
			saver.push(SD_ONE_MACROS);
				saver.push(SD_ONE_MACROS_NAME);
				saver<<d.name;
				saver<<di.GetNumValue();
				saver<<d.static_definition;
				saver.pop();

				saver.push(SD_ONE_MACROS_RANGE);
				saver<<(DWORD)d.value.size();
				for(DWORD ri=0;ri<d.value.size();ri++)
				{
					saver<<d.value[ri];
				}
				saver.pop();

			saver.pop();
		}
		saver.pop();
	}
	return true;
}
