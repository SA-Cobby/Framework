# Topic guide

Unreal Engine 5.4. Written for ages 10-18 learning with a tutor.

## Blueprint Classes

*Blueprints*

A Blueprint Class is a reusable plan for making game objects. You connect nodes to describe what they do. Each copy made from the class is called an instance.

**Try it together:** Make a Coin Blueprint. Place three copies in a level and give each one a different points value.

**Use case 1:** Collectable coins: one Blueprint describes spinning, being collected and adding points. Place copies around the level.

**Use case 2:** Puzzle doors: one Blueprint describes opening and closing. Give each door its own key requirement.

**Remember:** Changing the class can change all its copies. Changing a value on one placed instance usually changes only that copy.

## Variables

*Blueprints*

A variable is a named place to remember information. Its type says what it holds: a number, a true/false choice, text or a reference to another object. A variable remembers a value; a function carries out a task.

**Try it together:** Add a Health variable with a default value of 100. Get Health to read it. Set Health to change it to 80.

**Use case 1:** Player health: Health remembers how much health this player has left. The HUD (heads-up display) reads it to show the health bar.

**Use case 2:** Locked doors: IsLocked remembers true or false. The door checks it before deciding whether to open.

**Remember:** Choose a type that fits the information. Getting a variable reads it; it does not change it. Each character can have its own Health value.

## Functions

*Blueprints*

A function is a named set of steps you can call whenever you need that task. Inputs are information you give it; outputs are answers it returns. A function can read or change variables. The variable stores the information; the function works with it.

**Try it together:** Make a TakeHit function with a Damage input. Read Health, subtract Damage, then set Health to the new value.

**Use case 1:** Taking damage: TakeHit uses the Damage input to reduce the Health variable. Call the same function for spikes or enemy attacks.

**Use case 2:** Shop prices: CalculatePrice takes a base price and discount, then returns the final cost. Reuse it for every shop item.

**Remember:** Blueprint functions cannot use Delay. A pure function should calculate an answer without changing the game. Use an event for a sequence that needs to wait.

## Events

*Blueprints*

An event starts a chain of actions when something happens. BeginPlay happens when an object starts playing; overlap events happen when collision shapes overlap. You can also create and call Custom Events.

**Try it together:** Connect BeginPlay to a Print String node that says Ready! Run the level to see when it happens.

**Use case 1:** Checkpoints: an overlap event starts the steps that remember the new respawn location.

**Use case 2:** Timed doors: a Custom Event opens a door, waits with Delay, then closes it.

**Remember:** An event is a starting signal, not stored information. Custom Events do not return output values. Use functions when you need to return an answer.

## Macros

*Blueprints*

A macro wraps a group of Blueprint nodes into one reusable node. It can have several execution paths, such as Yes and No. A function suits a named task; a macro can make a repeated pattern of connections easier to read.

**Try it together:** Make a CheckScore macro with Score and Target inputs. Add a Branch and give it Reached and Not Yet execution outputs.

**Use case 1:** Score gates: reuse a macro that checks whether the player has enough points and follows the correct execution path.

**Use case 2:** Object checks: wrap repeated validity and condition checks into one clear node with Ready and Not Ready outputs.

**Remember:** A macro is expanded into nodes wherever it is used. Keep it small. Use a function when you want a normal task with inputs and a returned answer.

## Event Dispatchers

*Blueprints*

An Event Dispatcher announces that something happened. Other objects can bind a response, which means they sign up to listen. One announcement can notify several listeners.

**Try it together:** Add a HealthChanged dispatcher with a NewHealth input. Call it after Health changes, then bind the HUD to update its bar.

**Use case 1:** Boss defeated: one announcement tells the HUD to hide the boss bar and a door to unlock.

**Use case 2:** Score updates: a score manager announces NewScore. The HUD and a reward tracker each respond in their own way.

**Remember:** Listeners need a reference to the object sending the announcement. Avoid binding the same response repeatedly. Stop listening when the connection is no longer needed.

## Blueprint Interfaces

*Blueprints*

