#include "include.hpp"
#include "noise.cpp"
#include "vec3.cpp"

using namespace std;             

////////////////////////////////////////////////////     PROTOTYPES     ////////////////////////////////////////////////////////////////////

void attributeBind(GLuint buffer, int index, int points);
static GLuint createBuffer( GLenum target,  
                            const void *buffer_data,   
                            GLsizei buffer_size,   
                            GLenum usageHint 
);

void init(int width, int height); 
GLuint loadShaders(void);

void resize(int width, int height); 
void update(void);

void recordMouseMotion(GLint xMouse, GLint yMouse);
void keyDown(GLubyte key, GLint xMouse, GLint yMouse);
void keyUp(GLubyte key, GLint xMouse, GLint yMouse); 
void processUserInput(void);

float range_map(float value, float current_range[], float desired_range[]);
void swapVec3(vec3 *a, vec3 *b);
vec3 getVertexColor(float z);

void updateCamera(void);
void setupCamera(void);

void drawTerrain(vector<vector<vec3>> vertices, vector<vector<vec3>> colors);
void createTerrain(void);
void printTerrainInfo();

void calculateMovementSpeed(void);

//////////////////////////////////////////////////////          GLOBALS          ////////////////////////////////////////////////////////////////////
int window_width=600;
int window_height=600;

const char* fragment_shader = "fragment_shader.glsl";
const char* vertex_shader   = "vertex_shader.glsl"; 

float mouseX, mouseY;
float lastX, lastY;
float sensitivity = 0.05f;
float yaw, pitch;
float scroll_speed;
float scroll_boundary=25;


GLuint programID;                     
GLuint matrixID;   

GLuint vertex_buffer;
GLuint color_buffer;

glm::vec3 camera_position(0,0,0);
glm::vec3 camera_direction(0,0,0);
glm::vec3 camera_head(0,1.0,0);

glm::mat4 Model = glm::mat4(1.0f);
glm::mat4 View = glm::mat4(1.0f);
glm::mat4 Projection = glm::mat4(1.0f);
glm::mat4 MVP = glm::mat4(1.0f);

glm::vec3 movement(0.0,0.0,0.0);
float movement_speed;

float flatness=20.0;//highervalue = flatter terrain. (this is not equivalent to "smoother" terrain.)
float point_spread = 50;

GLenum fill_mode=GL_LINE;
bool toggle_fill_mode=false;

vector<vector<vec3>> vertices;
vector<vector<vec3>> colors;

float *terrain_control_variables[4]  = {&point_spread,   &flatness,   &octaves,   &persistence };
float terrain_control_signals[4]     = {      0,             0,           0,           0       };
float terrain_control_increments[4]  = {     1.0,           1.0,         1.0,         0.1      };
float terrain_control_bounds[8]      = {   1.0,100.0,     1.0,50.0,    1.0,8.0,     0.1,0.9    };

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

    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);

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
    
    //setup mouse position
    mouseX = window_width/2;
    mouseY = window_height/2;
    lastX = mouseX;
    lastY = mouseY;
    yaw=-90.0f;
    pitch=0.0f;

    //tell glut to ignore multiple keyboard callbacks when holding down a key
    glutIgnoreKeyRepeat(1); 

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

    setupCamera();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);
    glEnable(GL_POLYGON_OFFSET_FILL); 
    glPolygonOffset(2.0,2.0); 

    createTerrain();
    printTerrainInfo(); 
}





//////////////////////////////////////////////////////         DRAW LOOP         ////////////////////////////////////////////////////////////////////

void update(void)
{   
    processUserInput();
    updateCamera();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear screen

//    createTerrain();
    drawTerrain(vertices, colors);
    
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

    calculateMovementSpeed();
}


////////////////////////////////////////////////////// MOUSE/KEYBOARD CALLBACKS  ////////////////////////////////////////////////////////////////


