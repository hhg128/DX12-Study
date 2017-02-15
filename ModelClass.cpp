#include "stdafx.h"
#include "ModelClass.h"


MeshClass::MeshClass()
{

}

MeshClass::~MeshClass()
{

}

//////////////////////////////////////////////////////////////////////////

ModelClass::ModelClass()
{
}


ModelClass::~ModelClass()
{
}

void ModelClass::LoadTextures()
{
	for (auto& texture : m_TextureMap)
	{
		std::string textureFileName;
		StringHelper::ConvertWStringToString(texture.second->Filename, textureFileName);


	}
}
