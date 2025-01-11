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
* Hoist and expand file_buffer
* Use the filebuffer when parsing textures/sounds
* Reenable enemy spawning
* Load BMP
* Be able to draw the part of the image based on input char
* Draw Text based on the image
* Figure out spacings


* Draw Text
    * Figure out which character to sample based on the pixel
        * This is done on the CPU via uv as if youre drawing the image?
    * Sample the image using bilinear sampling
    * Take the median of the three values (rgb) to get a sample
    * Convert the sample back to a signed distance, using the inverse of the distanceColor function (colorDistance)
    * If the signed distance >= 0 then it's inside otherwise outside

    * We can add anti aliasing by choosing a threshold value t
        * that we then divide the distance with to get a weight w that's clamped between -1 and 1
        * Then return a weighted average of the two colors
            * (1-w)/2 x outsideColor + (1 + w)/2 x insideColor

    * He has an example pixel shader in his thesis
    * https://gyazo.com/dad19c474ffd88a9a0c20d59b5a20808

* Render score
* UI
* Fully hoist the renderer
