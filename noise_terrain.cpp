/*
    Author: Lucas Parzych

    Instances of this class represent terrain blocks. 

    These blocks do not have their own VAO, rather, they rely on a global one. 
    The appropriate vertex buffer objects are stored and re-binded each render pass. 
*/

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
    float seed;
    float flatness;           //highervalue = flatter terrain. (this is not equivalent to "smoother" terrain.)
    //float persistence       //global hook declared in noise.cpp
    //float octaves           //global hook declared in noise.cpp

    GLenum fill_mode; //display outline or solid color?


    glm::vec3 center_position;
    
    
    int block_size; //create an X by X grid of vertices(each are "point_spread pixels apart)

    int amount_of_vertices;
    
    int element_buffer_size; //argument to match COUNT parameter in glDrawElements calls


    float range_map(float value, float r1[], float r2[]);                                                                                                                       
    vec3 getVertexColor(float height);
    void load(void);
    void tessellate(void);
    public:
        noise_terrain(int b, float p, float f, float mah, float mih, float s, GLenum fm);
        void printInfo(void);
        void toggleFillMode(void);
        glm::vec3 neighbor(glm::vec3 relative_location);
        void create(glm::vec3 position);
        void draw(void);
    
        float control_signals[7]; //parallel to the (private) control_* arrays. 
                                  //request an increment through this array   
};



//constructor
noise_terrain::noise_terrain(int b, float p, float f, float mah, float mih, float s, GLenum fm)
{
    block_size= b;
    point_spread= p;
    flatness= f;
    max_height= mah;
    min_height= mih;
    seed= s;
    fill_mode=fm;

    usage_hint= GL_STATIC_DRAW;

    amount_of_vertices= block_size*block_size;
    element_buffer_size= (amount_of_vertices-(block_size*2)+1)*6;   
}






/*
    Creates terrain based on control parameters.

    Fills "vertices" and "colors" vectors.
    calls load().
*/
void noise_terrain::create(glm::vec3 position)
{
    glBindVertexArray(VertexArrayID);   
    center_position=position;

    vertices.clear();
    colors.clear();
    
    float current_range[2]={-1.0, 1.0};
    float desired_range[2]={min_height, max_height};
    
    int centerX=center_position.x/point_spread;
    int centerY=center_position.y/point_spread;
    int half_block_size=block_size/2;

    //use perlin_noise_2D(x,y) to grab height values and fill the "vertices" and "colors" vectors
    for(int y=centerY-half_block_size; y<centerY+half_block_size; y++)
    {
        for(int x=centerX-half_block_size; x<centerX+half_block_size; x++)
        {
            float z=perlin_noise_2D((x+seed)/flatness, (y+seed)/flatness);

            vec3 vertex_color=getVertexColor(z);
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
    glBindVertexArray(VertexArrayID);   
    tessellate(); //create and fill element_buffer
    int buffer_size=amount_of_vertices*3;

     GLfloat vertex_bus[buffer_size];  //vertices
     GLfloat color_bus[buffer_size];   //vertex colors
     GLfloat color_bus2[buffer_size];  //filled with background color to help draw wireframe

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
}





/*
    Fills the element buffer, effectively tessellating the terrain 
    with individual triangles. 
*/
void noise_terrain::tessellate(void) 
{
    glBindVertexArray(VertexArrayID);   
    GLuint element_bus[element_buffer_size];
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
}




//   Binds the appropriate buffers and draws the terrain
void noise_terrain::draw(void)
{
    glUseProgram(programID);
    glBindVertexArray(VertexArrayID);   

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);

    attributeBind(vertex_buffer, vertex_attribute_loc, 3);
    
    if(fill_mode==GL_FILL)
    {
        attributeBind(color_buffer, color_attribute_loc, 3);
    }
    else
    {
        attributeBind(color_buffer2, color_attribute_loc, 3);
    }

    //draw solid triangles
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, element_buffer_size, GL_UNSIGNED_INT, (void*)0);                                                                                                                                                       

    if(fill_mode==GL_LINE)
    {
        //swap out current color buffer for the true vertex colors and bind it to AA 1
        attributeBind(color_buffer, color_attribute_loc, 3);

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
    float red;
    float blue;
    float green;
    float contrast_offset= 0.2;
    if(z >= -1.0 && z <= -0.2) 
    {
        green=0.0f; 
        red = (1.0f + z)-contrast_offset;
        blue = abs(z);
    }
    else if(z <= 0)
    {
        green=abs(z);
        red=(1.0+z)-contrast_offset;
        blue=abs(z);
    }
    else if(z <= 0.2) 

    {
        blue=0.0f;
        green=z+contrast_offset;
        red=1.0f-(z);
    }
    else if(z<=0.68)
    {
        blue=z-contrast_offset;
        green=z+contrast_offset;
        red=1.0f-z;
    }
    else
    {
        blue=z-contrast_offset;
        green=z;
        red=1.0f-(z-contrast_offset);
    }
    vertex_color=vec3(red,green,blue);
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
    
    cout<<"Min Height: ";
    cout<<min_height;

    cout<<"\tMax Height: ";
    cout<<max_height;

    cout<<"\t\tseed: ";
    cout<<seed;

    cout<<endl;
    cout<<endl;
}




/*
    Switches the fill mode for this terrain block.
*/
void noise_terrain::toggleFillMode(void)
{
    if(fill_mode==GL_LINE)
    {
        fill_mode=GL_FILL;
    }
    else
    {
        fill_mode=GL_LINE;
    }
}




/*
    Takes a vec3 indicating the x/y position of a block relative to this one.
    Returns the center position that block. 

    E.G. neighbor(glm::vec2(1,1)) will give you the center position of the block
    in front/to the right of this one. 

    Relies on a constant block size.
*/
glm::vec3 noise_terrain::neighbor(glm::vec3 position)
{
    glm::vec3 neighbor_position = center_position;

    int true_block_size = (block_size*point_spread)-point_spread;
    
    neighbor_position.x+=(true_block_size*position.x);
    neighbor_position.y+=(true_block_size*position.y);

    return neighbor_position;
}
