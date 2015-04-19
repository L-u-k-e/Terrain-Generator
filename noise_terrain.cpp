using namespace std;

class noise_terrain
{
    //buffer object handles
    GLuint vertex_buffer;
    GLuint color_buffer;
    GLuint color_buffer2;
    GLuint element_buffer;

    GLenum usage_hint;

    //structured data to be unpaked and stuffed into the buffer objects
    vector<vec3> vertices;
    vector<vec3> colors;  

    //terrain control parameters
    float min_height;         //minimum height value
    float max_height;         //maximum height value
    float point_spread;       //separate the vertices by X pixels
    float flatness;           //highervalue = flatter terrain. (this is not equivalent to "smoother" terrain.)
    //float persistence       //global hook declared in noise.cpp
    //float octaves           //global hook declared in noise.cpp

    GLenum fill_mode; //display outline or solid color?


    //0: point_spread  --  1: flatness  --  2: octaves  --  3: persistence   
    float *control_variables[6];    //pointers to terrain control parameters themselves
    float control_increments[6];    //increment amounts to use when signaled to change the respective parameters
    float control_bounds[12];       //min and max bounds on the respective parameter values


    int block_size;         //create an X by X grid of vertices(each are "point_spread pixels apart)
    int amount_of_vertices;
    
    int element_buffer_size;      //argument to match COUNT parameter in glDrawElements calls

    float range_map(float value, float r1[], float r2[]);                                                                                                                       
    vec3 getVertexColor(float height);
    void load(void);
    void tessellate(void);
    
    public:
        noise_terrain(void);    
        void printInfo(void);
        void toggle_fill_mode(void);
        void create(glm::vec3 camera_position);
        void draw(void);

        float control_signals[6]; //parallel to the (private) control_* arrays. 
                                  //request an increment through this array   
};



/*
constructor: initializes instance variables
*/
noise_terrain::noise_terrain(void)
{
    usage_hint=GL_STATIC_DRAW;

    int half_height=window_height/2;

    block_size=100;
    amount_of_vertices=block_size*block_size;
    element_buffer_size=(amount_of_vertices-(block_size*2))*6;
    point_spread=50.0;
    flatness=20.0;
    max_height=window_height;
    min_height=-max_height;

    
    //set up control_* arrays
    float hh=half_height;
    float wh=window_height;
    
    float *init_variables[6]  = {&point_spread,   &flatness,   &octaves,   &persistence,   &min_height,   &max_height };                                                                          
    float init_signals[6]     = {      0,             0,           0,           0,             0,             0       };
    float init_increments[6]  = {     1.0,           1.0,         1.0,         0.1,           1.0,           1.0      };
    float init_bounds[12]     = {   1.0,100.0,     1.0,50.0,    1.0,8.0,     0.1,0.9,       -wh,hh,        -hh,wh     };

    copy(init_variables, init_variables+6, control_variables);
    copy(init_signals, init_signals+6, control_signals);
    copy(init_increments, init_increments+6, control_increments);
    copy(init_bounds, init_bounds+12, control_bounds);


    fill_mode=GL_FILL;
}





/*
    Creates terrain based on control parameters.

    Fills "vertices" and "colors" vectors.
    calls load().
*/
void noise_terrain::create(glm::vec3 camera_position)
{
    vertices.clear();
    colors.clear();
    
    float current_range[2]={-1.0, 1.0};
    float desired_range[2]={min_height, max_height};
    
    int seed=200000;

    int centerX=camera_position.x/point_spread;
    int centerY=camera_position.y/point_spread;
    int half_block_size=block_size/2;

    //use perlin_noise_2D(x,y) to grab height values and fill the "vertices" and "colors" vectors
    for(int y=centerY-half_block_size; y<centerY+half_block_size; y++)
    {
        for(int x=centerX-half_block_size; x<centerX+half_block_size; x++)
        {
            float z=perlin_noise_2D((x+seed)/flatness, (y+seed)/flatness);

            vec3 vertex_color=getVertexColor(z);
            cout<<vertex_color.red()<<endl;
            colors.push_back(vertex_color);
            
            z = range_map(z, current_range, desired_range);
            vec3 vertex_position(x*point_spread, y*point_spread, z);
            vertices.push_back(vertex_position);
        }
    }
    
    load();
}



