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

vec4 pf1(int pid, float time, out vec4 color)
{
	float scale = 0.01;
	float blur = 0.2;
	vec3 pos = vec3(0); 
	pos.x += (pid%1000)*0.01 - 5;
	pos.y += (pid/1000)*0.01 - 5;
	pos+=vec3(cos(pid%743*16.0+time) , sin(pid%1235*0.31+time) , sin(pid%125*0.61+time))*0.01;
	pos+=sin(pos.yzx+time*0.5)*0.8;
	pos+=cos(pos.zxy*4+time*0.5)*0.1;
	pos+=cos(pos.yzx*16+time*0.5)*0.01;
	pos+=cos(pos.yzx*64+time*0.15)*0.005;
	pos.z+=2;
	color=vec4(vec3(0.2,0.5,0.9)*2, blur);
	return vec4(pos, scale);
}


float osin(float x){ return sin(x*PI/4.0); }
vec2 osin(vec2 x){ return sin(x*PI/4.0); }
vec3 osin(vec3 x){ return sin(x*PI/4.0); }
vec4 osin(vec4 x){ return sin(x*PI/4.0); }

vec4 pf2(int pid, float time, out vec4 color)
{
	float scale = 0.3+sin((pid%884)*311.172)*0.3;
	scale=scale*scale+0.001;
	float blur = min(1,scale*5);
	vec3 pos = vec3(0);
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
		float amp=0.2+pow(osin(obj/64.0-time)*0.5+.5,2);
		amp/=scale*scale*20;
		blur=mix(blur,1,smoothstep(56,64,time));
		amp*=smoothstep(66,60,time);
		color=vec4((vec3(0.6+sin(spiral*34.12)*0.5, 0.7, 0.5))*1.9*amp, blur);
	}
	else 
	{
		pos=(fract(sin(vec3(pid)*vec3(0.63116,0.523,0.69312))*51.123)*2-1)*16;
		pos.y+=64+16+64+64*sin(pid);
		pos.y+=(time)*sin(pid%7451*214.125)-8*smoothstep(63,68,time)-64*pow(sin(pid%613476)*.5+.5,4);
		pos+=sin(pos);
		pos.z+=16;
		scale*=0.01;
		vec3 c = vec3(0.9, 0.6, 0.5)*0.03 ;
		scale+=abs(4+100/(1+time/2)+osin(time*0.25)*2-pos.z)*0.05;
		float fadein=smoothstep(64,72,time);
		float fadein2=smoothstep(72,80,time);
		float fadein3=smoothstep(88,96,time);
		c=mix(c,c.yxz, osin(time/2)*fadein);
		c=mix(c,c.zyx, osin(time/4)*fadein);
		pos+=sin(pos.yxz*.5)*fadein2*2;
		pos+=sin(pos.yxz*.25)*fadein3*4;
		color=vec4(mix(c,c.zyx,sin(pid*.125+.5))/pow(scale,1.9)*0.6*(0.01+fadein), blur);
	}
	pos.y-=time;
	return vec4(pos, scale);
}

float df_obelisk(vec3 p)
{
	vec2 location = vec2(100,0);
	vec2 walls = abs(p.xz-location);
	return max(max(walls.x, walls.y)-3.0-p.y*0.1, -22-p.y);
}

float df(vec3 p, float time){
	vec3 pm1 = p; 	pm1.xz= mod(p.xz+2,4)-2;
	vec3 pm2 = p; 	pm2.xz= mod(rotate(p.xz,0.4)+1.4,2.8)-1.4;
	vec3 pm4 = p; 	pm4.xz= mod(rotate(p.xz,1.2),16)-8;
	float sphere = length(pm1)-1.2;
	sphere=min(sphere, length(pm2)-1);
	sphere=min(sphere, length(pm4)-5);
	vec3 pm3 = p; 	pm3.xz= mod(rotate(p.xz,0.7)+0.9,1.8)-0.9;
	sphere = max(sphere, length(pm3.xz)-0.5);
	float floor = 1-p.y;
	sphere=min(sphere, df_obelisk(p));
	return min(sphere,floor);
}

