using namespace std;
class skybox
{
    //ID for appropriate shader
    GLuint skyboxID;

    //cube map texture itself
    GLuint box;

    //image dimensions (every side must be the same dimensions)
    //int image_width;
   // int image_height;
    
    //VP matrix
    glm::mat4 View;                                                                                                                                                
    glm::mat4 Projection;
    glm::mat4 VP;

    //location of unform variable in fragment shader to identify the texture 
    GLuint textureID;
    
    //location of VP matrix in vertex shader.
    GLuint projectionID;
    GLuint viewID;

    //location of attribute array to use to pass input to vertex shader
    int AA_index;

    //vertex buffer object
    GLuint VBO;

    void loadImage(const char *file_name, GLenum side_target);
    public:
        skybox(void);
        void load(void);
        void update(glm::mat4 view, glm::mat4 projection);
        void draw(void);
};

skybox::skybox(void)
{
}


void skybox::update(glm::mat4 view, glm::mat4 projection)
{
    view = glm::mat4(glm::mat3(view));

    projectionID  = glGetUniformLocation(skyboxID, "projection");
    viewID  = glGetUniformLocation(skyboxID, "view");

    glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);  
    glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);  
    
}

void LoadBitmap(const char* FilePath, int &width, int &height, vector<unsigned char> &Pixels)                                                                              
{
    width = 0;
    height = 0;
    short BitsPerPixel = 0;
        
    std::fstream hFile(FilePath, std::ios::in | std::ios::binary);
    if (!hFile.is_open()) throw std::invalid_argument("Error: File Not Found.");

    hFile.seekg(0, std::ios::end);
    int Length = hFile.tellg();
    hFile.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> FileInfo(Length);
    hFile.read(reinterpret_cast<char*>(FileInfo.data()), 54);

    if (FileInfo[0] != 'B' && FileInfo[1] != 'M')
    {   
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. Bitmap Required.");
    }   

    if (FileInfo[28] != 24 && FileInfo[28] != 32) 
    {   
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. 24 or 32 bit Image Required.");
    }   

    BitsPerPixel = FileInfo[28];
    width = FileInfo[18] + (FileInfo[19] << 8);
    height = FileInfo[22] + (FileInfo[23] << 8);
    std::uint32_t PixelsOffset = FileInfo[10] + (FileInfo[11] << 8);
    std::uint32_t size = ((width * BitsPerPixel + 31) / 32) * 4 * height;
    Pixels.resize(size);

    hFile.seekg(PixelsOffset, std::ios::beg);
    hFile.read(reinterpret_cast<char*>(Pixels.data()), size);
    hFile.close();
}


void skybox::load(void)
{
    skyboxID = loadShaders("skybox_fragment_shader.glsl", "skybox_vertex_shader.glsl");

    //image_width;
    //image_height;
    
    //create the cube texture object handle
    glGenTextures(1, &box);

    //paint each side of the cube
    loadImage("skyboxes/galaxy/NegativeX.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
    loadImage("skyboxes/galaxy/PositiveX.png", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    loadImage("skyboxes/galaxy/NegativeY.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
    loadImage("skyboxes/galaxy/PositiveY.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
    loadImage("skyboxes/galaxy/NegativeZ.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    loadImage("skyboxes/galaxy/PositiveZ.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);

 /*   loadImage("Color.bmp", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
    loadImage("Color.bmp", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    loadImage("Color.bmp", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
    loadImage("Color.bmp", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
    loadImage("Color.bmp", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    loadImage("Color.bmp", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
*/
   
    //set filter options
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //set wrap options 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //generate mipmap
    //glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    //create box to splat cube texture onto 
    GLfloat D =1.0f; 
    GLfloat vertices[] = {
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

    //generate cube VBO
    VBO = createBuffer(GL_ARRAY_BUFFER, vertices, sizeof(GLfloat) * (36*3), GL_STATIC_DRAW);

    //set up projection matrix
   // Projection = glm::ortho(0.0f,(float) window_width, 0.0f, (float) window_height, 0.1f, 10000.0f);

    //get location of uniform variables in the shader
    textureID = glGetUniformLocation(skyboxID, "textureID");

    //set the uniform variable indicating active texture unit (found in fragment shader) to 0
    glUniform1i(textureID, 0);

    //get location of attribute array in shader
    AA_index = glGetAttribLocation(skyboxID, "position"); 

    glUseProgram(programID);
}

void skybox::loadImage(const char *file_name, GLenum side_target)
{
    //specify texture unit
    glActiveTexture(GL_TEXTURE0);
    
    //bind texture object to target
    glBindTexture(GL_TEXTURE_CUBE_MAP, box);

    int width, height;
    //load the image
    unsigned char* image = SOIL_load_image(file_name, &width, &height, 0, SOIL_LOAD_RGB); 
    glTexImage2D(side_target, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

   // vector<unsigned char> image;
   // LoadBitmap("Color.bmp", width, height, image); 
   // glTexImage2D(side_target, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data());
    
    //clean up
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); 

}

void skybox::draw()
{
     
     glUseProgram(skyboxID);                                                                                                                                

     // Set uniform variable for texture in shader
     glUniform1i(textureID, 0);

     //bind texture object to target
     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_CUBE_MAP, box);     

     //set up cube vertices
     glEnableVertexAttribArray(AA_index);
     attributeBind(VBO, AA_index, 3);

     //we were never here!! Don't tell the depth buffer. sssshhhh!!!!
     glDepthMask(GL_FALSE); 

     //draw the sky box
     glDrawArrays(GL_TRIANGLES, 0, 36);

     //turn depth writing on again
     glDepthMask(GL_TRUE); 


}
