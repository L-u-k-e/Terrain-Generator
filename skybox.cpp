using namespace std;
class skybox
{
    //ID for appropriate shader
    GLuint skyboxID;

    //cube map texture itself
    GLuint box;
    
    //location of unform variable in fragment shader to identify the texture unit 
    GLuint textureID;
    
    //location of VP matrix in vertex shader.
    GLuint projectionID;
    GLuint viewID;

    //vertex array object
    //holds all of the vertex info necessary for rendering
    GLuint VAO;

    GLuint loadCubemap(vector<const GLchar*> faces);
    public:
        skybox(void);
        void load(glm::mat4 projection);
        void update(glm::mat4 view);
        void draw(void);
};

skybox::skybox(void)
{

}


void skybox::update(glm::mat4 view)
{
    //remove the translation dimension from the camera's view matrix.
    //view = glm::mat4(glm::mat3(view));

    //Inform the vertex shader of the new view matrix 
    glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);  
    
}

void skybox::load(glm::mat4 projection)
{
    //Load cube map shaders
    skyboxID = loadShaders("skybox_fragment_shader.glsl", "skybox_vertex_shader.glsl");

    //Get location of uniform variables in the shader program
    textureID = glGetUniformLocation(skyboxID, "textureID");        //texture unit      (fragment shader)
    projectionID  = glGetUniformLocation(skyboxID, "projection");   //projection matrix (vertex shader)
    viewID  = glGetUniformLocation(skyboxID, "view");               //view matrix       (vertex shader)

    //Set the uniform variable indicating the active texture unit (found in fragment shader) to 0
    glUniform1i(textureID, 0);

    //Pass projection matrix to shader, This isn't going to change so we can set it up now. 
    glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);  

    //Load the cube map, paint each side
    vector<const GLchar*> faces;
    faces.push_back("skyboxes/galaxy/right.png");
    faces.push_back("skyboxes/galaxy/left.png");
    faces.push_back("skyboxes/galaxy/top.png");
    faces.push_back("skyboxes/galaxy/bottom.png");
    faces.push_back("skyboxes/galaxy/back.png");
    faces.push_back("skyboxes/galaxy/front.png");
    box = loadCubemap(faces);
   
    //Create unit cube to splat cube-map texture onto 
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

    //enable explicit attribute array for vertex shader input
    int AA_index=2;
    glEnableVertexAttribArray(AA_index);

    //Generate VBO
    GLuint VBO = createBuffer(GL_ARRAY_BUFFER, vertices, sizeof(GLfloat) * (36*3), GL_STATIC_DRAW); //buffer_tools.cpp

    //Generate VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    attributeBind(VBO, AA_index, 3); //buffer_tools.cpp
    glBindVertexArray(0);
}


GLuint skybox::loadCubemap(vector<const GLchar*> faces)
{
    GLuint tempTexture;
    glGenTextures(1, &tempTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tempTexture);

    int width,height;
    unsigned char* image;
    for(GLuint i = 0; i < faces.size(); i++)
    {
        image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        SOIL_free_image_data(image);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return tempTexture;
}

void skybox::draw()
{
    glUseProgram(skyboxID);                                                                                                                                

    //don't write to Z buffer
    glDepthMask(GL_FALSE); 

    //bind the skybox VAO (sets up all vertex data)
    glBindVertexArray(VAO);

    //bind texture object to cube-map target on texture unit 0 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, box);     

    //draw the sky box
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //turn depth writing on again
    glDepthMask(GL_TRUE); 
    
    //unbind VAO
    glBindVertexArray(0);
    
}
