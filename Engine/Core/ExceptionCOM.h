#pragma once
#include <comdef.h>
#include <string>
#include <sstream>
#include <iomanip>

namespace Graphics
{
    class Exception
    {
    public:
        Exception(HRESULT hr, const wchar_t* msg, const wchar_t* file, const wchar_t* function, int line)
        {
            std::wstring error = _com_error(hr).ErrorMessage();

            std::wstringstream ss;
            ss << std::hex // 출력할 숫자를 16진수로 해석
                << std::uppercase // 16진수 출력시 대문자로 표시
                << std::setfill(L'0') // 출력 폭을 맞추기위해 남는 공간을 L'0'으로 채워라
                << std::setw(8) // 출력 최소 폭을 8칸으로 설정
                << hr;

            auto hrHex = ss.str();

            log.append(L"HRESULT: 0x").append(hrHex)
                .append(L"\nMessage: ").append(msg)
                .append(L"\nDescription: ").append(error)
                .append(L"\nFile: ").append(file)
                .append(L"\nFunction: ").append(function)
                .append(L"\nLine: ").append(std::to_wstring(line));
        }

        const std::wstring& what_str() const { return log; }
        const wchar_t* what() const { return log.c_str(); }

    private:
        std::wstring log;
    };

// 매크로용 유니코드 문자열 변환
#define WIDE_STR_STEP2(x) L##x
#define WIDE_STR_STEP1(x) WIDE_STR_STEP2(x)

#define WFILE WIDE_STR_STEP1(__FILE__)
#define WFUNC WIDE_STR_STEP1(__FUNCTION__)

// COM 에러 검사 매크로
#define COM_ERROR_IF_FAILED(hr, msg) \
        if (FAILED(hr)) throw Graphics::Exception(hr, msg, WFILE, WFUNC, __LINE__);
}