#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 data;

layout(location = 0) out vec4 outColor;

layout (binding = 0) uniform UniformBufferObject {
	vec2 resolution;
	float time;
	float reserved;
} ubo;

void main() {
	vec2 uv = data.xy;
	float pid = data.z;
	////
	float circle = (1-length(uv))*16;
	circle = smoothstep(0, 1, circle);
	float t = ubo.time;
	float amp = pow(1.0-mod(t, 2),4)*0.5+0.5;
	amp*=circle;
	if (amp<0.1)discard;
	float fx = pow(1-mod(t,8)/8, 8);
	vec3 color = vec3(0.5,0.3,0.2)*(fract(sin(sin(pid*39.123)*51.32)*121.21)+0.5);
    outColor = vec4(mix(color, color.yzx, fx)*amp, 1.0);
}