#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform UniformBufferObject {
	vec2 resolution;
	float time;
	float reserved;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;
};

// draw a single triangle which covers the whole screen

vec2 positions[3] = vec2[](
    vec2(-3, -1),
    vec2(+1, -1),
    vec2(+1, +3)
); 

void main() {
	#define REJECT() {gl_Position=vec4(-4,-4,-4,1);return;} 
	int vid = gl_VertexIndex;
	if (vid > 3) REJECT();
	int tid = vid/3; // triangle id
	int pid = tid/2; // polygon id
	int issecondtris = tid%2;
	vec2 uv = positions[vid%3+issecondtris*2];
	gl_Position = vec4(uv, 0.0, 1.0);
}