A Blueprint Interface is a shared list of actions that different objects can promise to support. Each object decides how to respond. The caller does not need a separate command for every object type.

**Try it together:** Make an Interact interface function. Let a door open and a chest give treasure when they receive the same Interact message.

**Use case 1:** Interaction tools: the same player action can open a door, press a button or collect a key.

**Use case 2:** Damageable objects: send one damage interface message to a robot, crate or target; each handles it differently.

**Remember:** An interface does not store variables or write the shared behaviour for you. Each object must implement its own response.

## Blueprint Function Libraries

*Blueprints*

A Blueprint Function Library is a shared toolbox of functions. Different Blueprints can call its functions without each keeping their own copy of the same node setup.

**Try it together:** Create a CalculateDamage function with BaseDamage and Armour inputs. Call it from two different weapon Blueprints.

**Use case 1:** Shared maths: use one damage calculation for swords, arrows and traps.

**Use case 2:** Time displays: use one formatting function to turn a race time into the text used by several HUD widgets.

**Remember:** Pass the information a function needs through its inputs. A library is not a place to keep a player's changing state, and functions cannot use Delay.

## Level Design & Gameplay Flow

*Level design*

Level design means arranging spaces, routes, challenges and rewards to support what the player does. Gameplay flow is how these moments connect as the player moves through a level. Good flow helps players understand their choices and keeps the experience moving.

**Try it together:** Sketch a start, a goal and three moments between them. Label what the player does at each moment, then build a simple version with your tutor.

**Use case 1:** Platform route: a safe opening lets players practise a jump before using it to reach a reward.

**Use case 2:** Puzzle journey: a small room teaches a switch rule, then a larger room asks the player to combine two switches.

**Remember:** A level is more than scenery. Test whether players understand where they can go and what they can do before adding detailed decoration.

## Blockouts & Scale

*Level design*

A blockout is a rough, playable level made from simple shapes. It lets you test distances, sizes and routes before making detailed art. Scale means how big things are compared with the player and their movement.

**Try it together:** Use cubes to make a short route with a doorway, a ramp and a jump. Play it with the actual character and adjust the shapes with your tutor.

**Use case 1:** Jump testing: try different gap widths with the same character before deciding which jumps belong in the level.

**Use case 2:** Vehicle testing: build a rough corner wide enough for the vehicle to turn, then add scenery after it feels right.

**Remember:** Measure by playing, not only by looking in the editor. A gap must fit the character's jump, and a corridor needs room for both the character and the camera.

## Routes & Player Guidance

*Level design*

A route is a path through a level. Guidance helps players notice useful paths and goals. Landmarks are easy-to-recognise features, such as a tower. Lighting, shapes, signs and carefully placed rewards can also help players find their way.

**Try it together:** Build two paths to a goal. Use a landmark and a clear entrance to suggest the main route, then ask another student where they think they should go.

**Use case 1:** Adventure landmark: a tall tower helps players stay oriented while exploring several connected paths.

**Use case 2:** Optional platform route: a visible collectable suggests a harder side path, while the main route remains clear.

**Remember:** A clue that makes sense to its designer may confuse a new player. Test it without explaining the answer. Do not rely on colour alone to communicate a route.

## Pacing & Checkpoints

*Level design*

Pacing is the pattern of quiet, busy, easy and challenging moments. Checkpoints are places where progress can be remembered so players do not need to repeat too much after a mistake. Different games need different patterns.

**Try it together:** Plan a short sequence: learn a skill, practise it, try a challenge, then pause to explore. Decide with your tutor where a fair checkpoint would help.

**Use case 1:** Platforming lesson: teach a moving platform above a safe floor, test the skill over a gap, then offer a checkpoint.

**Use case 2:** Action level: follow a busy encounter with a quieter space where players can recover and notice the next goal.

**Remember:** Constant difficulty can become tiring. Give players useful feedback and chances to learn. Test how much they must repeat after failing, not just how hard the challenge is.

## Space, Cover & Sightlines

*Level design*