vec4 pf3(int pid, float time, out vec4 color)
{
	float scale = 0.1;
	float blur = 0.1;
	vec3 pos = vec3(sin(time/4)*4,-4,cos(time/4)-2); 
	
	vec3 dir = fract(sin(vec3(pid%115347*0.5321,pid%82345*0.12345,pid%5123*0.14123))*412.5317)-.5;
	dir=normalize(dir);
	pos+=dir;
	
	for (int i=0; i<50; i++){
		vec3 p = pos;
		p.yz=rotate(p.yz,0.5);
		p.z+=time;
		p.y-=4;
		float dist=df(p, time);
		if (dist<0.001) break;
		pos+=dir*dist;
	}
	pos.z+=4;
	//scale+=abs(8-pos.z)*0.05;
	color=vec4(vec3(0.5,0.4,0.3)*0.4, blur);
	return vec4(pos, scale);
}


vec3 path(float time, float id){
	float ang = time+id;
	float movement = smoothstep(62,72,time);
	vec3 pos = vec3(0,-6, 0);
	pos.x += (time*0.1+.2*time*smoothstep(64,128,time)+sin(id*.93)*id*0.7)*movement;
	pos.y += (sin(ang)+sin(ang*0.9)+sin(id*.4)*id*0.7)*0.5*movement;
	pos.z += (cos(ang)+cos(ang*0.9)+sin(id*.71)*id*3)*0.5*movement;
	vec3 pos2 = vec3(100,-4,0);
	float time2=time-256;
	float radi2 = 4+10/(1+8+time/2-240);
	float converge = smoothstep(256+16*3,256+16*4, time);
	pos2.y-=10+time2/4+sin(id*12)*2*(1-converge);
	radi2=mix(radi2, radi2*(1-converge)*4, converge);
	vec3 pos3 = vec3(100,-4, 0);
	pos3 += sin(vec3(id*23.4,id*17.41,id*4.71)+time*0.1)*vec3(6,2,6) + sin(pos);
	pos2.x+=sin(ang)*radi2;
	pos2.z+=cos(ang)*radi2;
	pos = mix(pos, pos3, smoothstep(256-16*8,256-16*3, time));
	pos = mix(pos, pos2, smoothstep(256-16*(2.25),256-16*2, time));
	return pos;
}
vec3 path(float time){
	return path(time, 0);
	//return vec3((sin(time/4.2)+sin(time/4))*2+4+time,-6+sin(time/7),sin(time/4.1)+cos(time/4)-4+time*0.25);
}

vec3 cluster_color(float clusterid){
	return max(vec3(0.1),vec3(0.5+sin(clusterid*9.21)*.5,0.5+sin(clusterid*5.81)*.5,0.5+sin(clusterid)*.5));
}

