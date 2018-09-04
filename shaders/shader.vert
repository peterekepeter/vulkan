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
    vec2(0.0, -0.2),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
); 

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.4, 0.6, 0.0),
    vec3(0.2, 0.0, 0.4)
);

vec2 rotate(vec2 v, float a){
	float c=cos(a), s=sin(a);
	return vec2(v.x*c + v.y*s, -v.x*s + v.y*c);
}

void main() {
	int vid = gl_VertexIndex;
	int pid = vid/3;
	vec2 aspectCorrection = vec2(ubo.resolution.y/ubo.resolution.x, 1.0f);
	vec2 pos = positions[gl_VertexIndex%3];
	pos = rotate(pos, ubo.time+pid*0.1)*0.5; 
	pos.x+=pid*0.05 - 1;
	pos.y += (pid%16)*0.1 - 0.5;
	pos = pos * aspectCorrection;
    gl_Position = vec4(pos, 0.0, 1.0);
    fragColor = colors[gl_VertexIndex%3];
}