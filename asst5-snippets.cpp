//==================================================================
// STEP 0: Include the newly provided "geometry.h"
//==================================================================
#include "geometry.h"
#include "material.h"

//==================================================================
// STEP 1: Replace the "------- Shaders" section with the following.
//         In particular you need to delete g_numShaders, g_shaderFiles,
//         g_shaderFilesGl2, g_shaderStates, and so on
//==================================================================

// --------- Materials
// This should replace all the contents in the Shaders section, e.g., g_numShaders, g_shaderFiles, and so on
static shared_ptr<Material> g_cubeDiffuseMat[2],
                            g_bumpFloorMat,
                            g_arcballMat; 


//==================================================================
// STEP 2: Edit the "------- Geometry" section with the following:
//
//         In particular you need to delete the FIELD_OFFSET macro,
//         struct VertexPN, and struct Geometry since they are now
//         superseded by the provided geometry header.
//
//=================================================================


//======================================================================
// STEP 3: Replace initGround(), initCube(), and initSphere() functions
//         with the following defintion. This ensures VertexPNTBX and
//         SimpleIndexedGeometryPNTBX are used, which provides extra
//         vertex attributes used for Bump Mapping later
//=======================================================================
static void initGround() {
  int ibLen, vbLen;
  getPlaneVbIbLen(vbLen, ibLen);

  // Temporary storage for cube Geometry
  vector<VertexPNTBX> vtx(vbLen);
  vector<unsigned short> idx(ibLen);

  makePlane(g_groundSize*2, vtx.begin(), idx.begin());
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

//======================================================================
// STEP 4: Change the definitions of sendProjectionMatrix and 
//         sendModelViewNormalMatrix to use Uniforms
//         instead of ShaderState
//=======================================================================

// takes a projection matrix and send to the shaders
inline void sendProjectionMatrix(Uniforms& uniforms, const glm::mat4& projMatrix) {
  uniforms.put("uProjMatrix", projMatrix);
}

// takes MVM and its normal matrix to the shaders
inline void sendModelViewNormalMatrix(Uniforms& uniforms, const glm::mat4& MVM, const glm::mat4& NMVM) {
  uniforms.put("uModelViewMatrix", MVM).put("uNormalMatrix", NMVM);
}

//=========================================================================
// STEP 5: Simplify drawStuff() by using materials
//         In particular, all occurence of
//             const ShaderState& curSS
//         should probably be replaced with
//             Uniforms& uniforms
//         and occurences of `curSS' be replaced with `uniforms'
//=========================================================================

//-------------------------
// For drawStuff(...)
//-------------------------

// drawStuff just takes in a picking flag, and no curSS
static void drawStuff() {

  // Declare an empty uniforms
  Uniforms uniforms;

  // Get your projection matrix into proj mat as usual
  ...

  // send proj. matrix to be stored by uniforms,
  // as opposed to the current vtx shader
  sendProjectionMatrix(uniforms, projmat);


  // get your eyeRbt, invEyeRbt and stuff as usual
  ...

  // get the eye space coordinates of the two light as usual
  // suppose they are stored as Cvec3 eyeLight1 and eyeLight2
  ...

  // send the eye space coordinates of lights to uniforms
  uniforms.put("uLight", eyeLight1);
  uniforms.put("uLight2", eyeLight2);


  // For draw ground
  // ---------------
  // set the ground object frame 
  const glm::mat4 groundRbt = glm::translate(glm::vec3(0.0f, g_groundY, 0.0f));
  ... 

  // Use uniforms as opposed to curSS
  sendModelViewNormalMatrix(uniforms, MVM, NMVM);
  
  // draw ground geometry with its shader
  g_bumpFloorMat->draw(*g_ground, uniforms); 


   // For draw cubes:
   // ---------------
   for (int i = 0; i < 2; i++) 
   {
      ...
      
      // Use uniforms as opposed to curSS
      sendModelViewNormalMatrix(uniforms, MVM, NMVM);

      // draw cube with diffuse shader
      g_cubeDiffuseMat[i]->draw(*g_cube, uniforms); 
   }



  // For arcball drawing:
  //----------------------

  // calculate arcball MVM as usual and store, say in, MVM
  ...

  // Use uniforms as opposed to curSS
  sendModelViewNormalMatrix(uniforms, MVM, normalMatrix(MVM));

  // No more glPolygonMode calls

  g_arcballMat->draw(*g_sphere, uniforms);

  // No more glPolygonMode calls

}


//=========================================================================
// STEP 6: display() and pick() should now use the simplified drawStuff()
//         as follows
//=========================================================================


static void display(GLFWwindow *window) {
  // No more glUseProgram

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

  drawStuff();               // no more curSS

  glfwSwapBuffers(window); 

  checkGlErrors();
}


//=========================================================================
// STEP 7: Remove the initShaders() function and replace it with the
//         following initMaterials(), and call it in main()
//=========================================================================
static void initMaterials() {
  // Create some prototype materials
  Material diffuse("./shaders/basic.vert", "./shaders/diffuse.frag");
  Material solid("./shaders/basic.vert", "./shaders/solid.frag");

  // copy diffuse prototype and set red color
  g_cubeDiffuseMat[0].reset(new Material(diffuse));
  g_cubeDiffuseMat[0]->getUniforms().put("uColor", glm::vec3(1.0f, 0.0f, 0.0f));

  // copy diffuse prototype and set blue color
  g_cubeDiffuseMat[1].reset(new Material(diffuse));
  g_cubeDiffuseMat[1]->getUniforms().put("uColor", glm::vec3(0.0f, 0.0f, 1.0f));

  // normal mapping material
  g_bumpFloorMat.reset(new Material("./shaders/normal.vert", "./shaders/normal.frag"));
  g_bumpFloorMat->getUniforms().put("uTexColor", shared_ptr<ImageTexture>(new ImageTexture("Fieldstone.ppm", true)));
  g_bumpFloorMat->getUniforms().put("uTexNormal", shared_ptr<ImageTexture>(new ImageTexture("FieldstoneNormal.ppm", false)));

  // copy solid prototype, and set to wireframed rendering
  g_arcballMat.reset(new Material(solid));
  g_arcballMat->getUniforms().put("uColor", glm::vec3(0.2f, 0.8f, 0.3f));
  g_arcballMat->getRenderStates().polygonMode(GL_FRONT_AND_BACK, GL_LINE);
};

...

int main(int argc, char * argv[]) {
  ...
  initGLState();

  // remove initShaders() and add initMaterials();
  // initShaders();
  initMaterials();

  initGeometry();
  ...
}


//=========================================================================
// STEP 8: Minor fix up of keyboard(): In keyboard(), remove the handling
//         of 'f' key, since we no longer have g_activeShader and so on.
//         You could have achived the same effect using
//         g_overridingMaterial, but we'll pass.
//=========================================================================

static void keyboard(const unsigned char key, const int x, const int y) {
  ...
  // Remove the following:
  //case GLFW_KEY_F:
  //  g_activeShader ^= 1
  //  break;
  ...
}


//=========================================================================
// CONGRATULATIONS! YOUR PROGRAM SHOULD NOW COMPILE AND RUN
//=========================================================================
