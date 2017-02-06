#pragma once

using namespace DirectX;

class ModelClass
{
public:
	ModelClass();
	~ModelClass();

	void DrawIndexedInstanced();

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

public:
	int VertexBufferSize() { return m_VertexArray.size() * sizeof(VertexType); }
};

