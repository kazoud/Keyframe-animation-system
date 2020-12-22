#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// #define GLFW_DLL                // uncomment to use dynamic libraries on Windows


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "glmutils.h"
#include "geometrymaker.h"
#include "geometry.h"
#include "material.h"

#include "ppm.h"
#include "glsupport.h"
#include "arcball.h"
#include "script.h"

using namespace std; // for string, vector, iostream, and other standard C++ stuff

// G L O B A L S ///////////////////////////////////////////////////

static const float g_frustMinFov = 60.0;  // A minimal of 60 degree field of view
static float g_frustFovY = g_frustMinFov; // FOV in y direction (updated by updateFrustFovY)

static const float g_frustNear = -0.1;  // near plane
static const float g_frustFar = -50.0;  // far plane
static const float g_groundY = -2.0;    // y coordinate of the ground
static const float g_groundSize = 10.0; // half the ground length

static int g_windowWidth = 512;
static int g_windowHeight = 512;
static bool g_mouseClickDown = false; // is the mouse button pressed
static bool g_mouseLClickButton, g_mouseRClickButton, g_mouseMClickButton;
static bool g_spaceDown = false;            // space state, for middle mouse emulation
static double g_mouseClickX, g_mouseClickY; // coordinates for mouse click event
static int g_activeShader = 0;

static int g_ms_between_keyframes = 2000;
static int g_animate_fps = 30;
static float g_anim_time = 0.0f;
static bool g_animation_on = false;

// --------- Shaders

static shared_ptr<Material> g_cubeDiffuseMat[2],
g_bumpFloorMat,
g_arcballMat;


// --------- Geometry

// Vertex buffer and index buffer associated with the ground and cube geometry
static shared_ptr<Geometry> g_ground, g_cube;
// Vertex buffer and index buffer of the sphere geometry
static shared_ptr<Geometry> g_sphere;

//Pointer to animation script
static std::shared_ptr<Script> g_script;

// --------- Scene

static const glm::vec3 g_light1(2.0, 3.0, 14.0), g_light2(-2, -3.0, -5.0); // define two lights positions in world space
static glm::mat4 g_skyRbt = glm::translate(glm::vec3(0.0, 0.25, 4.0));
static glm::mat4 g_objectRbt[2] = { glm::translate(glm::vec3(-1.0f, 0, 0)),
                                    glm::translate(glm::vec3(1.0f, 0, 0)) };
static glm::vec3 g_objectColors[2] = { glm::vec3(1, 0, 0),
                                       glm::vec3(0, 0, 1) };
static glm::mat4 g_ballRbt = glm::translate(glm::vec3(0.f, 0.0f, 0.f));
static glm::vec3 g_ballColor(0.2f, 0.8f, 0.3f);  //  greenish

// --------- User interface variables

static const int g_nObjects = 2;             // 2 cubes
static const int g_nViews = g_nObjects + 1;  // 2 cubes and sky
static int g_currentView = 0;                // initialize to sky view
static int g_activeObject = 0;               // initialize to sky
static int g_skyAMatrixChoice = 0;           // 0 = world-sky, 1 = sky-sky


static int g_arcballScreenRadius = 0.25 * std::fmin(g_windowWidth, g_windowHeight);
static float g_arcballScale = 1.0f / g_arcballScreenRadius;


///////////////// END OF G L O B A L S //////////////////////////////////////////////////


static void initGround() {
    int ibLen, vbLen;
    getPlaneVbIbLen(vbLen, ibLen);

    // Temporary storage for cube Geometry
    vector<VertexPNTBX> vtx(vbLen);
    vector<unsigned short> idx(ibLen);

    makePlane(g_groundSize * 2, vtx.begin(), idx.begin());
    g_ground.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vbLen, ibLen));
}

static void initCubes() {
    int ibLen, vbLen;
    getCubeVbIbLen(vbLen, ibLen);

    // Temporary storage for cube Geometry
    vector<VertexPNTBX> vtx(vbLen);
    vector<unsigned short> idx(ibLen);

    makeCube(1, vtx.begin(), idx.begin());
    g_cube.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vbLen, ibLen));
}

