#version 450
#extension GL_ARB_separate_shader_objects : enable

// outputs rendering to 8-bit sRGB
// assumes output is R8G8B8A8_UINT

layout (set = 0, binding = 0) uniform UniformBufferObject {
	float image_count;
} ubo;

// assumes unnormalized sampler coords
layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, gl_FragCoord.xy);
}