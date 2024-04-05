#include "StdAfx.h"
#include "ObjStatistic.h"
#include "Render\3dx\Lib3dx.h"
#include "Game\CameraManager.h"
#include "Game\RenderObjects.h"
#include "Environment\Environment.h"
#include "Water\CoastSprites.h"
#include "Render\shader\shaders.h"
#include "Render\src\Grass.h"
#include "Render\D3D\D3DRender.h"
#include "Render\src\TexLibrary.h"
#include "Render\d3d\D3DRenderTileMap.h"
#include "VistaRender\postEffects.h"
#include "FileUtils\FileUtils.h"
#include <commctrl.h>

void ShowGraphicsStatistic()
{
#ifndef _FINAL_VERSION_
	static cObjStatistic statistics;
	statistics.RecalculateAndShow();
#endif _FINAL_VERSION_
}

#ifndef _FINAL_VERSION_

static int SortColumn = -1;
static BOOL SortAscending = TRUE;
static int cur_column = 0;

cTexture* cObjStatistic::GetTexture(string name)
{
	string sname = normalizePath(name.c_str());
	string uniqName = GetTexLibrary()->CreateUniqueName(sname.c_str());
	return GetTexLibrary()->FindTexture(uniqName.c_str());
}

void cObjStatistic::SetTexturesByObject(char* name)
{
	//cObject3dx* foundObj = NULL;
	bool found = false;
	int i;
	for (i=0; i<uniqueObjects.size(); i++)
	{
		//cObject3dx* obj = uniqueObjects[i].obj;
		UniqObj& obj = uniqueObjects[i];
		if (!lstrcmp(obj.name.c_str(),name))
		{
			//foundObj = obj;
			found = true;
			break;
		}
	}

	if (!found)
		return;
	int size = 0;
	vector<string> textures;
	LVITEM itm;
	textures  = uniqueObjects[i].textures; //foundObj->GetAllTextureName(textures);
	cTexLibrary* texLab  = GetTexLibrary();
	ListView_DeleteAllItems(hTexturesByObjectListView);
	for(i=0; i<textures.size(); i++)
	{
		cTexture* texture =  GetTexture(textures[i]);
		if (texture)
		{
			int newitem;
			itm.mask = LVIF_TEXT;
			itm.pszText = (LPSTR)texture->name();
			itm.iItem   = 0;
			itm.iSubItem= 0;
			newitem = ListView_InsertItem(hTexturesByObjectListView,&itm);

			string str = GetFormatedSize(texture->CalcTextureSize());
			itm.pszText = (char*)str.c_str();
			itm.iItem   = newitem;
			itm.iSubItem= 1;
			ListView_SetItem(hTexturesByObjectListView,&itm);
		}
	}

}

bool IsNumber(string str )
{
	if (str.find_first_not_of(" 0123456789") != string::npos)
			return false;
		else
			return true;
}

int NumberCompare( string str1, string str2 )
{
	for (int index = str1.length(); index>-1; index--)
		if (str1[index] == ' ')
			str1.erase(index,1);

	for (int index = str2.length(); index>-1; index--)
		if (str2[index] == ' ')
			str2.erase(index,1);

	const int iNumber1 = atoi( str1.c_str() );
	const int iNumber2 = atoi( str2.c_str() );

	if( iNumber1 < iNumber2 )
		return -1;

	if( iNumber1 > iNumber2 )
		return 1;

	return 0;
}

void cObjStatistic::SetItemData(HWND ListWnd, int nItem, DWORD_PTR dwData)
{
	LVITEM itm;
	itm.mask = LVIF_PARAM;
	itm.iItem = nItem;
	itm.lParam = (LPARAM)dwData;
	ListView_SetItem(ListWnd, &itm);
}

int CALLBACK CompareFunction( LPARAM lParam1, LPARAM lParam2, LPARAM lParamData )
{
	char pszText1[100];
	char pszText2[100];
	int cchTextMax = 100;
	ListView_GetItemText((HWND)lParamData, lParam1, SortColumn, (char*)(&pszText1[0]), cchTextMax);
	cchTextMax = 100;
	ListView_GetItemText((HWND)lParamData, lParam2, SortColumn, (char*)(&pszText2[0]), cchTextMax);

	if( IsNumber(pszText1) )
		return SortAscending ? NumberCompare( pszText1, pszText2 ) : NumberCompare( pszText2, pszText1 );
	else
		return SortAscending ? lstrcmp( pszText1, pszText2 ) : lstrcmp( pszText2, pszText1 );

}

void cObjStatistic::Sort(HWND ListWnd, int iColumn, BOOL bAscending )
{
	SortColumn = iColumn;
	SortAscending = bAscending;
	for (int i=0; i<ListView_GetItemCount(ListWnd); i++)
	{
		SetItemData(ListWnd,i,i);
	}
	ListView_SortItemsEx(ListWnd, CompareFunction, (LPARAM)ListWnd);
}
void cObjStatistic::SelectChange()
{
	cur_column = TabCtrl_GetCurSel(hTabControl);
	ShowWindow(hListView,cur_column==0?SW_SHOW:SW_HIDE);
	ShowWindow(hTextureListView,cur_column==1?SW_SHOW:SW_HIDE);
	ShowWindow(hObjectNamesListView,cur_column==2?SW_SHOW:SW_HIDE);
	ShowWindow(hTexturesByObjectListView,cur_column==2?SW_SHOW:SW_HIDE);
	ShowWindow(hObjectListView,cur_column==3?SW_SHOW:SW_HIDE);
	ShowWindow(hEffectListView,cur_column==4?SW_SHOW:SW_HIDE);
	ShowWindow(hSimplyObjectListView,cur_column==5?SW_SHOW:SW_HIDE);
	ShowWindow(hPostEffects,cur_column==6?SW_SHOW:SW_HIDE);
	ShowWindow(hDebrisObjectListView,cur_column==7?SW_SHOW:SW_HIDE);
	ShowWindow(hVisibleObjects,cur_column==8?SW_SHOW:SW_HIDE);
}

