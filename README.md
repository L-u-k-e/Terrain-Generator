# Terrain-Generator

THIS IS SHERBERT MOUNTAIN..... erm, I mean, noise based terrain generation with c++ (OpenGL)


coming along:
![4-24-1](https://raw.githubusercontent.com/L-u-k-e/Terrain-Generator/master/screenshots/4-24-1.png)
![4-25-1](https://raw.githubusercontent.com/L-u-k-e/Terrain-Generator/master/screenshots/4-25-1.png)
![4-25-2](https://raw.githubusercontent.com/L-u-k-e/Terrain-Generator/master/screenshots/4-25-2.png)
![4-25-3](https://raw.githubusercontent.com/L-u-k-e/Terrain-Generator/master/screenshots/4-25-3.png)

--------------------------------------------------------------------------------

Language and Environment:
---

    Developed on Linux (Ubuntu 14.04 LTS). 

    OPENGL 3.0 
    Shader Version: 130

    compile with g++ using the following options:            

    	-lglut -lGLEW -lGL -lGLU -lSOIL -std=c++11

--------------------------------------------------------------------------------

Camera Controls:
---

	You can fly around the scene (no-clip)

	Move forwards:   w
	Move backwards:  s
	Strafe right:    d
	Strafe left:     a


--------------------------------------------------------------------------------

Scene Manipulation:
---

Scene Manipulation:
	
	You can manipulate the terrain in a variety of ways.

	Persistence Up:   P (uppercase)
	Persistence Down: p (lowercase)
	Persistence significance: 
		Persistence plays an important role in the noise function. Specifically, 
		it is the rate at which the amplitude is increased each successive 
		octave.


	Octaves Up:	  	O (uppercase)
	Octaves Down:   o (lowercase)
	Octave Significance:
		The amount of octaves specifies how many different noise funtions to add
		together (each time doubling the frequency) to get the final 
		result. Generally speaking, higher octaves = more detail, but after
		about 8 or so, you can't tell the difference.


	Flatness Up:   I (uppercase)
	Flatness Down: i (lowercase)
	Flatness Significance:
		When feeding x/y value pairs into the noise function, they are each 
		divided by the value of the flatness first. As the name implies, upping
		the flatness value "flattens out" the terrain. This is because upping 
		the flatness (divisor) causes the points you end up sampling to be 
		closer together.


	Point-spread Up:    U (uppercase)
	Point-spread Down:  u (lowercase)
	Point-spread Significance:
		Point spread is the distance between vertices. This doesn't affect the 
		output of the noise function. However, it has a significant impact on 
		the final result. 


	Min-Range Up:   Y (uppercase)
	Min-Range Down: y (lowercase)
	Max-Range Up:   T (uppercase)
	Max-Range Down: t (lowercase)
	Range Significance:
		When the noise function spits out noise values,they are intially between
		-1 and 1. They are mapped to the range min - max. This does not affect 
		the vertex coloring, however. Although it should for best effect. 


	Seed Up:	R (uppercase)
	Seed Down:  r (lowercase) 
	Seed significance:
		changing the seed effectively gives you new terrain without manipulating
		any parameters. 

		Technically however, because this value is just is an input offset to
		the noise function, which is entirely deterministic, you could also 
		think of it as giving you a different chunk of the same landscape. 

		If you make the seed increment small enough, you'll acually see the 
		terrain scroll across the screen as you change the seed. 


	Radius Up:   N (uppercase)
	Radius Down: n (lowercase)
	Radius Significance:
		Radius is the block-distance from the center to the perimeter of the 
		entire landscape. Its default value is 0, so there is one block total. 
		Upping this value to 1, surrounds the center block with 1 block on each 
		side, yeilding a landscape comprised of 9 total blocks. 


	Toggle fill mode:    m (lowercase)
		You can render in either full-color mode or in "wireframe mode"

		Wireframe mode is actually a wireframe outline over solid black 
		triangles. Wireframe is currently the default. 

		Rendering in full color yeilds a SIGNIFICANT performance advantage 
		however. 
