////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include "CameraClass.h"
#include "UploadBuffer.h"

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
	Vertex() {}
	//Vertex(float x, float y, float z, float r, float g, float b, float a) : Pos(x, y, z), color(r, g, b, z) {}

	XMFLOAT3 Pos;
	XMFLOAT2 UV;
	//XMFLOAT4 color;
};

struct ConstantBuffer
{
	XMFLOAT4X4 view;
	XMFLOAT4X4 proj;

	float fDeltaTime;
	// Constant buffers are 256-byte aligned in GPU memory. Padding is added
	// for convenience when computing the struct's size.
	float padding[31];
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
	
	void Tick(float fDelta);
	bool Render();

	bool BuildShader();
	bool BuildInputLayout();
	bool BuildRootSignatures();
	bool BuildGeometry();
	bool BuildPSO();

	bool CreateVertexBuffer();
	bool CreateIndexBuffer();

	bool CreateConstantBufferViewDescriptorHeap();
	bool CreateConstantBufferView();
	
	bool CreateDevice();
	bool CreateCommandQueue();
	bool CreateFactory();
	bool CreateSwapChain(const int screenHeight, const int screenWidth, HWND hwnd);
	
	bool CreateRenderTargetViewDescriptorHeap();
	bool CreateRenderTargetView();

	bool CreateDepthStencilViewDescriptorHeap();
	bool CreateDepthStencilView();
	
	bool CreateCommandAllocator();
	bool CreateCommandList();
	
	bool CreateFence();

	void OnCamera(float fDelta);

	void LoadTexture(std::string texFilename);

	UINT64 mCurrentFence = 0;
	void FlushCommandQueue();

private:
	ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);

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

	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	D3D12_VIEWPORT		m_Viewport;
	D3D12_RECT			m_ScissorRect;

	ComPtr<ID3D12Resource>		m_pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW	m_VertexBufferView;

	ComPtr<ID3D12Resource>		m_pIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW		m_indexBufferView;

	ComPtr<ID3D12DescriptorHeap>	m_cbvHeap;
	ComPtr<ID3D12Resource>			m_pConstantBuffer;
	D3D12_INDEX_BUFFER_VIEW			m_ConstantBufferView;
	UINT8*							m_pConstantBufferData;

	ComPtr<IDXGIFactory4>	m_dxgiFactory;

	ComPtr<ID3D12DescriptorHeap>	m_DepthStencilViewHeap;
	ComPtr<ID3D12Resource>			m_DepthStencilBuffer;



	int m_nWindowWidth, m_nWindowHeight;

	int rtvDescriptorSize;

	CameraClass m_Camera;


	std::unique_ptr<UploadBuffer<ConstantBuffer>> PassCB = nullptr;

	std::unique_ptr<Texture> mTextures;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	ComPtr<ID3D12Resource> m_texture;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
};

#endif