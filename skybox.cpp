class skybox
{
    vector<GLuint> box;

    int image_width;
    int image_height;

    GLuint loadImage(const char *file_name, int image_width, int image_height);
    public:
        skybox(void);
        void load(void);
        void update(void);
        void draw(void);
};

skybox::skybox(void)
{
    vector<string> images;
    images.push_back("skyboxes/galaxy/NegativeX.png");
    images.push_back("skyboxes/galaxy/PositiveX.png");
    images.push_back("skyboxes/galaxy/NegativeY.png");
    images.push_back("skyboxes/galaxy/PositiveY.png");
    images.push_back("skyboxes/galaxy/NegativeZ.png");
    images.push_back("skyboxes/galaxy/PositiveZ.png");

    image_width=4096;
    image_height=4096;

    for(int i=0; i<6; i++)
    {
        box.push_back(loadImage(images[i].c_str(), image_width, image_height));
    }
}

GLuint skybox::loadImage(const char *file_name, int image_width, int image_height)
{
    //create texture object
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    //set wrapping options
    float borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  

    //set filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //load the image
    unsigned char* image = SOIL_load_image(file_name, &image_width, &image_height, 0, SOIL_LOAD_RGB); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    
    //generate mipmap
    glGenerateMipmap(GL_TEXTURE_2D);
    
    //clean up
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0); 

    return texture;
}

void skybox::draw()
{
     glUseProgram(textureID);                                                                                                                                

     // Use renderedTexture as the 2D texture
     glBindTexture(GL_TEXTURE_2D, renderedTexture);

     // Set uniform variable for texture in shader
     glUniform1f(uniformTextureID, 0);

     glEnableVertexAttribArray(tex_vertex_attribute_loc);                                                                                                                       
     glEnableVertexAttribArray(tex_UV_attribute_loc);
     
     glBindTexture(GL_TEXTURE_2D, texture);
     // Define how to read vertex_buffer                                                                                                                                    
     glVertexAttribPointer(
        tex_vertex_attribute_loc,                  // attribute array index
        3,                  // number of components (3 = 3D point)
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
        );


    
     glDisableVertexAttribArray(tex_vertex_attribute_loc);                                                                                                                       
     glDisableVertexAttribArray(tex_UV_attribute_loc);
}
