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

vec2 rotate(vec2 v, float a){
	float c=cos(a), s=sin(a);
	return vec2(v.x*c + v.y*s, -v.x*s + v.y*c);
}

vec3 pf1(int pid, float time)
{
	vec3 pos = vec3(0); 
	pos.x += (pid%1000)*0.01 - 5;
	pos.y += (pid/1000)*0.01 - 5;
	pos+=vec3(cos(pid%743*16.0+time) , sin(pid%1235*0.31+time) , sin(pid%125*0.61+time))*0.01;
	pos+=sin(pos.yzx+time*0.5)*0.8;
	pos+=cos(pos.zxy*4+time*0.5)*0.1;
	pos+=cos(pos.yzx*16+time*0.5)*0.01;
	pos+=cos(pos.yzx*64+time*0.15)*0.005;
	pos.z+=2;
	return pos;
}

vec3 pf2(int pid, float time)
{
	vec3 pos = vec3(0);
	//float ang = pid*3.14159/4 + time*2;
	//pos.xy = vec2(cos(ang), sin(ang));
	float ang = time/2*(pid+1);
	pos.xyz = vec3(
		cos(ang), 
		(pid*0.5-2)*1.5, 
		sin(ang)
		);
	pos.z+=4;
	return pos;
}

void main() {
	int vid = gl_VertexIndex;
	int tid = vid/3; // triangle id
	int pid = tid/2; // polygon id
	int issecondtris = tid%2;
	vec2 uv = positions[vid%3+issecondtris*2];
	
	//vec3 pos=pf1(pid, ubo.time);
	
	vec3 pos = pf2(pid, ubo.time);
	vec3 pos2 = pf2(pid, ubo.time+0.1);
	
	float scale = 0.1;
	float size = scale/pos.z;
	float size2 = scale/pos2.z;
	
	// calculate shape
	// project to 2d
	vec2 pos_2d = pos.xy/pos.z;
	vec2 pos2_2d = pos2.xy/pos2.z;
	vec2 diff_2d = pos2_2d.xy-pos_2d.xy;
	float angle = atan(diff_2d.x, diff_2d.y) + 3.14159/2;
	float len = length(diff_2d);
	vec2 displace = rotate(uv, angle);
	float amp = size/(size+len);
	amp*=100;
	
	//float size = pow(abs(cos(sin(pid*3.4125)*32.12)),2)*0.02;
	//if (pid>7) size=0; // discard
	

	vec2 aspectCorrection = vec2(ubo.resolution.y/ubo.resolution.x, 1.0f);
	
    gl_Position = vec4((mix(pos_2d+displace*size, pos2_2d+displace*size2, -uv.x*.5+.5))*aspectCorrection, 0.5, 1);
    data.xy = uv;
	data.z = pid;
	data.w = amp; // undefined for now
}