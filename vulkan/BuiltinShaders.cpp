#include "pch.h"
#include "BuiltinShaders.h"
#include <Windows.h>
#include "resource.h"

BuiltinShaders::BuiltinShaders()
    : m_h_instance(GetModuleHandle(NULL))
{
}

const BuiltinShaders::BinaryData& BuiltinShaders::get_fullscreen_triangle_vert_shader()
{
    return get_cached_resource(FULLSCREEN_TRIANGLE_VERT_RESOURCE);
}

const BuiltinShaders::BinaryData& BuiltinShaders::get_sdr_output_frag_shader()
{
    return get_cached_resource(SDR_OUTPUT_FRAG_RESOURCE);
}

const BuiltinShaders::BinaryData& BuiltinShaders::get_cached_resource(int id)
{
    using std::get;
    for (auto& entry : m_cache) {
        if (get<0>(entry) == id) {
            return get<1>(entry);
        }
    }
    m_cache.emplace_back(id, load_resource(id));
    return get<1>(m_cache.back());
}

BuiltinShaders::BinaryData BuiltinShaders::load_resource(int id)
{
    HRSRC res = FindResource(m_h_instance, MAKEINTRESOURCE(id), RT_RCDATA);
    if (!res)
        throw std::runtime_error("resource " + std::to_string(id) + " not found");
    HGLOBAL res_handle = LoadResource(NULL, res);
    if (!res_handle)
        throw std::runtime_error("resource " + std::to_string(id) + " failed to load");
    char* res_data = (char*)LockResource(res_handle);
    DWORD res_size = SizeofResource(NULL, res);
    return BinaryData{ res_data, res_size };
}
