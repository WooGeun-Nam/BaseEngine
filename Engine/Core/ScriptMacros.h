#pragma once
#include "Component.h"
#include <string>

// Type definition for RegisterScript function pointer
typedef void (*RegisterScriptFunc)(const std::string& className, Component* (*factory)());

// Global pointer to the RegisterScript function (set by engine at DLL load time)
#ifdef SCRIPTS_EXPORTS
    // In Scripts.dll: declare as external that will be initialized by engine
    extern RegisterScriptFunc g_RegisterScript;
#endif

// Macro to register a script component (use outside class definition)
// This creates a factory function and a registration function
#define REGISTER_SCRIPT(ClassName) \
    namespace { \
        Component* Create##ClassName() { return new ClassName(); } \
        struct Register##ClassName##Helper { \
            Register##ClassName##Helper() {} \
            void Register(RegisterScriptFunc registerFunc) { \
                if (registerFunc) registerFunc(#ClassName, &Create##ClassName); \
            } \
        }; \
        Register##ClassName##Helper g_##ClassName##Helper; \
    } \
    extern "C" __declspec(dllexport) void Register##ClassName(RegisterScriptFunc registerFunc) { \
        g_##ClassName##Helper.Register(registerFunc); \
    }
