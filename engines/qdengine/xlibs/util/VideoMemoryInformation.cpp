#define _WIN32_DCOM
//#include <iostream>
//using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#include <algorithm>

#include "VideoMemoryInformation.h"

# pragma comment(lib, "wbemuuid.lib")


int GetVideoMemory()
{
    HRESULT hres;
/*
    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x" 
            << hex << hres << endl;
        return 1;                  // Program has failed.
    }
*/
    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    // Note: If you are using Windows 2000, you need to specify -
    // the default authentication credentials for a user by using
    // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
    // parameter of CoInitializeSecurity ------------------------

    hres =  CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );

                      
    if (FAILED(hres))
    {
        //cout << "Failed to initialize security. Error code = 0x" 
        //    << hex << hres << endl;
        //CoUninitialize();
        return 0;                    // Program has failed.
    }
    
    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if (FAILED(hres))
    {
        //cout << "Failed to create IWbemLocator object."
        //    << " Err code = 0x"
        //    << hex << hres << endl;
        //CoUninitialize();
        return 0;                 // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices *pSvc = NULL;
	
    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pSvc                    // pointer to IWbemServices proxy
         );
    
    if (FAILED(hres))
    {
        //cout << "Could not connect. Error code = 0x" 
        //     << hex << hres << endl;
        pLoc->Release();     
        //CoUninitialize();
        return 0;                // Program has failed.
    }

    //cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name 
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
        //cout << "Could not set proxy blanket. Error code = 0x" 
        //    << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();     
        //CoUninitialize();
        return 0;               // Program has failed.
    }

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    // For example, get the name of the operating system
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"), 
        bstr_t("SELECT * FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &pEnumerator);
    
    if (FAILED(hres))
    {
        //cout << "Query for operating system name failed."
        //    << " Error code = 0x" 
        //    << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        //CoUninitialize();
        return 0;               // Program has failed.
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------
 
    IWbemClassObject *pclsObj;
    ULONG uReturn = 0;

	int video_memory=0;
   
    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
            &pclsObj, &uReturn);

        if(0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        hr = pclsObj->Get(L"AdapterRAM", 0, &vtProp, 0, 0);
		if(SUCCEEDED(hr))
			video_memory=vtProp.intVal;
        VariantClear(&vtProp);
    }

    // Cleanup
    // ========
    
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    pclsObj->Release();
    //CoUninitialize();

    return video_memory;   // Program successfully completed.
}

#include <d3d9.h>

D3DFORMAT getBackBufferFormat(IDirect3D9* lpD3D, bool fullscreen, bool stencil, bool alpha)
{
	D3DFORMAT BackBufferFormat=D3DFMT_X8R8G8B8;
	DWORD Adapter=0;
	if(!fullscreen){
		D3DDISPLAYMODE d3ddm;
		DWORD Adapter=0;
		if(lpD3D->GetAdapterDisplayMode(Adapter,&d3ddm) == S_OK){
			BackBufferFormat = d3ddm.Format;
			if(0 && stencil)
				BackBufferFormat = D3DFMT_A8R8G8B8;
			else
				BackBufferFormat = D3DFMT_X8R8G8B8;
		}
	}
	else{
		if(0 && stencil)
			BackBufferFormat = D3DFMT_A8R8G8B8;
		else{
			if(lpD3D->GetAdapterModeCount(Adapter,D3DFMT_X8R8G8B8)>0)
				BackBufferFormat = D3DFMT_X8R8G8B8;
			else
				BackBufferFormat = D3DFMT_R8G8B8;
		}
	}
	if(alpha)
		BackBufferFormat=D3DFMT_A8R8G8B8;
	return BackBufferFormat;
}

bool getSupportedResolutions(IDirect3D9* lpD3D, bool fullscreen, bool stencil, bool alpha, std::vector<Vect2i>& modes)
{
	bool needToFreeD3D = false;
	if(!lpD3D){
		lpD3D = Direct3DCreate9(D3D_SDK_VERSION/*_MY*/);
		needToFreeD3D = true;
		if(!lpD3D)
			return false;
	}

	D3DFORMAT BackBufferFormat = getBackBufferFormat(lpD3D, fullscreen, stencil, alpha);

	DWORD Adapter = 0;
	UINT modesCount = lpD3D->GetAdapterModeCount(Adapter, BackBufferFormat);

	for(UINT i = 0; i < modesCount; ++i){
		D3DDISPLAYMODE mode;
		if(FAILED(lpD3D->EnumAdapterModes(Adapter, BackBufferFormat, i, &mode)))
			return false;

		Vect2i size(int(mode.Width), int(mode.Height));
		if(std::find(modes.begin(), modes.end(), size) == modes.end())
			modes.push_back(size);
	}
	if(needToFreeD3D)
		if(lpD3D){
			lpD3D->Release();
			lpD3D = 0;
		}
	return true;
}

bool CheckDeviceType(IDirect3D9* lpD3D, int xscr,int yscr, bool fullscreen, bool stencil, bool alpha, std::vector<DWORD>* multisamplemode)
{
	DWORD Adapter=0;
	bool needToFreeD3D = false;
	if(!lpD3D){
		lpD3D = Direct3DCreate9(D3D_SDK_VERSION/*_MY*/);
		needToFreeD3D = true;
		if(!lpD3D)
			return false;
	}

	D3DFORMAT BackBufferFormat = getBackBufferFormat(lpD3D, fullscreen, stencil, alpha);
	UINT modes = lpD3D->GetAdapterModeCount(Adapter, BackBufferFormat);

	if(fullscreen)
	{
		bool found=false;
		for(UINT i=0;i<modes;i++)
		{
			D3DDISPLAYMODE mode;
			if(FAILED(lpD3D->EnumAdapterModes(Adapter,
					BackBufferFormat,
					i,
					&mode
			)))
				return false;

			if(mode.Width==xscr && mode.Height==yscr && 
				mode.Format==BackBufferFormat)
			{
				found=true;
				break;
			}
		}

		if(!found)
			return false;
	}

	if(multisamplemode)
	{
		multisamplemode->push_back(D3DMULTISAMPLE_NONE);

		//for(DWORD nsample=D3DMULTISAMPLE_2_SAMPLES;nsample<=D3DMULTISAMPLE_16_SAMPLES;nsample++)
		// FIXME!
		for(DWORD nsample=D3DMULTISAMPLE_2_SAMPLES;nsample<=D3DMULTISAMPLE_8_SAMPLES;nsample++)//Ограничили, чтобы не было проблем с недостатком видеопамяти.
		{
			DWORD QualityLevels;
			HRESULT hr=lpD3D->CheckDeviceMultiSampleType(Adapter,
					D3DDEVTYPE_HAL,
					BackBufferFormat,
					!fullscreen,
					(D3DMULTISAMPLE_TYPE)nsample,
					&QualityLevels
				);

			if(hr==S_OK)
			{
				multisamplemode->push_back(nsample);
			}
		}
	}

	if(needToFreeD3D)
		if(lpD3D){
			lpD3D->Release();
			lpD3D = 0;
		}
	return true;
}