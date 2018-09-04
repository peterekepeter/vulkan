#version 450
#extension GL_ARB_separate_shader_objects : enable

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
	
	vec3 pos = vec3(0); 
	
	pos.x += (pid%1000)*0.01 - 5;
	pos.y += (pid/1000)*0.01 - 5;
	pos+=vec3(cos(pid%743*16.0+ubo.time) , sin(pid%1235*0.31+ubo.time) , sin(pid%125*0.61+ubo.time))*0.01;
	pos+=sin(pos.yzx+ubo.time*0.5)*0.8;
	pos+=cos(pos.zxy*4+ubo.time*0.5)*0.1;
	pos+=cos(pos.yzx*16+ubo.time*0.5)*0.01;
	pos+=cos(pos.yzx*64+ubo.time*0.15)*0.005;
	pos.z+=2;
	
	float size = pow(abs(cos(sin(pid*3.4125)*32.12)),2)*0.01;
	
	//if (pid<400000) size=0;// discard
	

	vec2 aspectCorrection = vec2(ubo.resolution.y/ubo.resolution.x, 1.0f);
	
	pos = (pos + vec3(uv*0.01,0.0))*aspectCorrection.xyy;
    gl_Position = vec4(pos, pos.z);
    data.xy = uv;
	data.z = pid;
	data.w = 0; // undefined for now
}