#include "libraries.hpp"
#include "globals.cpp"
#include "buffer_tools.cpp"
#include "noise.cpp"
#include "vec3.cpp"
#include "noise_terrain.cpp"
#include "camera.cpp"
#include "mouse_event_handler.cpp"
#include "keyboard_event_handler.cpp"
#include "loadShaders.cpp"
#include "skybox.cpp"

using namespace std;             

////////////////////////////////////////////////////     PROTOTYPES     ////////////////////////////////////////////////////////////////////

void init(int width, int height); 

void resize(int width, int height); 
void update(void);

void recordMouseMotion(GLint xMouse, GLint yMouse);
void keyDown(GLubyte key, GLint xMouse, GLint yMouse);
void keyUp(GLubyte key, GLint xMouse, GLint yMouse); 
void processUserInput(void);

void swapVec3(vec3 *a, vec3 *b);

//////////////////////////////////////////////////////          GLOBALS          ////////////////////////////////////////////////////////////////////


camera cam;
noise_terrain terrain;
noise_terrain terrain2;
noise_terrain terrain3;
noise_terrain terrain4;
noise_terrain terrain5;
noise_terrain terrain6;
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
    textureID = loadShaders(texture_fragment_shader, texture_vertex_shader);

    vertex_attribute_loc = glGetAttribLocation(programID, "position");                                                                                                     
    color_attribute_loc = glGetAttribLocation(programID, "color");

    tex_vertex_attribute_loc = glGetAttribLocation(textureID, "position");                                                                                 
    tex_UV_attribute_loc = glGetAttribLocation(textureID, "texcoords");


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

    terrain.create(cam.position);
    terrain2.create(terrain.neighbor(glm::vec3(1,0,0)));
    terrain3.create(terrain.neighbor(glm::vec3(1,1,0)));
    terrain4.create(terrain.neighbor(glm::vec3(0,1,0)));
    terrain5.create(terrain.neighbor(glm::vec3(-1,1,0)));
    terrain6.create(terrain.neighbor(glm::vec3(-1,0,0)));

       glEnableVertexAttribArray(vertex_attribute_loc);                                                                                                                 
       glEnableVertexAttribArray(color_attribute_loc);

}





//////////////////////////////////////////////////////         DRAW LOOP         ////////////////////////////////////////////////////////////////////

void update(void)
{   
    processUserInput();
    cam.update();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear screen

    terrain.draw();
    terrain2.draw();
    terrain3.draw();
    terrain4.draw();
    terrain5.draw();
    terrain6.draw();
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
            terrain.control_signals[0]=-1.0;    //point spread
            break;
        case 'U':
            terrain.control_signals[0]=1.0;
            break;

        case 'i':
            terrain.control_signals[1]=-1.0;    //flatness
            break;
        case 'I':
            terrain.control_signals[1]=1.0;
            break;

        case 'o':
            terrain.control_signals[2]=-1.0;    //octaves
            break;
        case 'O':
            terrain.control_signals[2]=1.0;
            break;

        case 'p':
            terrain.control_signals[3]=-1.0;   //persistence
            break;
        case 'P':
            terrain.control_signals[3]=1.0;
            break;

        case 'm':
            terrain.toggleFillMode();
            //createTerrain();
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
            terrain.control_signals[0]=0.0;    //point spread
            break;
        case 'U':
            terrain.control_signals[0]=0.0;
            break;

        case 'i':
            terrain.control_signals[1]=0.0;    //flatness
            break;
        case 'I':
            terrain.control_signals[1]=0.0;
            break;

        case 'o':
            terrain.control_signals[2]=0.0;    //octaves
            break;
        case 'O':
            terrain.control_signals[2]=0.0;
            break;

        case 'p':
            terrain.control_signals[3]=0.0;   //persistence
            break;
        case 'P':
            terrain.control_signals[3]=0.0;
            break;


        default:
            break;
    }

}

//////////////////////////////////////////////////////   PERSONAL ABSTRACTIONS   ////////////////////////////////////////////////////////////////////


void processUserInput(void)
{
    /*if(toggle_fill_mode)
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
    }*/
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


