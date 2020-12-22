#pragma once 

#define GLM_FORCE_QUAT_DATA_WXYZ
#define _USE_MATH_DEFINES             // let Windows load math constants

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>      // rotation, translation, scaling transforms

#include <iostream>
#include <string>
#include <iomanip>

static const double M_EPS = 1e-6;

std::ostream &operator<<(std::ostream &out, const glm::mat4 &m)
{
   const float *a = glm::value_ptr(m);
   for (int i = 0; i < 4; i++)
   {
      for (int j = 0; j < 4; j++)
         out << std::setw(10) << a[4 * j + i] << " ";
      out << std::endl;
   }
   return out;
}

std::ostream &operator<<(std::ostream &out, const glm::mat3 &m)
{
   const float *a = glm::value_ptr(m);
   for (int i = 0; i < 3; i++)
   {
      for (int j = 0; j < 3; j++)
         out << std::setw(10) << a[3 * j + i] << " ";
      out << std::endl;
   }
   return out;
}

glm::mat4 transFact(const glm::mat4 &m)
{
   glm::vec3 t = glm::vec3(m[3]);
   glm::mat4 result = glm::translate(t);
   return result;
}

glm::mat4 linFact(const glm::mat4 &m)
{
   glm::mat3 lin = glm::mat3(m);
   glm::mat4 result = glm::mat4(lin);
   return result;
}

glm::mat4 normalMatrix(const glm::mat4 &m)
{
   glm::mat4 invm = glm::affineInverse(m);
   invm[3] = glm::vec4(0.0f);
   return glm::transpose(invm);
}

glm::mat4 makeProjection(const float fovy, const float aspectRatio, const float zNear, const float zFar)
{
   // glm::perspective takes positive zNear and zfar, and flips 3rd row compared to textbook version
   glm::mat4 proj = glm::perspective(glm::radians(fovy), aspectRatio, -zNear, -zFar);
   proj[2][2] *= -1.0f; // flip signs of third row
   proj[3][2] *= -1.0f;
   return proj;
}

// this is an alternative to the makeProjection() above
glm::mat4 makeProjection2(const float fovy, const float aspectRatio, const float zNear, const float zFar)
{
   glm::mat4 r(0.0f);
   const double ang = fovy * 0.5 * M_PI / 180;
   const double f = std::abs(std::sin(ang)) < M_EPS ? 0 : 1 / std::tan(ang);
   if (std::abs(aspectRatio) > M_EPS)
      r[0][0] = f / aspectRatio; // 1st row

   r[1][1] = f; // 2nd row

   if (std::abs(zFar - zNear) > M_EPS)
   { // 3rd row
      r[2][2] = (zFar + zNear) / (zFar - zNear);
      r[3][2] = -2.0 * zFar * zNear / (zFar - zNear);
   }

   r[2][3] = -1.0; // 4th row
   return r;
}
