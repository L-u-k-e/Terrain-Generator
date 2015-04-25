/*
    Author: Lucas Parzych

    This is main, enjoy your stay. 
*/

#include "libraries.hpp"
#include "globals.cpp"
#include "buffer_tools.cpp"
#include "noise.cpp"
#include "vec3.cpp"
#include "noise_terrain.cpp"
#include "camera.cpp"
#include "loadShaders.cpp"
#include "skybox.cpp"
#include "blockManager.cpp"

using namespace std;             


void init(int width, int height); 

void resize(int width, int height); 
void update(void);

void recordMouseMotion(GLint xMouse, GLint yMouse);
void keyDown(GLubyte key, GLint xMouse, GLint yMouse);
void keyUp(GLubyte key, GLint xMouse, GLint yMouse); 




camera cam;
skybox background; 
blockManager terrain;





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
    glutCreateWindow("Terrain Generation");
    
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
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    //set clear color to black
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Create and compile our GLSL program from the shaders
    programID = loadShaders(fragment_shader, vertex_shader);
    matrixID = glGetUniformLocation(programID, "MVP");    

    // Enable attribute array 0 and 1                                                                                                                                      
    glEnableVertexAttribArray(vertex_attribute_loc);
    glEnableVertexAttribArray(color_attribute_loc);

    // Use shader program                                                                                                      
    glUseProgram(programID);
    
    // Use default framebuffer (window) and set viewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_width, window_height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);
    glEnable(GL_POLYGON_OFFSET_FILL); 
    glPolygonOffset(2.0,2.0); 

    terrain.loadBlocks(glm::vec3(0,0,0), 0);
    background.load(cam.Projection);
}























//////////////////////////////////////////////////////         DRAW LOOP         ////////////////////////////////////////////////////////////////////

void update(void)
{   
    glUseProgram(programID);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    cam.update();
    background.update(cam.View);
    terrain.update();

    background.draw();
    terrain.drawBlocks();

    glutSwapBuffers(); 
   // std::this_thread::sleep_for(std::chrono::milliseconds(7));
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

        case 'Y':
            terrain.control_signals[4]=1.0;    //min height
            break;
        case 'y':
            terrain.control_signals[4]=-1.0;
            break;

        case 'T':
            terrain.control_signals[5]=1.0;    //max height
            break;
        case 't':
            terrain.control_signals[5]=-1.0;
            break;

        case 'R':
            terrain.control_signals[6]=1.0;    //seed
            break;
        case 'r':
            terrain.control_signals[6]=-1.0;
            break;

        case 'N':
            terrain.control_signals[7]=1.0;    //radius
            break;
        case 'n':
            terrain.control_signals[7]=-1.0;
            break;

        case 'm':
            terrain.toggleFillMode();
            break;




        case '0':
            terrain.preset(0);
            break;
        case '1':
            terrain.preset(1);
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

        case 'Y':
            terrain.control_signals[4]=0.0;    //min height
            break;
        case 'y':
            terrain.control_signals[4]=0.0;
            break;

        case 'T':
            terrain.control_signals[5]=0.0;    //max height
            break;
        case 't':
            terrain.control_signals[5]=0.0;
            break;

        case 'R':
            terrain.control_signals[6]=0.0;    //seed
            break;
        case 'r':
            terrain.control_signals[6]=0.0;
            break;

        case 'N':
            terrain.control_signals[7]=0.0;    //radius
            break;
        case 'n':
            terrain.control_signals[7]=0.0;
            break;


        default:
            break;
    }
}