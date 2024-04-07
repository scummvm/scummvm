#include "stdafx.h"
#include "HLSL.h"
#include "ProcessString.h"
#include "ProcessInclude.h"
#include <direct.h>
#include "saver\saver.h"
/*
 Новый формат - внутри psl, vsl в комментариях,
имя шейдера, дефайны и версия шейдера в одной строчке.
Есть проверка на то, что константы не меняют регистров.

Шейдеры выбираются и группируются по названиям в дефайнах
 - неуниверсально, но удобно для вертексных шейдеров.
Шейдеры выбираются и группируются по именам в программе -
 - универсально, но громоздко. Удобно для пиксельных шейдеров.

/Ffilename
  /Nname /Ddefine=off,1,2 /Ttarget_profile

*/
bool BuildHLSL(ShaderLine& sl,const char* out_filename);
extern int shaders_compiled;

string out_dir;

bool getFileTime(const char* fname,FILETIME& last_write)
{
	FILETIME creation, last_access;
	HANDLE file = CreateFile( fname, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
	if(file==INVALID_HANDLE_VALUE)
		return false;

	if(!GetFileTime( file, &creation, &last_access, &last_write ))
	{
		CloseHandle(file);
		return false;
	}

	CloseHandle(file);
	return true;
}

FILETIME getFileTime(const char* fname)
{
	FILETIME creation, last_access, last_write;
	HANDLE file = CreateFile( fname, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
	if(file==INVALID_HANDLE_VALUE)
	{
		string s="File cannot open to read: ";
		s+=fname;
		printf("%s\n",s.c_str());
		exit(1);
		last_write.dwLowDateTime=last_write.dwHighDateTime=0;
		return last_write;
	}

	if(!GetFileTime( file, &creation, &last_access, &last_write ))
	{
		string s="File cannot read time: ";
		s+=fname;
		printf("%s\n",s.c_str());
		exit(1);
		return last_write;
	}

	CloseHandle(file);
	return last_write;
}

struct LinkInfo
{
	string in_name;
	string out_name;
};

bool MustLink(vector<LinkInfo>& link_files,const char* out_name)
{
	FILETIME out_time;

	if(!getFileTime(out_name,out_time))
		return true;

	SYSTEMTIME sys_out;
	FileTimeToSystemTime(&out_time,&sys_out);

	for(vector<LinkInfo>::iterator it=link_files.begin();it!=link_files.end();it++)
	{
		LinkInfo& li=*it;
		FILETIME in_time=getFileTime(li.out_name.c_str());
		SYSTEMTIME sys_in;
		FileTimeToSystemTime(&in_time,&sys_in);
		if(CompareFileTime(&in_time,&out_time)>=0)
		{
			return true;
		}
	}

	return false;
}

bool MustCompile(vector<string>& dependices,const char* out_name)
{
	FILETIME out_time;
	if(!getFileTime(out_name,out_time))
		return true;

	SYSTEMTIME sys_out;
	FileTimeToSystemTime(&out_time,&sys_out);

	for(vector<string>::iterator it=dependices.begin();it!=dependices.end();it++)
	{
		string& li=*it;
		FILETIME in_time=getFileTime(li.c_str());
		SYSTEMTIME sys_in;
		FileTimeToSystemTime(&in_time,&sys_in);
		if(CompareFileTime(&in_time,&out_time)>=0)
		{
			return true;
		}
	}

	return false;
}

void Link(vector<LinkInfo>& link_files,const char* out_name)
{
	CSaver saver;
	if(!saver.Init(out_name))
	{
		printf("Cannot write %s\n",out_name);
		exit(1);
	}

	for(vector<LinkInfo>::iterator it=link_files.begin();it!=link_files.end();it++)
	{
		LinkInfo& li=*it;
		saver.push(SD_SUPER_SHADER);
			saver.push(SD_SUPER_SHADER_NAME);
			saver<<li.in_name;
			saver.pop();

			CLoadDirectoryFile load;
			if(!load.Load(li.out_name.c_str()))
			{
				printf("Cannot load %s\n",li.out_name.c_str());
				exit(1);
			}
			saver.write(load.GetData(),load.GetDataSize());
		saver.pop();
	}

}

void HexConvert(const char* in,const char* out)
{
	FILE* f=fopen(in,"rb");
	if(f==NULL)
	{
		printf("Cannot open %s\n",in);
		exit(1);
	}
	if(fseek(f,0,SEEK_END))
		return;

	int size=ftell(f);
	if(fseek(f,0,SEEK_SET))
		return;

	char* data=new char[size];
	fread(data,1,size,f);

	FILE* fout=fopen(out,"wt");
	if(fout==NULL)
	{
		printf("Cannot open %s\n",out);
		exit(1);
	}

	OutHexBuffer(fout,(DWORD*)data,size);
	fprintf(fout,"\tint shader_size=%i;\n",size);

	fclose(fout);

	delete[] data;
	fclose(f);

}

void File(const char* progect_file_name)
{
	const char* out_file="all_shaders.inc";

	bool must_all_recompile=true;
	{
		FILETIME out_time;
		if(!getFileTime(out_file,out_time))
		{
			must_all_recompile=true;
		}else
		{
			FILETIME in_time;
			if(!getFileTime(progect_file_name,in_time))
			{
				must_all_recompile=true;
			}else
			{
				must_all_recompile=CompareFileTime(&in_time,&out_time)>=0;
			}
		}
	}

	FILE* f=fopen(progect_file_name,"rb");
	if(f==NULL)
	{
		printf("Cannot open %s\n",progect_file_name);
		exit(1);
	}
	if(fseek(f,0,SEEK_END))
		return;

	int size=ftell(f);
	if(fseek(f,0,SEEK_SET))
		return;

	char* data=new char[size+1];
	fread(data,1,size,f);
	data[size]=0;

	Process p(data);

	delete[] data;
	fclose(f);

	vector<LinkInfo> link_files;

	vector<ShaderLine>& sla=p.Get();
	for(DWORD line=0;line<sla.size();line++)
	{
		ShaderLine& sl=sla[line];
		vector<string> include;
		ProcessInclude(sl.filename.c_str(),include);
		//for(DWORD i=0;i<include.size();i++)
		//{
		//	printf("%s\n",include[i].c_str());
		//}

		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		_splitpath(sl.out_filename.c_str(),drive,dir,fname,ext);

		string out_name=dir;
		if(!out_name.empty())
		{
			char cend=*out_name.rbegin();
			if(cend!='\\' && cend!='/')
			{
				out_name+='\\';
			}
		}

		out_name+=fname;
		LinkInfo li;
		li.in_name=sl.out_filename;
		for(string::iterator it=out_name.begin();it!=out_name.end();it++)
		{
			char& c=*it;
			if(c=='\\' || c=='/')
			{
				c='_';
			}
		}
		
		out_name=out_dir+out_name;

		bool is_vsl=stricmp(ext,".vsl")==0;
		bool is_psl=stricmp(ext,".psl")==0;

		if(is_vsl || is_psl)
		{
			if(is_vsl)
				out_name+=".svsl";
			else
				out_name+=".spsl";

			li.out_name=out_name;

			include.push_back(sl.filename.c_str());

			if(MustCompile(include,li.out_name.c_str()) || must_all_recompile)
			{
				if(!BuildHLSL(sl,out_name.c_str()))
				{
					printf("Error compile %s -> %s\n",sl.filename.c_str(),li.out_name.c_str());
					remove(out_name.c_str());
					exit(1);
				}
			}
		}else
		{
			printf("Unknown extension %s\n",sl.filename.c_str());
			exit(1);
		}

		link_files.push_back(li);
	}

	if(MustLink(link_files,out_file) || must_all_recompile)
	{
		printf("Linking shader...\n");
		const char* hex_file="all_shaders.hex";
		Link(link_files,hex_file);
		HexConvert(hex_file,out_file);
		remove(hex_file);
	}

}

int main(int argc, char* argv[])
{
	if(argc>=3)
	{
		out_dir=argv[2];
		char* part;
		char filename_full[512];
		if(GetFullPathName(out_dir.c_str(),sizeof(filename_full),filename_full,&part)==0)
		{
			printf("Error (bad directory name) %s\n",out_dir.c_str());
		}

		out_dir=filename_full;
		char cend=*out_dir.rbegin();
		if(cend!='\\' && cend!='/')
		{
			out_dir+='\\';
		}

		mkdir(out_dir.c_str());
	}

	if(argc>=2)
	{

		if(_chdir(argv[1])!=0)
		{
			printf("Cannot change dir to %s",argv[1]);
			exit(1);
		}
	}


	File("shaders.txt");
	printf("Sahers compiled =%i",shaders_compiled);
	return 0;
	if(false)
	{
		_controlfp( _controlfp(0,0) & ~(EM_OVERFLOW | EM_ZERODIVIDE | EM_DENORMAL |  EM_INVALID),  MCW_EM ); 
		_clearfp();
	}

	if(argc<2)
	{
		printf("This utility convert psh vsh vsl file to C .h file\n");
		printf("\tShaderToH.exe fname /Eentry_point /Ddefine=value /Ooutname\n");
		return 1;
	}

	char* in=argv[1];

	char out[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	char* outname=fname;
	_splitpath(in,drive,dir,fname,ext);
	for(int i=2;i<argc;i++)
	{
		char* c=argv[i];
		if(c[0]=='/' && c[1]=='O')
		{
			outname=c+2;
		}
	}

	strcat(dir,"o");
	{
		char out_dir[_MAX_PATH];
		strcpy(out_dir,drive);
		strcat(out_dir,dir);
		CreateDirectory(out_dir,NULL);
	}
	strcat(dir,"\\");

	vector<D3DXMACRO> macros;
	for(i=2;i<argc;i++)
	{
		char* c=argv[i];
		if(c[0]=='/' && c[1]=='D')
		{
			D3DXMACRO d;
			d.Name=c+2;
			d.Definition=NULL;
			for(char* p=c+2;*p;p++)
			{
				if(*p=='=')
				{
					*p=0;
					d.Definition=p+1;
					break;
				}
			}
			macros.push_back(d);
		}
	}
	if(!macros.empty())
	{
		D3DXMACRO d;
		d.Name=NULL;
		d.Definition=NULL;
		macros.push_back(d);
	}

	if(stricmp(ext,".psh")==0)
	{
		_makepath(out,drive,dir,outname,"ph");
	}else
	if(stricmp(ext,".vsh")==0)
	{
		_makepath(out,drive,dir,outname,"vh");
	}else
	if(stricmp(ext,".vsl")==0)
	{
		_makepath(out,drive,dir,outname,"vl");
		printf("Converting %s -> %s\n",in,out);
		const char* entry_point="main";
		for(i=2;i<argc;i++)
		{
			char* c=argv[i];
			if(c[0]=='/' && c[1]=='E')
			{
				entry_point=c+2;
			}
		}

		return ProcessHLSL(in,out,fname,ext,entry_point,macros);
	}else
		_makepath(out,drive,dir,outname,"h");

	return ProcessASM(in,out,fname,ext,NULL,macros);
}