void recordMouseMotion(GLint xMouse, GLint yMouse)
{
    mouseX=xMouse;
    mouseY=window_height - yMouse;
    //---------------------------------------
    float xoffset = xMouse - lastX;
    float yoffset = lastY - yMouse; // Reversed since y-coordinates range from bottom to top
    lastX = xMouse;
    lastY = yMouse;
    
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   +=  xoffset;
    pitch +=  yoffset; 
}

void keyDown(GLubyte key, GLint xMouse, GLint yMouse)
{
    switch (key) 
    {
        case 27:  // ESCAPE key! Time to quit!
            exit(0);
            break;

        case 'w':
            if(!movement.z) movement.z=-1.0;
            break;

        case 's':
            if(!movement.z) movement.z=1.0;
            break;

        case 'a':
            if(!movement.x) movement.x=-1.0;
            break;

        case 'd':
            if(!movement.x) movement.x=1.0;
            break;



        case 'u':
            terrain_control_signals[0]=-1.0;    //point spread
            break;
        case 'U':
            terrain_control_signals[0]=1.0;
            break;

        case 'i':
            terrain_control_signals[1]=-1.0;    //flatness
            break;
        case 'I':
            terrain_control_signals[1]=1.0;
            break;

        case 'o':
            terrain_control_signals[2]=-1.0;    //octaves
            break;
        case 'O':
            terrain_control_signals[2]=1.0;
            break;

        case 'p':
            terrain_control_signals[3]=-1.0;   //persistence
            break;
        case 'P':
            terrain_control_signals[3]=1.0;
            break;

        case 'm':
            //toggle_fill_mode=true;
            createTerrain();
            break;

        default:
            break;
    } 
}

void keyUp(GLubyte key, GLint xMouse, GLint yMouse)
{
    switch (key) 
    {
        case 27:  // ESCAPE key! Time to quit!
            exit(0);
            break;

        case 'w':
            if(movement.z == -1.0) movement.z=0;
            break;

        case 's':
            if(movement.z == 1.0) movement.z=0;
            break;

        case 'a':
            if(movement.x == -1.0) movement.x=0;
            break;

        case 'd':
            if(movement.x == 1.0) movement.x=0;
            break;

        case 'u':
            terrain_control_signals[0]=0.0;    //point spread
            break;
        case 'U':
            terrain_control_signals[0]=0.0;
            break;

        case 'i':
            terrain_control_signals[1]=0.0;    //flatness
            break;
        case 'I':
            terrain_control_signals[1]=0.0;
            break;

        case 'o':
            terrain_control_signals[2]=0.0;    //octaves
            break;
        case 'O':
            terrain_control_signals[2]=0.0;
            break;

        case 'p':
            terrain_control_signals[3]=0.0;   //persistence
            break;
        case 'P':
            terrain_control_signals[3]=0.0;
            break;


        default:
            break;
    }

}

//////////////////////////////////////////////////////   PERSONAL ABSTRACTIONS   ////////////////////////////////////////////////////////////////////
void calculateMovementSpeed(void)
{
    scroll_speed=((window_width + window_height)/2)/4;
    movement_speed=scroll_speed;
}
void processUserInput(void)
{
    if(toggle_fill_mode)
    {
        if(fill_mode==GL_LINE)
        {
            fill_mode=GL_FILL;
        }
        else fill_mode=GL_LINE;

        toggle_fill_mode=false;
    }
    
    bool new_terrain_required=false;
    for(int i=0; i<4; i++)
    {
        float *variable=terrain_control_variables[i];
        float min=terrain_control_bounds[i*2];
        float max=terrain_control_bounds[(i*2)+1];
        if(terrain_control_signals[i]!=0.0)
        {
            new_terrain_required=true;
        }
        
        *variable += terrain_control_signals[i] * terrain_control_increments[i];

        if(*variable<min) *variable=min;
        else if(*variable>max) *variable=max;
        
        terrain_control_signals[i]=0.0;
    }

/*  point_spread+=(terrain_control_signal[0]*terrain_control_increment[0]);
    flatness+=(terrain_control_signal[1]*terrain_control_increment[1]);
    octaves+=(terrain_control_signal[2]*terrain_control_increment[2]);
    persistence+=(terrain_control_signal[3]*terrain_control_increment[3]);
*/
    if(new_terrain_required)
    {
        createTerrain();
        printTerrainInfo(); 
    }
}

