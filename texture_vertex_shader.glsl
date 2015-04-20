#version 130 
#extension GL_ARB_explicit_attrib_location : require

// Just using position and coordinates
in vec3 position;
in vec2 texcoords;

out vec2 UV; 

void main()
{
	// Already passing in normalized device coordinates
	// So we don't have to do anything to them
	gl_Position = vec4(position, 1.0);	
	UV = texcoords;
}


