#include "stdafx.h"
#include "systemclass.h"
#include "ResourceManager.h"
#include "TextureManager.h"
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

	// 지금은 필요없다.
	//ConvertAxisSystem(pScene);

	ModelClass* modelClass = new ModelClass;
	m_ModelMap[fbxFileName] = modelClass;

	// texture 구하기
	ReadTextureInfo(pScene, modelClass);

	// Mesh 구하기
	const int nNodeCount = pScene->GetSrcObjectCount<FbxNode>();
	for (int nNodeIndex = 0; nNodeIndex < nNodeCount; nNodeIndex++)
	{
		FbxNode* pNode = pScene->GetSrcObject<FbxNode>(nNodeIndex);
		if (pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			FbxMesh* pMesh = pNode->GetMesh();
			unsigned int nControlPointCount = pMesh->GetControlPointsCount();
			int nTriangleCount = pMesh->GetPolygonCount();

			MeshClass* meshClass = new MeshClass;
			meshClass->m_pModel = modelClass;

			ReadMatrix(pNode, meshClass);
			ReadVertex(pMesh, nControlPointCount, meshClass);
			ReadIndex(pMesh, nTriangleCount, meshClass);
			ReadUV(pMesh, nTriangleCount, meshClass);

			ReadTextureId(pMesh, meshClass);

			ReadNormal(pMesh, nControlPointCount, meshClass);

			m_ModelMap[fbxFileName]->m_MeshArray.push_back(meshClass);
		}
	}

	modelClass->LoadTextures();
}

void CResourceManager::ReadTextureInfo(FbxScene* pScene, ModelClass* modelClass)
{
	const int nTextureCount = pScene->GetTextureCount();
	for (int i = 0; i < nTextureCount; ++i)
	{
		FbxTexture* pTexture = pScene->GetTexture(i);
		FbxFileTexture* pFileTexture = FbxCast<FbxFileTexture>(pTexture);
		std::string textureFileName = pFileTexture->GetFileName();
		std::string textureInternalName = pFileTexture->GetName();
		int64_t uId = pFileTexture->GetUniqueID();

		size_t pos = textureFileName.rfind("\\");
		textureFileName = textureFileName.substr(pos + 1);

		auto Tex = std::make_unique<Texture>();
		StringHelper::ConvertStringToWString(textureFileName, Tex->Filename);
		Tex->Name = textureInternalName;

		modelClass->m_TextureMap[i] = Tex->Filename;
		modelClass->m_TextureIndexMap[Tex->Filename] = i;

		// 텍스처 매니저에 등록한다.
		gSystem->TextureManager->m_TextureMap[Tex->Filename] = std::move(Tex);
	}
}

void CResourceManager::ReadTextureId(FbxMesh* pMesh, MeshClass* meshClass)
{
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
						FbxFileTexture* pFileTexture = FbxCast<FbxFileTexture>(pTexture);
						std::string textureFileName = pFileTexture->GetFileName();
						std::string textureInternalName = pFileTexture->GetName();
						int64_t uId = pFileTexture->GetUniqueID();

						size_t pos = textureFileName.rfind("\\");
						textureFileName = textureFileName.substr(pos + 1);

						std::wstring texWFileName;
						StringHelper::ConvertStringToWString(textureFileName, texWFileName);

						int texIndex = meshClass->m_pModel->m_TextureIndexMap[texWFileName];

						meshClass->m_TexterIdArray.push_back(uId);
						meshClass->m_textureIndex = texIndex;
					}
				}
			}
		}
	}
}

void CResourceManager::ReadMatrix(FbxNode* pNode, MeshClass* meshClass)
{
	FbxAMatrix& lGlobalTransform = pNode->EvaluateGlobalTransform();
	FbxAMatrix& lLocalTransform = pNode->EvaluateLocalTransform();

	XMFLOAT4X4 mat;
	mat._11 = static_cast<float>(lGlobalTransform.mData[0].mData[0]);
	mat._12 = static_cast<float>(lGlobalTransform.mData[0].mData[2]);
	mat._13 = static_cast<float>(lGlobalTransform.mData[0].mData[1]);
	mat._14 = static_cast<float>(lGlobalTransform.mData[0].mData[3]);

	mat._21 = static_cast<float>(lGlobalTransform.mData[2].mData[0]);
	mat._22 = static_cast<float>(lGlobalTransform.mData[2].mData[2]);
	mat._23 = static_cast<float>(lGlobalTransform.mData[2].mData[1]);
	mat._24 = static_cast<float>(lGlobalTransform.mData[2].mData[3]);

	mat._31 = static_cast<float>(lGlobalTransform.mData[1].mData[0]);
	mat._32 = static_cast<float>(lGlobalTransform.mData[1].mData[2]);
	mat._33 = static_cast<float>(lGlobalTransform.mData[1].mData[1]);
	mat._34 = static_cast<float>(lGlobalTransform.mData[1].mData[3]);

	mat._41 = static_cast<float>(lGlobalTransform.mData[3].mData[0]);
	mat._42 = static_cast<float>(lGlobalTransform.mData[3].mData[2]);
	mat._43 = static_cast<float>(lGlobalTransform.mData[3].mData[1]);
	mat._44 = static_cast<float>(lGlobalTransform.mData[3].mData[3]);

	meshClass->m_mat = mat;
}