A sightline is a clear line of view between places. Open areas, corners and objects change what players can see and where they can move. Cover can block a view or an attack, depending on the game's rules.

**Try it together:** Block out a small target arena. Compare an empty version with one containing large blocks and two connected routes. Play both and discuss what changed.

**Use case 1:** Robot target arena: blocks interrupt long views and a second route gives players another way around the space.

**Use case 2:** Exploration reveal: a bend hides a new area until the player reaches an opening, creating a clear moment of discovery.

**Remember:** Visual cover may not block gameplay unless collision and game rules support it. Check what players can see from their actual camera position.

## Designing for a Genre

*Level design*

Start with the actions your game asks players to repeat. Platformers often use readable landing spots; shooters often use sightlines and connected encounter spaces; puzzle games often use rooms that focus attention on a rule. These choices help the gameplay flow.

**Try it together:** Take one square room and sketch it twice: first for jumping between platforms, then for solving a switch puzzle. Explain why the routes and object positions change.

**Use case 1:** Racing layout: readable corners and a clear track support the loop of steering, adjusting speed and completing laps.

**Use case 2:** Survival layout: resource areas and safe return routes support exploring, gathering and returning to a base.

**Remember:** These are common patterns, not rules every game must follow. Genres can mix. Choose a layout because it supports your game's actions, then test it with players.

## Game Genres & Core Loops

*Game types and cameras*

A genre is a useful name for a kind of game, such as platformer, shooter or puzzle game. A core loop is a set of actions players repeat, such as explore, collect and upgrade. A game can combine several genres.

**Try it together:** Choose a game you know. Describe what the player repeatedly does using three action words. Sketch a space that would support those actions.

**Use case 1:** Platformer loop: move, jump and collect. Platforms and rewards give those actions a route to follow.

**Use case 2:** Adventure loop: explore, discover and unlock. Connected locations and new routes help the player keep making progress.

**Remember:** A genre label is a starting point, not a complete design. First-person and third-person describe viewpoints; either can be used by different game genres.

## Platformers

*Game types and cameras*

Platformers focus on movement through spaces, often using jumping, timing and landing. Some use a side view and others let players move through a 3D world. Level layouts often help players judge distances and practise movement skills.

**Try it together:** Build a short route with a safe practice jump, a wider jump and a visible finish. Test it with the character and camera you intend to use.

**Use case 1:** Side-view course: arrange platforms in a readable sequence so players can judge the next jump.

**Use case 2:** 3D collectable challenge: use broad landing areas and optional higher paths to reward exploring and movement practice.

**Remember:** Keep landing spots readable. Check that the camera shows important jumps and that the character can actually reach the platforms. Use fair recovery points while students are learning.

## Shooters

*Game types and cameras*

Shooters focus on aiming and using ranged game tools, such as blasters that hit robot targets. They can use first-person, third-person or other cameras. Levels often combine aiming spaces, movement choices and clear goals.

**Try it together:** With your tutor, sketch an arena with robot targets, a starting area and two routes. Mark which targets are visible from each route before building it.

**Use case 1:** Target course: use short, clearly separated areas so players can practise aiming before trying moving targets.

**Use case 2:** Arena challenge: connected routes and view-blocking props let players change position instead of staying in one place.

**Remember:** An interesting layout is not just a long corridor of targets. Check visibility, safe starting positions and whether one position has too much advantage. Keep the game theme suitable for your class.

## Puzzle Games

*Game types and cameras*

Puzzle games ask players to understand rules and use them to solve problems. Levels often introduce one idea clearly, give useful feedback and then combine ideas into new challenges. Puzzles can use many different camera viewpoints.

**Try it together:** Make a room where a switch opens a door. Add clear feedback, then design a second room that uses the same rule in a new way.

**Use case 1:** Switch rooms: place switches and doors so players can see how their actions affect the room.

**Use case 2:** Object puzzles: provide a small space to experiment with moving blocks before using the same rule in a larger challenge.

**Remember:** A player should be able to notice the information needed to solve the puzzle. Confusing controls or an unclear camera can hide the idea you meant to teach.

## Adventure Games & RPGs

