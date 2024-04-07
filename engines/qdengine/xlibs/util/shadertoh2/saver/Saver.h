//** 1999 Creator - Balmer **//
#pragma once

#include "XMath\xmath.h"

#define WR(x) fwrite(&x,sizeof(x),1,f);

class CSaver
{
	vector<DWORD> p;
	DWORD m_Data;
public:
	FILE* f;
	CSaver(const char* name);
	CSaver();
	~CSaver();

	DWORD GetData(){return m_Data;}
	DWORD SetData(DWORD dat){return m_Data = dat;}

	bool Init(const char* name);

	void push(const unsigned long id);//Вызывать при начале записи блока
	void pop();//Вызывать при окончании записи блока

protected:
	void push();
public:
	inline CSaver& operator<<(const char *x);
	inline CSaver& operator<<(const string& x){*this<<x.c_str(); return *this;}
	inline CSaver& operator<<(bool x){WR(x); return *this;};
	inline CSaver& operator<<(char x){WR(x); return *this;};
	inline CSaver& operator<<(unsigned char x){WR(x); return *this;};
	inline CSaver& operator<<(int x){WR(x); return *this;};
	inline CSaver& operator<<(unsigned int x){WR(x); return *this;};
	inline CSaver& operator<<(short x){WR(x); return *this;};
	inline CSaver& operator<<(unsigned short x){WR(x); return *this;};
	inline CSaver& operator<<(long x){WR(x); return *this;};
	inline CSaver& operator<<(unsigned long x){WR(x);return *this;};
	inline CSaver& operator<<(const float& x){WR(x);return *this;};
	inline CSaver& operator<<(const double& x){WR(x);return *this;};

	inline CSaver& operator<<(Vect3f& x){WR(x);return *this;};
	inline CSaver& operator<<(Vect2f& x){WR(x);return *this;};
	inline CSaver& operator<<(MatXf& x){WR(x);return *this;};

	void write(const void* data,int size){fwrite(data,size,1,f);}
};
#undef WR
#define WR(x) fwrite(&x,sizeof(x),1,s.f);
//#define WRC(x) {DWORD w=x; fwrite(&w,sizeof(w),1,s.f);}

inline CSaver& CSaver::operator<<(const char *x)
{
	if(x)fwrite(x,strlen(x)+1,1,f);
	else fwrite("",strlen("")+1,1,f);
	return *this;
}

void SaveString(CSaver& s,LPCSTR str,DWORD ido);

#pragma pack(1)
#pragma warning(disable: 4200)
struct CLoadData
{
	DWORD id;
	int size;
	BYTE data[];
};

#pragma warning(default: 4200)
#pragma pack()

class CLoadDirectory
{
protected:
	BYTE *begin,*cur;
	int size;
public:
	CLoadDirectory(BYTE* data,DWORD _size);
	CLoadDirectory(CLoadData* ld);
	CLoadData* next();

	BYTE* GetData(){return begin;};
	int GetDataSize(){return size;};
};

class CLoadDirectoryFile:public CLoadDirectory
{
public:
	CLoadDirectoryFile();
	~CLoadDirectoryFile();
	bool Load(LPCSTR filename);
};

///Читает лишь в случае, если rd_cur_pos+sizeof(x)<=ld->size
#define RD(x) if(rd_cur_pos+(int)sizeof(x)<=ld->size){memcpy(&x,ld->data+rd_cur_pos,sizeof(x));rd_cur_pos+=sizeof(x);}
class CLoadIterator
{
	CLoadData* ld;
	int rd_cur_pos;
public:
	CLoadIterator(CLoadData* _ld):ld(_ld),rd_cur_pos(0){};
	inline void operator>>(bool& i){RD(i);}
	inline void operator>>(char& i){RD(i);}
	inline void operator>>(unsigned char& i){RD(i);}
	inline void operator>>(short& i){RD(i);}
	inline void operator>>(unsigned short& i){RD(i);}
	inline void operator>>(int& i){RD(i);}
	inline void operator>>(unsigned int& i){RD(i);}
	inline void operator>>(long& i){RD(i);}
	inline void operator>>(unsigned long& i){RD(i);}
	inline void operator>>(float& i){RD(i);}
	inline void operator>>(double& i){RD(i);}
	inline void operator>>(void *& i){RD(i);}

	LPCSTR LoadString()
	{
		if(rd_cur_pos>=ld->size)return "";

		LPCSTR s=(LPCSTR)(ld->data+rd_cur_pos);
		rd_cur_pos+=strlen(s)+1;
		return s;
	}

	inline void operator>>(string& s){s=LoadString();}
	inline void operator>>(Vect3f& i){RD(i);}
	inline void operator>>(Vect2f& i){RD(i);}
	inline void operator>>(MatXf& i){RD(i);}
};

template<class T>
void operator>>(CLoadIterator& it,vector<T>& v)
{
	DWORD size=0;
	it>>size;
	v.resize(size);
	for(int i=0;i<size;i++)
	{
		it>>v[i];
	}
}

template<class T>
CSaver& operator<<(CSaver& s,vector<T>& v)
{
	DWORD size=v.size();
	s<<size;
	vector<T>::iterator it;
	FOR_EACH(v,it)
	{
		s<<*it;
	}
	return s;
}

#undef RD
