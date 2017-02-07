////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "d3dclass.h"
#include "systemclass.h"
#include "ResourceManager.h"
#include "ModelClass.h"

D3DClass::D3DClass()
{
}

bool D3DClass::Initialize(int screenHeight, int screenWidth, HWND hwnd)
{	
	m_nWindowWidth = screenWidth;
	m_nWindowHeight = screenHeight;

	CreateFactory();
	CreateDevice();

	CreateCommandQueue();
	
	CreateSwapChain(screenHeight, screenWidth, hwnd);

	CreateRenderTargetViewDescriptorHeap();
	CreateRenderTargetView();

	CreateDepthStencilViewDescriptorHeap();
	CreateDepthStencilView();

	CreateConstantBufferViewDescriptorHeap();
	CreateConstantBufferView();

	BuildRootSignatures();

	BuildShader();

	BuildInputLayout();
	BuildPSO();

	CreateCommandAllocator();
	CreateCommandList();
		
	CreateFence();

	// Fill out the Viewport
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = static_cast<FLOAT>(screenWidth);
	m_Viewport.Height = static_cast<FLOAT>(screenHeight);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.right = screenWidth;
	m_ScissorRect.bottom = screenHeight;
	
	//gSystem->m_pResourceManager->Load("plane.fbx");
	//gSystem->m_pResourceManager->Load("plane.fbx");
	//gSystem->m_pResourceManager->Load("cube_size_1.fbx");
	//gSystem->m_pResourceManager->Load("humanoid.fbx");
	//gSystem->m_pResourceManager->Load("BG.fbx");
	gSystem->m_pResourceManager->Load("Cube FBX\\cube_with_texture.fbx");

	return true;
}


void D3DClass::Shutdown()
{
	int error;

	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if(m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}
		
	// Close the object handle to the fence event.
	error = CloseHandle(m_fenceEvent);
	if(error == 0)
	{
	}

	return;
}


void D3DClass::Tick(float fDelta)
{
	OnCamera(fDelta);
}

bool D3DClass::Render()
{
	HRESULT result;
	
	// Reset (re-use) the memory associated command allocator.
	result = m_commandAllocator[m_CurrentBufferIndex].Get()->Reset();
	if(FAILED(result))
	{
		return false;
	}

	// Reset the command list, use empty pipeline state for now since there are no shaders and we are just clearing the screen.
	result = m_commandList->Reset(m_commandAllocator[m_CurrentBufferIndex].Get(), m_pipelineState.Get());
	if(FAILED(result))
	{
		return false;
	}

	// Record commands in the command list now.
	// Start by setting the resource barrier.
	m_commandList->ResourceBarrier(1, 
		&CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTarget[m_CurrentBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));


	int renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), m_CurrentBufferIndex, renderTargetViewDescriptorSize);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DepthStencilViewHeap->GetCPUDescriptorHandleForHeapStart());

	// Set the back buffer as the render target.
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	
	// Then set the color to clear the window to.
	float color[4] = { 0.f, 0.3f, 0.5f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, color, 0, NULL);
	m_commandList->ClearDepthStencilView(m_DepthStencilViewHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//////////////////////////////////////////////////////////////////////////
	// something start

	m_commandList->SetGraphicsRootSignature(mRootSignature.Get()); // set the root signature
	m_commandList->RSSetViewports(1, &m_Viewport); // set the viewports
	m_commandList->RSSetScissorRects(1, &m_ScissorRect); // set the scissor rects
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
	m_commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView); // set the vertex buffer (using the vertex buffer view)
	m_commandList->IASetIndexBuffer(&m_indexBufferView);

	auto passCB = PassCB->Resource();
	m_commandList->SetGraphicsRootConstantBufferView(0, passCB->GetGPUVirtualAddress());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_commandList->SetGraphicsRootDescriptorTable(1, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	//ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	//m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

	int baseVertexLocation = 0;
	for (auto& iter : gSystem->m_pResourceManager->m_ModelMap)
	{
		int vertexCount = iter.second->m_VertexArray.size();
		int triangleCount = iter.second->m_IndexArray.size();

		m_commandList->DrawIndexedInstanced(triangleCount * 3, 1, 0, baseVertexLocation, 0);
		baseVertexLocation += vertexCount;
	}

	// something end
	//////////////////////////////////////////////////////////////////////////

	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTarget[m_CurrentBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Close the list of commands.
	result = m_commandList->Close();
	if(FAILED(result))
	{
		return false;
	}

	// Load the command list array (only one command list for now).
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };

	// Execute the list of commands.
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


	// Present as fast as possible.
	result = m_swapChain->Present(0, 0);
	if (FAILED(result))
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	// Signal and increment the fence value.
	unsigned long long fenceToWaitFor;
	fenceToWaitFor = m_fenceValue;
	result = m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor);
	if(FAILED(result))
	{
		return false;
	}
	m_fenceValue++;

	// Wait until the GPU is done rendering.
	if(m_fence->GetCompletedValue() < fenceToWaitFor)
	{
		result = m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent);
		if(FAILED(result))
		{
			return false;
		}
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	// Alternate the back buffer index back and forth between 0 and 1 each frame.
	m_CurrentBufferIndex == 0 ? m_CurrentBufferIndex = 1 : m_CurrentBufferIndex = 0;

	return true;
}

