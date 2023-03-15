2023
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
 - [ ] Area surrounded by four static walls
 - [ ] Static circles
 - [ ] Controllable circle
 - [ ] Support dynamic circles, the amount will be decided later
 - [ ] Render scene

Render velocities and forces
 - [ ] Only when paused?

Store previous frames and add ability to move freely between them and to continue physics at any given point
 - [ ] Specify the data to be stored
 - [ ] Store data after each frame
 - [ ] When paused, moving between frames is possible
 - [ ] When unpaused, physics should continue from the current frame

Render all collision steps
 - [ ] When paused each collision check and resolve can be viewed
 - [ ] Possibility to move between collision steps

Fix collision issues with multiple colliders
 - [ ] Use the test bench and new tools

Integrate circle physics to game logic
 - [ ] Use with player collisions
 - [ ] Use with enemy collisions
