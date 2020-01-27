# Todo

 - [ ] Improve build tools
 - [ ] Coding conventions
 - [ ] Debug logging 
 - [ ] Divide code to modules
 - [ ] Memory management
 - [ ] Game as a library
 - [ ] Hot reload
 - [ ] Graphics cleanup
 - [ ] Player movement
 - [ ] Collision detection
 - [ ] Enemy AI
 - [ ] Text rendering
 - [ ] Procedural level generation
 - [ ] Animations
 - [ ] Sounds
 - [ ] Linux support

--

X Replace glew
  X Fetch GL functions from the system by hand
 - Replace GLM
  - Make own implementation of all used math functions
  - Use SSE
 - Replace Box2D
  - Implement own collision engine
  - Divide objects into groups: enemies, player, bullets, walls etc.
  - Use circular bounding box for moving objects
  - Implement a simple raycast for AI
 X Replace lodepng
  X Use TGA file format
  X Convert all textures to tga
  X Load converted tga textures
  X Remove old png files
  X Remove lodepng
 X Replace SDL
 - Move game code into own library
  - Load game code dynamically (hot reload)
 - Use custom memory allocation
  - Platform code allocates large chunk of memory and passes it to the game library
 - Clean assets
  - Remove unused meshes
  - Remove unused textures
  - Remove unused variables
 - Clean code
  - Remove old files
  - Naming conventions
   - Defines
   - Structures and types
   - Functions
 - OpenGL
  - Fix indexing
  - Render wireframed meshes

Improve build tools
 - Use clang formatting
 - Proper difftool (meld is not working good enough)
 - Move up/down to the next empty line
 - Create a script to go the project folder
 - Create proper build, run and debug scripts
  - Drop the requirement to type ./build.bat, just build should be enough
  - Or just use windows terminal...

When all third party code is replaced, it's time to move on:

- Debug logging
 - Log macros
- Write all remaining code in C
 - Re-design entity system
  - No overengineering
- Software renderer
 - 3D walls
 - Sprites
 - Particle system
 - Lights
 - Animations
  - Skeleton?
- Gameplay improvements
 - Game feel and balance
  - Screen shake?
 - More weapons, enemies
 - Improved AI
 - Improved map generation
 - In-game map editor?
- Multiplayer?
 - Local?
 - Network?
- Sound system
 - Write an own mixer?
- Move rendering code into own library
 - In future, can switch between software or hardware rendering
 - Vulkan? 