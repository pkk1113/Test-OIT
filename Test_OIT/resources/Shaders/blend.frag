#version 430 core

layout(binding = 0) uniform sampler2D color_map;

in vec2 texCoord;

layout(location = 0) out vec4 frag_color;

void main()
{
	frag_color = texture(color_map, texCoord);
}