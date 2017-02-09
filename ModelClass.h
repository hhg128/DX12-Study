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

	int m_textureIndex = 0;

	std::vector<std::string> m_TexterIdNameArray;
};

class ModelClass
{
public:
	ModelClass();
	~ModelClass();

public:
	std::vector<MeshClass*>		m_MeshArray;

	std::unordered_map<std::string, std::string> m_TextureMap;
};

