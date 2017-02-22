#pragma once

class CTextureManager
{
public:
	CTextureManager();
	~CTextureManager();

	void Initialize();

	void	LoadTextureFromFile(std::wstring fileName);
	int		LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow);

	int		GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);

	std::unordered_map<std::wstring, std::unique_ptr<Texture>>		m_TextureMap;
};

