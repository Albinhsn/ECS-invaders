# Invaders
Doing a basic invaders to teach myself ECS, win32, Audio and Multithreading and a bunch of other things

The ECS way is to create systems or components to solve things
* Needs extra data for something
    * Create a component
* THen create the system that solves that problem

# Things i'd like to solve in this project
* ECS
* Basic Audio
* Multithreading
* Scaling textures
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


# ToDo
* Get a handle of the timer stuff in win32
* Fix dt bug?
* Make like a TypeComponent or smth
* Make enemies shoot as well
    * Both you and this can have a cd and just check it?
* Make the actual drawings we do be targeting a bitmap not always the actual framebuffer
* Make the rect and texture take and X and Y axis

* Look at Handmade Hero for rendering scaled/rotated rectangles and texture mapping
    * We need send a basis
      * I.e we send a origin, x/y axis (x/y axis include the width/height)
    * Then just use the dot product to figure out if the pixel is inside

* Audio
    * Write a WAV parser
    * Sound Mixer
* UI
    * IM Layout library
    * Render the layout


----
* CommandBuffer
    * SpawnEnemy
    * DecideWhenToSpawnEnemies
    * EnemyShoot

* UI
    * Button
    * Text

* Audio
    * Shoot Bullet
    * Take Damage / Explosion


* Fully hoist the software_renderer out of the win32_software_renderer and just recreate when neccessary
