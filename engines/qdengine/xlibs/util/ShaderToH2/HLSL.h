#pragma once

int ProcessHLSL(const char* in,const char* out,const char* fname,const char* ext,const char* entry_point,vector<D3DXMACRO>& define);
int ProcessASM(const char* in,const char* out,const char* fname,const char* ext,const char* entry_point,vector<D3DXMACRO>& define);

bool OutHexBuffer(FILE* fout,DWORD* pd,int size);
