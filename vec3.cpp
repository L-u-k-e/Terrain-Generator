using namespace std;
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
        //considers anything less than -9999 as empty
        //use with caution, dummy function might walk off the edge of the cliff. 
        int pos=0;
        while(true)
        {
            if(data[pos] >= -9999)
            {
                pos++;
            }
            else break;
        }
        data[pos]=x;
        data[pos+1]=y;
        data[pos+2]=z;
    }
    
    float red()
    {
        return x;
    }

    float green()
    {
        return y;
    }

    float blue()
    {
        return z;
    }

    void red(float val)
    {
        x=val;
    }

    void green(float val)
    {
        y=val;
    }

    void blue(float val)
    {
        z=val;
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
