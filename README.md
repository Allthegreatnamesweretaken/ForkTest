# 700120-2425-lab-final

This final lab exercise forms a substantive part of the assessment for both 700106 Real-Time Graphics and 700120 C++ Programming and Design.

## Concept

To create a demo level for a 2.5D platforming game

## Mandatory Features

These features, when perfectly implemented, will provide approx 60% of the final mark. In addition, the list of features are a guide to the building blocks and functionality expected of the simulation.

### Platforms

Horizontal platforms of varying length, with various gaps between them.  The player can only move on these platforms and jump between them.

### Scene composition

The scene is to be composed of 4 distinct depth planes.  From near to far, these are:

- Foreground - contains detailed objects e.g. a rock or grass
- Middle-ground - contains the platforms and the player
- Background - contains any scenery e.g. cave walls, cliffs, buildings, etc
- Skybox - provides the backdrop e.g. sky

### Player

The visual representation of the player is undefined

The player is controlled using the following keys

- move and look left, 'A'
- move and look right, 'D'
- look up, 'W'
- look down, 'S'
- jump, 'Space Bar'

The jump mechanics use the players 'look' vector to determine the jump direction.  ie if the player is looking up and to the left, then the player will jump up and to the left.

The look at direction should be clearly indicated on the player

### Objects

You are required to place objects within the scene (in either fore, mid or background planes) that demonstrates one or more of the shading, animation and lighting effects listed below.  Advanced effects (see below for a full description) are marked with a (*)

The type of object are not defined, although they should be different, and selected to clearly enhance the chosen shading, lighting, animation effects.

Items placed on any of the planes can be either 2D or 3D.  This is undefined.

At least one of the objects should consist of a mesh containing 100's of polygons

### Model loader

