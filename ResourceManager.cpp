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

	// texture 구하기
	const int nTextureCount = pScene->GetTextureCount();
	for (int i = 0; i < nTextureCount; ++i)
	{
		FbxTexture* pTexture = pScene->GetTexture(i);
		FbxFileTexture* pFileTexture = FbxCast<FbxFileTexture>(pTexture);
		std::string textureFileName = pFileTexture->GetFileName();

		size_t pos = textureFileName.rfind("\\");
		textureFileName = textureFileName.substr(pos+1);
	}
	
	ModelClass* modelClass = new ModelClass;
	m_ModelMap[fbxFileName] = modelClass;

	// Mesh 구하기
	const int nNodeCount = pScene->GetSrcObjectCount<FbxNode>();
	for (int nIndex = 0; nIndex < nNodeCount; nIndex++)
	{
		FbxNode* pNode = pScene->GetSrcObject<FbxNode>(nIndex);
		if (pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			FbxMesh* pMesh = pNode->GetMesh();
			unsigned int nControlPointCount = pMesh->GetControlPointsCount();
			int nTriangleCount = pMesh->GetPolygonCount();

			MeshClass* meshClass = new MeshClass;

			FbxNode* p = pNode->GetParent();

			FbxDouble3 pos = pNode->LclTranslation.Get();
			FbxDouble3 rot = pNode->LclRotation.Get();
			FbxDouble3 scale = pNode->LclScaling.Get();

			XMFLOAT3 meshPos = XMFLOAT3(pos.mData[0], pos.mData[1], pos.mData[2]);
			XMFLOAT3 meshRot = XMFLOAT3(rot.mData[0], rot.mData[1], rot.mData[2]);
			XMFLOAT3 meshScale = XMFLOAT3(scale.mData[0], scale.mData[1], scale.mData[2]);
			meshClass->m_vPos = meshPos;
			meshClass->m_vRot = meshRot;
			meshClass->m_vScale = meshScale;

			FbxAMatrix& lGlobalTransform = pNode->EvaluateGlobalTransform();
			FbxAMatrix& lLocalTransform = pNode->EvaluateLocalTransform();

			// Vertex List
			for (int i = 0; i < nControlPointCount; ++i)
			{
				// Max 는 Y가 D3D 의 Z임
				XMFLOAT3 currPosition;
				currPosition.x = static_cast<float>(pMesh->GetControlPointAt(i).mData[0]);
				currPosition.y = static_cast<float>(pMesh->GetControlPointAt(i).mData[2]);
				currPosition.z = static_cast<float>(pMesh->GetControlPointAt(i).mData[1]);

				MeshClass::VertexType vertex;
				vertex.Pos = currPosition;
				meshClass->m_VertexArray.push_back(vertex);
			}

			for (int i = 0; i < nTriangleCount; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					XMFLOAT2 outUV;

					FbxGeometryElementUV* vertexUV = pMesh->GetElementUV(0);
					if (vertexUV)
					{
						switch (vertexUV->GetMappingMode())
						{
						case FbxGeometryElement::eByControlPoint:
							switch (vertexUV->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
							{
								outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(i).mData[0]);
								outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(i).mData[1]);
							}
							break;

							case FbxGeometryElement::eIndexToDirect:
							{
								int index = vertexUV->GetIndexArray().GetAt(i);
								outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
								outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
							}
							break;

							default:
								throw std::exception("Invalid Reference");
							}
							break;

						case FbxGeometryElement::eByPolygonVertex:
							switch (vertexUV->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
							case FbxGeometryElement::eIndexToDirect:
							{
								int tIndex = pMesh->GetTextureUVIndex(i, j);
								outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(tIndex).mData[0]);
								outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(tIndex).mData[1]);
							}
							break;

							default:
								throw std::exception("Invalid Reference");
							}
							break;
						}
					}

					int ctrlPointIndex = pMesh->GetPolygonVertex(i, j);
					MeshClass::VertexType& vertex = meshClass->m_VertexArray[ctrlPointIndex];
					vertex.UV.x = outUV.x;
					vertex.UV.y = 1.f - outUV.y;
				}
			}

			// Index List
			for (int i = 0; i < nTriangleCount; ++i)
			{
				int indexA = pMesh->GetPolygonVertex(i, 0);
				int indexB = pMesh->GetPolygonVertex(i, 2);
				int indexC = pMesh->GetPolygonVertex(i, 1);

				// Max 는 CCW로 인덱싱 되어 있음, D3D는 CW가 기본임, 그래서 순서를 바꿔줌
				MeshClass::IndexType index;
				index.a = indexA;
				index.b = indexB;
				index.c = indexC;
				meshClass->m_IndexArray.push_back(index);
			}

			m_ModelMap[fbxFileName]->m_MeshArray.push_back(meshClass);
		}
	}
}

void CResourceManager::LoadTexture(std::string fbxFileName)
{
	auto bricksTex = std::make_unique<Texture>();
	bricksTex->Name = "bricksTex";
	bricksTex->Filename = L"../../Textures/bricks.dds";
	//DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), bricksTex->Filename.c_str(), bricksTex->Resource, bricksTex->UploadHeap);
}

void CResourceManager::Export()
{
}
