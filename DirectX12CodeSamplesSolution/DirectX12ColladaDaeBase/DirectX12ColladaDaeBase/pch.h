//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <collection.h>
#include <ppltasks.h>

#include "App.xaml.h"


#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include "Common\d3dx12.h"
#include <pix.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <set>
#include <map>  
#include <limits>
#include <agile.h>
#include <concrt.h>
#include <collection.h>
#if defined(_DEBUG)
#include <dxgidebug.h>
#endif

#include <shcore.h>
#include <xmllite.h>
#define ChkHr(stmt) do { hr = stmt; if (FAILED(hr)) return hr; } while(0)