void CResourceManager::ReadVertex(FbxMesh* pMesh, size_t nControlPointCount, MeshClass* meshClass)
{
	// Vertex List
	for (size_t i = 0; i < nControlPointCount; ++i)
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
}

void CResourceManager::ReadIndex(FbxMesh* pMesh, size_t nTriangleCount, MeshClass* meshClass)
{
	// Index List
	for (size_t i = 0; i < nTriangleCount; ++i)
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
}

void CResourceManager::ReadTexture()
{

}

void CResourceManager::ReadNormal(FbxMesh* pMesh, size_t nControlPointCount, MeshClass* meshClass)
{
	if (pMesh->GetElementNormalCount() < 1)
	{
		throw std::exception("Invalid Normal Number");

		StringHelper::OutputDebugString("Invalid Normal Number");
	}

	FbxGeometryElementNormal* vertexNormal = pMesh->GetElementNormal(0);
	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		ReadNormalPerControlPoint(pMesh, nControlPointCount, meshClass);
		break;
	case FbxGeometryElement::eByPolygonVertex:
		ReadNormalPerPolygonVertex(pMesh, nControlPointCount, meshClass);
		break;
	}
}

void CResourceManager::ReadNormalPerControlPoint(FbxMesh* pMesh, int nControlPointCount, MeshClass* meshClass)
{
	FbxGeometryElementNormal* vertexNormal = pMesh->GetElementNormal(0);
	int nFaceCount = pMesh->GetPolygonCount();

	for (size_t i = 0; i < nFaceCount; ++i)
	{
		for (size_t j = 0; j < 3; ++j)
		{
			int ctrlPointIndex = pMesh->GetPolygonVertex(i, j);
			MeshClass::VertexType& vertex = meshClass->m_VertexArray[ctrlPointIndex];

			XMFLOAT3 outNormal;
			switch (vertexNormal->GetMappingMode())
			{
			case FbxGeometryElement::eByControlPoint:
				switch (vertexNormal->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				{
					outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
					outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[2]);
					outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
				}
				break;

				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexNormal->GetIndexArray().GetAt(ctrlPointIndex);
					outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
					outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
					outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
				}
				break;

				default:
					throw std::exception("Invalid Reference");
				}
				break;
			}

			vertex.Normal = outNormal;
		}
	}	
}

void CResourceManager::ReadNormalPerPolygonVertex(FbxMesh* pMesh, int nControlPointCount, MeshClass* meshClass)
{
	FbxGeometryElementNormal* vertexNormal = pMesh->GetElementNormal(0);
	int nFaceCount = pMesh->GetPolygonCount();

	for (size_t i = 0; i < nFaceCount; ++i)
	{
		for (size_t j = 0; j < 3; ++j)
		{
			int ctrlPointIndex = pMesh->GetPolygonVertex(i, j);
			MeshClass::VertexType& vertex = meshClass->m_VertexArray[ctrlPointIndex];
			
			XMFLOAT3 outNormal;
			switch (vertexNormal->GetMappingMode())
			{
			case FbxGeometryElement::eByPolygonVertex:
				switch (vertexNormal->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				{
					outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
					outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[2]);
					outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
				}
				break;

				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexNormal->GetIndexArray().GetAt(ctrlPointIndex);
					outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
					outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
					outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
				}
				break;

				default:
					throw std::exception("Invalid Reference");
				}
				break;
			}

			vertex.Normal = outNormal;
		}
	}
}

void CResourceManager::ReadUV(FbxMesh* pMesh, size_t nTriangleCount, MeshClass* meshClass)
{
	// UV
	for (size_t i = 0; i < nTriangleCount; ++i)
	{
		for (size_t j = 0; j < 3; ++j)
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
}

void CResourceManager::ConvertAxisSystem(FbxScene* pScene)
{
	// Convert Axis System to what is used in this example, if needed
	FbxAxisSystem SceneAxisSystem = pScene->GetGlobalSettings().GetAxisSystem();
	FbxAxisSystem OurAxisSystem(FbxAxisSystem::DirectX);
	if (SceneAxisSystem != OurAxisSystem)
	{
		OurAxisSystem.ConvertScene(pScene);
	}
}
