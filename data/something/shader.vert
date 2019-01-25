#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) out vec4 data;
layout(location = 1) out vec4 data2;


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

#define PI (355.0/113.0)

vec2 rotate(vec2 v, float a){
	float c=cos(a), s=sin(a);
	return vec2(v.x*c + v.y*s, -v.x*s + v.y*c);
}

// https://www.iquilezles.org/www/articles/smin/smin.htm
float smin( float a, float b, float k )
{
    float h = max( k-abs(a-b), 0.0 )/k;
    return min( a, b ) - h*h*k*(1.0/4.0);
}
float sminCubic( float a, float b, float k )
{
    float h = max( k-abs(a-b), 0.0 )/k;
    return min( a, b ) - h*h*h*k*(1.0/6.0);
}

struct part_t{
	vec3 pos;
	float size;
	vec3 energy;
	float blur;	
};

part_t new_part(){
	part_t part;
	part.size = 0.002;
	part.blur = 0.0;
	part.pos = vec3(-1,-1,-1); // clip out
	part.energy = vec3(0.5,0.5,0.5);
	return part;
}

part_t pf1(int pid, float time){
	part_t part = new_part();
	part.size = 0.01;
	part.blur = 0.2;
	vec3 pos = vec3(0); 
	pos.x += (pid%1000)*0.01 - 5;
	pos.y += (pid/1000)*0.01 - 5;
	pos+=vec3(cos(pid%743*16.0+time) , sin(pid%1235*0.31+time) , sin(pid%125*0.61+time))*0.01;
	pos+=sin(pos.yzx+time*0.5)*0.8;
	pos+=cos(pos.zxy*4+time*0.5)*0.1;
	pos+=cos(pos.yzx*16+time*0.5)*0.01;
	pos+=cos(pos.yzx*64+time*0.15)*0.005;
	pos.z+=2;
	part.pos = pos;
	part.energy = vec3(0.2,0.5,0.9)*2;
	return part;
}

part_t pf1b(int pid, float time){
	part_t p = new_part();
	p.size = 0.01;
	p.blur = 0;
	vec3 pos = vec3(0); 
	pos.x += (pid%1000)*0.01 - 5;
	pos.y += (pid/1000)*0.01 - 5;
	pos.yz = rotate(pos.yz, 1);
	pos+=vec3(cos(pid%743*16.0+time) , sin(pid%1235*0.31+time) , sin(pid%125*0.61+time))*0.01;
	float ws = 0.1;
	pos+=sin(pos.yzx+ws*time*0.5)*0.8;
	pos+=cos(pos.zxy*4+ws*time*0.5)*0.1;
	pos+=cos(pos.yzx*16+ws*time*0.5)*0.01;
	pos+=cos(pos.yzx*64+ws*time*0.15)*0.005;
	pos.xz = rotate(pos.xz, pow(sin(time*0.1), 5.0));
	float hash1 = sin(pid*31.119);
	float hash2 = fract(sin(hash1*82.123)*93.241);
	float hash3 = fract(hash2*34.1532);
	float power=pow(hash2,4.0)+0.1 + pow(hash3, 500.0)*89;
	pos.z+=2;
	p.size = abs(pos.z-2)*0.02  + 0.01;
	p.energy=vec3(hash1,hash2,0.9)*power;
	p.pos = pos;
	return p;
}

float osin(float x){ return sin(x*PI/4.0); }
vec2 osin(vec2 x){ return sin(x*PI/4.0); }
vec3 osin(vec3 x){ return sin(x*PI/4.0); }
vec4 osin(vec4 x){ return sin(x*PI/4.0); }
#define ss smoothstep