static void initSphere() {
    int ibLen, vbLen;
    getSphereVbIbLen(20, 10, vbLen, ibLen);

    // Temporary storage for sphere Geometry
    vector<VertexPNTBX> vtx(vbLen);
    vector<unsigned short> idx(ibLen);
    makeSphere(1, 20, 10, vtx.begin(), idx.begin());
    g_sphere.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vtx.size(), idx.size()));
}

void initAnimation()
{
    std::vector<glm::mat4*> object_ptrs;          // pointers to objects in scene
    object_ptrs.push_back(&g_skyRbt);
    object_ptrs.push_back(&g_objectRbt[0]);
    object_ptrs.push_back(&g_objectRbt[1]);

    g_script.reset(new Script(object_ptrs));
}

static bool arcball_in_use(void)
{
    int object = g_activeObject;
    int view = g_currentView;
    int sky_wrt = g_skyAMatrixChoice;

    if ((object == 0 && view == 0 && sky_wrt == 0) ||
        (object != 0 && view == 0))
        return true;
    else
        return false;
}

static bool z_translating(void)
{
    // is middle, or (left and right), or (left + space) button down?
    if (g_mouseMClickButton ||
        (g_mouseLClickButton && g_mouseRClickButton) ||
        (g_mouseLClickButton && !g_mouseRClickButton && g_spaceDown))
        return true;
    else
        return false;
}


// takes a projection matrix and send to the the shaders
inline void sendProjectionMatrix(Uniforms& uniforms, const glm::mat4& projMatrix) {
    uniforms.put("uProjMatrix", projMatrix);
}

// takes MVM and its normal matrix to the shaders
inline void sendModelViewNormalMatrix(Uniforms& uniforms, const glm::mat4& MVM, const glm::mat4& NMVM) {
    uniforms.put("uModelViewMatrix", MVM).put("uNormalMatrix", NMVM);
}

// update g_frustFovY from g_frustMinFov, g_windowWidth, and g_windowHeight
static void updateFrustFovY()
{
    if (g_windowWidth >= g_windowHeight)
        g_frustFovY = g_frustMinFov;
    else
    {
        const double RAD_PER_DEG = 0.5 * M_PI / 180;
        g_frustFovY = atan2(sin(g_frustMinFov * RAD_PER_DEG) * g_windowHeight / g_windowWidth, cos(g_frustMinFov * RAD_PER_DEG)) / RAD_PER_DEG;
    }
}

// perspective projection
static glm::mat4 makeProjectionMatrix()
{
    glm::mat4 proj = makeProjection(g_frustFovY, (float)g_windowWidth / (float)g_windowHeight, g_frustNear, g_frustFar);
    return proj;
}


