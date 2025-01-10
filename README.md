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
* Font rendering


# ToDo
* Get a handle of the timer stuff in win32
  * Fix dt bug?
* Support more of the WAV formats :)
  * Try to use them like before with GlobalSound
* Test Loading Game Sounds
* Test looping a sound from gamecode
* Create Inv_PlaySound
* Test Sound mixer
* Hoist and expand file_buffer

* Audio
    * Every frame we write 33 ms (the next frame) of sound into a buffer
      * I.e we need to do audio mixing every frame in the game code
    * That buffer gets copied from on the audio thread

* Draw Text
* Render score
* UI
* Fully hoist the renderer