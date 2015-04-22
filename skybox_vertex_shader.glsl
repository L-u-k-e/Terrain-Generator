#version 130 
#extension GL_ARB_explicit_attrib_location : require

layout (location = 2) in vec3 position;
uniform mat4 view;    
uniform mat4 projection;    

out vec3 TexCoords; 

void main()
{
	gl_Position = projection * view * vec4(position, 1.0);	
	TexCoords = position;
}


