#version 430 core

layout(location = 3) uniform vec2 inv_screen_size;
layout(location = 4) uniform float alpha;
layout(location = 5) uniform vec3 color;

layout(binding = 0) uniform sampler2D depth_map;

in vec3 world_position;
in vec3 world_normal;

layout(location = 0) out vec4 frag_color;

void main()
{
	float depth = texture(depth_map, gl_FragCoord.xy * inv_screen_size).r;
	
	float eps = 0.01f;

	if(gl_FragCoord.z <= depth + eps) {
		discard;
		return;
	}

	float a = 0.2f;
	float d = max(0.f, dot(normalize(vec3(1, 1, 1)), world_normal));
	
	frag_color = vec4((a + d) * color, alpha);
}