void updateCamera(void)
{

    if(mouseX>window_width-scroll_boundary)
    {
        yaw += scroll_speed * sensitivity;
    }
    else if(mouseX < scroll_boundary)
    {
        yaw -= scroll_speed * sensitivity;
    }

    if(mouseY>window_height-scroll_boundary)
    {
        pitch += scroll_speed * sensitivity;
    }
    else if(mouseY < scroll_boundary)
    {
        pitch -= scroll_speed * sensitivity;
    }
    

    if(pitch > 89.0f)
    {
        pitch = 89.0f;
    }
    if(pitch < -89.0f)
    {
        pitch = -89.0f;
    }

    camera_direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    camera_direction.y = sin(glm::radians(pitch));
    camera_direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    camera_direction = glm::normalize(camera_direction);  

    
    if(movement.z!=0.0)
    {
        camera_position -= (movement.z * movement_speed * camera_direction);
    }

    if(movement.x!=0.0)
    {
        camera_position -= (movement.x * movement_speed) * glm::normalize(glm::cross(camera_head, camera_direction));
    }

    View = glm::lookAt( 
        camera_position, 
        camera_position + camera_direction, 
        camera_head
    );

    MVP = Projection * View * Model;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
}











//-----------------------------------------------------------------------------------------------


void createTerrain(void)
{
    int seed=200000;
    vertices.clear();
    colors.clear();
    float current_range[2]={-1.0, 1.0};
    float desired_range[2]={(float) -window_height, (float)window_height};
    
    int w=(window_width/point_spread);
    int h=(window_height/point_spread);
    int block_size=100;
    int half_block_size=block_size/2;
    int centerX=camera_position.x/point_spread;
    int centerY=camera_position.y/point_spread;

    for(int y=centerY-half_block_size; y<centerY+half_block_size; y++)
    {
        vector<vec3> current_strip;
        vector<vec3> current_colors;
        for(int x=centerX-half_block_size; x<centerX+half_block_size; x++)
        {
            int xVal=x;
            for(int i=0;i<2;i++)
            {
                int yVal=y+i;
                float z=perlin_noise_2D((xVal+seed)/flatness, (yVal+seed)/flatness);

                vec3 vertex_color=getVertexColor(z);
                current_colors.push_back(vertex_color);
                
                z = range_map(z, current_range, desired_range);
                vec3 vertex_position(xVal*point_spread, yVal*point_spread, z);
                current_strip.push_back(vertex_position);
            }
        }
        vertices.push_back(current_strip);
        colors.push_back(current_colors);
    }
}


