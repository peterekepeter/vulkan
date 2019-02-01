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

float slash(float x) { return fract(sin(x*37.9137311331)*71.7373137379); }
float flash(float x) { return fract(x*177.79313731); }

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


float osin(float x){ return sin(x*PI/4.0); }
vec2 osin(vec2 x){ return sin(x*PI/4.0); }
vec3 osin(vec3 x){ return sin(x*PI/4.0); }
vec4 osin(vec4 x){ return sin(x*PI/4.0); }

float smootherstep(float edge0, float edge1, float x) {
  // Scale, and clamp x to 0..1 range
  x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
  // Evaluate polynomial
  return x * x * x * (x * (x * 6 - 15) + 10);
}

#define ss smoothstep
#define sss smootherstep

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

part_t mix_part(part_t a, part_t b, float amount){
	part_t y;
	#define mix_prop(prop) y.prop=mix(a.prop,b.prop,amount)
	mix_prop(pos);
	mix_prop(size);
	mix_prop(energy);
	mix_prop(blur);
	#undef mix_prop
	return y;
}

void pf1(int pid, float time, out part_t part){
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
}

void pf1b(int pid, float time, out part_t p){
	p.blur = 0;
	float t=time*.5;
	vec3 pos = vec3(0); 
	pos.x += (pid%1000)*0.01 - 5;
	pos.y += (pid/1000)*0.01 - 3.5;
	
	pos.xy*=-5;
	pos.y-=8;
	pos.z+=8-4*ss(0,12,t)-sss(22,24,t)*64-ss(16,20,t)*4;
	pos.yz = rotate(pos.yz, 0.3+ss(8,16,t)*0.3+ss(16,20,t)*1);
	pos.x*=(pos.z+10)*0.08;
	float ws = 0.1+ss(10,15,t);
	float wa = 0.1+ss(1,19,t);
	float wt=t*2+(pos.x+pos.z)*0.1;
	pos+=sin(pos.yzx*0.2*ws+wt*.9+sin(pos.yzx*0.2+wt))*wa*1.6;
	pos+=cos(pos.zxy*0.4*ws+wt*.96)*wa*cos(pos.zxy*0.5+wt*.94)*0.8;
	pos+=cos(pos.yzx*0.8*ws+wt*.95)*wa*0.4;
	pos+=cos(pos.yzx*1.6*ws+wt*.94)*cos(pos.yzx*3+ws*wt*2)*wa*0.2;
	pos.xz = rotate(pos.xz, sin(time*0.1)*0.1);
	float hash1 = sin(pid*31.119);
	float hash2 = fract(sin(hash1*82.123)*93.241);
	float hash3 = fract(hash2*34.1532);
	float power=pow(hash2,4.0)*32+0.1 + pow(hash3, 400.0)*859;
	if (pid>400000){
		power*=ss(2,8,t*0.8-pid/100000.0);
	}
	pos.z+=3+4*ss(4,8,t);
	float len=length(pos);
	if (len<5.0) {
		pos=pos/len*5.0;
	}
	p.size = abs(pos.z-8-t*0.1)*0.03 + 0.03;
	p.energy=vec3(hash2*0.3+0.7+sin((pos.x+pos.z)*0.05+t*0.2)*0.3,hash2*0.7+0.4+sin(pos.x*0.05)*0.2,0.9+sin(t*0.2)*0.4)*power;
	p.pos = pos;
}

void pf2(int pid, float time, out part_t p){
	float scale = 0.3+sin((pid%884)*311.172)*0.3;
	scale=scale*scale+0.001;
	float blur = 0;
	vec3 pos = vec3(0);
	vec3 color;
	//float ang = pid*3.14159/4 + time*2;
	//pos.xy = vec2(cos(ang), sin(ang));
 
	pos=(fract(sin(vec3(pid)*vec3(0.63116,0.523,0.69312))*51.123)*2-1)*16;
	pos.y+=64+16+64+64*sin(pid);
	pos.y+=(time)*sin(pid%7451*214.125)-8*ss(63,68,time)-64*pow(sin(pid%613476)*.5+.5,4);
	
	pos+=sin(pos);
	pos.z+=16;
	scale*=0.01;
	vec3 c = vec3(0.9, 0.5, 0.6);
	float pif=pid/1000000.0;
	pos.z+=ss(132,140,time+pif*2)*-40;;
	scale+=abs(4+100/(1+time/2)+osin(time*0.25)*2-pos.z)*0.05;
	float fadein=ss(60,71,time);
	float fadein2=ss(80,88,time);
	float fadein3=ss(96,112,time);
	c=mix(c,c.yxz, osin(time*0.37)*fadein);
	c=mix(c,c.zyx, osin(time*0.21)*fadein);
	pos+=sin(pos.yxz*.5)*fadein2*2;
	pos+=sin(pos.yxz*.25)*fadein3*4;
	pos+=cos(pos)*ss(112,120,time);
	color=mix(c,c.zyx,sin(pid*.122+.5))*80.0*(0.01+fadein);
	color.rgb+=sin(vec3(pos.y)*vec3(.03,.02,.04))*8;
	
	pos.y-=time;
	p.pos=pos;
	p.size = scale;
	p.energy = color;
	p.blur = blur;
}


