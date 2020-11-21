//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// VMR9Graph.h: interface for the CVMR9Graph class.
//
//=============================================================================

#if !defined(AFX_VMR9GRAPH_H__449FDB5B_6719_4134_B5A7_B651C08D109E__INCLUDED_)
#define AFX_VMR9GRAPH_H__449FDB5B_6719_4134_B5A7_B651C08D109E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dshow.h>
#include <Vmr9.h>

//#pragma comment( lib, "strmiids.lib" )
//#pragma comment( lib, "Quartz.lib" )
//#pragma comment( lib, "d3d9.lib" )
//#pragma comment( lib, "d3dx9.lib" )

#define WM_MEDIA_NOTIF		(WM_APP + 777)

class CVMR9Graph  
{
	// Constructor / destructor
public:
	CVMR9Graph();
	CVMR9Graph(HWND MediaWindow, IDirect3DDevice9 *device, int NumberOfStream = 4);
	~CVMR9Graph();

	// Methods
public:
	// Graph configuration
	void SetNumberOfLayer(int nNumberOfLayer);
	BOOL SetMediaWindow(HWND MediaWindow);
	BOOL SetMediaFile(const char* pszFileName, bool withSound, int nLayer = 0);
	BOOL PreserveAspectRatio(BOOL bPreserve = TRUE);
	IBaseFilter* AddFilter(const char* pszName, const GUID& clsid);

	// Graph control
	BOOL PlayGraph();
	BOOL StopGraph();
	BOOL ResetGraph();
  OAFilterState GetState();
	IMediaEvent* GetPtrMediaEvent();
	IMediaControl* GetPtrMediaControl();
	IMediaSeeking* GetPtrMediaSeeking();
	IBasicAudio* GetPtrBasicAudio();


	// Layer control
	BOOL GetVideoRect(LPRECT pRect);
	int GetAlphaLayer(int nLayer);
	BOOL SetAlphaLayer(int nLayer, int nAlpha);
	DWORD GetLayerZOrder(int nLayer);
	BOOL SetLayerZOrder(int nLayer, DWORD dwZOrder);
	BOOL SetLayerRect(int nLayer, RECT layerRect);

	// Bitmap control
	BOOL SetBitmapParams(int nAlpha, COLORREF cTransColor, RECT bitmapRect);

	// Reflected from window
	BOOL Repaint();
	BOOL Resize();

	// helper
	LPCTSTR GetLastError();

  // Internal
	BOOL BuildAndRenderGraph(bool withSound);

protected:
	// INIT helper methods
	void InitDefaultValues();
	void ReleaseAllInterfaces();

	// GRAPH methods
	BOOL BuildFilterGraph(bool withSound);
	BOOL BuildVMR();
	BOOL BuildSoundRenderer();
	BOOL RenderGraph();

	// DIRECT3D methods
	BOOL BuildDirect3d();


	// LAYER helper methods
	BOOL IsValidLayer(int nLayer);
	VMR9NormalizedRect NormalizeRect(LPRECT pRect);

	// DSOW helper methods
	HRESULT AddToRot(IUnknown *pUnkGraph);
	void RemoveFromRot();
	IPin* GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir);
	void ReportError(const char* pszError, HRESULT hrCode);
	HRESULT GetNextFilter(IBaseFilter *pFilter, PIN_DIRECTION Dir, IBaseFilter **ppNext);
	BOOL RemoveFilterChain(IBaseFilter* pFilter, IBaseFilter* pStopFilter);
	HRESULT AddFilterByClsid(IGraphBuilder *pGraph, LPCWSTR wszName, const GUID& clsid, IBaseFilter **ppF);

	// Attributes
public:
  bool UseAVISound;

protected:
	DWORD						m_dwRotId;
	char						m_pszErrorDescription[1024+MAX_ERROR_TEXT_LEN];
	int							m_nNumberOfStream;
  const char*    m_pszFileName;
  long m_oldWndProc;
	// MEDIA WINDOW
	HWND						m_hMediaWindow;
	// SRC interfaces array
	IBaseFilter*				m_srcFilterArray[10];
	// SOUND interfaces
	IBaseFilter*				m_pDirectSoundFilter;
	// GRAPH interfaces
	IUnknown*					m_pGraphUnknown;
	IGraphBuilder*				m_pGraphBuilder;
	IFilterGraph*				m_pFilterGraph;
	IFilterGraph2*				m_pFilterGraph2;
	IMediaControl*				m_pMediaControl;
  IMediaSeeking*				m_pMediaSeeking;
	//IMediaEvent*				m_pMediaEvent;
	IMediaEventEx*				m_pMediaEventEx;
	// VMR9 interfaces
	IBaseFilter*				m_pVMRBaseFilter;
	IVMRFilterConfig9*			m_pVMRFilterConfig;
	IVMRMixerBitmap9*			m_pVMRMixerBitmap;
	IVMRMixerControl9*			m_pVMRMixerControl;
	IVMRMonitorConfig9*			m_pVMRMonitorConfig;
	IVMRWindowlessControl9*		m_pVMRWindowlessControl;
	// DIRECT3D interfaces
	//IDirect3DDevice9*			m_pD3DDevice;
	IDirect3DSurface9*			m_pD3DSurface;
};

#endif // !defined(AFX_VMR9GRAPH_H__449FDB5B_6719_4134_B5A7_B651C08D109E__INCLUDED_)
