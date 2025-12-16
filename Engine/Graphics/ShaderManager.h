#pragma once

#include <string>
#include <map>
#include <wrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>

class ShaderManager
{
public:
    bool Initialize(ID3D11Device* device);

    ID3D11VertexShader* LoadVertexShader(const std::wstring& file, const std::string& entry, ID3DBlob** outBlob = nullptr);
    ID3D11PixelShader* LoadPixelShader(const std::wstring& file, const std::string& entry);

private:
    Microsoft::WRL::ComPtr<ID3DBlob> Compile(const std::wstring& file, const std::string& entry, const std::string& target);

private:
    ID3D11Device* device = nullptr;

    std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11VertexShader>> vertexShaderCache;
    std::map<std::wstring, Microsoft::WRL::ComPtr<ID3DBlob>> vertexShaderBytecodeCache;
    std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11PixelShader>>  pixelShaderCache;
};
