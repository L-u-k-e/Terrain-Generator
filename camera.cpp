using namespace std;
class camera
{
    public:
        float yaw, pitch;

        glm::vec3 move_signal;
        float move_signal_speed;  
        float sensitivity; 
        float scroll_speed, scroll_boundary;

        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 head;

        glm::mat4 Model;
        glm::mat4 View; 
        glm::mat4 Projection; 
        glm::mat4 MVP; 

        float fov_y; 

        camera(void);
        void calculateMovementSpeed(void);
        void setupMVP(void);
        void update(void);
};

camera::camera(void)
{
    yaw=-90.0f;
    pitch=0.0f;

    scroll_boundary=25;
    sensitivity = 0.05f;

    position = glm::vec3(0,0,0);
    direction = glm::vec3(0,0,0);
    head = glm::vec3(0,1.0,0);

    Model = glm::mat4(1.0f);
    View = glm::mat4(1.0f);
    Projection = glm::mat4(1.0f);
    MVP = glm::mat4(1.0f);
    fov_y=45.0f;

    move_signal=glm::vec3(0.0,0.0,0.0);

    setupMVP();
}

void camera::setupMVP(void)
{
    //setup view, projection and model matrices

    //model
    Model = glm::rotate(
        Model, 
        glm::radians(-90.0f), 
        glm::vec3(1.0f, 0.0f, 0.0f)
    ); 
    
    //view
    float Z = window_height / (2 * tan(glm::radians(fov_y/2.0)));
    position= glm::vec3(window_width/2, window_height/2, Z); 
    direction= glm::vec3(0,0,-1.0);
    View = glm::lookAt( 
        position, 
        position + direction, 
        head
    );

    //projection
    float aspect_ratio=(float) window_width / (float) window_height;
    Projection = glm::perspective(glm::radians(fov_y), aspect_ratio, 0.1f, 10000.0f);
    
    MVP = Projection * View * Model;
    //glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
}


void camera::calculateMovementSpeed(void)
{
    scroll_speed=((window_width + window_height)/2)/4;
    move_signal_speed=scroll_speed;
}

void camera::update(void)
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

    direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    direction.y = sin(glm::radians(pitch));
    direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    direction = glm::normalize(direction);  

    
    if(move_signal.z!=0.0)
    {
        position -= (move_signal.z * move_signal_speed * direction);
    }

    if(move_signal.x!=0.0)
    {
        position -= (move_signal.x * move_signal_speed) * glm::normalize(glm::cross(head, direction));
    }

    View = glm::lookAt( 
        position, 
        position + direction, 
        head
    );

    MVP = Projection * View * Model;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
}



