#pragma once

#include <iostream>
#include "glmutils.h"

// Return the screen space projection in terms of pixels of a 3d point
// given in eye-frame coordinates.
//
// Ideally you should never call this for a point behind the Z=0 plane,
// since such a point wouldn't be visible.
//
// But if you do pass in a point behind Z=0 plane, we'll just
// print a warning, and return the center of the screen.
inline glm::vec2 getScreenSpaceCoord(const glm::vec3& p,
                                 const glm::mat4& projection,
                                 double frustNear, double frustFovY,
                                 int screenWidth, int screenHeight) {
  if (p[2] > -M_EPS) {
    std::cerr << "WARNING: getScreenSpaceCoord of a point near or behind Z=0 plane. Returning screen-center instead." << std::endl;
    return glm::vec2((screenWidth-1)/2.0, (screenHeight-1)/2.0);
  }
  glm::vec4 q = projection * glm::vec4(p, 1);
  glm::vec3 clipCoord = glm::vec3(q) / q[3];
  return glm::vec2(clipCoord[0] * screenWidth / 2.0 + (screenWidth - 1)/2.0,
               clipCoord[1] * screenHeight / 2.0 + (screenHeight - 1)/2.0);
}


// Return the scale between 1 unit in screen pixels and 1 unit in the eye-frame
// (or world-frame, since we always use rigid transformations to represent one
// frame with resepec to another frame)
//
// Ideally you should never call this using a z behind the Z=0 plane,
// since such a point wouldn't be visible.
//
// But if you do pass in a point behind Z=0 plane, we'll just print a warning, 
// and return 1
inline double getScreenToEyeScale(double z, double frustFovY, int screenHeight) {
  if (z > -M_EPS) {
    std::cerr << "WARNING: getScreenToEyeScale on z near or behind Z=0 plane. Returning 1 instead." << std::endl;
    return 1;
  }
  return -(z * tan(frustFovY * M_PI/360.0)) * 2 / screenHeight;
}