bool D3DClass::BuildShader()
{
	mvsByteCode = CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	return true;
}

bool D3DClass::BuildInputLayout()
{
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	return true;
}

bool D3DClass::BuildRootSignatures()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

	CD3DX12_ROOT_PARAMETER constant[2];
	constant[0].InitAsConstantBufferView(0, 0);
	constant[1].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	// 지금은 '루트 시그니처'에 아무런 '루트 파라미터'가 없지만 나중을 위해서 일부러 넣었다.
	std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameter;
	slotRootParameter.push_back(constant[0]);
	slotRootParameter.push_back(constant[1]);

	//const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
	//	0, // shaderRegister
	//	D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
	//	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
	//	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
	//	D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//CD3DX12_STATIC_SAMPLER_DESC sampler[1] = { linearWrap };

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//rootSignatureDesc.Init(slotRootParameter.size(), slotRootParameter.data(), 1, sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	rootSignatureDesc.Init(_countof(constant), constant, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		return false;
	}

	hr = m_device->CreateRootSignature( 0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool D3DClass::BuildGeometry()
{
	m_commandList->Reset(m_commandAllocator[m_CurrentBufferIndex].Get(), nullptr);

	CreateVertexBuffer();
	CreateIndexBuffer();

	LoadTexture("texture");

	m_commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//CreateSRVBufferView();

	return true;
}

bool D3DClass::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize()
	};
	
	CD3DX12_RASTERIZER_DESC resterize_desc(D3D12_DEFAULT);
	resterize_desc.FillMode = D3D12_FILL_MODE_SOLID;		// 테스트를 위해 FillMode, CullMode 를 보기 쉽게 한다
	resterize_desc.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState = resterize_desc;

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	
	HRESULT hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool D3DClass::CreateVertexBuffer()
{
	std::vector<Vertex> VertexVector;

	for (auto& iter : gSystem->m_pResourceManager->m_ModelMap)
	{
		const auto& vertexArray = iter.second->m_VertexArray;
		for (size_t j = 0; j < vertexArray.size(); ++j)
		{
			Vertex v;
			v.Pos = vertexArray[j].Pos;
			v.UV = vertexArray[j].UV;

			VertexVector.push_back(v);
		}
	}

	int nVertexBufferSize = VertexVector.size() * sizeof(Vertex);

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(nVertexBufferSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr, 
		IID_PPV_ARGS(&m_pVertexBuffer));

	m_pVertexBuffer->SetName(L"Vertex Buffer Resource Heap!!!");


	ID3D12Resource* vBufferUploadHeap;
	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(nVertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));

	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = VertexVector.data();
	vertexData.RowPitch = nVertexBufferSize;
	vertexData.SlicePitch = nVertexBufferSize;
						
	//m_commandList->Reset(m_commandAllocator[m_CurrentBufferIndex].Get(), nullptr);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pVertexBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(m_commandList.Get(), m_pVertexBuffer.Get(), vBufferUploadHeap, 0, 0, 1, &vertexData);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	//m_commandList->Close();
	//ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	//m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(Vertex);
	m_VertexBufferView.SizeInBytes = nVertexBufferSize;

	return true;
}

