#version 130
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 MVP;

out vec3 fragmentColor;
void main()
{
	// Transforming vertex with MVP matrix
    vec4 v = vec4(position, 1.0);
	gl_Position = MVP * v;

	// Just output vertex color directly
	fragmentColor = color;
}
