#pragma once

class CTexture;

class CTextureManager
{
public:
	CTextureManager();
	~CTextureManager();

	std::unordered_map<std::wstring, CTexture*>		m_TextureMap;
};

