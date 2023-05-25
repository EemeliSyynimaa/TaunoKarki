2023
---

Code maintenance
 - [ ] Remove references to struct game_state in all functions and pass the used variables instead
 - [ ] Create structs for basic shapes like circle, line
 - [ ] Create entity system
   - [ ] Make a plan
 - [ ] Create generic renderer system
   - [ ] Render prites
   - [ ] Render cubes
   - [ ] Render particles
   - [ ] Render primitives
 - [ ] Create proper game world
   - [ ] Store multiple levels
 - [X] Store game config in one header file
 - [ ] Redefine memory management

Fix remaining issues with collisions
 - [ ] Use the test bench and new tools
 - [ ] Static circles shouldn't move at all

Integrate circle physics to game logic
 - [ ] Use with player collisions
 - [ ] Use with enemy collisions

Done
---

Separate game logic
 - [X] Commit existing changes to remote (fix linux and merge with the old WIP commit)
 - [X] Separate game and circle logic to own files (update, render and other functions only)
 - [X] Create an interface for different states
   - [X] Functions for init, update and render
   - [X] Each state can have it's own struct for storage, for example the current game state struct
 - [X] Create states for game and physics test bench (old circle logic)
 - [X] Make game state default and commit to master

Create test bench for physics and collision
 - [X] Make physics state default and commit to physics
 - [X] Area surrounded by four static walls
 - [X] Static circles
 - [X] Controllable circle
 - [X] Support dynamic circles, the amount will be decided later
 - [X] Render scene

Render circle movement vector
 - [X] Only when paused

Store previous frames and add ability to move freely between them and to continue physics at any given point
 - [X] Specify the data to be stored
 - [X] Store data after each frame
 - [X] When paused, moving between frames is possible
 - [X] When unpaused, physics should continue from the current frame