void pf3_0(int pid, float time, out part_t p){
	vec3 pos = vec3(pid%1000 - 500, pid/1000 - 500, 0);
	pos *= 0.001;
	pos.z+=1;
	p.energy = vec3(0.4,0.5,0.9);
	p.pos = pos;
}

void pf3(int pid, float time, out part_t part){
	vec3 pos = vec3(pid%1000 - 500, pid/1000 - 500, 0);
	pos *= 0.001;
	float hash1 = fract(sin(pid*0.3168)*9.125);
	float hash2 = fract(sin(pid%1000)*51.12);
	float hash3 = fract(hash1*95.123 + hash2*17.1234);
	//pos.y += (pow(hash3,10)+0.0001)*pow(max(0,time-10),2);
	float distr = time * pow(hash3,4);
	pos.xy += (cos(pos.yx*vec2(4+distr,8)+4*0.8+time*0.1)+sin(pos.yx*0.7+distr)+vec2(.5,.7))*distr;
	
	pos.z+= 1; 
	part.pos=pos;
	part.size=0.002;
	part.energy = vec3(0.5,0.5,0.5)*0.1;
	part.blur = 0.0;
}

void pf_galaxy(int pid, float time, out part_t part){
	if (pid>200000) return;
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
}

void pf_starfield(int pid, float time, out part_t part, out float t_implode){
	if (pid>400000) return;
	float t=time*0.5;
	float pif = pid/400000.0;
	float hash1 = slash(pif*4.1236);
	float hash2 = slash(hash1);
	float hash3 = slash(hash2*41.2);
	vec3 p = vec3(hash1, hash2, hash3) - .5;
	float hash4 = flash(hash3);
	float hash5 = flash(hash4);
	float hash6 = flash(hash5);
	float clip_far = 8;
	p+=sin(p*32+t*0.1)*0.04*ss(8,12,t);
	p+=sin(p.yzx*16-t*0.1)*0.1*ss(4,8,t);
	p.xy*=mix(1,length(p.xy),ss(6,12,t)*0.8);
	p.xy*=mix(1,(0.8-0.85*sss(20,24,t))/length(p.xy),ss(18,20,t));
	vec2 xy=p.xy;
	p.xy*=mix(vec2(1,1),sin(p.xy*(888))*0.5+1.5,ss(26,28,t)-ss(32,38,t));
	p.z-=smin(time*0.1,time*0.01+ss(2,5,t)*time*time*0.02-6,0.5);
	float z = p.z;
	p.xy=mix(p.xy,xy*2,(sin(z*16+time*2)*.5+.5)*(ss(24,26,t)-ss(28,32,t)));
	
	p.z=fract(p.z);

	//donut

	p.xy*=12;
	p.z*=clip_far;
	float clip_far_value = min((clip_far-p.z)*0.5,1);
	p.xy=rotate(p.xy,p.z*0.05*ss(4,6,t)-1*ss(7,19,t));
	p.xz=rotate(p.xz,p.z*0.2*(ss(4,10,t)-2*ss(6,12,t)+ss(9,13,t)));
	float search_phase = time*0.1;
	float clip=min(p.z,clip_far_value);

	float donutify=sss(30,34,t);
	vec3 p2=p.xyz;
	p2/=vec3(2,2,clip_far*12);
	/*p2.z=sin(z*PI*2)*0.3;
	p2.xy*=cos(z*PI*2)*0.5+2;*/
	//p2.xz=rotate(p2.xz,-0.5);
	float trans=ss(37,39,t);
	p2.xz=rotate(p2.xz+vec2(1,0), 1.5-p.z*donutify+(32-0.8 *time)*ss(40,44,t))-vec2(1,0)*(1-trans);
	// donut shape established
	float implode_steps = hash5+p2.z;
	implode_steps*=2;
	implode_steps-=fract(implode_steps); 

	t_implode = t-49-p2.x+ implode_steps;
	t_implode=max(0,t_implode*0.5);
	p2+=sin(vec3(91.4,141.5,95.5)*hash6)*t_implode*4*hash1;
	
	p2.xy=rotate(p2.xy,(t-34)*0.1);
	p2.yz=rotate(p2.yz,.7*(t-24)*sss(37,39,t));
	p2.xz=rotate(p2.xz,(sin(t*1.5)*0.3+t-35)*ss(43,45,t));
	p2.y+=t_implode*t_implode*4;
	
	p2.z+=1.7*trans;
	p=mix(p,p2,sss(30,34,t));
	//p=p2;
	
	//p.xy += 0.01*p.z*p.z*p.z*sin(vec2(search_phase,search_phase+PI/2))*ss(8,16,time);
	p.z+=2-1.9*ss(0,1,time);

	float implode_flash = max(0,t_implode / (t_implode*t_implode+0.1));
	float fadein = ss(0,1,t)+ss(0,10,t)-ss(16,32,t)*1.6+implode_flash;
	part.size=0.004;
	part.energy=mix(vec3(0.4+sin(z)*0.4,0.2+sin(z*0.07)*0.1,0.1), vec3(0.1,0.3+sin(z*0.34)*0.2,0.4+sin(z*1.37)*0.3), hash4)*fadein*clip*hash5*3;
	part.blur=hash6;
	if (pid>390000){
		part.size*=(1+hash6)*32;
		part.blur=1;
		part.energy.xyz*=16;
	}
	part.pos = p;
}