You are required to write your own model loader for the Wavefront OBJ file format [Wavefront OBJ File Format Summary](https://www.fileformat.info/format/wavefrontobj/egff.htm).

Your model loader should be able to load basic models (i.e. those required for this assessment).  There is no requirement to load all models supported by the OBJ format.

### Particles

A particle system is required within the scene.  It's exact purpose is left open. e.g. rain, snow, explosion, etc

### Shading effects

1. Per-pixel shading (i.e. Phong)
2. Per-vertex shading (i.e. Gouraud)
3. texturing mapping
4. bump mapping (*)
5. displacement mapping (*)
6. toon or cell shading (*)
7. environment mapping (*)
8. HDR rendering (*)
9. Deferred rendering (*)

### Animation effects

1. movement under gravity (e.g. falling from ceiling to floor)
2. translation (e.g. moving left and right)

### Lighting effects

1. daylight, a mix of ambient and directional
2. torch, a spot light, located on the player and orientated in the same direction that the player is looking (a small about of ambient would also be beneficial)
3. shadow mapping (*)
4. advanced shadow mapping (*)

### Camera

The application should contain a single camera, positioned in such a way that the player is always located in the centre of the screen.

The camera is oriented to look directly into the scene.

The camera is constrained to move automatically with the player.

The camera only has the following controls

- zoom in, '='
- zoom out, '-'

### Controls

Note: the following key assignments are non-negotiable; they are expected to work in each submission to ease the demonstration process

- 'ESC' exits the application
- 'r' resets the application to its initial state
- 't' toggles between daylight and torch mode
- '<'/'>' decrease/increase a factor that globally slows/speeds-up time-dependent effects (i.e. speed of falling objects, etc)
- camera controls as specified previously
- player controls as specified previously

### Configuration File

Scene elements and their animations paths as well as lights and camera configurations are read from a configuration file.

## Advanced Effects

Each advanced effects has an associated difficultly point. The maximum mark available for each effect is this difficulty point. Only 15 marks are available for implementing advanced effects. These 15 marks can be gained either by perfectly implementing effects worth 15 points, or by partially implementing effects worth up to 20 points. Any effects implemented beyond 20 points will not be marked. Marks for advanced effects will be capped at 15.

### Shading

- [2 points] bump mapping of geometry
- [1 points*] displacement mapping of geometry [*builds on bump mapping]
- [2 points] toon or cell shading of geometry
- [2 points] environment cube mapping on all surfaces with a high specular component

### Advanced Shadows

- [4 points] Shadow mapping
- [2 points*] Advanced shadow mapping (e.g. PCF, VSM) [*builds on shadow mapping]

### Scene level rendering modes

- [4 points] Deferred rendering, including the ability to switch the visualization for the various MRT buffers
- [2 points] High-dynamic range rendering and tone-mapped post-processing

### Novel features

- [6 points] Extra marks are awarded for any novel features that enhance the simulation, beyond the initial specification.

## UI

Statistics and control elements using IMGUI

## Hints

The following may help you to successfully complete your work:

- Produce a paper visualization (e.g., story-board sketches) of your project and check it with a member of the module team
- Produce a top-level software design and check it with a member of the module team.
- Prototype your ideas to help produce a more detailed design
- Test your software at each stage of development
- Make use of Copilot
- Document as you go

## Implementation

The final implementation must be able to run on a standard windows PC in the graphics lab, i.e. RBB-335.

ONLY the following libraries and APIs are permitted:

- DirectX 11 SDK
- DirectMath
- [IMGUI](http://github.com/ocornut/imgui)

Examples of libraries and APIs that are NOT permitted include:

- DirectX toolkit
- WinRT
- OpenGL / Vulkan

The use of any language feature within C++ 11, 14, 17, 20 or 22 is permitted (provided it is supported by the Microsoft compiler within VS2022).

## Code submission

Please ensure the following requirements are met when submitting your source code.

- only submit source code that is used in this application (ie. not all your tutorials)
- clean the solution prior to submission
- ensure that the solution builds, without the need to install any additional librairies
- ensure that Parasoft C++ Static Test runs on the solution
- suppress Parasoft C++ Static Test warnings from any library header files, by placing these comment blocks at the very beginning and end of the files

```c++
// parasoft-begin-suppress ALL "suppress all violations"
```

```c++
// parasoft-end-suppress ALL "suppress all violations"
```

The code can ONLY be submitted via the provided GitHub Classroom repository (ie Lab I (the final lab), a link to which is provided on Canvas)

It is your responsibility to make very sure that all the source code files required to make your application are committed to the GitHub Classroom repository. It’s easy to forget to add or push some while you are developing. A good way to check this is to check out your project to a different folder and see if you can build it as follows:

1. Clone your repository in to an empty folder.
2. Open the solution in Visual Studio
3. Run your project(s). Remember that to achieve full marks your submitted code must compile and run.
4. If any of those previous steps doesn't work you probably forgot to add/commit/push one or more of your files. GOTO step 1.

Do not leave it until the last minute to commit and push your work. Committing your repository all in one go may take upwards of an hour to complete. When you do commit your work it is recommended that you include the phrase "ACW FINAL" in the commit log. This indicates that you have made a submission. You can do this multiple times before the deadline if you need to. Submissions made after the deadline will be subject to the usual penalties.

## Lab Book

You are required to add the following two entries to your Lab Book:

### Design (700120) – [word limit 1000]

- A textual description of each class giving the name, role and responsibilities
- Class diagram(s) containing 
  - main classes
  - service/utility classes
- Interaction diagram(s) for significant components of the software design
- A critique of the design, including
  - The merits of the design
  - Weaknesses of the design
  - What would you change and why

### Graphics (700106) – [word limit 1000]

- Document and critique the algorithms used, including:
  - Geometry representation and processing
  - Shading and lighting
  - Shadow generation
- Explain how application objects and their graphics representation are connected; discuss the advantages/disadvantages of your approach
- Explain how application-object behaviour is updated and how these updates are propagated to the graphics representation
- Discuss potential extensions (especially with respect to non-implemented items from the advanced feature list) as well as potential scalability issues
- What feature (not necessarily mentioned before) would you have liked to add?

The lab book is to be submitted via Canvas

## Video

A 3 to 5 minute video with audio commentary. The video should highlight each of the key elements of your implementation. It is strongly suggested that you work through the list of mandatory and optional features within the specification, showing each in turn.

The video is to be submitted via Canvas

## Demonstration

You will be required to demonstrate your solution, the purpose of which is

- To assess the quality and quantity of the features you have chosen to implement.  We’ll work through each feature in the specification.
- To assess your knowledge and understanding of your implementation.  We’ll ask you a series of questions related to how and why you chose to implement a certain feature in the way you did.
  
We recognise that your solution may consist of a fairly large code base, some of which you may not have touched for several weeks prior to the demo.  To simplify the process, we’ll release a set of possible questions before the demonstration, to enable you to prepare your answer.
