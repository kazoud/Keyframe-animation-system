# Keyframe-animation-system

<ul>
<li>This project contains a scene with three objects: A red cube, a blue cube, and the eye through which the user looks at the scene</li>
<li>Their positions and rotations are stored as 4x4 matrices</li>
<li>Users can create keyframes to store the current position and rotation of the objects. The keyframe is stored as a vector of matrices</li>
<li>Created keyframes are added to a list of keyframes implemented as a doubly linked list</li>
<li>An animation that interpolates between all the keyframes can then be played</li>
</ul>

Commands to interact with the objects: 
<ul>
<li>Left click: Rotate the object</li>
<li>Right click: Translate the object</li>
<li>Middle mouse: Translate in the z-direction</li>
<li>'O' key: Toggle between which object to control</li>
<li>'V' key: View the scene from the point of view of the controlled object</li>
<li>'N' key: Create a new keyframe</li>
<li>Left/Right arrow keys: Go to the previous/next keyframe</li>
<li>'U' key: Update an existing keyframe</li>
<li>'C' key: Copy an already existing keyframe to the current scene</li>
<li>'Y' key: Play the animation</li>

</ul>

## Dependencies

[GLFW](https://www.glfw.org) <br>
[glew](http://glew.sourceforge.net/) <br>
[GLM](https://glm.g-truc.net/0.9.9/index.html)
