//
//  util.h
//
//  (c) 2003 Jon K. Grossmann
//           jgrosman@uab.edu
//
//  provides Quaternion and Vector objects
//

#ifndef __util_h_
#define __util_h_


//**********************************************************************
//
//  Vector3CL - 3D vector
//
//**********************************************************************

class Vector3CL
{
protected:
  // there should be nothing else allocated in the class other
  // than three floats:
  float f[3];

public:
  Vector3CL();
  Vector3CL(float x, float y, float z);

  void  setValue(float x, float y, float z);
  void  setValue(float *xyz);
  void  clear(); // set to 0,0,0 without pushing 0,0,0 on stack
  inline float &operator[](int idx) { return f[idx]; }
  inline float operator[](int idx) const { return f[idx]; }
  inline float *raw()               { return f; }
  void operator=(const Vector3CL &v);
  void  operator*=(float m);
  void  operator+=(Vector3CL &v);
  void  operator-=(Vector3CL &v);
  void  cross(Vector3CL &vec);
  float dot(Vector3CL &vec) const;
  float normalize();  // returns length before normalized
  float length() const;
};

//**********************************************************************
//
//  QuaternionCL - rotation quaternion
//
//**********************************************************************

class QuaternionCL
{
protected:
  float q[4];

public:
  QuaternionCL();

  void getAxisAngle(Vector3CL &vec, float &angle) const;
  void setValue(Vector3CL &vec, float angle);
  void operator*=(QuaternionCL &rot);
  void operator=(QuaternionCL &rot);
  inline float &operator[](int idx) { return q[idx]; }  
  void normalize();
  float norm() const;
  void reset();
};


#endif // __util_h_
