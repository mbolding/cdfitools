//
//  util.cpp
//
//  (c) 2003 Jon K. Grossmann
//           jgrosman@uab.edu
//
//  Implementation of Vector and Quaternion classes
//

#include <math.h>
#include <stdlib.h>
#include "util.h"


//**********************************************************************
//
//  Vector3CL implementation
//
//**********************************************************************

Vector3CL::Vector3CL()
{
  f[0] = f[1] = f[2] = 0.0;
}

Vector3CL::Vector3CL(float x, float y, float z)
{
  f[0] = x;
  f[1] = y;
  f[2] = z;
}

void Vector3CL::setValue(float x, float y, float z)
{
  f[0] = x;
  f[1] = y;
  f[2] = z;
}

void Vector3CL::setValue(float *xyz)
{
  f[0] = xyz[0];
  f[1] = xyz[1];
  f[2] = xyz[2];
}

void Vector3CL::clear()
{
  f[0] = f[1] = f[2] = 0.0;
}

void Vector3CL::operator=(const Vector3CL &v)
{
  f[0] = v[0];
  f[1] = v[1];
  f[2] = v[2];
}

void Vector3CL::operator*=(float m)
{
  f[0] *= m;
  f[1] *= m;
  f[2] *= m;
}

void Vector3CL::operator+=(Vector3CL &v)
{
  f[0] += v[0];
  f[1] += v[1];
  f[2] += v[2];
}

void Vector3CL::operator-=(Vector3CL &v)
{
  f[0] -= v[0];
  f[1] -= v[1];
  f[2] -= v[2];
}

void Vector3CL::cross(Vector3CL &v)
{
  float x =  f[1]*v[2] - f[2]*v[1];
  float y = -f[0]*v[2] + f[2]*v[0];
  float z =  f[0]*v[1] - f[1]*v[0];
  setValue(x,y,z);
}

float Vector3CL::dot(Vector3CL &v) const
{
  return (f[0]*v[0] + f[1]*v[1] + f[2]*v[2]);
}

float Vector3CL::normalize()
{
  float len = length();
  if (len == 0.0)
    return 0.0;
  f[0] /= len;
  f[1] /= len;
  f[2] /= len;
  return len;
}

float Vector3CL::length() const
{
  return sqrtf(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
}

//**********************************************************************
//
//  QuaternionCL
//
//**********************************************************************

QuaternionCL::QuaternionCL()
{
  q[0] = q[3] = q[2] = 0.0;
  q[1] = 1.0;
}

void QuaternionCL::reset()
{
  q[0] = q[3] = q[2] = 0.0;
  q[1] = 1.0;
}

void QuaternionCL::getAxisAngle(Vector3CL &vec, float &angle) const
{
  float s = q[0]*q[0] + q[1]*q[1] + q[2]*q[2];
  if (s == 0.0)
    return;
  float x = q[0] / s;
  float y = q[1] / s;
  float z = q[2] / s;
  vec.setValue(x,y,z);
  angle = 2.0 * acosf(q[3]);
}

void QuaternionCL::setValue(Vector3CL &v, float angle)
{
  Vector3CL vec;
  vec = v;
  vec.normalize();
  vec *= sinf(angle / 2.0);
  q[0] = vec[0];
  q[1] = vec[1];
  q[2] = vec[2];
  q[3] = cosf(angle / 2.0);
}

void QuaternionCL::operator=(QuaternionCL &rot)
{
  q[0] = rot[0];
  q[1] = rot[1];
  q[2] = rot[2];
  q[3] = rot[3];
}

void QuaternionCL::operator*=(QuaternionCL &r)
{
  float f[4];
  f[0] = (r[3]*q[0] + r[0]*q[3] + r[1]*q[2] - r[2]*q[1]);
  f[1] = (r[3]*q[1] + r[1]*q[3] + r[2]*q[0] - r[0]*q[2]);
  f[2] = (r[3]*q[2] + r[2]*q[3] + r[0]*q[1] - r[1]*q[0]);
  f[3] = (r[3]*q[3] - r[0]*q[0] - r[1]*q[1] - r[2]*q[2]);
  for (int i=0; i<4; i++)
    q[i] = f[i];
  normalize();
}

void QuaternionCL::normalize()
{
  float len = sqrtf(norm());
  if (len == 0.0)
    return;
  for (int i=0; i<4; i++)
    q[i] /= len;
}

float QuaternionCL::norm() const
{
  return (q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
}


