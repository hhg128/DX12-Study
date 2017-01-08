#pragma once

#include <fbxsdk.h>

class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();

	void Initialize();
	void Finalize();

private:
	FbxManager*		m_pSdkManager = nullptr;
	FbxScene*		m_pScene = nullptr;
};

