#include "stdafx.h"
#include "systemclass.h"
#include "TextureManager.h"
#include "ModelClass.h"
#include "d3dclass.h"

MeshClass::MeshClass()
{

}

MeshClass::~MeshClass()
{

}

//////////////////////////////////////////////////////////////////////////

ModelClass::ModelClass()
{
}


ModelClass::~ModelClass()
{
}

void ModelClass::LoadTextures()
{
	UINT mCbvSrvDescriptorSize = gD3dClass->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = m_TextureMap.size();
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	gD3dClass->m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (auto& textureName : m_TextureMap)
	{
		auto& texture = gSystem->TextureManager->m_TextureMap[textureName.second];
		std::string textureFileName;
		StringHelper::ConvertWStringToString(texture->Filename, textureFileName);

		AssertIfFailed(DirectX::CreateDDSTextureFromFile12(gD3dClass->m_device.Get(), gD3dClass->m_commandList.Get(), texture->Filename.c_str(), texture->Resource, texture->UploadHeap));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = texture->Resource->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = texture->Resource->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		gD3dClass->m_device->CreateShaderResourceView(texture->Resource.Get(), &srvDesc, hDescriptor);

		hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	}
}
