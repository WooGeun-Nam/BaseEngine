#include "Graphics/ShaderManager.h"
#include "Core/ExceptionCOM.h"

bool ShaderManager::Initialize(ID3D11Device* device)
{
    this->device = device;
    return true;
}

Microsoft::WRL::ComPtr<ID3DBlob> ShaderManager::Compile(const std::wstring& file, const std::string& entry, const std::string& target)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

    Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;

    HRESULT hr = D3DCompileFromFile(
        file.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry.c_str(),
        target.c_str(),
        flags,
        0,
        bytecode.GetAddressOf(),
        errors.GetAddressOf()
    );

    COM_ERROR_IF_FAILED(hr, L"Shader compile failed");

    return bytecode;
}

ID3D11VertexShader* ShaderManager::LoadVertexShader(const std::wstring& file, const std::string& entry, ID3DBlob** outBlob)
{
    std::wstring key = file + L"|" + std::wstring(entry.begin(), entry.end());
    auto it = vertexShaderCache.find(key);
    if (it != vertexShaderCache.end())
    {
        if (outBlob)
            vertexShaderBytecodeCache[key].CopyTo(outBlob); // NULL 대신 기존 bytecode를 전달
        return it->second.Get();
    }

    auto bytecode = Compile(file, entry, "vs_5_0");
    if (!bytecode) return nullptr;

    ID3D11VertexShader* vs = nullptr;
    HRESULT hr = device->CreateVertexShader(
        bytecode->GetBufferPointer(),
        bytecode->GetBufferSize(),
        nullptr,
        &vs
    );

    COM_ERROR_IF_FAILED(hr, L"CreateVertexShader failed");

    vertexShaderCache[key] = vs;
    vertexShaderBytecodeCache[key] = bytecode;            // bytecode 캐시 저장
    if (outBlob) bytecode.CopyTo(outBlob);                // 소유권 이전 대신 참조 증가

    return vs;
}

ID3D11PixelShader* ShaderManager::LoadPixelShader(const std::wstring& file, const std::string& entry)
{
    std::wstring key = file + L"|" + std::wstring(entry.begin(), entry.end());
    auto it = pixelShaderCache.find(key);
    if (it != pixelShaderCache.end())
        return it->second.Get();

    auto bytecode = Compile(file, entry, "ps_5_0");
    if (!bytecode) return nullptr;

    ID3D11PixelShader* ps = nullptr;
    HRESULT hr = device->CreatePixelShader(
        bytecode->GetBufferPointer(),
        bytecode->GetBufferSize(),
        nullptr,
        &ps
    );

    COM_ERROR_IF_FAILED(hr, L"CreatePixelShader failed");

    pixelShaderCache[key] = ps;
    return ps;
}
