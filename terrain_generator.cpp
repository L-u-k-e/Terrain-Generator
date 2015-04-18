#include "libraries.hpp"
#include "globals.cpp"
#include "noise.cpp"
#include "vec3.cpp"
#include "terrain.cpp"
#include "camera.cpp"
#include "mouse_event_handler.cpp"
#include "keyboard_event_handler.cpp"
#include "loadShaders.cpp"


using namespace std;             

////////////////////////////////////////////////////     PROTOTYPES     ////////////////////////////////////////////////////////////////////

void attributeBind(GLuint buffer, int index, int points);
static GLuint createBuffer( GLenum target,  
                            const void *buffer_data,   
                            GLsizei buffer_size,   
                            GLenum usageHint 
);

void init(int width, int height); 

void resize(int width, int height); 
void update(void);

void recordMouseMotion(GLint xMouse, GLint yMouse);
void keyDown(GLubyte key, GLint xMouse, GLint yMouse);
void keyUp(GLubyte key, GLint xMouse, GLint yMouse); 
void processUserInput(void);

float range_map(float value, float current_range[], float desired_range[]);
void swapVec3(vec3 *a, vec3 *b);
vec3 getVertexColor(float z);

void drawTerrain(vector<vector<vec3>> vertices, vector<vector<vec3>> colors);
void createTerrain(void);
void printTerrainInfo();

void calculateMovementSpeed(void);

//////////////////////////////////////////////////////          GLOBALS          ////////////////////////////////////////////////////////////////////


const char* fragment_shader = "fragment_shader.glsl";
const char* vertex_shader   = "vertex_shader.glsl"; 

GLuint vertex_buffer;
GLuint color_buffer;

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

camera cam;
//////////////////////////////////////////////////////        MAIN & INIT       ////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) 
{ 
    cout<<cam.head.y<<endl;
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
    programID = loadShaders(fragment_shader, vertex_shader);

    // Enable attribute array 0 and 1                                                                                                                                      
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Use shader program                                                                                                      
    glUseProgram(programID);
    
    // Use default framebuffer (window) and set viewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_width, window_height);

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
    cam.update();

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

    cam.calculateMovementSpeed();
}


////////////////////////////////////////////////////// MOUSE/KEYBOARD CALLBACKS  ////////////////////////////////////////////////////////////////


void recordMouseMotion(GLint xMouse, GLint yMouse)
{
    mouseX=xMouse;
    mouseY=window_height - yMouse;
 
    float xoffset = xMouse - lastX;
    float yoffset = lastY - yMouse; // Reversed since y-coordinates range from bottom to top
    lastX = xMouse;
    lastY = yMouse;
    
    xoffset *= cam.sensitivity;
    yoffset *= cam.sensitivity;

    cam.yaw   +=  xoffset;
    cam.pitch +=  yoffset; 
}

void keyDown(GLubyte key, GLint xMouse, GLint yMouse)
{
    switch (key) 
    {
        case 27:  // ESCAPE key! Time to quit!
            exit(0);
            break;

        case 'w':
            if(!cam.move_signal.z) cam.move_signal.z=-1.0;
            break;

        case 's':
            if(!cam.move_signal.z) cam.move_signal.z=1.0;
            break;

        case 'a':
            if(!cam.move_signal.x) cam.move_signal.x=-1.0;
            break;

        case 'd':
            if(!cam.move_signal.x) cam.move_signal.x=1.0;
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
            if(cam.move_signal.z == -1.0) cam.move_signal.z=0;
            break;

        case 's':
            if(cam.move_signal.z == 1.0) cam.move_signal.z=0;
            break;

        case 'a':
            if(cam.move_signal.x == -1.0) cam.move_signal.x=0;
            break;

        case 'd':
            if(cam.move_signal.x == 1.0) cam.move_signal.x=0;
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

    if(new_terrain_required)
    {
        createTerrain();
        printTerrainInfo(); 
    }
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
    int centerX=cam.position.x/point_spread;
    int centerY=cam.position.y/point_spread;

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