void drawTerrain(vector<vector<vec3>> vertices, vector<vector<vec3>> colors)
{
    for(int i=0; i<vertices.size(); i++)
    {
        int s=vertices[i].size();
        int buffer_size=s*3;

        GLfloat *vert = new GLfloat[buffer_size];  //vertices
        GLfloat *col = new GLfloat[buffer_size];   //vertex colors
        GLfloat *col2 = new GLfloat[buffer_size];  //filled with background color to help draw wireframe
        
        for (int j=0; j<buffer_size; j++) //intialize the arrays
        {
            vert[j] = -10000;  //see vec3::dump_into(array)
            col[j] = -10000;   //see vec3::dump_into(array)
            col2[j] = 0.0;     //fill with black
        }

        for(int j=0; j<s; j++) //fill the vertex and color arrays (soon to be buffers)
        {
            vertices[i][j].dump_into(vert);
            colors[i][j].dump_into(col);
        }

        //create vertex buffer and bind to AA 0
        glDeleteBuffers(1,&vertex_buffer);
        vertex_buffer = createBuffer(GL_ARRAY_BUFFER, vert, sizeof(GLfloat)*(buffer_size), GL_STATIC_DRAW);
        attributeBind(vertex_buffer, 0, 3);

        //create color buffer based on fill mode  and bind to AA 1.
        glDeleteBuffers(1,&color_buffer);
        if(fill_mode==GL_FILL)
        {
            color_buffer = createBuffer(GL_ARRAY_BUFFER, col, sizeof(GLfloat)*(buffer_size), GL_STATIC_DRAW);
        }
        else
        {
            color_buffer = createBuffer(GL_ARRAY_BUFFER, col2, sizeof(GLfloat)*(buffer_size), GL_STATIC_DRAW);
        }
        attributeBind(color_buffer, 1, 3);
       
        //draw polygons, colored based on fill mode
        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
        glDrawArrays(GL_TRIANGLE_STRIP,0,s);
        
       // glClear(GL_DEPTH_BUFFER_BIT);
        if(fill_mode==GL_LINE)
        {
            //increase vertex buffer z coords
           /* for(int j=1; j<buffer_size; j+=3)
            {
                vert[j]-=0.0;
            }

            //create vertex buffer and bind to AA 0
            glDeleteBuffers(1,&vertex_buffer);
            vertex_buffer = createBuffer(GL_ARRAY_BUFFER, vert, sizeof(GLfloat)*(buffer_size), GL_STATIC_DRAW);
            attributeBind(vertex_buffer, 0, 3);
*/
            //swap out current color buffer for the true vertex colors and bind it to AA 1
            glDeleteBuffers(1,&color_buffer);
            color_buffer = createBuffer(GL_ARRAY_BUFFER, col, sizeof(GLfloat)*(buffer_size), GL_STATIC_DRAW);
            attributeBind(color_buffer, 1, 3);

            //draw colored/wireframe polygons on top of invisible ones
            //This works because we increased the Z value of all GL_LINE polygons in the setup stage with glPolygonOffset()
            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
            glDrawArrays(GL_TRIANGLE_STRIP,0,s);
        }
        //clean up after yourself you savage
        delete [] vert;
        delete [] col;
        delete [] col2;
    }
}

vec3 getVertexColor(float z)
{
    vec3 vertex_color;
    if(z>0.45)
    {
        vertex_color=vec3(1.0,1.0,1.0);
    }
    else if(z>0.15)
    {
        vertex_color=vec3(0.4,0.4,0.4);
    }
    else if(z>-0.3)
    {
        vertex_color=vec3(0.0,1.0,0.0);
    }
    else
    {
        vertex_color=vec3(0.0,0.0,1.0);
    }
    return vertex_color;
}

float range_map(float value, float r1[], float r2[])
{
    //r1 = current range
    //r2 = desired range
    return (value - r1[0]) * (r2[1] - r2[0]) / (r1[1] - r1[0]) + r2[0];                                                                                                
}

void printTerrainInfo()
{
    cout<<endl;
    cout<<endl;
    cout<<"Octaves: ";
    cout<<octaves;
    cout<<"\tPersistence: ";
    cout<<persistence;
    cout<<"\tFlatness: ";
    cout<<flatness;
    cout<<"\tPoint Spread: ";
    cout<<point_spread;
    cout<<endl;
    cout<<endl;
}




void attributeBind(GLuint buffer, int index, int points)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(
        index,              // position or color 
        points,             // how many dimensions? 
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

//-----------------------------------------------------------------------------------------------

void setupCamera(void)
{
    //setup view, projection and model matrices
    float fov_y = 45.0f;

    //model
    Model = glm::rotate(
        Model, 
        glm::radians(-90.0f), 
        glm::vec3(1.0f, 0.0f, 0.0f)
    ); 
    
    //view
    float Z = window_height / (2 * tan(glm::radians(fov_y/2.0)));
    camera_position= glm::vec3(window_width/2, window_height/2, Z); 
    camera_direction= glm::vec3(0,0,-1.0);
    View = glm::lookAt( 
        camera_position, 
        camera_position + camera_direction, 
        camera_head
    );

    //projection
    float aspect_ratio=(float) window_width / (float) window_height;
    Projection = glm::perspective(glm::radians(fov_y), aspect_ratio, 0.1f, 10000.0f);
    
    MVP = Projection * View * Model;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
}


//-----------------------------------------------------------------------------------------------







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
