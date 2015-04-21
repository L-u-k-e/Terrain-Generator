#version 130 
 
in vec3 TexCoords;
 
out vec4 color; 
uniform samplerCube textureID;
 
void main(){
	color = texture( textureID, TexCoords);
}