/*
   Creates and fills all buffer objects necessary to draw the terrain
*/
void noise_terrain::load(void)
{
    tessellate(); //create and fill element_buffer
    
    int buffer_size=amount_of_vertices*3;

    GLfloat *vertex_bus = new GLfloat[buffer_size];  //vertices
    GLfloat *color_bus  = new GLfloat[buffer_size];  //vertex colors
    GLfloat *color_bus2 = new GLfloat[buffer_size];  //filled with background color to help draw wireframe
     
    //initialize the arrays
    for (int i=0; i<buffer_size; i++)
    {
        vertex_bus[i] = -10000;  //see vec3::dump_into(array)
        color_bus[i] = -10000;   //see vec3::dump_into(array)
        color_bus2[i] = 0.0;     //fill with black
    }

    //all aboard!
    //fill the vertex and color arrays (soon to be buffers)
    for(int i=0; i<amount_of_vertices; i++)
    {
        vertices[i].dump_into(vertex_bus);
        colors[i].dump_into(color_bus);
    }

    //create and fill the buffers
    glDeleteBuffers(1,&vertex_buffer);
    vertex_buffer = createBuffer(GL_ARRAY_BUFFER, vertex_bus, sizeof(GLfloat)*(buffer_size), usage_hint);

    glDeleteBuffers(1,&color_buffer);
    color_buffer = createBuffer(GL_ARRAY_BUFFER, color_bus, sizeof(GLfloat)*(buffer_size), usage_hint);
        
    glDeleteBuffers(1,&color_buffer2);
    color_buffer2 = createBuffer(GL_ARRAY_BUFFER, color_bus2, sizeof(GLfloat)*(buffer_size), usage_hint);

    delete [] vertex_bus;
    delete [] color_bus;
    delete [] color_bus2;
}


/*
    Fills the element buffer, effectively tessellating the terrain 
    with individual triangles. 
*/
void noise_terrain::tessellate(void) 
{
    GLuint *element_bus = new GLuint[element_buffer_size]; 

    int i=0;
    for(int y=0; y<block_size-1; y++)
    {
        int current_row_offset=y*block_size;
        for(int x=0; x<block_size-1; x++)
        {
            float square[4];
            square[0]=current_row_offset+block_size+x;  //top-left
            square[1]=current_row_offset+x;             //bottom-left
            square[2]=square[1]+1;                      //bottom-right
            square[3]=square[0]+1;                      //top-right
            
            element_bus[i]=square[0];
            i++;
            element_bus[i]=square[1];
            i++;
            element_bus[i]=square[2];
            i++;
            element_bus[i]=square[0];
            i++;
            element_bus[i]=square[2];  
            i++;
            element_bus[i]=square[3];
            i++;
            
        }
    }

    glDeleteBuffers(1, &element_buffer);
    element_buffer = createBuffer(GL_ELEMENT_ARRAY_BUFFER, element_bus, sizeof(GLuint)*element_buffer_size, usage_hint);

    delete [] element_bus;
}



/*
    Binds the appropriate buffers and draws the terrain
*/
void noise_terrain::draw(void)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);

    attributeBind(vertex_buffer, vertex_buffer_attribute_index, 3);
    
    if(fill_mode==GL_FILL)
    {
        attributeBind(color_buffer, 1, 3);
    }
    else
    {
        attributeBind(color_buffer2, 1, 3);
    }

    //draw solid triangles
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, element_buffer_size, GL_UNSIGNED_INT, (void*)0);                                                                                                                                                       

    if(fill_mode==GL_LINE)
    {
        //swap out current color buffer for the true vertex colors and bind it to AA 1
        attributeBind(color_buffer, 1, 3);

        //draw colored/wireframe polygons on top of black ones
        //this works because we increased the Z value of all GL_LINE polygons in the setup stage with glPolygonOffset()
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, element_buffer_size, GL_UNSIGNED_INT, (void*)0);                                                                                                                                                       
    }
}



/*
    Takes a height value between -1 and 1.
    Returns a color based on that height.
*/
vec3 noise_terrain::getVertexColor(float z)                                                                                                                                               
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


/*
    Maps $value, which is currently in range $r1[0]-$r1[1]
    to a new value that is in a new range,   $r2[0]-$r2[1].
    Returns the new value.
*/
float noise_terrain::range_map(float value, float r1[], float r2[])                                                                                                                       
{
    return (value - r1[0]) * (r2[1] - r2[0]) / (r1[1] - r1[0]) + r2[0];    
}


/*
    Prints out terrain control parameter info.
*/
void noise_terrain::printInfo(void)
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
