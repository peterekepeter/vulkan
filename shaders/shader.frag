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
	float circle = (1-length(uv))*4;
	circle = smoothstep(0, 1, circle);
	float t = ubo.time;
	float amp = 0.1 * data.w;
	amp*=circle;
	if (amp<0.01)discard;
	float fx = pow(1-mod(t,8)/8, 2);
	vec3 color = vec3(0.5,0.1,0.3);//*(fract(sin(sin(pid*39.123)*51.32)*121.21)+0.5);
    outColor = vec4(mix(color, color.yzx, fx)*amp, 1.0);
}