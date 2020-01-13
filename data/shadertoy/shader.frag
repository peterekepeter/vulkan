#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform UniformBufferObject {
	vec2 resolution;
	float time;
	float reserved;
} ubo;

layout(location = 0) out vec4 outColor;

// SHADERTOY COMPATIBILITY LAYER //////

#define iTime (ubo.time)
#define iResolution (ubo.resolution)

// CONTENT ////////////////////////////

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;

    // Time varying pixel color
    vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));

    // Output to screen
    fragColor = vec4(col,1.0);
}

// MAIN ///////////////////////////////
 
void main() {
	vec2 webgl_uv = vec2(
		gl_FragCoord.x,
		ubo.resolution.y - gl_FragCoord.y
	);
	mainImage(outColor, webgl_uv);
}