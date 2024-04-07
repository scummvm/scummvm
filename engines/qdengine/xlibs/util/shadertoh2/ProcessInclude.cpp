#include "StdAfx.h"
#include "ProcessInclude.h"

void SkipSpace(char*& cur);

struct Include
{
	bool once;
	string name;

	Include():once(false){}
};

void ProcessInclude(const char* filename,vector<Include>& include)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath(filename,drive,dir,fname,ext);
	string cur_dir=drive;
	cur_dir+=dir;

	char* part;
	char filename_full[512];
	if(GetFullPathName(filename,sizeof(filename_full),filename_full,&part)==0)
	{
		printf("Error (bad filename) %s\n",filename);
		exit(1);
	}

	for(vector<Include>::iterator it=include.begin();it!=include.end();it++)
	{
		char include_full[512];
		Include& ic=*it;
		if(GetFullPathName(ic.name.c_str(),sizeof(include_full),include_full,&part)==0)
		{
			printf("Error (bad filename) %s\n",filename);
			exit(1);
		}

		if(stricmp(filename_full,include_full)==0)
		{
			if(ic.once)
			{
				//Рекурсивный инклюд
				return;
			}else
			{
				ic.once=true;
				break;
			}
		}
	}


	FILE* f=fopen(filename,"rt");
	if(f==NULL)
	{
		printf("Cannot open %s\n",filename);
		exit(1);
	}
	int line=0;
	char buf[512];
	while(fgets(buf,sizeof(buf),f))
	{
		line++;
		char* c=buf;
		SkipSpace(c);

		char inc[]="#include";
		int sz=sizeof(inc)-1;
		if(memcmp(c,inc,sz)!=0)
			continue;
		c+=sz;
		SkipSpace(c);
		if(*c!='"')
		{
			printf("Error %s(%i)\n",filename,line);
			exit(1);
		}
		c++;
		char* begin=c;

		while(*c && *c!='"')
		{
			c++;
		}

		if(*c!='"')
		{
			printf("Error %s(%i)\n",filename,line);
			exit(1);
		}

		if(c>begin)
		{
			Include inc;
			inc.name.assign(begin,c);
			inc.name=cur_dir+inc.name;

			bool found=false;
			char filename_full1[512];
			if(GetFullPathName(inc.name.c_str(),sizeof(filename_full1),filename_full1,&part)==0)
			{
				printf("Error (bad filename) %s\n",filename);
				exit(1);
			}

			for(vector<Include>::iterator it=include.begin();it!=include.end();it++)
			{
				char include_full[512];
				Include& ic=*it;
				if(GetFullPathName(ic.name.c_str(),sizeof(include_full),include_full,&part)==0)
				{
					printf("Error (bad filename) %s\n",ic.name.c_str());
					exit(1);
				}

				if(stricmp(filename_full1,include_full)==0)
				{
					found=true;
					break;
				}
			}

			if(!found)
				include.push_back(inc);
		}
	}
	fclose(f);

	for(DWORD iti=0;iti<include.size();iti++)
	{
		Include& ic=include[iti];
		string fname=ic.name;
		if(!ic.once)
			ProcessInclude(fname.c_str(),include);
	}
}

void ProcessInclude(const char* filename,vector<string>& include)
{
	vector<Include> inc;
	ProcessInclude(filename,inc);
	include.resize(inc.size());
	for(DWORD i=0;i<inc.size();i++)
		include[i]=inc[i].name;
}
