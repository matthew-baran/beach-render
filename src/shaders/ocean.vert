#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

struct Wave
{
	vec2 wave_dirs;
	float freq;
	float phase;
	float phase_offset;
	float amplitude;
	float chop;
};
#define NUM_WAVES 1
uniform Wave[NUM_WAVES] geom_waves;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
};
uniform Material material;

out vec3 FragPos;
out vec3 WorldPos;
out vec3 Normal;

out vec3 Color;
out vec2 TexCoords;

out float depth;
out float surface_elev;
out float att_factor;
out float particle_phase;
out float wave_width;

const float PI = 3.14159265359;

vec3 getNewPosition(float cur_depth);
vec3 getNewNormal(vec3 pos, float cur_depth);
vec2 setFoamCoords();
float getElevation(float x, float z);
float updateSpeed(float freq, float phase, float cur_depth);

void main()
{
	float cur_depth = max(0, -getElevation(aPos.x, aPos.z));

	vec3 new_pos = getNewPosition(cur_depth);
	vec3 new_norm = getNewNormal(new_pos, cur_depth);

	// Debug: turn off geom waves
	//new_pos = vec3(aPos.x, 0.0, aPos.z);
	//new_norm = aNormal;

	gl_Position = projection * view * model * vec4(new_pos, 1.0);

	TexCoords = setFoamCoords();
	WorldPos = vec3(model * vec4(new_pos, 1.0));
	FragPos = vec3(view * model * vec4(new_pos, 1.0));
	Normal = new_norm;
	float gamma = 2.2;
//	Color = vec3(9./255., 12./255., 11./255.);
	Color = pow(vec3(36./255., 48./255., 46./255.), vec3(gamma));	// remove gamma here...
}

vec3 getNewPosition(float cur_depth)
{
	vec3 new_pos = vec3(0, 0, 0);

	float tot_chop = 0.4 + clamp(2 - cur_depth, 0, 1) / 2;

	for (int i = 0; i < NUM_WAVES; ++i)
	{
		float amp = geom_waves[i].amplitude;

		// Conform waves to an arbitrary non-linear direction
//		vec2 tst_dir = normalize(vec2(335-aPos.x, -25-aPos.z));
//		float angle = geom_waves[i].freq * dot(tst_dir, vec2(aPos.x, aPos.z)) - geom_waves[i].phase;
//		new_pos.x += amp * geom_waves[i].chop * tst_dir.x * cos(angle);
//		new_pos.z += amp * geom_waves[i].chop * tst_dir.y * cos(angle);
//		new_pos.y += amp * sin(angle);

		float phi = updateSpeed(geom_waves[i].freq, geom_waves[i].phase - geom_waves[i].phase_offset, cur_depth);
		phi += geom_waves[i].phase_offset;

		float chop = tot_chop / (geom_waves[i].freq * amp * NUM_WAVES);

		// attempt to incorporate depth...
//		float angle = geom_waves[i].freq * dot(geom_waves[i].wave_dirs, vec2(aPos.x, -aPos.z)) - phi;
//		new_pos.x += (amp / max(.1, tanh(geom_waves[i].freq*cur_depth))) * chop * geom_waves[i].wave_dirs.x * sin(angle);
//		new_pos.z += (amp / max(.1, tanh(geom_waves[i].freq*cur_depth))) * chop * -geom_waves[i].wave_dirs.y * sin(angle);
//		new_pos.y += amp * cos(angle);

		float angle = geom_waves[i].freq * dot(geom_waves[i].wave_dirs, vec2(aPos.x, -aPos.z)) - phi;
		new_pos.x += amp * chop * geom_waves[i].wave_dirs.x * cos(angle);
		new_pos.z += amp * chop * -geom_waves[i].wave_dirs.y * cos(angle);
		new_pos.y += amp * sin(angle);

		// warning warning - assumes 1 wave only
		particle_phase = angle;
	}

	float PI_2 = PI / 2;
	float sin_width = (2 * PI / geom_waves[0].freq) * abs( clamp(mod(particle_phase, 2*PI) - PI_2, -PI_2, PI_2) / (2*PI));
	wave_width = 2 * (sin_width - geom_waves[0].amplitude * tot_chop * cos(particle_phase));

	// sample the texture at the new x/z position
	surface_elev = getElevation(aPos.x+new_pos.x, aPos.z+new_pos.z);

	att_factor = clamp((-surface_elev)/1., 0, 1);	// linear ramp from 1m to 0m depth
//	new_pos.y = att_factor*new_pos.y + (1-att_factor)*.30;
//	new_pos.y = new_pos.y * att_factor;

	if (new_pos.y < (surface_elev + .15))
	{
		new_pos.y = surface_elev+0.15;
	}

	depth = new_pos.y - surface_elev;

	return vec3(aPos.x + new_pos.x, new_pos.y, aPos.z + new_pos.z);
}

vec3 getNewNormal(vec3 newPos, float cur_depth)
{
	vec3 new_norm = vec3(0, 1, 0);

	float tot_chop = 0.4 + clamp(2-cur_depth, 0, 1) / 2;

	for (int i = 0; i < NUM_WAVES; ++i)
	{
		// Conform waves to an arbitrary non-linear direction
//		vec2 tst_dir = normalize(vec2(335-aPos.x, -25-aPos.z));
//		float angle = geom_waves[i].freq*dot(tst_dir, vec2(newPos.x, newPos.z)) - geom_waves[i].phase;
//		new_norm.x -= tst_dir.x * geom_waves[i].freq * geom_waves[i].amplitude * cos(angle);
//		new_norm.z -= tst_dir.y * geom_waves[i].freq * geom_waves[i].amplitude * cos(angle);
//		new_norm.y -= geom_waves[i].chop * geom_waves[i].freq * geom_waves[i].amplitude * sin(angle);

		float chop = tot_chop / (geom_waves[i].freq * geom_waves[i].amplitude * NUM_WAVES);

		float phi = updateSpeed(geom_waves[i].freq, geom_waves[i].phase - geom_waves[i].phase_offset, cur_depth);
		phi += geom_waves[i].phase_offset;

		float angle = geom_waves[i].freq*dot(geom_waves[i].wave_dirs, vec2(newPos.x, -newPos.z)) - phi;
		new_norm.x -= geom_waves[i].wave_dirs.x * geom_waves[i].freq * geom_waves[i].amplitude * cos(angle);
		new_norm.z -= -geom_waves[i].wave_dirs.y * geom_waves[i].freq * geom_waves[i].amplitude * cos(angle);
		new_norm.y -= chop * geom_waves[i].freq * geom_waves[i].amplitude * sin(angle);
	}

	return new_norm;
}

vec2 setFoamCoords()
{
	// use old position elevation to get foam tex coords
	float elev = getElevation(aPos.x, aPos.z);
	float max_foam_depth = 0.5;
	float v = 1 - (max_foam_depth + elev) / max_foam_depth;
	float u = aPos.z / 5;
	return vec2(u, v);
}

float getElevation(float x, float z)
{
	float elev = vec3(texture(material.texture_diffuse1, vec2((x - 25)/50, -z/50))).x;
	return (elev - 0.5) * 6;
}

float updateSpeed(float freq, float phase, float cur_depth)
{
//	return phase;
	phase = phase / sqrt(freq*9.8);
	return phase * max(1, floor(sqrt(9.8*freq*max(0, tanh(freq*cur_depth))))); // apply depth factor to wave speed
}