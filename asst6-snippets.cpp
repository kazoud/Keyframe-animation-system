// Task 1:
// =======
  // change ball rendering from wireframe to transparent 
  g_arcballMat.reset(new Material(solid));
  g_arcballMat->getUniforms().put("uColor", 0.4f * glm::vec3(1.0f, 1.0f, 1.0f));
  // g_arcballMat->getRenderStates().polygonMode(GL_FRONT_AND_BACK, GL_LINE);
  g_arcballMat->getRenderStates().enable(GL_BLEND);
  g_arcballMat->getRenderStates().blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

  // you will also need to set the desired alpha value in the fragment shader


// to enable multisampling: 
// request 4 samples when creating the OpenGL context
glfwWindowHint(GLFW_SAMPLES, 4);
// enable multisample rasterization in OpenGL for all objects
glEnable(GL_MULTISAMPLE);        // this is enabled by default



// Task 2:
// =======

// add the header file
#include "script.h" 

// add a global variable to access the animation script
static std::shared_ptr<Script> g_script; 

// create the script with the objects to animate
void initAnimation()
{
    std::vector<glm::mat4*> object_ptrs;          // pointers to objects in scene
    object_ptrs.push_back(&g_skyRbt);
    object_ptrs.push_back(&g_objectRbt[0]);
    object_ptrs.push_back(&g_objectRbt[1]);

    g_script.reset(new Script(object_ptrs));
}

// complete script.cpp and edit the keyboard() callback to hook up
// various keys to actions that edit the animation script


// Task 3:
// ========

// Tips: 
// glm::quat(m) returns the quaternion corresponding to the rotation in the glm::mat4 RBT m
// glm::mat4(q) returns the mat4 RBT corresponding to a rotation quaternion q
// glm::vec3(m[3]) returns the translation vector 
// glm::translate(v) returns a mat4 RBT corresponding to a vec3 translation vector v


// Task 4:
// =======

// add the global variables to control playback 
static int g_ms_between_keyframes = 2000;  // 2 seconds between keyframes 
static int g_animate_fps = 30;             // frames/sec to render during playback
static bool g_animation_on = false;        // used in rendering loop
static float g_anim_time;                  // animation clock in ms

// complete the two interpolation methods:
// void interpolate_from_current(float alpha);   // 0 <= alpha < 1, copies to scene
// bool interpolate(float t);                    // called from animation/rendering loop
// and then hook everything together
