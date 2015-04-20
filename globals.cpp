float mouseX, mouseY;
float lastX, lastY;

GLuint programID, matrixID;

int window_width=600;
int window_height=600;

const char* fragment_shader = "fragment_shader.glsl";                                                                                                                      
const char* vertex_shader   = "vertex_shader.glsl"; 
GLint vertex_attribute_loc;                                                                                                                                                
GLint color_attribute_loc;

const char* texture_fragment_shader = "skybox_fragment_shader.glsl";                                                                                                                      
const char* texture_vertex_shader   = "skybox_vertex_shader.glsl";
GLint tex_vertex_attribute_loc;       
GLint tex_UV_attribute_loc;     
