/*
	Author: Lucas Parzych

	Globals
*/

float mouseX, mouseY;
float lastX, lastY;

GLuint programID, matrixID;

int window_width=800;
int window_height=600;

const char* fragment_shader = "fragment_shader.glsl";                                                                                                                      
const char* vertex_shader   = "vertex_shader.glsl"; 
GLint vertex_attribute_loc = 0;                                                                                                                                                
GLint color_attribute_loc = 1;

//Global VAO
GLuint VertexArrayID;

const char* texture_fragment_shader = "skybox_fragment_shader.glsl";                                                                                                                      
const char* texture_vertex_shader   = "skybox_vertex_shader.glsl";     

float octaves=8;
float persistence=0.5;