*Game types and cameras*

Adventure games often focus on exploration, discoveries and stories. RPG means role-playing game; many RPGs add character choices and growing abilities. These types can overlap. Their levels often connect goals, interesting places and optional activities.

**Try it together:** Sketch a small hub with a main goal, an optional conversation and a locked route. Decide how the player learns about each one.

**Use case 1:** Exploration quest: a landmark guides players towards a goal while side paths lead to optional discoveries.

**Use case 2:** Returning with an ability: an earlier locked route becomes useful after the player learns a skill or finds a key.

**Remember:** A large map is not automatically interesting. Give locations a purpose and make goals understandable. Leave room for different player choices where your game supports them.

## Racing Games

*Game types and cameras*

Racing games challenge players to complete a course, beat a time or compete for position. Track width, turns, slopes and visible markers shape how players steer and change speed. Cameras may show the road from inside or outside the vehicle.

**Try it together:** Block out a short track with a straight, a wide bend and a sharper corner. Drive it at the intended speed and adjust the route with your tutor.

**Use case 1:** Beginner circuit: wide bends and clear markers help players learn steering before they try tighter turns.

**Use case 2:** Time trial: varied corners and optional shortcuts let players practise and improve their route through the course.

**Remember:** Test with the real vehicle controls. Corners that look clear from above may be hard to read while driving. Plan recovery if a player leaves the track.

## Survival Games

*Game types and cameras*

Survival games often ask players to manage limited resources while exploring and staying safe. Players may gather materials, craft useful items and improve a base. Level layouts can turn the choice of where to travel into part of the challenge.

**Try it together:** Sketch a safe base, two resource areas and routes between them. Give the routes different travel times or game obstacles, then discuss the choices they create.

**Use case 1:** Gathering trip: nearby materials support learning, while a farther area offers new materials and a longer return journey.

**Use case 2:** Base planning: space for building and several access routes let players decide where to place helpful structures.

**Remember:** Explain important needs clearly and avoid making beginners guess essential rules. Test whether useful resources can be found and whether repeated journeys remain interesting.

## Sandbox Games

*Game types and cameras*

Sandbox games give players tools and freedom to choose many of their own goals. Building, experimenting and exploring are common activities. The world needs space and clear rules that support useful experiments.

**Try it together:** Design a small building area with a few simple tools or objects. Ask another student to choose something to make without giving them a single required solution.

**Use case 1:** Building playground: open space and modular pieces let players invent their own structures.

**Use case 2:** Physics experiments: a safe test area lets players combine ramps, balls and movable objects to discover different results.

**Remember:** Freedom still needs understandable controls and feedback. Start with a manageable space, and explain how players can undo mistakes or reset an experiment.

## First-person vs Third-person

*Game types and cameras*

First-person shows the world from the character's viewpoint. Third-person shows the character from outside their body, often from behind. The same genre can use either. Your camera choice changes what players can see and how they judge movement and space.

**Try it together:** With your tutor, compare the same doorway, corner and jump from a first-person and a third-person camera. Note what is easy to see and what gets hidden.

**Use case 1:** First-person exploration: inspect objects closely from the character's view. Keep important clues visible at that camera height.

**Use case 2:** Third-person platforming: seeing the character can help players judge body position and landings. Leave space behind and above for the camera.

**Remember:** Neither viewpoint is always better. First-person can make feet and nearby edges harder to judge. Third-person needs room for the camera and checks to stop walls blocking the view.

## Side-view & Top-down

*Game types and cameras*

A side-view camera looks across the action, often showing movement left and right. A top-down camera looks down at the play area. These are viewpoints, not genres, and they can be used with 2D art or 3D worlds.

**Try it together:** Sketch a simple collectable route from the side, then sketch it from above. Compare which paths, obstacles and clues the player could see.

**Use case 1:** Side-view platformer: a clear outline helps players read platform heights and gaps.

**Use case 2:** Top-down puzzle: a view across a room helps players plan routes between switches and movable blocks.

