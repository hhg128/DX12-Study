////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_


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

//////////////
// INCLUDES //
//////////////
#include <d3d12.h>
#include <dxgi1_4.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

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

struct Vertex
{
	Vertex(float x, float y, float z, float r, float g, float b, float a) : Pos(x, y, z), color(r, g, b, z) {}

	XMFLOAT3 Pos;
	XMFLOAT4 color;
};

////////////////////////////////////////////////////////////////////////////////
// Class name: D3DClass
////////////////////////////////////////////////////////////////////////////////
class D3DClass
{
public:
	D3DClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	
	bool Render();

	bool BuildShader();
	bool BuildInputLayout();
	bool BuildRootSignatures();
	bool BuildDescriptorHeaps();
	bool BuildConstantBuffers();
	bool BuildGeometry();
	bool BuildPSO();

	bool CreateVertexBuffer();
	bool CreateIndexBuffer();
	
	bool CreateDevice();
	bool CreateCommandQueue();
	bool CreateFactory();
	bool CreateSwapChain(const int screenHeight, const int screenWidth, HWND hwnd);
	
	bool CreateRenderTargetViewDescriptorHeap();
	bool CreateRenderTargetView();
	
	bool CreateCommandAllocator();
	bool CreateCommandList();
	
	bool CreateFence();

private:
	ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target)
	{
		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		HRESULT hr = S_OK;

		ComPtr<ID3DBlob> byteCode = nullptr;
		ComPtr<ID3DBlob> errors;
		hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

		if (errors != nullptr)
			OutputDebugStringA((char*)errors->GetBufferPointer());

		AssertIfFailed(hr);

		return byteCode;
	}

private:

	static const int m_nBackBufferCount = 2;

	ComPtr<ID3D12Device>			m_device;
	ComPtr<ID3D12CommandQueue>		m_commandQueue;
	ComPtr<IDXGISwapChain3>			m_swapChain;

	ComPtr<ID3D12DescriptorHeap>	m_renderTargetViewHeap;
	ComPtr<ID3D12Resource>			m_backBufferRenderTarget[m_nBackBufferCount];
	
	unsigned int m_CurrentBufferIndex;

	ComPtr<ID3D12CommandAllocator>		m_commandAllocator[2];
	ComPtr<ID3D12GraphicsCommandList>	m_commandList;
	ComPtr<ID3D12PipelineState>			m_pipelineState;
	ComPtr<ID3D12Fence>					m_fence;
	HANDLE								m_fenceEvent = 0;
	unsigned long long					m_fenceValue;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

	ComPtr<ID3D12Resource>	mUploadCBUffer;

	ComPtr<ID3D12PipelineState> mPSO = nullptr;

	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	D3D12_VIEWPORT		m_Viewport;
	D3D12_RECT			m_ScissorRect;

	ComPtr<ID3D12Resource>		m_pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW	m_VertexBufferView;

	ComPtr<ID3D12Resource>		m_pIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW		m_indexBufferView;

	ComPtr<IDXGIFactory4>	m_dxgiFactory;

	ComPtr<ID3D12DescriptorHeap>	m_DepthStencilViewHeap;
	ComPtr<ID3D12Resource>			m_DepthStencilBuffer;


	int rtvDescriptorSize;
};

#endif