bool D3DClass::CreateIndexBuffer()
{
	std::vector<int> copyIndexArray;

	for (auto& iter : gSystem->m_pResourceManager->m_ModelMap)
	{
		const auto& indexArray = iter.second->m_IndexArray;
		for (size_t j = 0; j < indexArray.size(); ++j)
		{
			copyIndexArray.push_back(indexArray[j].a);
			copyIndexArray.push_back(indexArray[j].b);
			copyIndexArray.push_back(indexArray[j].c);
		}
	}

	int nIndexBufferSize = copyIndexArray.size() * sizeof(int);

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(nIndexBufferSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_pIndexBuffer));

	m_pIndexBuffer->SetName(L"Index Buffer Resource Heap!!!");

	ID3D12Resource* vBufferUploadHeap;
	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(nIndexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));

	vBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = copyIndexArray.data();
	indexData.RowPitch = nIndexBufferSize;
	indexData.SlicePitch = nIndexBufferSize;

	//m_commandList->Reset(m_commandAllocator[m_CurrentBufferIndex].Get(), nullptr);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pIndexBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(m_commandList.Get(), m_pIndexBuffer.Get(), vBufferUploadHeap, 0, 0, 1, &indexData);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pIndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	//m_commandList->Close();
	//ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	//m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_indexBufferView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = nIndexBufferSize;

	return true;
}

bool D3DClass::CreateConstantBufferViewDescriptorHeap()
{
	HRESULT result;

	// Initialize the render target view heap description for the two back buffers.
	D3D12_DESCRIPTOR_HEAP_DESC constantBufferViewHeapDesc = {};

	// Set the number of descriptors to two for our two back buffers.  Also set the heap type to render target views.
	constantBufferViewHeapDesc.NumDescriptors = 1;
	constantBufferViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	constantBufferViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	// Create the render target view heap for the back buffers.
	result = m_device->CreateDescriptorHeap(&constantBufferViewHeapDesc, IID_PPV_ARGS(&m_cbvHeap));
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool D3DClass::CreateConstantBufferView()
{
	PassCB = std::make_unique<UploadBuffer<ConstantBuffer>>(m_device.Get(), 1, true);

	HRESULT hr;

	const UINT constantBufferSize = sizeof(ConstantBuffer) * m_nBackBufferCount;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pConstantBuffer)
	));

	m_pConstantBuffer->SetName(TEXT("Constant Buffer!!"));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_pConstantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (sizeof(m_pConstantBuffer) + 255) & ~255;	// CB size is required to be 256-byte aligned.
	m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	hr = m_pConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBufferData));
	if (FAILED(hr))
	{
		return false;
	}
	ZeroMemory(m_pConstantBufferData, constantBufferSize);

	return true;
}

bool D3DClass::CreateDevice()
{
	HRESULT hr;

	D3D_FEATURE_LEVEL featureLevel;
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the Direct3D 12 device.
	hr = D3D12CreateDevice(NULL, featureLevel, __uuidof(ID3D12Device), (void**)&m_device);
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"Could not create a DirectX 12 device. I will try to make the warp device .", L"DirectX Device Failure", MB_OK);

		ComPtr<IDXGIAdapter> warpAdapter;
		m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

		hr = D3D12CreateDevice( warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create a DirectX 12.1 device.  The default video card does not support DirectX 11.0.", L"DirectX Device Failure", MB_OK);
			return false;
		}
	}

	return true;
}

bool D3DClass::CreateCommandQueue()
{
	// Create the command queue.
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;

	HRESULT hr;
	hr = m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_commandQueue));
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool D3DClass::CreateFactory()
{
	// 이것저것 있지만 일단 factory를 만들기만 하자
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));
	
	return true;
}

