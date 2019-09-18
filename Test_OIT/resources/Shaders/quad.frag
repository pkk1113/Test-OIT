#version 430 core

layout(binding = 0) uniform sampler2D map;

in vec2 texCoord;

layout(location = 0) out vec4 frag_color;

void main()
{
	frag_color = texture(map, texCoord);

	if(texCoord.x <= 0.01f || texCoord.x >= 0.99f ||
		texCoord.y <= 0.01f || texCoord.y >= 0.99f) 
	{
		frag_color = vec4(0.5f, 0.5f, 0.5f, 1);
	}
}