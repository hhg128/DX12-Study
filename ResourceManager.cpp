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

	// 마지막으로 SdkManager를 정리하자.
	if (m_pSdkManager)	m_pSdkManager->Destroy();
}

void CResourceManager::Load(const std::string fbxFileName)
{
	FbxIOSettings* pIOSettings = FbxIOSettings::Create(m_pSdkManager, IOSROOT);
	m_pSdkManager->SetIOSettings(pIOSettings);

	FbxImporter* pImporter = FbxImporter::Create(m_pSdkManager, "");
	if ( !pImporter->Initialize(fbxFileName.c_str(), -1, m_pSdkManager->GetIOSettings()) )
	{
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", pImporter->GetStatus().GetErrorString());
		exit(-1);
	}

	// Create a new scene so it can be populated by the imported file.
	FbxScene* pScene = FbxScene::Create(m_pSdkManager, "myScene");

	// Import the contents of the file into the scene.
	pImporter->Import(pScene);

	const int lNodeCount = pScene->GetSrcObjectCount<FbxNode>();
	for (int lIndex = 0; lIndex < lNodeCount; lIndex++)
	{
		FbxNode* pNode = pScene->GetSrcObject<FbxNode>(lIndex);

		if (pNode->GetGeometry())
		{
			FbxMesh* lMesh = pNode->GetMesh();
			int lVertexCount = lMesh->GetControlPointsCount();
			
			FbxVector4* lVertexArray = NULL;
			lVertexArray = new FbxVector4[lVertexCount];
			memcpy(lVertexArray, lMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

			printf("");
		}
	}
}
