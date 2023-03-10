# Todo

2023

- [ ] Finish physics (March)
 - [ ] Entity to entity collisions

- Ideas:
 - Levels:
  - make some rooms higher than others so they can be viewable from upper
    floors!
  - make walls thinner (doesn't need to be so blocky)
 - General:
  - create a floor selection panel in the elevator (starting) room. The camera
    should zoom in when player enters the elevator and the current floor
    should be highlighted. Also the next floor should be greenlighted when
    it's reachable
 - Enemy AI:
  - when the player disappears from view, search the whole area where they
    seemed to have gone => ability for being able to search a room quickly or
    thoroughly would be awesome
  - enemy should move it's head around while moving, just to look around
  - enemy should react to foot steps as well
  - enemies should be able to communicate via shouting (way too cool!)
 - Weapons:
  - leveling is stupid, think of how to improve it
  - maybe add limited amount of bullets
 - Graphics:
  - primitive renderer
   - one (?) vertex buffer for all primitives
   - should handle transparency properly
   - functions for primitives
    - circle
    - triangle
    - line
    - rect
  - floor tiles look absolutely awful and doesn't go well together
  - render player characters as sprites: separate head, body, legs, arms and
    weapons
  - render text
  - shadows and lighting
  - screenshake
  - raytracing???
 - Physics:
  - Record physics steps so it's possible to go back in time
 - Fun:
  - create physical doors, not just doorways!
  - create windows:
   - one that can be seen through but not shot
   - one that can be both seen and shot through and once broken also
     traveled through

Bugs
 - enemy rotates to wrong direction (very randomly)
 - enemy can see through walls (ray cast doesn't always work in corners?)

2022

- [ ] Finish physics (May)
 - [ ] Entity to entity collisions
- [ ] Finish AI (June)
 - [X] Enemy sleeps
 - [X] Enemy wanders around if bored
 - [X] Enemy looks around if looking for something
 - [X] Enemy reacts to player
  - [X] Enemy shoots at player
  - [X] Enemy moves to last seen player position
 - [X] Enemy reacts to sounds
  - [X] Enemy reacts to gun sounds
  - [ ] Enemy reacts to foot steps
  - [X] Enemy moves towards the origin of sound
  - [ ] Sound distance is rendered as circles
 - [X] Enemy reacts to being shot at
  - [X] Enemy looks at the shot direction
  - [X] Enemy moves in that general direction to find the shooter
 - [ ] Enemy follows path
  - [X] Path is trimmed so the enemy doesn't have to visit each node
  - [ ] Enemy stops exactly at the path end
  - [X] Enemy stops if the path is changed during movement
- [X] Add collectible items (July)
 - [X] Add weapon boxes
 - [X] Add health boxes
 - [X] Add weapon level up boxes
 - [X] Enemies drops the weapon it used when dead
 - [X] Health and level up boxes are picked up automatically
 - [X] Player can choose whether to pick up the weapon box
- [ ] Level generation (August)
 - [X] Level always starts from an elevator
 - [X] Create levels that are more like floors in tall buildings, not caves
 - [X] Rooms can have different floors
 - [ ] Rooms can have different walls
- [ ] Make a game out of it (September)
 - [X] Player starts from the ground level
 - [X] Player can move between levels via elevator
 - [ ] Player can see lower levels if there's no ceiling or floors blocking
 - [X] Player is required to kill all enemies before being able to move up
 - [ ] Enemies spawn with some logic, not just at random places
 - [X] Enemies can spawn in different AI states
 - [X] Game is over when the player dies
 - [ ] Player gets score based on the level reached and the number of kills
- [ ] Finish gameplay (October)
- [ ] Clean up unfinished code (November)
 - [ ] All the listed things that couldn't have been done in time
 - [ ] Clean the code base
  - [ ] The game source file is bound to be huge at this point
   - [ ] Remove unused code
   - [ ] Decide which code to move to own file (make a plan)
   - [ ] Move the code
 - [ ] If time
  - [ ] Build on linux
  - [ ] Create ECS
  - [ ] Graphics stuff
   - [ ] Window blocks (blocks movement, can be seen through)
   - [ ] Door blocks (blocks view, can be opened)
   - [ ] Shadows and lighting
   - [ ] Screenshake
   - [X] Enemy and player faces react to happenings (sleepy, watchful etc)
   - [ ] Render text
- [ ] Polish the MVP (December)
 - [ ] Main menu
  - [ ] Start game
  - [ ] Scores
  - [ ] Exit game
 - [ ] Pause menu
  - [ ] Continue
  - [ ] Exit to menu
 - [ ] Game over menu
  - [ ] Start game
  - [ ] Exit to menu
 - [ ] Create release package and put it available
  - [ ] Windows package
  - [ ] Linux package

# Old list

 - [X] Improve build tools
 - [X] Coding conventions
 - [X] Debug logging 
 - [X] Memory management
 - [X] Game as a library
 - [X] Hot reload
 - [X] Recording
 - [X] Player movement
 - [X] Camera follow
 - [ ] Collision detection
  - [X] Player collision
  - [X] Bullet collision
  - [X] Enemy collision
  - [ ] Entity-entity collisions
 - [X] Fix coordinate system (zero position at floor level)
 - [ ] Enemy AI
  - [X] Path finding (A*)
  - [ ] Proper movement logic
   - [X] Acceleration
   - [X] Random number generation
   - [X] Raycast (against walls)
   - [X] Move towards the farthest visible step
   - [ ] Get current time
   - [ ] Stop at the goal
  - [ ] Aim logic
   - [X] Separate aim angle from look direction
   - [X] Render aim direction
   - [X] Render look direction
   - [X] Take shooter's velocity into account
   - [ ] Take target's velocity into account
   - [X] Render line of sight
    - [X] Render line of sight as triangles
    - [X] Render line of sight as polygon (generate triangles)
  - [ ] Finite state machine
   - [ ] Wander state
   - [ ] Pursue state
   - [ ] Attack state
   - [ ] Escape state
 - [ ] Ray casting
  - [X] Against line segment
  - [ ] Against circle
  - [ ] Against AABB
  - [ ] Against OBB
 - [ ] Reserve the first element of array as null or default 
 - [ ] Physics
  - [ ] Use forces for movement
  - [ ] Add friction
  - [X] Rotate smoothly
  - [ ] Proper bullet physics
 - [ ] Controller input
 - [ ] Local multiplayer
 - [ ] Text rendering
 - [ ] Procedural level generation
 - [ ] Animations
 - [ ] Sounds
 - [ ] Linux support
