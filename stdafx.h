#pragma once

/////////////
// LINKING //
/////////////
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <iostream>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "d3d12.h"
#include "d3dx12.h"
#include "d3dUtil.h"
#include <Shlwapi.h>
#include <chrono>

#include <wincodec.h>

//////////////
// INCLUDES //
//////////////
#include <d3d12.h>
#include <dxgi1_4.h>

#include <fbxsdk.h>
#include <fbxsdk/scene/fbxaxissystem.h>
#include "StringHelper.h"


#ifndef ReturnFalseIfFailed
#define ReturnFalseIfFailed(x)				\
{                                           \
    HRESULT hr__ = (x);                     \
    if(FAILED(hr__)) { return false; }		\
}

#define AssertIfFailed(x)				\
{                                           \
    HRESULT hr__ = (x);                     \
    if(FAILED(hr__)) { assert(false); }		\
}
#endif