part_t pf2(int pid, float time)
{
	part_t p = new_part();
	float scale = 0.3+sin((pid%884)*311.172)*0.3;
	scale=scale*scale+0.001;
	float blur = 0;
	vec3 pos = vec3(0);
	vec3 color;
	//float ang = pid*3.14159/4 + time*2;
	//pos.xy = vec2(cos(ang), sin(ang));
	if (pid<80000)
	{
		
		int spiral = pid/4000;
		float obj=pid%4000;
		obj+=osin(time*0.2+pid)*64;
		obj+=time*64;
		float ang = (time/2+osin(time)/4+(obj/64.0+1));
		pos.xyz = vec3(
			cos(ang)+sin(spiral*227.13352)*2*spiral, 
			(obj/256.0)*8+spiral*2, 
			sin(ang) + spiral
			);
		pos.y=64-abs(pos.y-32);
		pos.y+=96;
		pos.xz*=osin(pos.y/16+3.14159/2)*osin(pos.y/13)*8 ;
		pos.z+=8;
		pos+=sin(pos*2+pid+0.01*time/scale)/16;
		scale+=abs(4+100/(1+time/2)+osin(time*0.25)*2-pos.z)*0.05;
		float amp=0.2+pow(osin(obj/64.0-time)*0.5+.5,2)*255;
		blur=mix(blur,1,smoothstep(56,64,time));
		amp*=smoothstep(66,60,time);
		color=vec3(0.6+sin(spiral*34.12)*0.5, 0.7, 0.5)*1.9*amp;
	}
	else 
	{
		pos=(fract(sin(vec3(pid)*vec3(0.63116,0.523,0.69312))*51.123)*2-1)*16;
		pos.y+=64+16+64+64*sin(pid);
		pos.y+=(time)*sin(pid%7451*214.125)-8*smoothstep(63,68,time)-64*pow(sin(pid%613476)*.5+.5,4);
		pos+=sin(pos);
		pos.z+=16;
		scale*=0.01;
		vec3 c = vec3(0.9, 0.6, 0.5);
		scale+=abs(4+100/(1+time/2)+osin(time*0.25)*2-pos.z)*0.05;
		float fadein=smoothstep(64,72,time);
		float fadein2=smoothstep(72,80,time);
		float fadein3=smoothstep(88,96,time);
		c=mix(c,c.yxz, osin(time/2)*fadein);
		c=mix(c,c.zyx, osin(time/4)*fadein);
		pos+=sin(pos.yxz*.5)*fadein2*2;
		pos+=sin(pos.yxz*.25)*fadein3*4;
		color=mix(c,c.zyx,sin(pid*.125+.5))*100.0*(0.01+fadein);
	}
	pos.y-=time;
	p.pos=pos;
	p.size = scale;
	p.energy = color;
	p.blur = blur;
	return p;
}


part_t pf3_0(int pid, float time){
	part_t p = new_part();
	vec3 pos = vec3(pid%1000 - 500, pid/1000 - 500, 0);
	pos *= 0.001;
	pos.z+=1;
	p.energy = vec3(0.4,0.5,0.9);
	p.pos = pos;
	return p;
}

part_t pf3(int pid, float time){
	part_t part = new_part();
	vec3 pos = vec3(pid%1000 - 500, pid/1000 - 500, 0);
	pos *= 0.001;
	float hash1 = fract(sin(pid*0.3168)*9.125);
	float hash2 = fract(sin(pid%1000)*51.12);
	float hash3 = fract(hash1*95.123 + hash2*17.1234);
	//pos.y += (pow(hash3,10)+0.0001)*pow(max(0,time-10),2);
	float destr_rep = mod(time,8.0f);
	float destr_id = time - destr_rep;
	float distr = max(0,mod(time,8.0f)-2.0f) * pow(hash3,4);
	pos.xy += (cos(pos.yx*vec2(4+distr,8)+4*0.8+time*0.1)+sin(pos.yx*0.7+distr)+vec2(cos(destr_id),sin(destr_id)))*distr;
	
	pos.z+= 1; 
	part.pos=pos;
	part.size=0.002;
	part.energy = vec3(0.5,0.5,0.5)*0.1;
	part.blur = 0.0;
	return part;
}

