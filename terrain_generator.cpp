#include "noise.cpp" //this file contains the perlin noise function and also all headers

using namespace std;             

////////////////////////////////////////////////////     STRUCTS      ////////////////////////////////////////////////////////////////////

struct vec3
{
    GLfloat x,y,z;
    vec3()
    {
        x=0;
        y=0;
        z=0;
    }
    vec3(GLfloat a, GLfloat b, GLfloat c)
    {
        x=a;
        y=b;
        z=c;
    }

    void dump_into(GLfloat data[])
    {
        //treats any value<0 as null
        //use with caution, dummy function might walk off the edge of the cliff. 
        int pos=0;
        while(true)
        {
            if(data[pos]>=0)
            {
                pos++;
            }
            else break;
        }
        data[pos]=x;
        data[pos+1]=y;
        data[pos+2]=z;
    }

    void print(void)
    {
        cout<<"\nX:\t";
        cout<<x;
        cout<<"\nY:\t";
        cout<<y;
        cout<<"\nZ:\t";
        cout<<z;
        cout<<"\n";
    }

};

struct rgb
{
    GLfloat red, green, blue;
    rgb(GLfloat r, GLfloat g, GLfloat b)
    {
        red=r;
        green=g;
        blue=b;
    }
    rgb()
    {
    } 
};


////////////////////////////////////////////////////     PROTOTYPES     ////////////////////////////////////////////////////////////////////

void attributeBind(GLuint buffer, int index, int points);
static GLuint createBuffer( GLenum target,  
                            const void *buffer_data,   
                            GLsizei buffer_size,   
                            GLenum usageHint 
);

void init(int width, int height); 

void update(void);
void resize(int width, int height); 
float range_map(float value, float current_range[], float desired_range[]);
void recordMouseMotion(GLint xMouse, GLint yMouse);

void swapVec3(vec3 *a, vec3 *b);
GLuint loadShaders(void);
//////////////////////////////////////////////////////          GLOBALS          ////////////////////////////////////////////////////////////////////

int window_width=600;
int window_height=600;

const char* fragment_shader = "fragment_shader.glsl";
const char* vertex_shader   = "vertex_shader.glsl"; 

int mouseX, mouseY;

GLuint programID;                     
GLuint matrixID;   

GLuint vertex_buffer;
GLuint color_buffer;

//////////////////////////////////////////////////////        MAIN & INIT       ////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) 
{  
    glutInit(&argc, argv);   
    
    init(window_width,window_height);

    glutReshapeFunc(resize);
    glutDisplayFunc(update);
    glutIdleFunc(update);

    glutMotionFunc(recordMouseMotion);
    glutPassiveMotionFunc(recordMouseMotion);

    glutMainLoop();
    return 0;
}

void init(int width, int height) 
{
    // Set display mode
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Set top-left display-window position
    glutInitWindowPosition(50, 100);

    // Set display-window width and height
    // Note that the aspect ratio matches what we did with 
    // gluOrtho2D; HOWEVER, it COULD have been different
    glutInitWindowSize(width, height);

    // Create display window
    glutCreateWindow("Assignment 1- Bresenham's algorithm");

    // Initializing GLEW
    GLenum err = glewInit();
    cout << "GLEW initialized; version " << glewGetString(GLEW_VERSION) << endl;

    // Create Vertex Array Object (VAO)
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    //set clear color to black
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    // Create and compile our GLSL program from the shaders
    programID = loadShaders();

    // Enable attribute array 0 and 1                                                                                                                                      
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Use shader program                                                                                                      
    glUseProgram(programID);
    
    // Use default framebuffer (window) and set viewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_width, window_height);

}





//////////////////////////////////////////////////////         DRAW LOOP         ////////////////////////////////////////////////////////////////////