bool D3DClass::CreateSwapChain(const int screenHeight, const int screenWidth, HWND hwnd)
{
	HRESULT result;

	DXGI_MODE_DESC backBufferDesc = {};					// this is to describe our display mode
	backBufferDesc.Width = screenWidth;					// buffer width
	backBufferDesc.Height = screenHeight;				// buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;								// multisample count (no multisampling, so we just put 1, since we still need 1 sample)

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = m_nBackBufferCount;					// number of buffers we have
	swapChainDesc.BufferDesc = backBufferDesc;						// our back buffer description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// this says the pipeline will render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// dxgi will discard the buffer (data) after we call present
	swapChainDesc.OutputWindow = hwnd;								// handle to our window
	swapChainDesc.SampleDesc = sampleDesc;							// our multi-sampling description
	swapChainDesc.Windowed = true;									// set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

	// Finally create the swap chain using the swap chain description.	
	ComPtr<IDXGISwapChain> swapChain;
	result = m_dxgiFactory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, swapChain.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// Next upgrade the IDXGISwapChain to a IDXGISwapChain3 interface and store it in a private member variable named m_swapChain.
	// This will allow us to use the newer functionality such as getting the current back buffer index.
	result = swapChain.As(&m_swapChain);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool D3DClass::CreateRenderTargetViewDescriptorHeap()
{
	HRESULT result;

	// Initialize the render target view heap description for the two back buffers.
	D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc = {};

	// Set the number of descriptors to two for our two back buffers.  Also set the heap type to render target views.
	renderTargetViewHeapDesc.NumDescriptors = 2;
	renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	renderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// Create the render target view heap for the back buffers.
	result = m_device->CreateDescriptorHeap(&renderTargetViewHeapDesc, IID_PPV_ARGS(&m_renderTargetViewHeap));
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool D3DClass::CreateRenderTargetView()
{
	HRESULT result;

	// Get the size of the memory location for the render target view descriptors.
	int renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Get a handle to the starting memory location in the render target view heap to identify where the render target views will be located for the two back buffers.
	CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle(m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
	// Create a RTV for each buffer (double buffering is two buffers, tripple buffering is 3).
	for (int i = 0; i < m_nBackBufferCount; i++)
	{
		// first we get the n'th buffer in the swap chain and store it in the n'th
		// position of our ID3D12Resource array
		result = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBufferRenderTarget[i]));
		if (FAILED(result))
		{
			return false;
		}

		// the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
		m_device->CreateRenderTargetView(m_backBufferRenderTarget[i].Get(), nullptr, renderTargetViewHandle);

		// we increment the rtv handle by the rtv descriptor size we got above
		renderTargetViewHandle.Offset(1, renderTargetViewDescriptorSize);
	}

	return true;
}

bool D3DClass::CreateDepthStencilViewDescriptorHeap()
{
	HRESULT result;

	// Initialize the render target view heap description for the two back buffers.
	D3D12_DESCRIPTOR_HEAP_DESC depthStencilViewHeapDesc = {};

	// Set the number of descriptors to two for our two back buffers.  Also set the heap type to render target views.
	depthStencilViewHeapDesc.NumDescriptors = 1;
	depthStencilViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	depthStencilViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// Create the render target view heap for the back buffers.
	result = m_device->CreateDescriptorHeap(&depthStencilViewHeapDesc, IID_PPV_ARGS(&m_DepthStencilViewHeap));
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool D3DClass::CreateDepthStencilView()
{
	HRESULT result;

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format			= DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension	= D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags			= D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format					= DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth		= 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil	= 0;

	result = m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_nWindowWidth, m_nWindowHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_DepthStencilBuffer));
	if (FAILED(result))
	{
		return false;
	}

	m_DepthStencilBuffer->SetName(L"Depth/Stencil Resource Heap");

	m_device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &depthStencilDesc, m_DepthStencilViewHeap->GetCPUDescriptorHandleForHeapStart());


	return true;
}

bool D3DClass::CreateCommandAllocator()
{
	HRESULT result;

	for (int i = 0; i < m_nBackBufferCount; i++)
	{
		result = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i]));
		if (FAILED(result))
		{
			return false;
		}
	}

	return true;
}

bool D3DClass::CreateCommandList()
{
	HRESULT result;

	// Finally get the initial index to which buffer is the current back buffer.
	m_CurrentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create a basic command list.
	result = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[m_CurrentBufferIndex].Get(), NULL, IID_PPV_ARGS(&m_commandList));
	if (FAILED(result))
	{
		return false;
	}

	// Initially we need to close the command list during initialization as it is created in a recording state.
	result = m_commandList->Close();
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool D3DClass::CreateFence()
{
	HRESULT result;

	// Create a fence for GPU synchronization.
	result = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	if (FAILED(result))
	{
		return false;
	}

	// Create an event object for the fence.
	m_fenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (m_fenceEvent == NULL)
	{
		return false;
	}

	// Initialize the starting fence value. 
	m_fenceValue = 1;

	return true;
}

DWORD					g_dwMouseX = 0;			// 마우스의 좌표
DWORD					g_dwMouseY = 0;			// 마우스의 좌표

