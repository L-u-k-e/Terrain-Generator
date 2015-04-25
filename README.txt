Author: Lucas Parzych
Date:   4/24/15
Email:  parzycl1@sunyit.edu
Github Repo:  https://github.com/L-u-k-e/Terrain-Generator
--------------------------------------------------------------------------------

THIS IS SHERBERT MOUNTIAN..... erm, I mean, noise based terrain generation in 
c++ and OpenGL that totally follows the spec in regards to the vertex colors ;)

--------------------------------------------------------------------------------
Extra Dependencies: 

	SOIL is used to load the images into a texture. 
	If you don't have it, the skybox won't render. 

--------------------------------------------------------------------------------
Language and Environment:

    Developed on Linux (Ubuntu 14.04 LTS). 

    OPENGL 3.0 
    Shader Version: 130

    compile with g++ using the following options:            

    	-lglut -lGLEW -lGL -lGLU -lSOIL -std=c++11

--------------------------------------------------------------------------------
Camera Controls:

	You can fly around the scene (no-clip)

	Move forwards:   w
	Move backwards:  s
	Strafe right:    d
	Strafe left:     a

--------------------------------------------------------------------------------
Scene Manipulation:
	
	You can manipulate the terrain in a variety of ways.

	Persistence Up:   P (uppercase)
	Persistence Down: p (lowercase)
	Persitence significance: 
		Persistence plays an important role in the noise function. Specifically, 
		it is the rate at which the amplitude is increased each successive 
		octave.


	Octaves Up:	  O (uppercase)
	Octaves Down: o (lowercase)
	Octave Significance:
		The amount of Octaves specifies how many different noise funtions to add
		together (each time doubling the frequency by 2) to get the final 
		result. Generally speaking, higher octaves = more detail, but after
		about 8 or so, you can't tell the difference.


	Flatness Up:   I (uppercase)
	Flatness Down: i (lowercase)
	Flatness Significance:
		When feeding x/y value pairs into the noise function, they are each 
		divided by the value of the flatness first. As the name implies, upping
		the flatness value "flattens out" the terrain.


	Point-spread Up:    U (uppercase)
	Point-spread Down:  u (lowercase)
	Point-spread Significance:
		Point spread is the distance between vertices.


	Min-Range Up:   Y (uppercase)
	Min-Range Down: y (lowercase)
	Max-Range Up:   T (uppercase)
	Max-Range Down: t (lowercase)
	Range Significance:
		When the noise function spits out noise values,they are intially between
		-1 and 1. They are mapped to the range min - max. This does not affect 
		the vertex coloring however, although it should for best effect. 


	Seed Up:	R (uppercase)
	Seed Down:  r (lowercase) 
	Seed significance:
		changing the seed affectively gives you new terrain without manipulating
		any parameters. 

		Technically however, because this value is just is an input offset to
		the noise function, which is the same each time and is entirely 
		deterministic, you could also think of it as giving you a different 
		chunk of the same landscape. 

		If you make the seed increment small enough, you'll acually see the 
		terrain scroll across the screen as you change the seed. 

	Radius Up:   N (uppercase)
	Radius Down: n (lowercase)
	Radius Significance:
		Radius is the block-distance from the center to the perimeter of the 
		entire landscape. Its default value is 0, so there is one block total. 
		Upping this value to 1, surrounds the center block with 1 block on each 
		side, giving 9 total blocks in the landscape. 


	Toggle fill mode:    m (lowercase)
		You can render in either full-color mode or in "wireframe mode"

		Wireframe mode is actually a wireframe outline over solid black 
		triangles. Wireframe is currently the default. 

		Rendering in full color yeilds a SIGNIFICANT performance advantage 
		however. 

--------------------------------------------------------------------------------
Bonus information:
	Due to performance, it was not feasible for me to implement infinite 
	terrain. However, it could conceivably be done through the blockManager 
	class. There is a comment about this in the blockmanager class itself. 