vec4 cluster(int pid, float time, out vec4 color, float clusterid)
{
	float id=clusterid;
	float scale;
	float blur = 0.1;
	vec3 pos=vec3(0);
	scale=0.02+pow(abs(sin(pid*42.7123)),256.0)/2;
	float ts=0.5+scale;
	pos=path(time-sin(pid+time/ts)*1-1,id);
	float amp=cos(pid+time/ts+1)*32;
	float deviate =  pow(abs(sin(pid*61.234)),32.0);
	blur=smoothstep(0,1,scale*8);
	scale-=deviate*0.01;
	pos+=sin(pos.yzx+vec3(pid*12.3,pid*9.512,pid*3.4))*deviate*0.25;
	//scale*=abs(sin(pid*6123.123));
	//pos=vec3(0);
	vec3 c = cluster_color(clusterid)*0.01/scale*amp;
	c=mix(c,c.zxy, deviate);
	color = vec4(c, blur);
	return vec4(pos, scale);
}
/*
vec4 pf4(int pid, float time, out vec4 color)
{
	float scale = 0.1;
	float blur = 0.1;
	vec3 pos=vec3(0);
	int bg_count = 100000;
	if (pid<bg_count)
	{		
		pos = path(time);
		vec3 c=vec3(0.5,0.4,0.3);
		if(pid<bg_count/2) {
			pos.x+=9;
			pos.y-=5;
			c=vec3(0.3,0.3,0.5);
		}
		vec3 dir = fract(sin(vec3(pid%115347*0.5321,pid%82345*0.12345,pid%5123*0.14123))*412.5317)-.5;
		dir.y+=0.7;
		//dir.xz = rotate(dir.xz, time/4);
		dir=normalize(dir);
		pos+=dir;
		for (int i=0; i<30; i++){
			vec3 p = pos;
			float dist=df(p, time);
			if (dist<0.001) break;
			pos+=dir*dist;
		}
		color=vec4(c*0.1, blur);
	}
	else if(pid<bg_count+400) {
		vec4 res = cluster(pid, time, color);
		pos = res.xyz;
		scale = res.w;
	} else {
		pos.z=-44;
		scale=0;
		return vec4(pos, scale);
	}
	// camera
	pos-=vec3(time, -5, 2);
	pos.y+=4+sin(time/3);
	pos.x+=sin(time/4);
	pos.x-=2;
	pos.z+=4+sin(time/5);
	pos.xz=rotate(pos.xz,0.4);
	pos.yz=rotate(pos.yz,-0.9);
	return vec4(pos, scale);
}
*/

