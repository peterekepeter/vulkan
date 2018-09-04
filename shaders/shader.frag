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
	float t = ubo.time;
	vec2 uv = gl_FragCoord.xy / ubo.resolution.xy;
	float amp = pow(1.0-mod(t, 1),4)+0.5;
	float fx = pow(1-mod(t,8)/8, 8);
    outColor = vec4(mix(fragColor, fragColor.yzx, fx)*amp, 1.0);
}