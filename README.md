# Invaders
Doing a basic invaders to teach myself ECS, win32, Audio and Multithreading and a bunch of other things

The ECS way is to create systems or components to solve things
* Needs extra data for something
    * Create a component
* Then create the system that solves that problem

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


* Audio
    * Figure out how we want to send sound from game <-> platform
        * Commands?
        * Different buffer?
        * This feels like it's just play and then you store what sounds you want, for how long etc
            * PlaySound(&Sound, Description);
                * Should the Usage code look like at least
            * Just store an Array of sounds that are currently playing
        * Do it in 32-bit float :)
    * Sound Mixer

* SDF Font Rendering
  * Render score
  
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