void update(void)
{   
    //                                 MVP Transformations
    //------------------------------------------------------------------------------------------
   // glm::mat4 Projection = glm::ortho(0.0f, (float) window_width, 0.0f, (float) window_height);
    float fov=45.0f;
    glm::mat4 Projection = glm::perspective(glm::radians(fov), (GLfloat) window_width / (GLfloat) window_height, 0.1f, 10000.0f);
    
    float tan_me = fov/2.0;
    float Z = window_height / (2 * tan(glm::radians(tan_me)));
    glm::mat4 View = glm::lookAt(
        glm::vec3(window_width/2, window_height/2, Z), // camera position
        glm::vec3(window_width/2, window_height/2, 0), // look at origin
        glm::vec3(0, 1, 0)  // Head is up
    );  
    
     glm::mat4 Model = glm::mat4(1.0f);
     //Model = glm::rotate(Model, -90.0f, glm::vec3(1.0f, 0.0f, 0.0f)); 

    // Create Model-View-Projection matrix
     glm::mat4 MVP = Projection * View * Model;
        
    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    /* 
    glm::mat4 View;
    View = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f), 
            glm::vec3(0.0f, 0.0f, 0.0f), 
            glm::vec3(0.0f, 1.0f, 0.0f));
*/

    //------------------------------------------------------------------------------------------------  
    //                                  Clear Screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    //------------------------------------------------------------------------------------------------
    //                                  Draw Stuff
    int point_spread = 50;
    vector<vec3> vertices;
    vector<vec3> colors;
    vertices.push_back(vec3(0,0,0));
    vertices.push_back(vec3(0,window_height,0));
    vertices.push_back(vec3(window_width,0,0));
    vertices.push_back(vec3(window_width,window_height,0));
    colors.push_back(vec3(1.0, 0.0, 0.0));
    colors.push_back(vec3(1.0, 0.0, 1.0));
    colors.push_back(vec3(0.0, 1.0, 0.0));
    colors.push_back(vec3(0.0, 0.0, 1.0));
    
    /*
    for(int y=0; y<(window_height / point_spread); y++)
    {
        for(int x=0; x<(window_width / point_spread); x++)
        {
            float z = perlin_noise_2D(x/10,y/10);
            vertices.push_back(vec3(x*point_spread,y*point_spread,z));
            colors.push_back(vec3(0.2,0.3,1.0));
            cout<<z<<endl;
        }
    }*/
    

    int s=vertices.size();
    int s2= s*3;
    GLfloat *vert = new GLfloat[s2];
    GLfloat *col = new GLfloat[s2];
    for (int i=0; i<s2; i++) 
    {
        vert[i] = -0.1; 
        col[i] = -0.1;
    }

    for(int i=0; i<s; i++)
    {
        vertices[i].dump_into(vert);
        colors[i].dump_into(col);
    }

    glDeleteBuffers(1,&vertex_buffer);
    vertex_buffer = createBuffer(GL_ARRAY_BUFFER, vert, sizeof(GLfloat)*(s2), GL_STATIC_DRAW);
    attributeBind(vertex_buffer, 0, 3);

    glDeleteBuffers(1,&color_buffer);
    color_buffer = createBuffer(GL_ARRAY_BUFFER, col, sizeof(GLfloat)*(s2), GL_STATIC_DRAW);
    attributeBind(color_buffer, 1, 3);

    glDrawArrays(GL_TRIANGLE_STRIP,0,s);

    delete [] vert;
    delete [] col;
    //------------------------------------------------------------------------------------------------
    glutSwapBuffers(); //swap buffers
    //------------------------------------------------------------------------------------------------
    std::this_thread::sleep_for(std::chrono::milliseconds(7)); //sleep
    //------------------------------------------------------------------------------------------------
}

void resize(int width, int height) 
{
    // Change the viewport; this is in WINDOW COORDINATES
    glViewport(0, 0, width, height);

    // Store current window width and height
    window_width = width;
    window_height = height;
}


////////////////////////////////////////////////////// MOUSE/KEYBOARD CALLBACKS  ////////////////////////////////////////////////////////////////


void recordMouseMotion(GLint xMouse, GLint yMouse)
{
    mouseX=xMouse;
    mouseY=window_height - yMouse;
}


//////////////////////////////////////////////////////   PERSONAL ABSTRACTIONS   ////////////////////////////////////////////////////////////////////

float range_map(float value, float r1[], float r2[])
{
    //r1 = current range
    //r2 = desired range
    return (value - r1[0]) * (r2[1] - r2[0]) / (r1[1] - r1[0]) + r2[0];                                                                                                
}

void attributeBind(GLuint buffer, int index, int points)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(
        index,              // position or color 
        points,             // points 
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );
}


GLuint createBuffer(GLenum target, const void *buffer_data, GLsizei buffer_size, GLenum usageHint) 
{
    GLuint buffer;
    glGenBuffers(1, &buffer); 
    glBindBuffer(target, buffer);
    glBufferData(target, buffer_size, buffer_data, usageHint);
    return buffer;
}


void swapVec3(vec3 *a, vec3 *b)
{
    vec3 temp(a->x,a->y,a->z);

    a->x=b->x;
    a->y=b->y;
    a->z=b->z;

    b->x=temp.x;
    b->y=temp.y;
    b->z=temp.z;
}


GLuint loadShaders(void)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_shader, std::ios::in);
    if (VertexShaderStream.is_open())
    {
        std::string Line = "";
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_shader, std::ios::in);
    if (FragmentShaderStream.is_open()){
        std::string Line = "";
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_shader);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);     
    
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);                                                                                                             
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_shader);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage(max(InfoLogLength, int(1)));
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}
