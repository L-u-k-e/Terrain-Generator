using namespace std;
class skybox
{
    //ID for appropriate shader
    GLuint skyboxID;

    //cube map texture itself
    GLuint box;

    //image dimensions (every side must be the same dimensions)
    int image_width;
    int image_height;
    
    //VP matrix
    glm::mat4 View;                                                                                                                                                
    glm::mat4 Projection;
    glm::mat4 VP;

    //location of unform variable in fragment shader to identify the texture 
    GLuint textureID;
    
    //location of VP matrix in vertex shader.
    GLuint matrixID;

    //location of attribute arrays to use to pass input to vertex shader
    GLuint AA_index_vertex;
    GLuint AA_index_uv;

    //actual cube vertex coords in 3D world space
    float vertices[18 * 6];

    GLuint loadImage(const char *file_name, int image_width, int image_height);
    public:
        skybox(void);
        void load(void);
        void update(glm::mat4 view);
        void draw(void);
};

skybox::skybox(void)
{
    skyboxID = loadShaders("skybox_fragment_shader.glsl", "skybox_vertex_shader.glsl");

    image_width=4096;
    image_height=4096;
    
    //create the cube texture object handle
    glGenTextures(1, &box);

    //paint each side of the cube
    loadImage("skyboxes/galaxy/NegativeX.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
    loadImage("skyboxes/galaxy/PositiveX.png", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    loadImage("skyboxes/galaxy/NegativeY.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
    loadImage("skyboxes/galaxy/PositiveY.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
    loadImage("skyboxes/galaxy/NegativeZ.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    loadImage("skyboxes/galaxy/PositiveZ.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);

    //set filter options
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    //set wrap options 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //generate mipmap
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    //create box to splat cube texture onto 
    float D =500.0f; 
    float temp[] = {
       -D,  D, -D,
       -D, -D, -D,
        D, -D, -D,
        D, -D, -D,
        D,  D, -D,
       -D,  D, -D,

       -D, -D,  D,
       -D, -D, -D,
       -D,  D, -D,
       -D,  D, -D,
       -D,  D,  D,
       -D, -D,  D,

        D, -D, -D,
        D, -D,  D,
        D,  D,  D,
        D,  D,  D,
        D,  D, -D,
        D, -D, -D,

       -D, -D,  D,
       -D,  D,  D,
        D,  D,  D,
        D,  D,  D,
        D, -D,  D,
       -D, -D,  D,

       -D,  D, -D,
        D,  D, -D,
        D,  D,  D,
        D,  D,  D,
       -D,  D,  D,
       -D,  D, -D,

       -D, -D, -D,
       -D, -D,  D,
        D, -D, -D,
        D, -D, -D,
       -D, -D,  D,
        D, -D,  D
    };
    copy(temp, temp+(18*6), vertices);

    //set up projection matrix
    Projection = glm::ortho(0.0f,(float) window_width, 0.0f, (float) window_height, 0.1f, 10000.0f);

    //get location of uniform variables in the shader
    textureID = glGetUniformLocation(skyboxID, "textureID");
    matrixID  = glGetUniformLocation(skyboxID, "VP");

    //get location of attribute arrays in shader
    AA_index_vertex = glGetAttribLocation(skyboxID, "position");                                                                                           
    AA_index_uv     = glGetAttribLocation(skyboxID, "texcoords");

}

void skybox::update(glm::mat4 view)
{
    //box moves with camera
    View = view;

    //calculate VP matrix and inform the vertex shader of the new value
    VP = Projection * View;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &VP[0][0]);  
    

}


void skybox::load(void)
{
    
}

GLuint skybox::loadImage(const char *file_name, GLenum side_target)
{
    //bind texture object to target
    glBindTexture(GL_TEXTURE_CUBE_MAP, box);

    //load the image
    unsigned char* image = SOIL_load_image(file_name, &image_width, &image_height, 0, SOIL_LOAD_RGB); 
    glTexImage2D(side_target, 0, GL_RGB, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    
    //clean up
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); 

    return texture;
}

void skybox::draw()
{

     glUseProgram(skyboxID);                                                                                                                                

     // Use renderedTexture as the 2D texture
     glBindTexture(GL_TEXTURE_2D, box);

     // Set uniform variable for texture in shader
     glUniform1f(textureID, 0);

     glEnableVertexAttribArray(AA_index_vertex);                                                                                                                       
     glEnableVertexAttribArray(AA_index_uv);
     
     glVertexAttribPointer(
        AA_index_vertex,                  // attribute array index
        3,                  // number of components (3 = 3D point)
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
        );
   


     glDisableVertexAttribArray(tex_vertex_attribute_loc);                                                                                                                       
     glDisableVertexAttribArray(tex_UV_attribute_loc);
}