static void drawStuff() {

    // Declare an empty uniforms
    Uniforms uniforms;

    // Get your projection matrix into proj mat as usual
    const glm::mat4 projmat = makeProjectionMatrix();

        // send proj. matrix to be stored by uniforms,
        // as opposed to the current vtx shader
        sendProjectionMatrix(uniforms, projmat);


    // get your eyeRbt, invEyeRbt and stuff as usual
        const glm::mat4 eyeRbt = (g_currentView == 0) ? g_skyRbt : g_objectRbt[g_currentView - 1];
        const glm::mat4 invEyeRbt = glm::inverse(eyeRbt);

        // get the eye space coordinates of the two light as usual
        // suppose they are stored as Cvec3 eyeLight1 and eyeLight2

        const glm::vec3 eyeLight1 = glm::vec3(invEyeRbt * glm::vec4(g_light1, 1)); // g_light1 position in eye coordinates
        const glm::vec3 eyeLight2 = glm::vec3(invEyeRbt * glm::vec4(g_light2, 1)); // g_light2 position in eye coordinates

        // send the eye space coordinates of lights to uniforms
        uniforms.put("uLight", eyeLight1);
        uniforms.put("uLight2", eyeLight2);


    // For draw ground
    // ---------------
    // set the ground object frame 
    const glm::mat4 groundRbt = glm::translate(glm::vec3(0.0f, g_groundY, 0.0f));
    glm::mat4 MVM = invEyeRbt * groundRbt;
    glm::mat4 NMVM = normalMatrix(MVM);

        // Use uniforms as opposed to curSS
        sendModelViewNormalMatrix(uniforms, MVM, NMVM);

    // draw ground geometry with its shader
    g_bumpFloorMat->draw(*g_ground, uniforms);


    // For draw cubes:
    // ---------------
    for (int i = 0; i < 2; i++)
    {
        MVM = invEyeRbt * g_objectRbt[i];
        NMVM = normalMatrix(MVM);

            // Use uniforms as opposed to curSS
            sendModelViewNormalMatrix(uniforms, MVM, NMVM);

        // draw cube with diffuse shader
        g_cubeDiffuseMat[i]->draw(*g_cube, uniforms);
    }



    // For arcball drawing:
    //----------------------

    // calculate arcball MVM as usual and store, say in, MVM
    g_ballRbt = (g_activeObject == 0) ? glm::mat4(1.0f) : g_objectRbt[g_activeObject - 1];
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw wireframe
    MVM = invEyeRbt * g_ballRbt;
    if (!z_translating())
        g_arcballScale = getScreenToEyeScale(MVM[3].z, g_frustFovY, g_windowHeight);
    MVM *= glm::scale(glm::vec3(g_arcballScale * g_arcballScreenRadius));
    NMVM = normalMatrix(MVM);

        // Use uniforms as opposed to curSS
        sendModelViewNormalMatrix(uniforms, MVM, normalMatrix(MVM));

    // No more glPolygonMode calls

    g_arcballMat->draw(*g_sphere, uniforms);

    // No more glPolygonMode calls

}


static void display(GLFWwindow* window) {
    // No more glUseProgram

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawStuff();               // no more curSS

    glfwSwapBuffers(window);

    checkGlErrors();
}

static void reshape(GLFWwindow* window, const int w, const int h)
{
    g_windowWidth = w;        // units are screen coordinates not pixels
    g_windowHeight = h;       // on mac retina display, 1 screen coord = 2 pixels
    int w_pixels, h_pixels;   // width/height of window in pixels, not screen coordinates
    glfwGetFramebufferSize(window, &w_pixels, &h_pixels);
    glViewport(0, 0, w_pixels, h_pixels);     // arguments must be in pixels 
    cerr << "Size of window is now " << w << "x" << h << endl;

    g_arcballScreenRadius = 0.25 * fmin(g_windowWidth, g_windowHeight);
    // cerr << "Arcball Radius: " << g_arcballScreenRadius << endl;
    updateFrustFovY();
    display(window);
}

glm::mat4 doMtoOwrtA(const glm::mat4& M, const glm::mat4& O, const glm::mat4& A)
{
    return A * M * glm::inverse(A) * O;
}

// Frame that we're manipulating the current object with respect to. This is:
//   - If we're manipulating a cube and the eye is the sky, this should be the cube-sky frame.
//   - If we're manipulating cube i and eye is cube j, this should be the cube i-cube j frame.
//   - If we're manipulating the sky camera and eye is the sky, we have two
//     viable frames, and pressing 'm' switches between them:
//      - World-sky frame (like orbiting around the world)
//      - Sky-sky frame (like moving your head)
static glm::mat4 setWrtFrame(int object, int view, int sky_pick)
{
    if ((object != 0) && (view == 0))
        return transFact(g_objectRbt[object - 1]) * linFact(g_skyRbt);  // cube-sky frame

    if ((object == 0) && (view == 0))
        return (sky_pick == 0) ? linFact(g_skyRbt) : g_skyRbt; // world-sky or sky-sky

    if ((object != 0) && (view != 0))
        return transFact(g_objectRbt[object - 1]) * linFact(g_objectRbt[view - 1]);

    return linFact(g_skyRbt);     // world-sky, default wrt frame, in case needed
}