**Remember:** Match controls and layouts to the view. Avoid hiding important paths behind objects. Check whether players can tell which surfaces or objects they can reach.

## Actors

*Gameplay foundations*

An Actor is a game object that can be placed or created in a level. Doors, lights and pickups can all be Actors. Actors use Components to give them parts and abilities.

**Try it together:** Make a Door Actor Blueprint. Add a Static Mesh for its shape and a collision box to detect a nearby player.

**Use case 1:** Collectable gems: place gem Actors around a level. Each disappears and awards points when collected.

**Use case 2:** Moving platforms: an Actor holds a platform mesh and the logic that moves it between two points.

**Remember:** An Actor does not need to run code every frame. Use an event when something happens, such as a player pressing Interact.

## Components

*Gameplay foundations*

A Component is a reusable part of an Actor. It can add a shape, sound or ability. An Actor Component adds behaviour; a Scene Component also has a position, rotation and size.

**Try it together:** Add a Health Actor Component to an enemy and a breakable crate. Let both use the same damage rules.

**Use case 1:** Shared health: a Health Component lets enemies and breakable props use the same health rules.

**Use case 2:** Torches: put a light and an audio Component on a torch Actor so its glow and sound move with it.

**Remember:** An Actor Component has no position of its own. Use a Scene Component when the part needs a location or attachment.

## Pawns / Characters

*Gameplay foundations*

A Pawn is an Actor that a player or AI Controller can control. A Character is a Pawn with useful walking and jumping features, including a Character Movement Component.

**Try it together:** Use a Character for a walking hero. Try a Pawn when making a drone with your own flying movement.

**Use case 1:** Platform heroes: a Character provides a starting point for running, jumping and falling.

**Use case 2:** Flying drones: a Pawn gives you a controllable object without needing the normal walking setup.

**Remember:** Possession means connecting a Controller to a Pawn. A Pawn will not automatically get the right controls just because it exists.

## Data Assets

*Data and organisation*

A Data Asset stores a set of settings that other objects can use. Think of it as a reusable information card. It helps keep game data separate from the code that uses it.

**Try it together:** With your tutor, make a Primary Data Asset class with Damage and Icon fields. Create an asset for a sword and another for a bow.

**Use case 1:** Weapon cards: store each weapon's damage, picture and description in its own asset.

**Use case 2:** Enemy types: give enemies different speed and attack settings by choosing different configuration assets.

**Remember:** Keep changing player information, such as current arrows, on a runtime object. A shared Data Asset is better for settings such as maximum arrows.

## Structs

*Data and organisation*

A Struct bundles related pieces of information into one type. For example, an inventory entry can hold an item name, a quantity and a condition value together.

**Try it together:** Create an ItemEntry Struct with ItemName and Quantity. Make an array, which is a list, of ItemEntry values for an inventory.

**Use case 1:** Inventory slots: keep an item identifier and quantity together in each slot.

**Use case 2:** Race results: group a racer's name, lap time and finishing position into one result.

**Remember:** Struct values are often copied. If you change an entry taken from an array, remember to put the updated entry back into the array.

## Enumerators (Enums)

*Data and organisation*

An Enum is a type with a short list of named choices. Each choice is an enumerator. It makes states easier to read than unexplained numbers.

**Try it together:** Make an EnemyState Enum with Idle, Patrol and Chase. Use Switch on EnemyState to choose what happens next.

**Use case 1:** Enemy states: store whether a guard is patrolling, chasing or searching.

**Use case 2:** Puzzle colours: choose Red, Blue or Green for a switch and check whether it matches the puzzle.

**Remember:** Use an Enum for a clear set of choices. A long catalogue of items is usually better stored in assets or a table.

## Data Tables

*Data and organisation*

A Data Table stores named rows of information, like a spreadsheet. Every row uses the same Struct, so each item has the same set of fields.

**Try it together:** Make a Struct with DisplayName, Price and Damage. Use it to create a Data Table and add a row for each weapon.

**Use case 1:** Shop catalogues: store names, prices and icons for many items in one place.

**Use case 2:** Enemy balancing: keep health, speed and reward values for several enemy types in a table.

