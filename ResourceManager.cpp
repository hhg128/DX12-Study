#include "ResourceManager.h"


CResourceManager::CResourceManager()
{
}


CResourceManager::~CResourceManager()
{
}

void CResourceManager::Initialize()
{
	m_pSdkManager = FbxManager::Create();
}

void CResourceManager::Finalize()
{
	m_pScene = nullptr;

	// ���������� SdkManager�� ��������.
	if (m_pSdkManager)	m_pSdkManager->Destroy();
}

void CResourceManager::Load(const std::wstring fbxFileName)
{

}