void D3DClass::OnCamera(float fDelta)
{
	if (gInput->IsKeyDown(VK_UP))
	{
		m_Camera.Pitch(-1.0f*fDelta);
	}
	if (gInput->IsKeyDown(VK_DOWN))
	{
		m_Camera.Pitch(1.0f*fDelta);
	}
	if (gInput->IsKeyDown(VK_RIGHT))
	{
		m_Camera.RotateY(1.0f*fDelta);
	}
	if (gInput->IsKeyDown(VK_LEFT))
	{
		m_Camera.RotateY(-1.0f*fDelta);
	}

	if (GetAsyncKeyState('W') & 0x8000)
		m_Camera.Walk(1.0f*fDelta);

	if (GetAsyncKeyState('S') & 0x8000)
		m_Camera.Walk(-1.0f*fDelta);

	if (GetAsyncKeyState('A') & 0x8000)
		m_Camera.Strafe(-1.0f*fDelta);

	if (GetAsyncKeyState('D') & 0x8000)
		m_Camera.Strafe(1.0f*fDelta);

	if (GetAsyncKeyState('R') & 0x8000)
	{
		POINT	pt;

		GetCursorPos(&pt);
		int dx = pt.x - g_dwMouseX;	// 마우스의 변화값
		int dy = pt.y - g_dwMouseY;	// 마우스의 변화값

		m_Camera.Pitch(dy*fDelta * 2);
		m_Camera.RotateY(dx*fDelta * 2);

		g_dwMouseX = pt.x;
		g_dwMouseY = pt.y;
	}
	
	// 카메라 행렬 업데이트
	m_Camera.UpdateViewMatrix();

	ConstantBuffer constantBuffer = {};

	XMMATRIX lookMat = m_Camera.GetView();
	XMMATRIX projMat = m_Camera.GetProj();
	
	XMStoreFloat4x4(&constantBuffer.view, XMMatrixTranspose(lookMat));
	XMStoreFloat4x4(&constantBuffer.proj, XMMatrixTranspose(projMat));

	constantBuffer.fDeltaTime = 109.f;

	UINT8* destination = m_pConstantBufferData + sizeof(ConstantBuffer) * m_CurrentBufferIndex;
	memcpy(destination, &constantBuffer, sizeof(ConstantBuffer));

	PassCB->CopyData(0, constantBuffer);
}

void D3DClass::LoadTexture(std::string texFilename)
{
	auto bricksTex = std::make_unique<Texture>();
	bricksTex->Name = "bricksTex";
	bricksTex->Filename = TEXT("bricks.dds");

	mTextures = std::move(bricksTex);
	
	AssertIfFailed(DirectX::CreateDDSTextureFromFile12(m_device.Get(), m_commandList.Get(), mTextures->Filename.c_str(), mTextures->Resource, mTextures->UploadHeap));

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = mTextures->Resource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = mTextures->Resource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	m_device->CreateShaderResourceView(mTextures->Resource.Get(), &srvDesc, hDescriptor);
}

void D3DClass::CreateSRVBufferView()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap));

	ComPtr<ID3D12Resource> textureUploadHeap;

	// Describe and create a Texture2D.
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Width = TextureWidth;
	textureDesc.Height = TextureHeight;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_texture)));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

	// Create the GPU upload buffer.
	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUploadHeap)));

	// Copy data to the intermediate upload heap and then schedule a copy 
	// from the upload heap to the Texture2D.
	std::vector<UINT8> texture = GenerateTextureData();

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &texture[0];
	textureData.RowPitch = TextureWidth * TexturePixelSize;
	textureData.SlicePitch = textureData.RowPitch * TextureHeight;

	UpdateSubresources(m_commandList.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Signal and increment the fence value.
	unsigned long long fenceToWaitFor;
	fenceToWaitFor = m_fenceValue;
	HRESULT result = m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor);
	if (FAILED(result))
	{
		return;
	}
	m_fenceValue++;

	// Wait until the GPU is done rendering.
	if (m_fence->GetCompletedValue() < fenceToWaitFor)
	{
		result = m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent);
		if (FAILED(result))
		{
			return;
		}
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

std::vector<UINT8> D3DClass::GenerateTextureData()
{
	const UINT rowPitch = TextureWidth * TexturePixelSize;
	const UINT cellPitch = rowPitch >> 3;		// The width of a cell in the checkboard texture.
	const UINT cellHeight = TextureWidth >> 3;	// The height of a cell in the checkerboard texture.
	const UINT textureSize = rowPitch * TextureHeight;

	std::vector<UINT8> data(textureSize);
	UINT8* pData = &data[0];

	for (UINT n = 0; n < textureSize; n += TexturePixelSize)
	{
		UINT x = n % rowPitch;
		UINT y = n / rowPitch;
		UINT i = x / cellPitch;
		UINT j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;		// R
			pData[n + 1] = 0x00;	// G
			pData[n + 2] = 0x00;	// B
			pData[n + 3] = 0xff;	// A
		}
		else
		{
			pData[n] = 0xff;		// R
			pData[n + 1] = 0xff;	// G
			pData[n + 2] = 0xff;	// B
			pData[n + 3] = 0xff;	// A
		}
	}

	return data;
}

//////////////////////////////////////////////////////////////////////////

ComPtr<ID3DBlob> D3DClass::CompileShader(
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