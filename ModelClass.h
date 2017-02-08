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

	int m_textureIndex = 0;
};

class ModelClass
{
public:
	ModelClass();
	~ModelClass();

public:
	std::vector<MeshClass*>		m_MeshArray;
};

