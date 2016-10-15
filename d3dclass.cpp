////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "d3dclass.h"


D3DClass::D3DClass()
{
}

bool D3DClass::Initialize(int screenHeight, int screenWidth, HWND hwnd)
{	
	CreateFactory();
	CreateDevice();

	CreateCommandQueue();
	
	CreateSwapChain(screenHeight, screenWidth, hwnd);

	CreateRenderTargetViewDescriptorHeap();
	CreateRenderTargetView();

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
	result = m_commandList->Reset(m_commandAllocator[m_CurrentBufferIndex].Get(), mPSO.Get());
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


	// Set the back buffer as the render target.
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, NULL);
	
	// Then set the color to clear the window to.
	float color[4] = { 0.f, 0.f, 0.5f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, color, 0, NULL);

	//////////////////////////////////////////////////////////////////////////
	// something start

	m_commandList->SetGraphicsRootSignature(mRootSignature.Get()); // set the root signature
	m_commandList->RSSetViewports(1, &m_Viewport); // set the viewports
	m_commandList->RSSetScissorRects(1, &m_ScissorRect); // set the scissor rects
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
	m_commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView); // set the vertex buffer (using the vertex buffer view)
	m_commandList->IASetIndexBuffer(&m_indexBufferView);

	m_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	m_commandList->DrawIndexedInstanced(6, 1, 0, 4, 0);
	

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
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	return true;
}

bool D3DClass::BuildRootSignatures()
{
	// 지금은 '루트 시그니처'에 아무런 '루트 파라미터'가 없지만 나중을 위해서 일부러 넣었다.
	std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameter;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(slotRootParameter.size(), slotRootParameter.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
	CreateVertexBuffer();
	CreateIndexBuffer();

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
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	
	HRESULT hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO));
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool D3DClass::CreateVertexBuffer()
{
	Vertex vList[] = {
		// first quad (closer to camera, blue)
		{ -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
		{ 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
		{ -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
		{ 0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },

		// second quad (further from camera, green)
		{ -0.75f,  0.75f,  0.7f, 0.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f,  0.0f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f },
		{ -0.75f,  0.0f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f,  0.75f,  0.7f, 0.0f, 1.0f, 0.0f, 1.0f }
	};

	int nVertexBufferSize = sizeof(vList);

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
	vertexData.pData = vList;
	vertexData.RowPitch = nVertexBufferSize;
	vertexData.SlicePitch = nVertexBufferSize;
						
	m_commandList->Reset(m_commandAllocator[m_CurrentBufferIndex].Get(), nullptr);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pVertexBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(m_commandList.Get(), m_pVertexBuffer.Get(), vBufferUploadHeap, 0, 0, 1, &vertexData);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	m_commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(Vertex);
	m_VertexBufferView.SizeInBytes = nVertexBufferSize;

	return true;
}

bool D3DClass::CreateIndexBuffer()
{
	DWORD iList[] = {
		// first quad (blue)
		0, 1, 2, // first triangle
		0, 3, 1, // second triangle
	};

	int nIndexBufferSize = sizeof(iList);

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
	indexData.pData = iList;
	indexData.RowPitch = nIndexBufferSize;
	indexData.SlicePitch = nIndexBufferSize;

	m_commandList->Reset(m_commandAllocator[m_CurrentBufferIndex].Get(), nullptr);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pIndexBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(m_commandList.Get(), m_pIndexBuffer.Get(), vBufferUploadHeap, 0, 0, 1, &indexData);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pIndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	m_commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_indexBufferView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = nIndexBufferSize;

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
