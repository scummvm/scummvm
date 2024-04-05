#ifndef __OBJSTATISTIC_H__
#define __OBJSTATISTIC_H__

#ifndef _FINAL_VERSION_
#include "Render\src\scene.h"
#include <vector>
#include <CommCtrl.h>


class cStatisticWnd
{
public:
	cStatisticWnd();
	~cStatisticWnd();
	HWND InitWindow(HINSTANCE hInstance);
private:
	HWND Wnd;
};


struct UniqObj
{
	int count;
	TextureNames textures;
	string name;
	int vbCount;
	int vbSize;
	int nodeCount;
	int materialCount;
	int animationSize;
	int polygonsCount;
	bool loaded;
};

struct UnicEffect
{
	string effectName;
	int effectCount;
	int	minParticleCount;
	int maxParticleCount;
	int particleCount;
};

class cObjStatistic
{
public:
	cObjStatistic();  
	~cObjStatistic();
	void RecalculateAndShow();
protected:
	HWND hWnd_;
	HWND hListView;
	HWND hEffectListView;
	HWND hObjectListView;
	HWND hSimplyObjectListView;
	HWND hDebrisObjectListView;
	HWND hTextureListView;
	HWND hTabControl;
	HWND hObjectNamesListView;
	HWND hTexturesByObjectListView;
	HWND hPostEffects;
	HWND hVisibleObjects;

	HIMAGELIST imageList;
	bool CreateWnd();
	friend LRESULT CALLBACK StatisticWndWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	virtual LRESULT WindowProc(UINT uMsg,WPARAM wParam,LPARAM lParam);
	void Sort(HWND ListWnd, int iColumn, BOOL bAscending );
	void SelectChange();
	void SetItemData(HWND ListWnd, int nItem, DWORD_PTR dwData);
	void SetTexturesByObject(char* name);
	cTexture* GetTexture(string name);
	string GetFormatedSize(DWORD size);
	void PrepareUniqueObjects(vector<cObject3dx*>& objects);
	void SetVisibleObjects();
	void PrepareUniqueEffects();
	void CalcTotalSimplyObjects();
	void CalcTotalVertexSize();
	void CalcTotalTextureSize();
	void CalcTotalTextureMapSize();
	void CalcTotalParticleCount(vector<cEffect*>& effects);
	void CalcMapSizes();
	void CalcOtherSizes();
	void SetTextureInfo(cObject3dx* obj, int TexCount, int TexSize);
	void Recalculate();

	vector<UnicEffect> uniqueEffects;
	vector<UniqObj> uniqueObjects;
	DWORD totalSimplyVertexSize;
	DWORD totalSimplyVertexCount;
	DWORD totalSimplyObjectCount;
	DWORD totalSimplyUniqObjectCount;
	DWORD totalSimplyUniqDebrisCount;
	DWORD totalVertexSize;
	DWORD totalVertexCount;
	DWORD totalParticleCount;
	DWORD totalTextureSize;
	DWORD totalVertexMapSize;
	DWORD totalIndexMapSize;
	DWORD totalVertexOtherSize;
	DWORD totalIndexOtherSize;
	DWORD totalTextureMapSize;
	DWORD totalSize;
	DWORD totalObjectsCount;
	DWORD totalEffects;
	DWORD backBufferSize;

	struct PolygonStatistic
	{
		string name;
		string type;
		int count;
		int summary_polygons;
		PolygonStatistic():count(0),summary_polygons(0){};
	};

	HWND CreateListView(HIMAGELIST imagelist=NULL);
	void InsetFormattedNum(const char* name,int value);
	void InsertColumn(HWND hwnd,int ncol,int length,const char* name);
	int InsertItem(HWND hwnd,int row,const string& name);
	int InsertItem(HWND hwnd,int row,const string& name,int image);
	void SetItem(HWND hwnd,int row,int col,const string& name);
};

#endif _FINAL_VERSION_
void ShowGraphicsStatistic();

#endif
