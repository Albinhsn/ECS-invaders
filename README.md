# Invaders
Doing a basic invaders to teach myself ECS, win32, Audio and Multithreading


# ToDo

* Asset loading
    * Just store uncompressed assets from a pool inside the renderer
    * Load at the start and refer to them at by idx
    * You call this when you initialize the game memory, let the game decide which textures to use

* ECS
    * Create Entity
    * Query Components

* UI 
    * Button
    * Text 

* Audio
    * Shoot Bullet
    * Take Damage / Explosion

* Additional Things 
    * SIMD 
    * Work Queue

* Fully hoist the software_renderer out of the win32_software_renderer and just recreate when neccessary
