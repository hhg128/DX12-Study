#include "stdafx.h"
#include "ResourceManager.h"
#include "ModelClass.h"

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
	// 관리하던 모델 정보를 정리한다.
	for (auto item : m_ModelMap)
	{
		delete item.second;
	}
	m_ModelMap.clear();


	m_pScene = nullptr;

	// 마지막으로 SdkManager를 정리하자.
	if (m_pSdkManager)	m_pSdkManager->Destroy();
}

void CResourceManager::Load(std::string fbxFileName)
{
	// 같은 이름을 가진 파일이 있었는지 확인해보고, 있었다면 로드하지 않고 넘어간다.
	auto search = m_ModelMap.find(fbxFileName);
	if (search != m_ModelMap.end())
	{
		std::string message_a = "[warning]" + fbxFileName + " already exist!";
		std::wstring message_w;

		message_w.assign(message_a.begin(), message_a.end());
		OutputDebugString(message_w.c_str());

		return;
	}

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

	// Convert Axis System to what is used in this example, if needed
	FbxAxisSystem SceneAxisSystem = pScene->GetGlobalSettings().GetAxisSystem();
	FbxAxisSystem OurAxisSystem(FbxAxisSystem::DirectX);
	if (SceneAxisSystem != OurAxisSystem)
	{
		OurAxisSystem.ConvertScene(pScene);
	}

	const int lNodeCount = pScene->GetSrcObjectCount<FbxNode>();
	for (int lIndex = 0; lIndex < lNodeCount; lIndex++)
	{
		FbxNode* pNode = pScene->GetSrcObject<FbxNode>(lIndex);

		if (pNode->GetGeometry())
		{
			FbxMesh* pMesh = pNode->GetMesh();
			int nControlPointCount = pMesh->GetControlPointsCount();
			int nTriangleCount = pMesh->GetPolygonCount();

			ModelClass* model = new ModelClass;

			// Vertex List
			for (int i = 0; i < nControlPointCount; ++i)
			{
				XMFLOAT3 currPosition;
				currPosition.x = static_cast<float>(pMesh->GetControlPointAt(i).mData[0]);
				currPosition.y = static_cast<float>(pMesh->GetControlPointAt(i).mData[2]);
				currPosition.z = static_cast<float>(pMesh->GetControlPointAt(i).mData[1]);

				ModelClass::VertexType vertex;
				vertex.Pos = currPosition;
				model->m_VertexArray.push_back(vertex);
			}

			// Index List
			for (int i = 0; i < nTriangleCount; ++i)
			{
				int indexA = pMesh->GetPolygonVertex(i, 0);
				int indexB = pMesh->GetPolygonVertex(i, 1);
				int indexC = pMesh->GetPolygonVertex(i, 2);

				ModelClass::IndexType index;
				index.a = indexA;
				index.b = indexB;
				index.c = indexC;
				model->m_IndexArray.push_back(index);
			}

			m_ModelMap[fbxFileName] = model;
		}
	}
}

void CResourceManager::Export()
{
}
