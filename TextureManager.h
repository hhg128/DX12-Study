#pragma once

class CTextureManager
{
public:
	CTextureManager();
	~CTextureManager();

	void Initialize();

	std::unordered_map<std::wstring, std::unique_ptr<Texture>>		m_TextureMap;
};