glm::mat4 getArcballRotation(float x, float y)
{
    const glm::mat4 eyeRbt = (g_currentView == 0) ? g_skyRbt : g_objectRbt[g_currentView - 1];
    const glm::mat4 invEyeRbt = glm::inverse(eyeRbt);
    const glm::mat4 MVM = invEyeRbt * g_ballRbt;
    const glm::mat4 projmat = makeProjectionMatrix();
    const glm::vec3 ball_center = glm::vec3(MVM[3]);
    const glm::vec2 sphere_center = getScreenSpaceCoord(ball_center, projmat, g_frustNear, g_frustFovY,
        g_windowWidth, g_windowHeight);
    const glm::vec2 p1 = glm::vec2(g_mouseClickX, g_mouseClickY) - sphere_center;
    const glm::vec2 p2 = glm::vec2(x, y) - sphere_center;
    const int r = g_arcballScreenRadius;

    float v1z = std::sqrt(std::fmax(0.0f, r * r - p1.x * p1.x - p1.y * p1.y));
    float v2z = std::sqrt(std::fmax(0.0f, r * r - p2.x * p2.x - p2.y * p2.y));
    glm::vec3 v1 = glm::vec3(p1, v1z);
    glm::vec3 v2 = glm::vec3(p2, v2z);

    glm::quat q = glm::normalize(glm::quat(glm::dot(v1, v2), glm::cross(v1, v2)));   // unit quaternion
    return glm::mat4_cast(q);
}


static void motion(GLFWwindow* window, double x, double y)
{
    const float dx = x - g_mouseClickX;
    const float dy = g_windowHeight - y - 1 - g_mouseClickY;

    glm::mat4 m, A;
    float translation_scale = arcball_in_use() ? g_arcballScale : 0.01f;  // screen coords to eye coords

    // generate the affine transformation m
    if (g_mouseLClickButton && !g_mouseRClickButton && !g_spaceDown) // left button down?
    {
        if (arcball_in_use())
            m = getArcballRotation(x, g_windowHeight - y - 1);
        else
            m = glm::rotate(glm::radians(-dy), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(dx), glm::vec3(0, 1, 0));
    }
    else if (g_mouseRClickButton && !g_mouseLClickButton) // right button down?
    {
        m = glm::translate(glm::vec3(dx, dy, 0.0f) * translation_scale);
    }
    else if (g_mouseMClickButton ||
        (g_mouseLClickButton && g_mouseRClickButton) ||
        (g_mouseLClickButton && !g_mouseRClickButton && g_spaceDown)) // middle or (left and right), or (left + space) button down?
    {
        m = glm::translate(glm::vec3(0, 0, -dy) * translation_scale);
    }

    // apply it to active object wrt to A
    if (g_mouseClickDown)
    {
        A = setWrtFrame(g_activeObject, g_currentView, g_skyAMatrixChoice);

        if ((g_currentView == 0) && (g_activeObject == 0)) {    // if eye is sky, and sky is active
            m = glm::inverse(m);               // signs are inverted when manipulating the eye frame
            g_skyRbt = doMtoOwrtA(m, g_skyRbt, A);
        }
        else if (g_activeObject != 0) {        // if cube is active
            int k = g_activeObject - 1;    // cube index
            g_objectRbt[k] = doMtoOwrtA(m, g_objectRbt[k], A);
        }
    }

    g_mouseClickX = x;
    g_mouseClickY = g_windowHeight - y - 1;
}

static void mouse(GLFWwindow* window, int button, int action, int mods)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    g_mouseClickX = x;
    g_mouseClickY = g_windowHeight - y - 1; // conversion from GLFW window-coordinate-system to OpenGL window-coordinate-system
    // cerr << "mouce click: " << x << " " << y << endl;

    g_mouseLClickButton |= (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS);
    g_mouseRClickButton |= (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS);
    g_mouseMClickButton |= (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS);

    g_mouseLClickButton &= !(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE);
    g_mouseRClickButton &= !(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE);
    g_mouseMClickButton &= !(button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE);

    g_mouseClickDown = g_mouseLClickButton || g_mouseRClickButton || g_mouseMClickButton;
}