LRESULT cObjStatistic::WindowProc(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) 
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd_,&ps);
			InvalidateRect(hListView,NULL,FALSE);
			InvalidateRect(hTextureListView,NULL,FALSE);
			InvalidateRect(hObjectListView,NULL,FALSE);
			InvalidateRect(hObjectNamesListView,NULL,FALSE);
			InvalidateRect(hTexturesByObjectListView,NULL,FALSE);
			InvalidateRect(hEffectListView,NULL,FALSE);
			InvalidateRect(hSimplyObjectListView,NULL,FALSE);
			InvalidateRect(hDebrisObjectListView,NULL,FALSE);
			InvalidateRect(hVisibleObjects,NULL,FALSE);
			EndPaint(hWnd_,&ps);
			break;
		}
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) 
		{
		case LVN_COLUMNCLICK:
			Sort(((LPNMHDR)lParam)->hwndFrom,((LPNMLISTVIEW)lParam)->iSubItem, ((LPNMLISTVIEW)lParam)->iSubItem == SortColumn ? !SortAscending : TRUE );
			break;
		case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW hdr = (LPNMLISTVIEW)lParam;
				if (hdr->iItem != -1 && cur_column==2 && hdr->hdr.hwndFrom == hObjectNamesListView && hdr->uNewState == 3)
				{
					char buf[500];
					ListView_GetItemText(hObjectNamesListView,hdr->iItem,0,(char*)&buf[0],500);
					SetTexturesByObject(buf);
				}
				break;
			}
		case TCN_SELCHANGE:
			{
				SelectChange();
				break;
			}
		}
		break;
	case WM_DESTROY:
		break;
	case WM_SIZE:
		{
			RECT rcl; 
			GetClientRect (hWnd_, &rcl); 

			SetWindowPos(hTabControl,NULL,0, 0, rcl.right, 30,0);
			SetWindowPos(hPostEffects,NULL, 0, 30, rcl.right, rcl.bottom-30, 0);
			SetWindowPos(hListView,NULL, 0, 30, rcl.right, rcl.bottom-30, 0);
			SetWindowPos(hEffectListView,NULL, 0, 30, rcl.right, rcl.bottom-30,0);
			SetWindowPos(hObjectListView,NULL, 0, 30, rcl.right, rcl.bottom-30,0);
			SetWindowPos(hObjectNamesListView,NULL, 0, 30, rcl.right/2, rcl.bottom-30,0);
			SetWindowPos(hTexturesByObjectListView,NULL, rcl.right/2, 30, rcl.right/2, rcl.bottom-30,0);
			SetWindowPos(hTextureListView,NULL, 0, 30, rcl.right, rcl.bottom-30,0);
			SetWindowPos(hSimplyObjectListView,NULL, 0, 30, rcl.right, rcl.bottom-30,0);
			SetWindowPos(hDebrisObjectListView,NULL, 0, 30, rcl.right, rcl.bottom-30,0);
			SetWindowPos(hVisibleObjects,NULL, 0, 30, rcl.right, rcl.bottom-30,0);
		}
		break;
	default:
		return DefWindowProc(hWnd_, uMsg, wParam, lParam);
	}
	return 0;
}
bool cObjStatistic::CreateWnd()
{
	WNDCLASSEX wcx; 
	wcx.cbSize = sizeof(wcx);
	wcx.style = CS_HREDRAW |CS_VREDRAW;
	wcx.lpfnWndProc = StatisticWndWndProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = NULL;
	wcx.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wcx.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcx.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wcx.lpszMenuName =  "MainMenu";
	wcx.lpszClassName = "StatisticClassName";
	wcx.hIconSm = NULL;
	RegisterClassEx(&wcx); 

	hWnd_ = CreateWindowEx(0,
		"StatisticClassName",
		"Statistic",
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		1000,
		700,
		NULL,
		NULL,
		NULL,
		NULL);

	if (!hWnd_)
		return false;
	ShowWindow(hWnd_, SW_SHOW);
	SetWindowLong(hWnd_,GWL_USERDATA,(LONG)this);

	RECT rcl; 
	GetClientRect (hWnd_, &rcl); 

	imageList = ImageList_Create(12,12,ILC_COLOR,1,1);
	ImageList_AddIcon(imageList,LoadIcon(NULL,IDI_INFORMATION));
	ImageList_AddIcon(imageList,LoadIcon(NULL,IDI_WARNING));

	hTabControl = CreateWindow(WC_TABCONTROL,NULL,WS_CHILD | WS_VISIBLE | WS_TABSTOP , 0, 0, rcl.right, 30, hWnd_, NULL, NULL, NULL);

	TCITEM pitem;
	pitem.mask = TCIF_TEXT;

	pitem.pszText = "Общая статистика";
	TabCtrl_InsertItem(hTabControl,0,&pitem);

	pitem.pszText = "Текстуры";
	TabCtrl_InsertItem(hTabControl,1,&pitem);

	pitem.pszText = "Текстуры по моделям";
	TabCtrl_InsertItem(hTabControl,2,&pitem);

	pitem.pszText = "Модели";
	TabCtrl_InsertItem(hTabControl,3,&pitem);

	pitem.pszText = "Эффекты";
	TabCtrl_InsertItem(hTabControl,4,&pitem);

	pitem.pszText = "Simply Модели";
	TabCtrl_InsertItem(hTabControl,5,&pitem);

	pitem.pszText = "Post Эффекты";
	TabCtrl_InsertItem(hTabControl,6,&pitem);

	pitem.pszText = "Осколки";
	TabCtrl_InsertItem(hTabControl,7,&pitem);

	pitem.pszText = "Видимые объекты";
	TabCtrl_InsertItem(hTabControl,8,&pitem);

	hPostEffects = CreateListView();

	InsertColumn(hPostEffects,0,255,"Эффект");
	InsertColumn(hPostEffects,1,255,"Размер текстур");

	hListView = CreateListView();
	InsertColumn(hListView,0,255,"Объекты");
	InsertColumn(hListView,1,255,"Количество");

	hEffectListView = CreateListView();

	InsertColumn(hEffectListView,0,166,"Название эффекта");
	InsertColumn(hEffectListView,1,166,"Количество");
	InsertColumn(hEffectListView,2,166,"Минимум частиц");
	InsertColumn(hEffectListView,3,166,"Максимум частиц");
	InsertColumn(hEffectListView,4,166,"В среднем частиц на эффект");
	InsertColumn(hEffectListView,5,166,"Всего частиц");

	hObjectListView	 = CreateListView(imageList);

	InsertColumn(hObjectListView,0,450,"Имя модели");
	InsertColumn(hObjectListView,1,50,"Кол-во");
	InsertColumn(hObjectListView,2,70,"Текстуры");
	InsertColumn(hObjectListView,3,80,"Размер Текстур");
	InsertColumn(hObjectListView,4,70,"Вершины");
	InsertColumn(hObjectListView,5,80,"Размер вершин");
	InsertColumn(hObjectListView,6,50,"Nodes");
	InsertColumn(hObjectListView,7,80,"Материалы");
	InsertColumn(hObjectListView,8,50,"Анимация");
	InsertColumn(hObjectListView,9,50,"Полигоны");

	hObjectNamesListView = 	CreateWindow(WC_LISTVIEW,NULL,WS_CHILD /*| WS_VISIBLE*/ | WS_VSCROLL | WS_HSCROLL | LVS_REPORT | WS_BORDER |  WS_TABSTOP , 0, 30, rcl.right/2, rcl.bottom-30, hWnd_, NULL, NULL, NULL);
	ListView_SetExtendedListViewStyle(hObjectNamesListView,LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	InsertColumn(hObjectNamesListView,0,520,"Имя модели");

	hTexturesByObjectListView=CreateWindow(WC_LISTVIEW,NULL,WS_CHILD /*| WS_VISIBLE*/ | WS_VSCROLL | WS_HSCROLL | LVS_REPORT | WS_BORDER |  WS_TABSTOP , rcl.right/2, 30, rcl.right/2, rcl.bottom-30, hWnd_, NULL, NULL, NULL);
	ListView_SetExtendedListViewStyle(hTexturesByObjectListView,LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	InsertColumn(hTexturesByObjectListView,0,400,"Имя текстуры");
	InsertColumn(hTexturesByObjectListView,1,200,"Размер");

	hTextureListView=CreateListView(imageList);

	InsertColumn(hTextureListView,0,500,"Имя текстуры");
	InsertColumn(hTextureListView,1,200,"Размер");
	InsertColumn(hTextureListView,2,200,"Тип");

	hSimplyObjectListView=CreateListView(imageList);

	InsertColumn(hSimplyObjectListView,0,520,"Имя модели");
	InsertColumn(hSimplyObjectListView,1,50,"Кол-во");
	InsertColumn(hSimplyObjectListView,2,80,"Размер Текстур");
	InsertColumn(hSimplyObjectListView,3,70,"Вершины");
	InsertColumn(hSimplyObjectListView,4,80,"Размер вершин");
	InsertColumn(hSimplyObjectListView,5,50,"Nodes");

	hDebrisObjectListView=CreateListView(imageList);

	InsertColumn(hDebrisObjectListView,0,520,"Имя модели");
	InsertColumn(hDebrisObjectListView,1,50,"Кол-во");
	InsertColumn(hDebrisObjectListView,2,80,"Размер Текстур");
	InsertColumn(hDebrisObjectListView,3,70,"Вершины");
	InsertColumn(hDebrisObjectListView,4,80,"Размер вершин");

	hVisibleObjects=CreateListView();
	InsertColumn(hVisibleObjects,0,500,"Имя объекта");
	InsertColumn(hVisibleObjects,1,100,"Тип");
	InsertColumn(hVisibleObjects,2,150,"Видимых полигонов");
	InsertColumn(hVisibleObjects,3,100,"Количество");
	InsertColumn(hVisibleObjects,4,150,"Cумма видимых полигонов");
	return true;
}

string cObjStatistic::GetFormatedSize(DWORD size)
{
	char s[200];
	sprintf(s,"%d",size);
	string bu = s;
	for (int j=bu.length(); j>0; j-=3)
		bu.insert(j," ");
	return bu;
}

cObjStatistic::cObjStatistic()
{
	hWnd_=NULL;
}

cObjStatistic::~cObjStatistic()
{
	if(hWnd_)
		SendMessage(hWnd_,WM_CLOSE,0,0);
	hWnd_=NULL;
}


void cObjStatistic::RecalculateAndShow()
{
	totalVertexSize = 0;
	totalTextureSize = 0;
	totalIndexMapSize = 0;
	totalIndexOtherSize = 0;
	totalVertexMapSize = 0;
	totalVertexOtherSize = 0;
	totalTextureMapSize = 0;
	totalObjectsCount = 0;
	totalEffects = 0;
	totalSimplyUniqObjectCount = 0;
	totalSimplyUniqDebrisCount = 0;

	uniqueEffects.clear();
	uniqueObjects.clear();
	if(hWnd_)
		SendMessage(hWnd_,WM_CLOSE,0,0);

	if (!CreateWnd())
		return;
	ShowWindow(hWnd_, SW_SHOW);

	Recalculate();

	InsetFormattedNum("РАЗМЕР ВСЕГО:",totalSize);
	InsetFormattedNum("Размер backBuffer",backBufferSize);
	InsetFormattedNum("Размер текстур",totalTextureSize);
	InsetFormattedNum("Размер текстур карты",totalTextureMapSize);
	InsetFormattedNum("Размер вешин карты",totalVertexMapSize);
	InsetFormattedNum("Размер индексов карты",totalIndexMapSize);
	InsetFormattedNum("Размер всех вершин",totalVertexOtherSize);
	InsetFormattedNum("Размер всех индексов ",totalIndexOtherSize);
	if(environment)InsetFormattedNum("Размер вершин травы",environment->grass()->GetVertexSize());
	InsetFormattedNum("Размер вершин простых объектов",totalSimplyVertexSize);
	InsetFormattedNum("Количество вершин простых объектов",totalSimplyVertexCount);
	InsetFormattedNum("Количество простых объектов",totalSimplyObjectCount);
	InsetFormattedNum("Количество уникальных простых объектов",totalSimplyUniqObjectCount);
	InsetFormattedNum("Количество уникальных осколков",totalSimplyUniqDebrisCount);
	

	InsetFormattedNum("Размер вершин объектов",totalVertexSize);
	InsetFormattedNum("Количество вершин объектов",totalVertexCount);
	InsetFormattedNum("Всего объектов",totalObjectsCount);
	InsetFormattedNum("Количество уникальных объектов",uniqueObjects.size());
	InsetFormattedNum("Количетсво частиц",totalParticleCount);
	InsetFormattedNum("Количество эффектов",totalEffects);
	InsetFormattedNum("Количество уникальных эффектов",uniqueEffects.size());

	InsetFormattedNum("Количество текстур",GetTexLibrary()->GetNumberTexture());
	if(environment)InsetFormattedNum("Количество движущихся прибрежных спрайтов",environment->GetCoastSprites()->GetMovingCoastSpritesCount());
	if(environment)InsetFormattedNum("Количество простых прибрежных спрайтов",environment->GetCoastSprites()->GetSimpleCoastSpritesCount());


	string str;
	int newitem;
	for(int index = 0; index<uniqueEffects.size(); index++)
	{
		UnicEffect& u=uniqueEffects[index];
		newitem=InsertItem(hEffectListView,index,u.effectName);
		SetItem(hEffectListView,newitem,1,GetFormatedSize(u.effectCount));
		SetItem(hEffectListView,newitem,2,GetFormatedSize(u.minParticleCount));
		SetItem(hEffectListView,newitem,3,GetFormatedSize(u.maxParticleCount));
		SetItem(hEffectListView,newitem,4,GetFormatedSize(u.particleCount / uniqueEffects[index].effectCount));
		SetItem(hEffectListView,newitem,5,GetFormatedSize(u.particleCount));
	}

	for(int index = 0; index<uniqueObjects.size(); index++)
	{
		UniqObj& u=uniqueObjects[index];
		InsertItem(hObjectNamesListView,index,u.name,u.loaded?-1:1);
		newitem =InsertItem(hObjectListView,index,u.name,u.loaded?-1:1);
		SetItem(hObjectListView,newitem,1,GetFormatedSize(u.count));

		int TexSize = 0;
		string str;
		vector<string>& textures = u.textures;
		int TexCount = textures.size();
		for(int i=0; i<textures.size(); i++)
		{
			cTexture* texture =  GetTexture(textures[i]);
			if (texture)
			{
				TexSize += texture->CalcTextureSize();
			}
		}


		SetItem(hObjectListView,newitem,2,GetFormatedSize(TexCount));
		SetItem(hObjectListView,newitem,3,GetFormatedSize(TexSize));
		SetItem(hObjectListView,newitem,4,GetFormatedSize(u.vbCount));
		SetItem(hObjectListView,newitem,5,GetFormatedSize(u.vbSize));
		SetItem(hObjectListView,newitem,6,GetFormatedSize(u.nodeCount));
		SetItem(hObjectListView,newitem,7,GetFormatedSize(u.materialCount));
		SetItem(hObjectListView,newitem,8,GetFormatedSize(u.animationSize));
		SetItem(hObjectListView,newitem,9,GetFormatedSize(u.polygonsCount));
	}

	cTexLibrary* texLab  = GetTexLibrary();
	for(int index = 0; index<texLab->GetNumberTexture(); index++)
	{
		cTexture* tex = texLab->GetTexture(index);
		newitem=InsertItem(hTextureListView,index,tex->name(),tex->GetLoaded()?-1:1);
		SetItem(hTextureListView,newitem,1,GetFormatedSize(tex->CalcTextureSize()));

		if (tex->IsAviScaleTexture())
		{
			str = "AviScale";
		}else
		if (tex->IsComplexTexture())
		{
			str = "Complex";
		}else
		if (tex->IsScaleTexture())
		{
			str = "Scale";
		}else
		if (tex->IsTexture2D())
		{
			str = "2D";
		}else
		{
			str = "3D";
		}

		{
			D3DSURFACE_DESC desc;
			tex->BitMap[0]->GetLevelDesc(0,&desc);
			switch(desc.Format)
			{
			case D3DFMT_DXT1:str += ", DXT1";break;
			case D3DFMT_DXT5:str += ", DXT5";break;
			case D3DFMT_V8U8:str += ", V8U8";break;
			case D3DFMT_A8R8G8B8:str += ", ARGB8";break;
			}
		}

		SetItem(hTextureListView,newitem,2,str);
	}

	//PostEffects
	if (environment)
	{
		// Заменить тут все после дописания класса PostEffectManager
		if(environment->PEManager())
		{
			newitem=InsertItem(hPostEffects,0,"Main textures");
			SetItem(hPostEffects,newitem,1,GetFormatedSize(environment->PEManager()->baseTexturesSize()));
		}
		if (environment->PEManager() && environment->PEManager()->isActive(PE_BLOOM))
		{
			newitem=InsertItem(hPostEffects,0,"Bloom");
			SetItem(hPostEffects,newitem,1,GetFormatedSize(environment->PEManager()->texturesSize(PE_BLOOM)));
		}
		if (environment->PEManager() && environment->PEManager()->isActive(PE_MONOCHROME))
		{
			newitem=InsertItem(hPostEffects,0,"Monochrome");
			SetItem(hPostEffects,newitem,1,GetFormatedSize(environment->PEManager()->texturesSize(PE_MONOCHROME)));
		}
		if (environment->PEManager() && environment->PEManager()->isActive(PE_UNDER_WATER))
		{
			newitem=InsertItem(hPostEffects,0,"UnderWater");
			SetItem(hPostEffects,newitem,1,GetFormatedSize(environment->PEManager()->texturesSize(PE_UNDER_WATER)));
		}
		if (environment->PEManager() && environment->PEManager()->isActive(PE_DOF))
		{
			newitem=InsertItem(hPostEffects,0,"Depth of Field");
			SetItem(hPostEffects,newitem,1,GetFormatedSize(environment->PEManager()->texturesSize(PE_DOF)));
		}
		if (environment->PEManager() && environment->PEManager()->isActive(PE_MIRAGE))
		{
			newitem=InsertItem(hPostEffects,0,"Mirage");
			SetItem(hPostEffects,newitem,1,GetFormatedSize(environment->PEManager()->texturesSize(PE_MIRAGE)));
		}
	}	
	//
	
	SetVisibleObjects();
	//DWORD totalTextureSize;
	//DWORD totalObjects;
	SelectChange();
}

void cObjStatistic::SetVisibleObjects()
{
	vector<cObject3dx*> objects;
	terScene->GetAllObject3dx(objects);
	typedef StaticMap<string,PolygonStatistic> smap;
	smap m;

	int index;
	for(index = 0; index<objects.size(); index++)
	{
		cObject3dx* p=objects[index];
		if(p->GetNumOutputPolygons()==0)
			continue;

		string key=p->GetFileName();
		key+=":cObject3dx";
		smap::iterator it=m.find(key.c_str());
		if(it==m.end())
		{
			PolygonStatistic ps;
			ps.name=p->GetFileName();
			ps.type="cObject3dx";
			m[key.c_str()]=ps;
			it=m.find(key.c_str());
		}

		it->second.count++;
		it->second.summary_polygons+=p->GetNumOutputPolygons();
	}

	vector<ListSimply3dx> simply_objects = terScene->GetAllSimply3dxList();
	for(index = 0; index<simply_objects.size(); index++)
	{
		cStaticSimply3dx* p=simply_objects[index].pStatic;
		if(p->GetNumOutputPolygons()==0)
			continue;

		string key=p->file_name;
		key+=":cSimply3dx";
		smap::iterator it=m.find(key.c_str());
		if(it==m.end())
		{
			PolygonStatistic ps;
			ps.name=p->file_name;
			ps.type="cSimply3dx";
			m[key.c_str()]=ps;
			it=m.find(key.c_str());
		}

		it->second.count+=p->GetNumOutputObjects();
		it->second.summary_polygons+=p->GetNumOutputPolygons();
	}

	index=0;
	for(smap::iterator it=m.begin();it!=m.end(); ++it,++index)
	{
		PolygonStatistic& ps=it->second;
		int newitem=InsertItem(hVisibleObjects,index,ps.name.c_str());
		SetItem(hVisibleObjects,newitem,1,ps.type.c_str());
		SetItem(hVisibleObjects,newitem,2,GetFormatedSize(ps.summary_polygons/ps.count));
		SetItem(hVisibleObjects,newitem,3,GetFormatedSize(ps.count));
		SetItem(hVisibleObjects,newitem,4,GetFormatedSize(ps.summary_polygons));
	}

}

void cObjStatistic::SetTextureInfo(cObject3dx* obj, int TexCount, int TexSize)
{
	int size = 0;
	TextureNames textures;
	obj->GetStatic()->GetTextureNames(textures);
	TexCount = textures.size();
	for(int i=0; i<textures.size(); i++)
	{
		cTexture* texture =  GetTexture(textures[i]);
		if (texture)
		{
			size += texture->CalcTextureSize();
		}
	}
	TexSize = size;
}

void cObjStatistic::PrepareUniqueObjects(vector<cObject3dx*>& objects)
{
	Camera* camera = cameraManager->GetCamera();

	vector<cStatic3dx*> libraryObjects;
	pLibrary3dx->GetAllElements(libraryObjects);
	for(int i = 0; i < libraryObjects.size(); ++i){
		cStatic3dx* staticObject = libraryObjects[i];
		if(staticObject->GetRef() == 1){
			UniqObj addObj;
			addObj.count = 0;
			staticObject->GetTextureNames(addObj.textures);
			addObj.name = staticObject->fileName();

			staticObject->GetVBSize(addObj.vbCount,addObj.vbSize);

			addObj.nodeCount = staticObject->nodes.size();
			addObj.materialCount = staticObject->materials.size();
			addObj.loaded = staticObject->GetLoaded();
			addObj.animationSize = 0;//staticObject->chains_block.GetBlockSize();
			addObj.polygonsCount = 0;
			uniqueObjects.push_back(addObj);
		}
	}
    	
	for(int i=0; i<objects.size(); i++)
	{
		cObject3dx* obj = objects[i];
		bool ident = false;
		//if(!camera->TestVisible(obj->GetPosition(),obj->GetStatic()->bound_box.min,obj->GetStatic()->bound_box.max))
		//	continue;
		for (int j=0; j<uniqueObjects.size(); j++)
		{
			UniqObj& uObj = uniqueObjects[j];
			//if(strcmp(obj->GetFileName(),uObj.obj->GetFileName()) == 0)
			if (!lstrcmp(obj->GetFileName(),uObj.name.c_str()))
			{
				vector<string> textures1;
				textures1.clear();
				//obj->GetStatic()->GetTextureNames(textures1);
				obj->GetTextureNames(textures1);
				
				if (textures1 == uObj.textures)
				{
					ident = true;
				}
			}

			if (ident)
			{
				uObj.count++;
				uObj.polygonsCount = obj->GetNumPolygons();

				break;
			}
		}
		if(!ident)
		{
			UniqObj addObj;
			addObj.count = 1;
			//obj->GetStatic()->GetTextureNames(addObj.textures);
			obj->GetTextureNames(addObj.textures);
			addObj.name = obj->GetFileName();
			addObj.vbCount=0;
			addObj.vbSize=0;

			obj->GetStatic()->GetVBSize(addObj.vbCount,addObj.vbSize);
			addObj.nodeCount = obj->GetNodeNum();
			addObj.materialCount = obj->GetMaterialNum();
			addObj.loaded = obj->GetStatic()->GetLoaded();
			addObj.animationSize = 0;//obj->GetStatic()->chains_block.GetBlockSize();
			addObj.polygonsCount = obj->GetNumPolygons();
			uniqueObjects.push_back(addObj);
		}
	}
}

void cObjStatistic::CalcTotalSimplyObjects()
{
	Camera* camera = cameraManager->GetCamera();
	LVITEM itm;
	string str;

	int newitem;
	vector<ListSimply3dx> simply_objects = terScene->GetAllSimply3dxList();
	vector<ListSimply3dx> debris_objects;

	vector<cStaticSimply3dx*> library_objects;
	pLibrarySimply3dx->GetAllElements(library_objects);
	for(int i = 0; i < library_objects.size(); i++){
		cStaticSimply3dx* simple = library_objects[i];
		if(simple->GetRef() == 1){
			ListSimply3dx listSimply3dx;
			listSimply3dx.pStatic = simple;
			simply_objects.push_back(listSimply3dx);
		}
	}
	for(int i = 0; i < simply_objects.size(); i++)
	{
		if(simply_objects[i].pStatic->isDebris)
		{
			debris_objects.push_back(simply_objects[i]);
			simply_objects.erase(simply_objects.begin()+i);
			i--;
		}
	}
	
	vector<cStatic3dx*> static_objects;
	pLibrary3dx->GetAllElements(static_objects);
	for(int i=0; i<static_objects.size();i++)
	{
		cStatic3dx* pStatic = static_objects[i];
		for(int j=0; j<pStatic->debrises.size(); j++)
		{
			cStaticSimply3dx* simple = pStatic->debrises[j];
			if(simple->GetRef() == 1){
				ListSimply3dx listSimply3dx;
				listSimply3dx.pStatic = simple;
				debris_objects.push_back(listSimply3dx);
			}
		}
	}

	totalSimplyUniqObjectCount = simply_objects.size();
	totalSimplyUniqDebrisCount = debris_objects.size();
	for(int index = 0; index<simply_objects.size(); index++)
	{
		cStaticSimply3dx* pStatic = simply_objects[index].pStatic;

		totalSimplyObjectCount += simply_objects[index].objects.size();

		itm.mask = LVIF_TEXT|LVIF_IMAGE;
		itm.pszText = (char*)simply_objects[index].pStatic->file_name.c_str();
		itm.iItem   = 0;
		itm.iSubItem= 0;
		if(simply_objects[index].pStatic->GetLoaded())
			itm.iImage = -1;
		else
			itm.iImage = 1;
		newitem = ListView_InsertItem(hSimplyObjectListView,&itm);

		str = GetFormatedSize(simply_objects[index].objects.size());
		itm.pszText = (char*)str.c_str();
		itm.iItem   = newitem;
		itm.iSubItem= 1;
		ListView_SetItem(hSimplyObjectListView,&itm);

		str = GetFormatedSize(simply_objects[index].pStatic->CalcTextureSize());
		itm.pszText = (char*)str.c_str();
		itm.iItem   = newitem;
		itm.iSubItem= 2;
		ListView_SetItem(hSimplyObjectListView,&itm);

		int number_vertex=0,vertex_size=0;
		for(int ilod=0;ilod<pStatic->lods.size();ilod++)
		{
			number_vertex+=pStatic->lods[ilod].vb.GetNumberVertex();
			vertex_size += pStatic->lods[ilod].vb.GetVertexSize()*pStatic->lods[ilod].vb.GetNumberVertex();
		}

		totalSimplyVertexCount+=number_vertex;
		totalSimplyVertexSize+=vertex_size;

		str = GetFormatedSize(number_vertex);
		itm.pszText = (char*)str.c_str();
		itm.iItem   = newitem;
		itm.iSubItem= 3;
		ListView_SetItem(hSimplyObjectListView,&itm);

		str = GetFormatedSize(vertex_size);
		itm.pszText = (char*)str.c_str();
		itm.iItem   = newitem;
		itm.iSubItem= 4;
		ListView_SetItem(hSimplyObjectListView,&itm);

		str = GetFormatedSize(simply_objects[index].pStatic->node_offset.size());
		itm.pszText = (char*)str.c_str();
		itm.iItem   = newitem;
		itm.iSubItem= 5;
		ListView_SetItem(hSimplyObjectListView,&itm);
	}

	for(int index = 0; index<debris_objects.size(); index++)
	{
		cStaticSimply3dx* pStatic = debris_objects[index].pStatic;


		itm.mask = LVIF_TEXT|LVIF_IMAGE;
		itm.pszText = (char*)pStatic->file_name.c_str();
		itm.iItem   = 0;
		itm.iSubItem= 0;
		if(pStatic->GetLoaded())
			itm.iImage = -1;
		else
			itm.iImage = 1;
		newitem = ListView_InsertItem(hDebrisObjectListView,&itm);

		str = GetFormatedSize(debris_objects[index].objects.size());
		itm.pszText = (char*)str.c_str();
		itm.iItem   = newitem;
		itm.iSubItem= 1;
		ListView_SetItem(hDebrisObjectListView,&itm);

		str = GetFormatedSize(pStatic->CalcTextureSize());
		itm.pszText = (char*)str.c_str();
		itm.iItem   = newitem;
		itm.iSubItem= 2;
		ListView_SetItem(hDebrisObjectListView,&itm);

		int number_vertex=0,vertex_size=0;
		for(int ilod=0;ilod<pStatic->lods.size();ilod++)
		{
			number_vertex+=pStatic->lods[ilod].vb.GetNumberVertex();
//			vertex_size += pStatic->lods[ilod].vb.GetVertexSize()*pStatic->lods[ilod].vb.GetNumberVertex();
		}

//		totalSimplyVertexCount+=number_vertex;
//		totalSimplyVertexSize+=vertex_size;

		str = GetFormatedSize(number_vertex);
		itm.pszText = (char*)str.c_str();
		itm.iItem   = newitem;
		itm.iSubItem= 3;
		ListView_SetItem(hDebrisObjectListView,&itm);

		str = GetFormatedSize(vertex_size);
		itm.pszText = (char*)str.c_str();
		itm.iItem   = newitem;
		itm.iSubItem= 4;
		ListView_SetItem(hDebrisObjectListView,&itm);
	}

}

void cObjStatistic::PrepareUniqueEffects()
{
	vector<cEffect*> effects;
	terScene->GetAllEffects(effects);
	totalEffects = effects.size();
	for(int i=0; i<effects.size(); i++)
	{
		cEffect* effect = effects[i];
		int particle_count = effect->GetParticleCount();
		bool ident = false;
		for (int j=0; j<uniqueEffects.size(); j++)
		{
			UnicEffect& uEff = uniqueEffects[j];
			if(strcmp(effect->GetName().c_str(),uEff.effectName.c_str()) == 0)
			{
				uEff.effectCount++;
				if (uEff.maxParticleCount<particle_count)
					uEff.maxParticleCount = particle_count;
				if (uEff.minParticleCount>particle_count)
					uEff.minParticleCount = particle_count;
				uEff.particleCount += particle_count;
				ident = true;
				break;
			}
		}
		if(!ident)
		{
			UnicEffect addEffect;
			addEffect.effectCount = 1;
			addEffect.effectName = effect->GetName();
			addEffect.maxParticleCount = particle_count;
			addEffect.minParticleCount = particle_count;
			addEffect.particleCount = particle_count;
			uniqueEffects.push_back(addEffect);
		}
	}
	CalcTotalParticleCount(effects);
}

void cObjStatistic::CalcTotalParticleCount(vector<cEffect*>& effects)
{
	for (int i=0; i<effects.size(); i++)
	{
		totalParticleCount += effects[i]->GetParticleCount();
	}
}

void cObjStatistic::CalcTotalVertexSize()
{
	for (int i=0; i<uniqueObjects.size(); i++)
	{
		totalVertexSize += uniqueObjects[i].vbSize;//obj->GetStatic()->vb.size*obj->GetStatic()->vb_size;
		totalVertexCount += uniqueObjects[i].vbCount;//obj->GetStatic()->vb_size;
	}
}

void cObjStatistic::CalcTotalTextureSize()
{
	for (int i=0; i<GetTexLibrary()->GetNumberTexture(); i++)
	{
		totalTextureSize += GetTexLibrary()->GetTexture(i)->CalcTextureSize();
	}
}
void cObjStatistic::CalcMapSizes()
{
	int total_,free_;
	tileMapRender->indexPool()->GetUsedMemory(total_,free_);
	totalIndexMapSize = total_;
	tileMapRender->vertexPool()->GetUsedMemory(total_,free_);
	totalVertexMapSize = total_;
}
void cObjStatistic::CalcOtherSizes()
{
	cSlotManagerInit<sSlotVB>& slotsVB  = gb_RenderDevice3D->GetSlotVB();
	cSlotManagerInit<sSlotIB>& slotsIB  = gb_RenderDevice3D->GetSlotIB();

	for (int i=0; i<slotsVB.size(); i++)
	{
		sSlotVB* slot = slotsVB[i];
		if(slot->init)
		totalVertexOtherSize += slot->NumberVertex*slot->VertexSize;
	}
	for (int i=0; i<slotsIB.size(); i++)
	{
		sSlotIB* slot = slotsIB[i];
		if(slot->init)
			totalIndexOtherSize += slot->NumberPolygon*slot->PolygonSize;
	}
}
void cObjStatistic::CalcTotalTextureMapSize()
{
	int total_,free_;
	tileMapRender->texturePool()->GetTilemapTextureMemory(total_,free_);
	totalTextureMapSize = total_;
}

void cObjStatistic::Recalculate()
{
	vector<cObject3dx*> objects;
	totalTextureSize = 0;
	totalVertexSize = 0;
	totalVertexCount = 0;
	totalSimplyVertexSize = 0;
	totalSimplyVertexCount = 0;
	totalSimplyObjectCount = 0;
	totalParticleCount = 0;
	totalTextureSize = 0;
	totalSize = 0;
	backBufferSize = 0;
	terScene->GetAllObject3dx(objects);
	totalObjectsCount = objects.size();
	PrepareUniqueObjects(objects);
	PrepareUniqueEffects();
	CalcTotalSimplyObjects();
	CalcTotalVertexSize();
	CalcTotalTextureSize();
	CalcTotalTextureMapSize();
	//CalcTotalParticleCount();
	CalcMapSizes();
	CalcOtherSizes();
	backBufferSize = gb_RenderDevice3D->GetBackBuffersSize();

	totalSize = totalIndexMapSize+
				totalIndexOtherSize+
				totalTextureMapSize+
				totalTextureSize+
				totalVertexMapSize+
				totalVertexOtherSize+backBufferSize;

}

void cObjStatistic::InsetFormattedNum(const char* name,int value)
{
	LVITEM itm;

	//Texture Size
	itm.mask = LVIF_TEXT;
	itm.pszText = (char*)name;
	itm.iItem   = 0;
	itm.iSubItem= 0;
	int newitem = ListView_InsertItem(hListView,&itm);

	//sprintf(bu,"%d",totalTextureSize);
	string str = GetFormatedSize(value);
	itm.pszText = (char*)str.c_str();
	itm.iItem   = newitem;
	itm.iSubItem= 1;
	ListView_SetItem(hListView,&itm);
}

void cObjStatistic::InsertColumn(HWND hwnd,int ncol,int length,const char* name)
{
	LVCOLUMN pcol;
	pcol.mask		= LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	pcol.fmt		= LVCFMT_LEFT;
	pcol.cx			= length;
	pcol.pszText	= (char*)name;
	ListView_InsertColumn(hwnd,ncol,&pcol);
}

int cObjStatistic::InsertItem(HWND hwnd,int row,const string& name)
{
	LVITEM itm;
	itm.mask = LVIF_TEXT;
	itm.pszText = (char*)name.c_str();
	itm.iItem   = row;
	itm.iSubItem= 0;
	return ListView_InsertItem(hwnd,&itm);
}

int cObjStatistic::InsertItem(HWND hwnd,int row,const string& name,int image)
{
	LVITEM itm;
	itm.mask = LVIF_TEXT|LVIF_IMAGE;
	itm.pszText = (char*)name.c_str();
	itm.iItem   = row;
	itm.iSubItem= 0;
	itm.iImage=image;
	return ListView_InsertItem(hwnd,&itm);
}

void cObjStatistic::SetItem(HWND hwnd,int row,int col,const string& name)
{
	LVITEM itm;
	itm.mask = LVIF_TEXT;
	itm.pszText = (char*)name.c_str();
	itm.iItem   = row;
	itm.iSubItem= col;
	ListView_SetItem(hwnd,&itm);
}

LRESULT CALLBACK StatisticWndWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	cObjStatistic* stat=(cObjStatistic*)GetWindowLong(hWnd,GWL_USERDATA);
	if(!stat)
		return DefWindowProc(hWnd,message,wParam,lParam);
	return stat->WindowProc(message,wParam,lParam);
}

HWND cObjStatistic::CreateListView(HIMAGELIST imagelist)
{
	RECT rcl; 
	GetClientRect (hWnd_, &rcl); 
	HWND hout= CreateWindow(WC_LISTVIEW,NULL,WS_CHILD /*| WS_VISIBLE*/ | WS_VSCROLL | WS_HSCROLL | LVS_REPORT | WS_BORDER |  WS_TABSTOP , 0, 30, rcl.right, rcl.bottom-30, hWnd_, NULL, NULL, NULL);
	if(imagelist)
		ListView_SetImageList(hout,imageList,LVSIL_SMALL);
	ListView_SetExtendedListViewStyle(hout,LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	return hout;
}

#endif _FINAL_VERSION_