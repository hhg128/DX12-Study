#include "stdafx.h"
#include "Texture.h"



CTexture::CTexture()
{
}


CTexture::~CTexture()
{
}

bool CTexture::LoadFromDDSFile(std::wstring fileName)
{
	m_TextureFileName = fileName;

	return true;
}
