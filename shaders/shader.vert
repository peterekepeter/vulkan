#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) out vec3 fragColor;


layout (binding = 0) uniform UniformBufferObject {
	vec2 resolution;
	float time;
	float reserved;
} ubo;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
	vec2 aspectCorrection = vec2(ubo.resolution.y/ubo.resolution.x, 1.0f);
    gl_Position = vec4(positions[gl_VertexIndex] * aspectCorrection, 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}