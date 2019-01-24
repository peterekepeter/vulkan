#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 data;
layout(location = 1) in vec4 data2; 

layout(location = 0) out vec4 outColor;

layout (binding = 0) uniform UniformBufferObject {
	vec2 resolution;
	float time;
	float reserved;
} ubo;
 
void main() {
	vec2 uv_screen = gl_FragCoord.xy/ubo.resolution.xy;
	vec2 uv = data.xy;
	float pid = data.z;
	float size = data.w;
	////
	float blur = data2.w;
	float lengthuv = length(uv);
	float q=mix(size*ubo.resolution.y*0.5,1,blur);
	float circle = (1-lengthuv-0.5)*q+0.5;
	circle = smoothstep(0, 1, circle);
	float t = ubo.time;
	vec3 amp = 1 * data2.xyz;
	amp*=circle;
	if (dot(amp,vec3(1))<0.01)discard;
	vec3 color = amp;
	color *= 1-length(uv_screen.xy-0.5)*1.4;
	//color.xyz*=mix(vec3(0.2,0.3,0.9), vec3(0.9,0.3,0.2), uv_screen.y);
	color.xyz+=color*fract(mod(ubo.time,1024)*71.312+mod(pid,512)*42.1238+121.831*sin(uv_screen.yxy*vec3(5379.71,2512.31,7321.137)*uv_screen.xyx*vec3(1374.21,4512.3,9321.23)));
	color=color/(1+color);
    outColor = vec4(color, 1);
}