vec4 pf5(int pid, float time, out vec4 color)
{
	float fadeout = smoothstep(256+16*5+1,256+16*5,time);
	float fadeout_final = smoothstep(256+16*6, 256+16*5, time);
	float fadin_geo_n_cluster = smoothstep(32,64, time)*fadeout;
	float fadein_geo = smoothstep(64,96, time)*fadeout;
	float scale = 0.1;
	float dof_center=1;
	float dof_amp=0;
	float blur = 0.1;
	vec3 pos=vec3(0);
	int bg_count = 300000;
	int st_count = 100000;
	float cluster_pcount = 400+8000*pow(smoothstep(56,160,time),4);
	float maxid = max(0,(cluster_pcount/400)-2);
	float cam_interpolate = smoothstep(256-16*4,256-16*2, time);
	float cam_interpolate2 = smoothstep(256-16*8,256-16*6, time);
	
		float snare = (1-fract(0.5+time/2))*smoothstep(256-16*2, 256-16*2+1,time)*smoothstep(256+16*4, 256+16*4-1, time);
	if (pid<st_count){
		// static
		float converge = pow(smoothstep(0,64, time+pow(abs(sin(pid*4)),1)*16),16);
		float converge2 = smoothstep(256+16*6,256+16*5,time+pow(sin(pid)*.5+.5,16)*.5+.5);
		converge*=converge2;
		if (converge>=.9999){
			// discard
			pos.z=-44; scale=0;	return vec4(pos, scale);
		}
		blur=0.8;
		dof_center = 4;
		dof_amp = 0.01 + 0.2/(1+time);
		pos=(fract(sin(vec3(pid)*vec3(0.63116,0.523,0.69312))*51.123)*2-1)*(16+(1-converge2)*time);
		pos.xz*=3;
		//pos.y+=(time)*sin(pid%7451*214.125)-8*smoothstep(63,68,time)-64*pow(sin(pid%613476)*.5+.5,4);
		pos+=sin(pos);
		pos.y-=10;
		//pos.z+=16;
		scale=0.01+0.1*pow(abs(sin(time+pid)),2);
		vec3 c = vec3(0.9, 0.6, 0.5)*0.03 ;
		//scale+=abs(4+100/(1+time/2)+osin(time*0.25)*2-pos.z)*0.05;
		float fadein=mix(osin(time*2)*time/64, 1, smoothstep(0,64,time));
		//fadein=1.0;
		// converge with path
		vec3 path_pos = path(time);
		pos=mix(pos+path_pos, path_pos, converge);
		c=mix(c,c.yxz, (osin(time/2)*.5+.5));
		c=mix(c,c.zyx, (osin(time/4)*.5+.5));
		c*=(0.1+pow(abs(sin(pid%123)),8.0));
		color=vec4(mix(c,c.zyx,sin(pid*.371+.5))/pow(scale,1.9)*0.2*( fadein), blur);
	}
	else if (pid<bg_count+st_count)
	{		
		float clusterid = min(pid%12,maxid);
		pos = path(time+pow(abs(sin(time+pid)),32)*-8, clusterid);
		vec3 c=cluster_color(clusterid)*vec3(0.9,0.7,0.4);
		vec3 dir = fract(sin(vec3(pid%115347*0.5321,pid%82345*0.12345,pid%5123*0.14123))*412.5317)-.5;
		scale=0.05+snare*0.03;
		dir.y+=mix(0.7,0,cam_interpolate2);
		//dir.xz = rotate(dir.xz, time/4);
		dir=normalize(dir);
		//pos+=dir;
		for (int i=0; i<50; i++){
			vec3 p = pos;
			float dist=df(p, time);
			if (dist<0.001) break;
			if (dist>20.0) break;
			pos+=dir*dist;
		}
		color=vec4(c*0.9*fadein_geo, blur);
	}
	else if(pid<st_count+bg_count+cluster_pcount) {
		int cluster_pid = pid-(st_count+bg_count);
		float id=cluster_pid/400;
		id=min(id,maxid);
		vec4 res = cluster(cluster_pid, time, color, id);
		color.xyz*=fadin_geo_n_cluster+snare;
		pos = res.xyz;
		scale = res.w;
	} else {
		// discard
		pos.z=-44; scale=0;	return vec4(pos, scale);
	}
	// camera
	vec3 cam_path = path(time);
	pos-=vec3(mix(cam_path.x,100,cam_interpolate),-5+cam_path.y*cam_interpolate*0.9, 2);
	pos.xz=rotate(pos.xz,(time-62)/16.0*smoothstep(62,68,time)+4);
	//pos.z+=1;
	//pos.y+=4+sin(time/3);
	//pos.x+=sin(time/4);
	//pos.z+=4+sin(time/5);
	//pos.xz=rotate(pos.xz,0.4);
	float time2=time-256;
	pos.yz=rotate(pos.yz,mix(-0.7, -0.2+time2*0.01, cam_interpolate));
	pos.y-=3*cam_interpolate;
	pos.z+=3+((16+time2/2)*0.1)*cam_interpolate2;
	
	float dofscale=abs(dof_center-pos.z)*dof_amp;
	scale+=dofscale;
	color.xyz*=pow(scale/(scale+dofscale),2)*fadeout_final;
	return vec4(pos, scale);
}

void main() {
	int vid = gl_VertexIndex;
	int tid = vid/3; // triangle id
	int pid = tid/2; // polygon id
	int issecondtris = tid%2;
	vec2 uv = positions[vid%3+issecondtris*2];
	
	//vec3 pos=pf1(pid, ubo.time);
	vec4 col_res1, col_res2;
	float time = ubo.time;
	vec4 pos_res1, pos_res2;
	
	// just call pf5
	pos_res1 = pf5(pid, ubo.time, col_res1);
	pos_res2 = pf5(pid, ubo.time+0.1, col_res2);
	
	float near_limit = 0.0;
	if(pos_res1.z<near_limit||pos_res2.z<near_limit) {
		gl_Position=vec4(-4,-4,-4,1);
		return;
	}
	
	float blur = col_res1.w;
	float scale = 0.02;
	
	float size1 = pos_res1.w/pos_res1.z;
	float size2 = pos_res2.w/pos_res2.z;
	
	// calculate shape
	// project to 2d
	vec2 pos_2d = pos_res1.xy/pos_res1.z;
	vec2 pos2_2d = pos_res2.xy/pos_res2.z;
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
	data2=mix(col_res1, col_res2, mixfac);
}