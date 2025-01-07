# Invaders
Doing a basic invaders to teach myself ECS, win32, Audio and Multithreading and a bunch of other things

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
* Make enemies shoot as well
    * Both you and this can have a cd and just check it?
* Make the actual drawings we do be targeting a bitmap not always the actual framebuffer
* Look at Handmade Hero for rendering scaled/rotated rectangles and texture mapping
    * We need to send a basis, since it's 2d a rotation is enough
    * These are day 92 and 93 respectively!!!

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
