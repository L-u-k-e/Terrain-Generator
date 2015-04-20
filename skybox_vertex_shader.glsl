#version 130 
#extension GL_ARB_explicit_attrib_location : require

in vec3 position;
in vec2 texcoords;

uniform mat4 VP;    

out vec2 UV; 

void main()
{
	gl_Position = VP * vec4(position, 1.0);	
	UV = texcoords;
}


