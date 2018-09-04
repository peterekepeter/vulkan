#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout (binding = 0) uniform UniformBufferObject {
	vec2 resolution;
	float time;
	float reserved;
} ubo;

void main() {
	vec2 uv = gl_FragCoord.xy / ubo.resolution.xy;
	float amp = pow(1.0-mod(ubo.time, 1),8)+0.5;
	vec3 offs = vec3(0,0,pow(1-mod(ubo.time,8)/8, 8));
    outColor = vec4(fragColor*amp + offs, 1.0);
}