#version 130

// Since we're using version 130 (OpenGL 3.0) AND we're passing in explicit vertex attribute arrays,
// we need to require that the GL_ARB_explicit_attrib_location extension is available on this system.
// This allows us to use the "layout" stuff later in the code.
#extension GL_ARB_explicit_attrib_location : require

// Positions are at location 0 --> same value we passed into glVertexAttribPointer() when setting the vertex buffer
// in = input data (as opposed to out = output data)
// vec3 = 3 component vector (x,y,z)
// position = name we'll use to refer to the position data (this could have been any name)
layout(location = 0) in vec3 position;

// Similar to above, except we're looking at location 1 (where we put the color buffer data)
layout(location = 1) in vec3 color;

// This is our Model-View-Projection matrix
uniform mat4 MVP;

// Output vertex color (which will be interpolated for each fragment)
out vec3 fragmentColor;

void main()
{
	// Transforming vertex with MVP matrix
    vec4 v = vec4(position, 1.0);
	gl_Position = MVP * v;

	// Just output vertex color directly
	fragmentColor = color;
}
