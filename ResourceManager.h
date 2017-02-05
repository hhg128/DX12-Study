#pragma once

using namespace DirectX;

class ModelClass;

class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();

	void Initialize();
	void Finalize();

	void Load(const std::string fbxFileName);

	void Export();

private:
	FbxManager*		m_pSdkManager = nullptr;
	FbxScene*		m_pScene = nullptr;

public:

	std::vector<ModelClass*> m_ModelArray;
};

