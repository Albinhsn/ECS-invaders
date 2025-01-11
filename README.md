# Invaders
Doing a basic invaders to teach myself ECS, win32, Audio and Multithreading and a bunch of other things

The ECS way is to create systems or components to solve things
* Needs extra data for something
    * Create a component
* Then create the system that solves that problem

# Things i'd like to solve in this project
* ECS
* Basic Audio
* Software Renderer
* Basic UI
* OpenGL renderer


# Things i'd like to solve in the next project
* Use a meter convention
* More advanced rendering
    * Reflections
    * Transparent objects
* Asset Loader
* Font rendering
* Linux support
* D3D11 Support
* Record and store inputs for playback
* SIMD
* Work Queue
* Roll your own printf
* Improved Font rendering
* Write an actual lexer/parser for custom file formats


# ToDo
* Hoist the software renderer to the new approach
  * Create Win32_Software_Renderer


* UI
* Fully hoist the renderer
  * In platform.h have
    * Begin Frame
      * Takes the platform_renderer and a pushbuffer
    * End Frame
      * Takes the platform_renderer and a pushbuffer
    * Init/Create
      * Returns a platform_renderer 
        * the platform_renderer is a pointer to the beginning of the actual renderer
  * The renderer allocates it's own memory and you make like a win32_opengl.c

* OpenGL renderer
