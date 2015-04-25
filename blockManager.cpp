/*
    Author: Lucas Parzych

    This class is (currently) a light weight wrapper around the 
    noise_terrain class.
    
    noise_terrain objects are meant to serve as blocks (subsections) of
    a larger and more complete representation of the landscape. For this reason,
    noise_terrain objects shouldn't be manipulated or created directly. This 
    wrapper is the intended manipulation proxy.

    The purpose of this class may not be immediately apparent, but it is 
    conceivable that this class could be extended in various ways to implement a 
    mechanism that achieves "infinite" terrain generation, among other things. 

    For example, rather than recaluclating the entire landscape, you could check camera position
    in the update function and recalculate on a block by block basis. 

    Once infinite generation is acheived, the parameter manipulation contol arrays could be moved 
    down to the block level. When this is done, you can scale values on a block by block basis. This 
    opens up the opportunity to use another noise function to determine large scale terrain features. 

    This could also be decided in many other ways, including manually. 
*/
class blockManager
{
    //vector of terrain blocks to render
    vector<noise_terrain> blocks;

    //terrain control parameters
    int block_size;
    float point_spread;
    float flatness;
    float min_height;
    float max_height;
    float seed;
    GLenum fill_mode;
    
    glm::vec3 center;
    float radius;

    static const int num_params= 8;
    float *control_variables[num_params];    //pointers to terrain control parameters themselves
    float control_increments[num_params];    //increment amounts to use when signaled to change the respective parameters
    float control_bounds[num_params*2];       //min and max bounds on the respective parameter values

    float presets[10][num_params];

    public:
        blockManager(void);
        void loadBlocks(glm::vec3 center, int radius);
        void update(void);
        void update(glm::vec3 centroid, int radius);
        void drawBlocks(void);
        void toggleFillMode(void);
        void preset(int preset_index);
        void loadPresets(void);

        //parameter manipulation request channels
        float control_signals[num_params];
            //0: point_spread
            //1: flatness
            //2: octaves
            //3: persistence
            //4: min height
            //5: max height
            //6: seed    
};

//Constructor
blockManager::blockManager(void)
{
    block_size=100;
    point_spread=10;
    flatness=25.0;
    max_height=window_height;
    min_height=-max_height/2;
    seed=600000;
    fill_mode=GL_LINE;


    //set up control_* arrays
    int half_height= window_height/2;
    float hh=half_height;
    float wh=window_height;

    float *variables[num_params]  = {&point_spread,   &flatness,   &octaves,   &persistence,   &min_height,   &max_height,    &seed,      &radius };                                                                          
    float signals[num_params]     = {      0,             0,           0,           0,              0,             0,           0,           0    };
    float increments[num_params]  = {     5.0,           5.0,         1.0,         0.1,            hh,             hh,        1000.0,       1.0   };
    float bounds[num_params*2]    = {   1.0,100.0,     1.0,300.0,   1.0,12.0,     0.1,0.9,        -wh,hh,        -hh,wh,   400000,800000,   0.0,2.0 };
    
    copy(variables,  variables+num_params,   control_variables );
    copy(signals,    signals+num_params,     control_signals   );
    copy(increments, increments+num_params,  control_increments);
    copy(bounds,     bounds+(num_params*2),  control_bounds    );

    loadPresets();
}


//creates all individual block objects and pushes them onto the blocks vector
void blockManager::loadBlocks(glm::vec3 center_pos, int r)
{
    center= center_pos;
    radius= r;

    noise_terrain centroid(block_size, point_spread, flatness, max_height, min_height, seed, fill_mode);
    centroid.create(center);
    for(int x=-radius; x<=radius; x++)
    {
        for(int y=-radius; y<=radius; y++)
        {
            noise_terrain *temp = new noise_terrain(block_size, point_spread, flatness, max_height, min_height, seed, fill_mode);
            noise_terrain block = *temp;
            block.create(centroid.neighbor(glm::vec3(x, y, 0)));
            blocks.push_back(block);
            delete temp;
        }
    }
    blocks.push_back(centroid);
}


//checks for changes in the parameter manipulation channels and recreates terrain if necessary.
void blockManager::update(void)
{
    bool new_terrain_required=false;
    for(int i=0; i<num_params; i++)
    {
        if(control_signals[i] != 0)
        {
            new_terrain_required=true;
            *control_variables[i]+= (control_signals[i]*control_increments[i]);
            if(*control_variables[i] > control_bounds[(i*2)+1])
            {
                *control_variables[i]= control_bounds[(i*2)+1];
            }
            else if(*control_variables[i] < control_bounds[i*2])
            {
                *control_variables[i]= control_bounds[i*2];
            }
        }
    }

    if(new_terrain_required)
    {
        blocks.clear();
        loadBlocks(center, radius);
        blocks[0].printInfo();
    }
}


//renders each individual block
void blockManager::drawBlocks(void)
{
    for(int i=0; i<blocks.size(); i++)
    {
        blocks[i].draw();
    }
}


//toggles between GL-LINE and GL_FILL fill mode
//this isn't a parameter manipulation channel because it doesn't require terrain re-creation.
void blockManager::toggleFillMode(void)
{
    fill_mode= (fill_mode == GL_LINE) ? GL_FILL : GL_LINE; 
    for(int i=0; i<blocks.size(); i++)
    {
        blocks[i].toggleFillMode();
    }
}


void blockManager::preset(int preset_index)
{
    for(int i=0; i<num_params; i++)
    {
        *control_variables[i]=presets[preset_index][i];
    }

    blocks.clear();
    loadBlocks(center, radius);
    blocks[0].printInfo();
}


void blockManager::loadPresets(void)
{
    float temp1[num_params]= {point_spread, flatness, octaves, persistence, min_height, max_height, seed, radius};
    for(int i=0; i<num_params; i++)
        presets[0][i]=temp1[i];

    float temp2[num_params]= {point_spread, 45.0f, 7, 0.6, (float) -window_height/2.0f, (float) window_height, seed, radius};
    for(int i=0; i<num_params; i++)
        presets[1][i]=temp2[i];

    float temp3[num_params]= {7.0, 20.0f, 8, 0.5, (float) -window_height/2.0f, (float) window_height/2.0f, seed, radius};
    for(int i=0; i<num_params; i++)
        presets[2][i]=temp3[i];


    //you can place up to 10 user defined presets here. Bind them to 0-9 on the keyboard.
    //Don't forget to create the approriate case statements in the keyboard callback switch statement. 
}