**Remember:** Look up a row using its row name, and handle a missing row. Keep changing player values outside the table. A Data Asset suits a separate configuration object; a Data Table suits many similar records.

## AI Controllers

*Artificial intelligence*

An AI Controller is the decision-maker for an AI-controlled Pawn. The Pawn is the body; the Controller tells it what to do. It can react to what it senses and request movement.

**Try it together:** Assign an AI Controller class to an enemy Character. Ask it to move the enemy to a target point.

**Use case 1:** Patrolling guards: the AI Controller sends a guard between patrol points and reacts when it notices the player.

**Use case 2:** Friendly companions: the Controller tells a companion to follow the player or wait in a safe place.

**Remember:** Check the Pawn's AI possession settings. Walking AI usually needs a Nav Mesh: an area that marks where it can travel.

## Behaviour Trees / Blackboards

*Artificial intelligence*

A Behaviour Tree organises an AI's choices into branches and tasks. A Blackboard is its shared notebook: it remembers information such as the target or the next place to visit.

**Try it together:** Make a Blackboard key called TargetActor. Let the tree chase when it has a target and patrol when it does not.

**Use case 1:** Guard decisions: patrol normally, chase a visible player, then search the last known location.

**Use case 2:** Animal needs: find food when hungry or choose a rest spot when tired. Blackboard values help choose the branch.

**Remember:** Choose the right Blackboard key type. Ask your tutor about observer aborts if the AI must stop its current task when a condition changes.

## Animation Blueprints

*Animation*

An Animation Blueprint decides the pose shown by a Skeletal Mesh. It uses information such as speed and whether the character is in the air to choose and blend animations.

**Try it together:** Assign an Animation Blueprint to your Character's mesh. Use its speed to change between idle and moving.

**Use case 1:** Player movement: show idle, running and jumping as the player changes what they are doing.

**Use case 2:** Enemy reactions: blend from normal movement into an alert pose when an enemy spots the player.

**Remember:** Use a compatible Skeleton and check the mesh's Animation Mode and Anim Class. Let gameplay code handle game rules while animation code focuses on poses.

## Animation Blend Spaces

*Animation*

A Blend Space mixes animations using numbers you supply. A 1D Blend Space uses one number, such as speed. A 2D Blend Space can use two, such as speed and direction.

**Try it together:** Place idle, walk and run animations along a Speed axis in a 1D Blend Space. Feed the character's speed into it.

**Use case 1:** Smooth running: blend from walking to running as speed increases instead of suddenly switching poses.

**Use case 2:** Directional movement: blend forward, backward and sideways movement using speed and direction.

**Remember:** The axis range must match your input values. Blending poses does not move the Character through the level by itself.

## Animation Chooser Tables

*Animation*

A Chooser Table picks an asset by checking information against rows of rules. It can choose an animation for the situation. In UE 5.4, the Chooser plugin is Beta, so use it with your tutor's guidance.

**Try it together:** Enable Chooser with your tutor. Create a table with stance and movement-direction conditions, then set an animation result for each row.

**Use case 1:** Directional crouching: when the stance is Crouching, use movement direction to select crouch-forward, crouch-backward, crouch-left or crouch-right. Include a crouch-idle fallback for no movement.

**Use case 2:** Hit reactions: use the direction of a hit and the character's stance to select a suitable reaction animation.

**Remember:** Provide a fallback when no row matches. Check row order and the information given to the table. The result still needs to be played; selecting it does not automatically blend it.

## Animation Sequences

*Animation*

An Animation Sequence is a single recorded animation clip for a Skeleton. Examples include one walk cycle, a wave or a jump. An Animation Blueprint decides when and how clips are used.

**Try it together:** Open a compatible Animation Sequence. Scrub the timeline to inspect the movement and preview whether it should loop.

**Use case 1:** Walk cycles: repeat a walking clip while a character moves.

**Use case 2:** Short gestures: play a wave or celebration clip when a character greets the player or wins.

**Remember:** Use a compatible Skeleton. Check whether the clip loops and whether it expects root motion, where movement comes from the animation itself.