// cycle over the eye frame being used
static void cycleEyeMode() {
    g_currentView = (g_currentView + 1) % g_nViews;
    cout << "Active eye is " << ((g_currentView == 0) ? "Sky" : "Cube " + to_string(g_currentView - 1)) << endl;
}

// cycle over object being manipulated.
static void cycleObject() {
    g_activeObject = (g_activeObject + 1) % g_nViews;
    cout << "Active object is " <<
        ((g_activeObject == 0) ? "Sky " : "Cube " + to_string(g_activeObject - 1)) << endl;
}

// toggle sky A matrix
static void toggleSkyAMatrix() {
    if ((g_currentView == 0) && (g_activeObject == 0)) {
        g_skyAMatrixChoice = !g_skyAMatrixChoice;
        cout << "Editing sky eye wrt " << (g_skyAMatrixChoice ? "sky-sky" : "cube-sky") << endl;
    }
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE: // ESC
        case GLFW_KEY_Q:      // q
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_H:
            cout << " ============== H E L P ==============\n\n"
                << "h\t\thelp menu\n"
                << "s\t\tsave screenshot\n"
                << "f\t\tToggle flat shading on/off.\n"
                << "o\t\tCycle object to edit\n"
                << "v\t\tCycle view\n"
                << "m\t\tToggle wrt frame (when manipulating sky eye)"
                << "drag left mouse to rotate\n"
                << endl;
            break;
        case GLFW_KEY_S:
            glFlush();
            writePpmScreenshot(g_windowWidth, g_windowHeight, "out.ppm");
            break;

        case GLFW_KEY_SPACE: // ' '
            g_spaceDown = true;
            break;
        case GLFW_KEY_V:
            cycleEyeMode();
            break;
        case GLFW_KEY_O:
            cycleObject();
            break;
        case GLFW_KEY_M:
            toggleSkyAMatrix();
            break;
        case GLFW_KEY_C:
            g_script->copy_to_scene();
            break;
        case GLFW_KEY_N:
            g_script->add_from_scene();
            break;
        case GLFW_KEY_U:
            g_script->update_from_scene();
            break;
        case GLFW_KEY_LEFT:
            g_script->retreat();
            break;
        case GLFW_KEY_RIGHT:
            g_script->advance();
            break;
        case GLFW_KEY_D:
            g_script->delete_current_frame();
            break;
        case GLFW_KEY_Y:
            if (g_script->nkeyframes() < 2)
            {
                std::cout << "Warning: You cannot start an animation with less than 2 keyframes." << std::endl;
            }
            else
            {
                g_script->init_playback();
                g_animation_on = !g_animation_on;
            }
            break;
        case GLFW_KEY_UP:
            g_ms_between_keyframes -= 300;
            break;
        case GLFW_KEY_DOWN:
            g_ms_between_keyframes += 300;
            break;
        }
        
    }
    else if (action == GLFW_RELEASE)
    {
        switch (key)
        {
        case GLFW_KEY_SPACE: // ' '
            g_spaceDown = false;
            break;
        }
    }
}

GLFWwindow* initGLFWState()
{
    // Initialize the library
    if (!glfwInit())
        return NULL;

    // request OpenGL 4.1 with core and comptability profiles
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE); // default framebuffer is double-buffered
    glfwWindowHint(GLFW_SAMPLES, 4); // Asking for a multisample buffer
    // default is: 8 bits for R, G, B, A; 24 bits for depth buffer; 8 bits for stencil buffer
    // change here if needed.

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(g_windowWidth, g_windowHeight, "Asst 6", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return NULL;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // register callbacks
    glfwSetKeyCallback(window, keyboard);            // key press callback
    glfwSetWindowSizeCallback(window, reshape);      // window reshape callback
    glfwSetCursorPosCallback(window, motion);        // mouse movement callback
    glfwSetMouseButtonCallback(window, mouse);       // mouse click callback

    return window;
}


