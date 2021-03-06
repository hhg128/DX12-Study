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

	void ReadTextureId(FbxMesh* pMesh, MeshClass* meshClass);
	void ReadTextureInfo(FbxScene* pScene, ModelClass* modelClass);

	void ReadMatrix(FbxNode* pNode, MeshClass* meshClass);
	void ReadVertex(FbxMesh* pMesh, size_t nControlPointCount, MeshClass* meshClass);
	void ReadIndex(FbxMesh* pMesh, size_t nTriangleCount, MeshClass* meshClass);
	
	void ReadTexture();
	void ReadUV(FbxMesh* pMesh, size_t nTriangleCount, MeshClass* meshClass);

	void ReadNormal(FbxMesh* pMesh, size_t nControlPointCount, MeshClass* meshClass);
	void ReadNormalPerControlPoint(FbxMesh* pMesh, int nControlPointCount, MeshClass* meshClass);
	void ReadNormalPerPolygonVertex(FbxMesh* pMesh, int nControlPointCount, MeshClass* meshClass);

private:
	void ConvertAxisSystem(FbxScene* pScene);

private:
	FbxManager*		m_pSdkManager = nullptr;
	FbxScene*		m_pScene = nullptr;

public:
	std::unordered_map<std::string, ModelClass*> m_ModelMap;
};

