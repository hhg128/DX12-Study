#pragma once

using namespace DirectX;

class ModelClass;
class MeshClass;

class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();

	void Initialize();
	void Finalize();

	void Load(std::string fbxFileName);
	void LoadTexture(std::string fbxFileName);

	void Export();

	void ReadVertex(FbxMesh* pMesh, size_t nControlPointCount, MeshClass* meshClass);
	void ReadIndex(FbxMesh* pMesh, size_t nTriangleCount, MeshClass* meshClass);
	void ReadNormal();
	void ReadTexture();
	void ReadUV(FbxMesh* pMesh, size_t nTriangleCount, MeshClass* meshClass);

private:
	FbxManager*		m_pSdkManager = nullptr;
	FbxScene*		m_pScene = nullptr;

public:
	std::unordered_map<std::string, ModelClass*> m_ModelMap;
};

