#pragma once

#include <fbxsdk.h>
#include <string>

class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();

	void Initialize();
	void Finalize();

	void Load(const std::string fbxFileName);

private:
	FbxManager*		m_pSdkManager = nullptr;
	FbxScene*		m_pScene = nullptr;
};