## Animation Montages

*Animation*

An Animation Montage arranges animation clips into sections that gameplay can control. It is useful for actions such as attacking or reloading, and can play through Slots in an Animation Blueprint.

**Try it together:** Create a Montage from an attack animation. Add a matching Slot node in the Animation Blueprint and trigger the Montage from gameplay.

**Use case 1:** Sword attacks: play an attack when the player presses the button and use sections for different parts of a combo.

**Use case 2:** Reloading: play a reload on the upper body while a suitable animation setup keeps the legs moving.

**Remember:** A Montage needs a matching Slot in the animation setup to appear. Check its blend settings so the action fits smoothly with other movement.

## Static Meshes

*Art and materials*

A Static Mesh is a 3D model without a bone-based animation rig. Static means its shape is not animated by a Skeleton; you can still move or rotate the object using it.

**Try it together:** Drag a Static Mesh into your level. Change its position, rotation and size, then check its collision.

**Use case 1:** Level scenery: build walls, rocks and platforms from reusable models.

**Use case 2:** Moving props: put a fan mesh in an Actor and rotate it, or move a platform mesh with Blueprint logic.

**Remember:** The mesh asset is the model; the Actor is a copy placed in the world. A visible shape does not guarantee it has suitable collision.

## Skeletal Meshes

*Art and materials*

A Skeletal Mesh is a 3D model attached to a Skeleton of bones. Moving the bones can bend and pose the model, which makes it useful for characters and creatures.

**Try it together:** Open a Skeletal Mesh and inspect its Skeleton. Preview a compatible animation to see how the bones move the model.

**Use case 1:** Animated heroes: bend arms and legs to show running, jumping and waving.

**Use case 2:** Creatures and machines: animate a dragon's wings or a robot's jointed body using bones.

**Remember:** Animations need a compatible Skeleton or a suitable retargeting setup. Retargeting adapts animations for another character rig.

## Materials

*Art and materials*

A Material describes how a surface looks when it is drawn. Its nodes can control colour, roughness, metal-like reflections and glow. You assign Materials to a mesh's material slots.

**Try it together:** Create a Material. Set a Base Color and compare low and high Roughness values on a sphere.

**Use case 1:** Surface styles: make a shiny metal robot and a rough stone wall look different.

**Use case 2:** Glowing objects: use an emissive surface to help players recognise a power-up or puzzle switch.

**Remember:** A Material changes appearance; it does not change the mesh's shape or collision by default. Very complex materials can make a game harder to run.

## Material Instances

*Art and materials*

A Material Instance uses a parent Material but lets you change exposed settings, called parameters. You can make variations without rebuilding the whole material graph.

**Try it together:** Add a colour parameter to a parent Material. Create two Material Instances and set one red and the other blue.

**Use case 1:** Team colours: reuse one character material with a red instance for one team and a blue instance for the other.

**Use case 2:** Damage feedback: create a Dynamic Material Instance and briefly change a glow parameter when the player is hit.

**Remember:** Only settings exposed as parameters can be changed this way. Use a Dynamic Material Instance when you need to change suitable parameters during play.

## Textures

*Art and materials*

A Texture stores image data that a Material or interface can use. Some textures hold colours or pictures; others hold surface information such as normal maps, which make lighting suggest small bumps.

**Try it together:** Import an image with your tutor and use it as the colour input of a Material or as an image in a Widget.

**Use case 1:** Surface detail: add a brick pattern and small-looking bumps to a wall without modelling every brick.

**Use case 2:** Interface art: display item icons, button images and a game logo in Widgets.

**Remember:** Large textures use more memory. Normal maps and other data textures need different settings from ordinary colour images; ask your tutor which settings fit.

## Levels / Maps

*Worlds and effects*

A Level, also called a map, stores a world layout with Actors such as scenery, lights and gameplay objects. Opening a map lets you work on that place in the game.

**Try it together:** Make a small practice map with a floor, lighting and a Player Start. Save it with a clear name.