float df(vec3 p, float time){
	p.z+=time;
	float anim = sin(time*0.1)*0.3+0.6;
	float dist = 1000.0;
	for (int i=0; i<4; i=i+1){
		vec3 p2 = p+vec3(i);
		p.z+=time*0.1;
		float rep = 2+14*(1-anim)+i;
		float ha=rep*0.5;
		vec3 mp=mod(p2,rep)-ha;
		vec3 id=p2-mp;
		vec3 axes = abs(mp)-.6+sin(sin(id+i+p.z*0.))*0.2;
		float box = max(max(axes.x, axes.y), axes.z);
		dist = smin(dist,box,0.5);
	}
	return max(dist, 2-length(mod(p.xy+2.0, 4.0)-2.0));
}

void pfx(int pid, float time, out part_t part){
	//if (pid>500000) return;
	float hash1 = slash(slash(pid/1000)+pid%1000);
	float hash2 = slash(hash1+pid%1000 );
	float hash3 = slash(hash2);
	float ab=pid%2;
	//pos.y += (pow(hash3,10)+0.0001)*pow(max(0,time-10),2);
	vec3 lpos=vec3(sin(time*0.3+ab),cos(time*0.3+ab*4),8+sin(time*0.4-ab)*4);
	vec3 dir=normalize(vec3(hash1-.5, hash2-.5, hash3-.5));
	dir.xy=rotate(dir.xy,time*0.1);dir.zy=rotate(dir.zy,time*0.05);
	vec3 pos = lpos;
	
	float travel = 0;
	for (int i=0; i<20; i++){
		float dist = df(pos,time);
		pos += dist*dir;
		if (dist<0.01) break;
	};
	float occ = 0.1;
	vec3 d2 = -normalize(pos);
	vec3 p2 = pos+d2*occ;
	for (int i=0; i<20; i++)
	{
		float dist = df(p2, time);
		p2 += dist*d2;
		occ = min(dist, occ);
		if (p2.z<0){
			occ = 0.1;
			break;
		}
		if (occ<0.01) break;
	}
	occ *= 10;
	 
	part.pos=pos;
	part.size=0.002+abs (pos.z-8)/pos.z*0.01 + pow(slash(pid*0.0001),16);
	part.energy = mix(vec3(0.8,0.25,0.1), vec3(0.1,0.25,0.4), ab)*16*occ;
	part.blur = 0.0;
}


// snowfall

// rain
 
#define EVAL(fn,time,pid) {fn(pid,time,part1);fn(pid,time+delta,part2);}
#define EVALX(fn,time,pid,x1,x2) {fn(pid,time,part1,x1);fn(pid,time+delta,part2,x2);}
#define EVAL2(fn,time,pid,part1,part2) {fn(pid,time,part1);fn(pid,time+delta,part2);}

void seqencer(int pid, out part_t part1, out part_t part2){
	float time = ubo.time;
	float delta = 0.1;
	part1 = new_part();
	part2 = new_part();
	float implode1 = 1.0;
	float implode2 = 1.0;
	if (time<117 && pid>=400000 && pid<600000){
		EVAL(pf_galaxy, time*0.65, pid-400000);
	}
	if (115.5<time && time<230 && pid<400000){
		EVALX(pf_starfield,time-115.5,pid,implode1,implode2);
		implode1=ss(0,4,implode1);
		implode2=ss(0,4,implode2);
	}
	if (204<time && time<270){
		part_t p1tmp=new_part(), p2tmp=new_part();
		EVAL2(pf1b, time-206, pid,p1tmp,p2tmp);
		part1 = mix_part(part1, p1tmp, implode1);
		part2 = mix_part(part2, p2tmp, implode2);
	}
	if (247<time && time<320){
		part_t p1tmp=new_part(), p2tmp=new_part();
		EVAL2(pf2, (time-251+51)*1.2 ,pid, p1tmp,p2tmp);
		float pidf=pid/1000000.0*9;
		float inter1 = ss(247,255,time-pidf);
		float inter2 = ss(247,255,time+delta-pidf);
		part1 = mix_part(part1, p1tmp, inter1);
		part2 = mix_part(part2, p2tmp, inter2);
	}
	/*if (310<time) {
		//EVAL(pf3, time-260, pid);
		part_t p1tmp=new_part(), p2tmp=new_part();
		EVAL2(pfx, time-310, pid, p1tmp,p2tmp);
		float inter1 = ss(0,-1,part1.pos.z);
		float inter2 = ss(0,-1,part2.pos.z);
		part1 = mix_part(part1, p1tmp, inter1);
		part2 = mix_part(part2, p2tmp, inter2);
	}*/
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