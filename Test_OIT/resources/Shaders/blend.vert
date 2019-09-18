#version 430 core

const vec2 aPosition[4] = {
	vec2(-1, -1),
	vec2(1, -1),
	vec2(1, 1),
	vec2(-1, 1),
};

const vec2 aTexCoord[4] = {
	vec2(0, 0),
	vec2(1, 0),
	vec2(1, 1),
	vec2(0, 1),
};

out vec2 texCoord;

layout(location = 0) uniform mat4 mat;

void main()
{
	gl_Position = mat * vec4(aPosition[gl_VertexID], 0, 1);

	texCoord = aTexCoord[gl_VertexID];
}

