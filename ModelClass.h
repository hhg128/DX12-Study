#pragma once

using namespace DirectX;

class MeshClass
{
public:
	MeshClass();
	~MeshClass();

public:
	struct VertexType
	{
		XMFLOAT3 Pos;
		XMFLOAT2 UV;
		XMFLOAT3 Normal;
	};

	struct IndexType
	{
		DWORD a, b, c;
	};

	std::vector<IndexType>		m_IndexArray;
	std::vector<VertexType>		m_VertexArray;

	XMFLOAT3 m_vPos;
	XMFLOAT3 m_vRot;
	XMFLOAT3 m_vScale;

	XMFLOAT4X4 m_mat;

	INT64 m_textureIndex = 0;

	std::vector<int64_t> m_TexterIdArray;

	ModelClass*		m_pModel = nullptr;
};

class ModelClass
{
public:
	ModelClass();
	~ModelClass();

	void LoadTextures();

public:
	std::vector<MeshClass*>		m_MeshArray;

	std::unordered_map<int, std::wstring> m_TextureMap;
	std::unordered_map<std::wstring, int> m_TextureIndexMap;
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
};

