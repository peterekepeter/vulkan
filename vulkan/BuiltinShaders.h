#pragma once
#include <vector>

class BuiltinShaders
{
public:

	class BinaryData
	{
	public:
		const char* data;
		uint32_t size;
	};

	BuiltinShaders();

	const BinaryData& get_fullscreen_triangle_vert_shader();
	const BinaryData& get_sdr_output_frag_shader();

private:
	HINSTANCE m_h_instance;
	std::vector<std::tuple<int, BinaryData>> m_cache;
	BinaryData load_resource(int id);
	const BinaryData& get_cached_resource(int id);
};