static void initGLState()
{
    glClearColor(128. / 255., 200. / 255., 255. / 255., 0.);
    glClearDepth(0.);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glReadBuffer(GL_BACK);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_MULTISAMPLE);
}



static void initGeometry()
{
    initGround();
    initCubes();
    initSphere();
    initAnimation();
}

static void initMaterials() {
    // Create some prototype materials
    Material diffuse("./shaders/basic.vert", "./shaders/diffuse.frag");
    Material solid("./shaders/basic.vert", "./shaders/solid.frag");

    // copy diffuse prototype and set red color
    g_cubeDiffuseMat[0].reset(new Material(diffuse));
    g_cubeDiffuseMat[0]->getUniforms().put("uColor", glm::vec3(1.0f, 0.0f, 0.0f));
    g_cubeDiffuseMat[0]->getUniforms().put("uTexColor", shared_ptr<ImageTexture>(new ImageTexture("smiley.ppm", true)));

    // copy diffuse prototype and set blue color
    g_cubeDiffuseMat[1].reset(new Material(diffuse));
    g_cubeDiffuseMat[1]->getUniforms().put("uColor", glm::vec3(0.0f, 0.0f, 1.0f));
    g_cubeDiffuseMat[1]->getUniforms().put("uTexColor", shared_ptr<ImageTexture>(new ImageTexture("smiley.ppm", true)));

    // normal mapping material
    g_bumpFloorMat.reset(new Material("./shaders/normal.vert", "./shaders/normal.frag"));
    g_bumpFloorMat->getUniforms().put("uTexColor", shared_ptr<ImageTexture>(new ImageTexture("Fieldstone.ppm", true)));
    g_bumpFloorMat->getUniforms().put("uTexNormal", shared_ptr<ImageTexture>(new ImageTexture("FieldstoneNormal.ppm", false)));

    // copy solid prototype, and set to wireframed/transparent rendering
    g_arcballMat.reset(new Material(solid));
    //g_arcballMat->getUniforms().put("uColor", glm::vec3(0.2f, 0.8f, 0.3f));
    g_arcballMat->getUniforms().put("uColor", 0.4f * glm::vec3(1.0f, 1.0f, 1.0f));
    //g_arcballMat->getRenderStates().polygonMode(GL_FRONT_AND_BACK, GL_LINE);
    g_arcballMat->getRenderStates().enable(GL_BLEND);
    g_arcballMat->getRenderStates().blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
};

bool testSlerping()
{
    //Testing Slerping
    glm::quat cube1rot = glm::quat(g_objectRbt[0]);
    glm::quat cube2rot = glm::quat(g_objectRbt[1]);
    glm::quat interpolatedRot = glm::slerp(cube1rot, cube2rot, 0.0f);

    return (cube1rot == interpolatedRot);
}

int main(int argc, char** argv)
{
    GLFWwindow* window = initGLFWState();
    assert(window);

    std::cout << "OpenGL version " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glewInit(); // load the OpenGL extensions

    if (!GLEW_VERSION_4_1)
    {
        std::cerr << "Error: OpenGL/GLSL v4.1 not supported" << std::endl;
        return 1;
    }

    initGLState();
    initMaterials();
    initGeometry();

    while (!glfwWindowShouldClose(window)) // Loop until the user closes the window
    {
        if (g_animation_on)
        {
            float t = g_anim_time / (float)g_ms_between_keyframes;
            bool end_reached = g_script->interpolate(t);

            if (!end_reached)
            {
                g_anim_time += 1000.0f / (float)g_animate_fps;
            }

            else
            {
                g_animation_on = false;
                std::cout << "Finished playing animation. " << std::endl;
                g_script->end_playback();
            }
        }
        display(window);  // Render
        glfwWaitEventsTimeout(1.0f / (float) g_animate_fps);
        glfwPollEvents(); // Poll for and process events
    }

    glfwTerminate();
    return 0;
}
