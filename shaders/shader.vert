#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) out vec4 data;


layout (binding = 0) uniform UniformBufferObject {
	vec2 resolution;
	float time;
	float reserved;
} ubo;

vec2 positions[5] = vec2[](
    vec2(-1, -1),
    vec2(+1, -1),
    vec2(+1, +1),
	vec2(-1, +1),
	vec2(-1, -1)
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
	int tid = vid/3; // triangle id
	int pid = tid/2; // polygon id
	vec2 uv = positions[vid%3+tid%2*2];
	
	vec2 pos = vec2(0); 
	
	pos.x += (pid%1000)*0.005 - 2;
	pos.y += (pid/1000)*0.005 - 2;
	pos+=vec2(cos(pid%743*16.0+ubo.time), sin(pid%1235*0.31+ubo.time))*0.01;
	pos+=sin(pos.yx+ubo.time*0.5)*0.8;
	pos+=cos(pos.yx*4+ubo.time*0.5)*0.1;
	pos+=cos(pos.yx*16+ubo.time*0.5)*0.01;
	
	float size = pow(abs(cos(sin(pid*3.4125)*32.12)),2)*0.01;
	
	//if (pid<400000) size=0;// discard
	
	pos+=uv*size;
	vec2 aspectCorrection = vec2(ubo.resolution.y/ubo.resolution.x, 1.0f);
	pos = pos * aspectCorrection;
    gl_Position = vec4(pos, 0.0, 1.0);
    data.xy = uv;
	data.z = pid;
	data.w = 0; // undefined for now
}