#version 130

// Interpolated values from the vertex shaders
in vec3 fragmentColor;

// Output color
out vec3 color;

void main() {
    color = fragmentColor;
}
