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
};

class ModelClass
{
public:
	ModelClass();
	~ModelClass();

public:
	std::vector<MeshClass*>		m_MeshArray;
};