// space
part_t pf4(int pid, float time){
	part_t part = new_part();
	if (pid>200000) return part;
	vec3 p = vec3(0,0,0);
	float pif = pid/20000.0;
	float t = time*.5;
	float hash = fract(sin(pif*651.319)*117.17);
	float hash2 = fract(sin(hash*921.12)*51.123+sin(pid*15.1356));
	float hash3 = fract(sin(hash2*132.315)*531.123);
	float hash4 = fract(sin(hash3*332.315)*131.123);
	float hash5 = fract(hash4*141.123);
	float hash6 = fract(hash5*141.123);
	float stretch = (mix(0.5,pow(hash,2)*8,ss(2,5,t)));
	float eye = mix(0, 0.1, ss(10,11,t));
	float radius = stretch + 0.1;
	float phaset = smin((t-36.5)*0.1+36.5,t, 1);
	float phase = mix(t,phaset/radius*0.1/radius,ss(4,6,t));
	//p.z+=hash3;
	float size = ss(20,21,t)*pow(hash3,6)*0.35+0.001;
	phase += ss(12,14,t)*hash2*PI*2;
	p.xy=sin(vec2(phase+PI/2,phase))*(stretch+eye);

	p.xz = rotate(p.xz,0.5*(hash5-.2)*ss(18,27,t));
	p.yz = rotate(p.yz,0.4*(hash6-.6)*ss(25,29,t));
	
	p.z+=(0.5-4*hash)*ss(34,36,t);//dolly zoom
	p.z-=pow(ss(37,38,t),4);//warp
	p.yz=rotate(p.yz, ss(16,18,t)-ss(30,36,t));
	float dof = abs(p.z+0.2)*0.04+0.003;
	part.size=size;
	vec3 color = mix(vec3(0.4+p.x*0.5,0.2,0.1), vec3(0.1,0.2,0.4), hash);
	float energy = pow(hash4,2)*2;
	p.z-=ss(28,32,t)*0.5;
	part.energy=mix(vec3(0.1,0.1,0.1), color*energy, ss(24,25,t));
	p.z+=1;
	part.pos = p;
	return part;
}

// tunnel


// snowfall

// rain

void seqencer(int pid, out part_t part1, out part_t part2){
	float time = ubo.time;
	float delta = 0.1;
	#define EVAL(fn) {part1=fn(pid,time);part2=fn(pid,time+delta);}
	EVAL(pf4);
}


void main() {
	#define REJECT() {gl_Position=vec4(-4,-4,-4,1);return;}
	int vid = gl_VertexIndex;
	int tid = vid/3; // triangle id
	int pid = tid/2; // polygon id
	int issecondtris = tid%2;
	vec2 uv = positions[vid%3+issecondtris*2];
	
	vec3 col_res1, col_res2;
	float time = ubo.time;
	
	part_t part1;
	part_t part2;
	seqencer(pid, part1, part2);

	float near_limit = 0.0;
	if(part1.pos.z<near_limit||part2.pos.z<near_limit) { REJECT(); }
	
	float size1 = part1.size/part1.pos.z;
	float size2 = part2.size/part2.pos.z;

	float size_min_threshold = 4.0/ubo.resolution.y;

	if (size1<size_min_threshold){
		part1.size*=size_min_threshold/size1;
		size1=size_min_threshold;
	}
	if (size2<size_min_threshold){
		part2.size*=size_min_threshold/size2;
		size2=size_min_threshold;
	}
	
	float e2c1 = part1.size*part1.size*6000;
	float e2c2 = part2.size*part2.size*6000;
	col_res1 = part1.energy / e2c1;
	col_res2 = part2.energy / e2c2;
	
	col_res1 = max(col_res1,0);
	col_res2 = max(col_res2,0);

	if(dot(col_res1,col_res2)<0.00001){ REJECT(); }

	float blur = (part1.blur + part2.blur)*.5;
	
	// calculate shape
	// project to 2d
	vec2 pos_2d = part1.pos.xy/part1.pos.z;
	vec2 pos2_2d = part2.pos.xy/part2.pos.z;
	vec2 diff_2d = pos2_2d.xy-pos_2d.xy;
	float angle = atan(diff_2d.x, diff_2d.y) + 3.14159/2;
	float len = length(diff_2d);
	vec2 displace = rotate(uv, angle);
	float amp1 = size1/(size1+len);
	float amp2 = size2/(size2+len);
	col_res1.xyz*=amp1;
	col_res2.xyz*=amp2;
	//float size = pow(abs(cos(sin(pid*3.4125)*32.12)),2)*0.02;
	//if (pid>7) size=0; // discard
	

	vec2 aspectCorrection = vec2(ubo.resolution.y/ubo.resolution.x, 1.0f);
	// blur optimization
	float blur_optimization = (1+blur*blur)*0.5+1/ubo.resolution.y;
	displace*=blur_optimization;
	uv*=blur_optimization;
	
	float mixfac = -uv.x*.5+.5;
    gl_Position = vec4((mix(pos_2d+displace*size1, pos2_2d+displace*size2, mixfac))*aspectCorrection, 0.5, 1);
    data.xy = uv;
	data.z = pid;
	data.w = mix(size1, size2, mixfac); // undefined for now
	// color
	data2=vec4(mix(col_res1, col_res2, mixfac),blur);
}