**Use case 1:** Game stages: create a forest map and a cave map with different layouts and challenges.

**Use case 2:** Test spaces: keep a small map for trying movement, enemies or puzzles before adding them to a larger level.

**Remember:** Choose the correct startup map and Game Default Map. Test that maps needed by the game are included when packaging.

## Level Sequences

*Worlds and effects*

A Level Sequence is a timeline for arranging changes over time. Sequencer can control cameras, objects, animation and other tracks to make scenes or scripted moments.

**Try it together:** Create a Level Sequence and add a camera track. Set two camera positions at different times and preview the move.

**Use case 1:** Intro scenes: move a camera through the level before handing control to the player.

**Use case 2:** Puzzle reveals: show a door opening and a camera pointing towards the newly unlocked route.

**Remember:** A sequence must be played to affect the scene. Check which objects its tracks are connected to and what should happen when it finishes.

## Niagara Systems

*Worlds and effects*

A Niagara System combines particle effects, such as sparks, smoke or magic. Its emitters describe how particles are created, move and disappear.

**Try it together:** With your tutor, create a Niagara System from a simple template. Change its colour and preview how long it lasts.

**Use case 1:** Collectable bursts: spawn a short sparkle effect when a player picks up a gem.

**Use case 2:** World atmosphere: add drifting smoke to a chimney or a magic trail behind a moving object.

**Remember:** Too many particles can slow a game down. Check whether the effect should loop and stop or remove it when it is no longer needed.

## Sounds

*Worlds and effects*

A Sound Wave contains imported audio. A Sound Cue can combine sound choices and rules, while MetaSounds can build more detailed sound behaviour. Audio Components can play sounds from Actors.

**Try it together:** Import a short sound and play it when a button is pressed. Compare a 2D interface sound with sound played at a world location.

**Use case 1:** Game feedback: play a short sound when the player collects a coin or presses a menu button.

**Use case 2:** World audio: place a looping waterfall sound that becomes quieter as the player walks away.

**Remember:** Keep volume comfortable. World sounds may need attenuation, which controls how sound changes with distance. Check whether a sound should loop.

## Input Actions / Mapping Contexts

*Player input*

With Enhanced Input, an Input Action names something the player can do, such as Jump or Move. An Input Mapping Context connects keys or controller inputs to those actions.

**Try it together:** Create a Jump Input Action and map Space Bar to it in a Mapping Context. Add the context to the local player's Enhanced Input subsystem and handle the action.

**Use case 1:** Keyboard and gamepad: map Space Bar and a gamepad button to the same Jump action.

**Use case 2:** Different control modes: switch between walking and driving Mapping Contexts when the player enters a vehicle.

**Remember:** The Mapping Context must be added for the local player. Choose the right action value type and trigger event. Check context priority when controls overlap.

## Widgets

*User interface and delivery*

A Widget builds part of a user interface, such as a health bar, button or inventory. Widget Blueprints use UMG, Unreal's interface tools. Editor Utility Widgets build tools inside the editor.

**Try it together:** Create a HUD Widget Blueprint with a health bar. Create an instance for the player and add it to the viewport so it appears on screen.

**Use case 1:** Game HUDs: show health, score and remaining time while the player plays.

**Use case 2:** Inventory menus: display item slots and let the player select an item to inspect or use.

**Remember:** If a menu will not respond, check its owning player, input mode and cursor settings. Update displays when values change rather than doing unnecessary work every frame.

## Project Packaging

*User interface and delivery*

Packaging turns your project into an application that can run outside the editor. Unreal builds the code and cooks assets, which means preparing them for the chosen platform.

**Try it together:** With your tutor, choose the Game Default Map, then use the Platforms toolbar menu to make a Development package for testing.

**Use case 1:** Classroom playtests: make a Windows build so another student can try the game without opening the project in the editor.

**Use case 2:** Final showcases: prepare a tested Shipping build for an agreed platform and check it on the presentation computer.

**Remember:** Test the packaged game, not only Play in Editor. Check that required maps and assets are included. Your tutor can help with platform SDKs and build errors.
