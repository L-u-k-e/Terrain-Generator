#version 130 
 
in vec2 UV;
 
out vec4 color; 
uniform sampler2D textureID;
 
void main(){
	color = texture( textureID, UV);
}
