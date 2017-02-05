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

	void Load(std::string fbxFileName);

	void Export();

private:
	FbxManager*		m_pSdkManager = nullptr;
	FbxScene*		m_pScene = nullptr;

public:
	std::unordered_map<std::string, ModelClass*> m_ModelMap;
};

