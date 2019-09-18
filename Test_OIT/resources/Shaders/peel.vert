#version 430 core

layout(location = 0) in vec3 aVertex;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(location = 0) uniform mat4 pmat;
layout(location = 1) uniform mat4 vmat;
layout(location = 2) uniform mat4 mmat;

out vec3 world_position;
out vec3 world_normal;

void main()
{
	world_position = vec3(mmat * vec4(aVertex, 1.f));

	world_normal = mat3(transpose(inverse(mmat))) * aNormal;

	gl_Position = pmat * vmat * mmat * vec4(aVertex, 1.f);
}