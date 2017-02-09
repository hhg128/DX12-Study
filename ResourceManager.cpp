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
	//FbxAxisSystem SceneAxisSystem = pScene->GetGlobalSettings().GetAxisSystem();
	//FbxAxisSystem OurAxisSystem(FbxAxisSystem::DirectX);
	//if (SceneAxisSystem != OurAxisSystem)
	//{
	//	OurAxisSystem.ConvertScene(pScene);
	//}


	ModelClass* modelClass = new ModelClass;
	m_ModelMap[fbxFileName] = modelClass;

	// texture 구하기
	const int nTextureCount = pScene->GetTextureCount();
	for (int i = 0; i < nTextureCount; ++i)
	{
		FbxTexture* pTexture = pScene->GetTexture(i);
		FbxFileTexture* pFileTexture = FbxCast<FbxFileTexture>(pTexture);
		std::string textureFileName = pFileTexture->GetFileName();
		std::string textureInternalName = pFileTexture->GetName();

		size_t pos = textureFileName.rfind("\\");
		textureFileName = textureFileName.substr(pos+1);

		modelClass->m_TextureMap[textureInternalName] = textureFileName;
	}

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

			XMFLOAT3 meshPos;// = XMFLOAT3(-pos.mData[0], pos.mData[1], pos.mData[2]);
			meshPos.x = static_cast<float>(pos.mData[0]);
			meshPos.y = static_cast<float>(pos.mData[1]);
			meshPos.z = static_cast<float>(pos.mData[2]);
			XMFLOAT3 meshRot = XMFLOAT3(rot.mData[0], rot.mData[1], rot.mData[2]);
			XMFLOAT3 meshScale = XMFLOAT3(scale.mData[0], scale.mData[1], scale.mData[2]);
			meshClass->m_vPos = meshPos;
			meshClass->m_vRot = meshRot;
			meshClass->m_vScale = meshScale;

			FbxAMatrix& lGlobalTransform = pNode->EvaluateGlobalTransform();
			FbxAMatrix& lLocalTransform = pNode->EvaluateLocalTransform();

			XMFLOAT4X4 mat;
			mat._11 = lGlobalTransform.mData[0].mData[0];
			mat._12 = lGlobalTransform.mData[0].mData[1];
			mat._13 = lGlobalTransform.mData[0].mData[2];
			mat._14 = lGlobalTransform.mData[0].mData[3];
			
			mat._21 = lGlobalTransform.mData[2].mData[0];
			mat._22 = lGlobalTransform.mData[2].mData[1];
			mat._23 = lGlobalTransform.mData[2].mData[2];
			mat._24 = lGlobalTransform.mData[2].mData[3];
			
			mat._31 = lGlobalTransform.mData[1].mData[0];
			mat._32 = lGlobalTransform.mData[1].mData[1];
			mat._33 = lGlobalTransform.mData[1].mData[2];
			mat._34 = lGlobalTransform.mData[1].mData[3];
			
			mat._41 = lGlobalTransform.mData[3].mData[0];
			mat._42 = lGlobalTransform.mData[3].mData[1];
			mat._43 = lGlobalTransform.mData[3].mData[2];
			mat._44 = lGlobalTransform.mData[3].mData[3];
			
			meshClass->m_mat = mat;

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

			// UV
			for (int i = 0; i < nTriangleCount; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					int ctrlPointIndex = pMesh->GetPolygonVertex(i, j);
					
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
								outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
								outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
							}
							break;

							case FbxGeometryElement::eIndexToDirect:
							{
								int index = vertexUV->GetIndexArray().GetAt(ctrlPointIndex);
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

					
					MeshClass::VertexType& vertex = meshClass->m_VertexArray[ctrlPointIndex];
					vertex.UV.x = outUV.x;
					vertex.UV.y = 1.f - outUV.y;
				}
			}

			// texture id
			for (int i = 0; i < pMesh->GetElementMaterialCount(); ++i)
			{
				FbxGeometryElementMaterial* pMaterialElement = pMesh->GetElementMaterial(i);
				if (pMaterialElement->GetMappingMode() == FbxGeometryElement::eAllSame)
				{
					FbxSurfaceMaterial* pMaterial = pMesh->GetNode()->GetMaterial(pMaterialElement->GetIndexArray().GetAt(0));
					int nMatId = pMaterialElement->GetIndexArray().GetAt(0);
					if (nMatId >= 0)
					{
						// diffuse 텍스처만 사용한다. 일단...
						FbxProperty Property;
						Property = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

						int nTextureCount = Property.GetSrcObjectCount<FbxTexture>();
						for (int textureIndex = 0; textureIndex < nTextureCount; ++textureIndex)
						{
							FbxTexture* pTexture = Property.GetSrcObject<FbxTexture>(textureIndex);
							if (pTexture)
							{
								//std::string textureInternalName = pTexture->GetName();

								FbxFileTexture* pFileTexture = FbxCast<FbxFileTexture>(pTexture);
								std::string textureFileName = pFileTexture->GetFileName();
								std::string textureInternalName = pFileTexture->GetName();

								meshClass->m_TexterIdNameArray.push_back(textureInternalName);
							}
						}
					}
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
