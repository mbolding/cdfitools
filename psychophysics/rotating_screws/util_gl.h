//**********************************************************************
//
//  MeshCL - keeps track of a 3D mesh
//
//**********************************************************************

#ifndef __util_gl_h_
#define __util_gl_h_


class MeshCL
{
protected:
  int iSize[2];              // size of mesh
  Vector3CL    *pvCoords;    // coords of vertices of mesh
  Vector3CL    *pvNormals;   // normals of Surface, per vertex
  Vector3CL    *pvFaceNorms; // face normals, temp used to calc pvNormals
  unsigned int *ipVI;        // vertex indices for quad strips

public:
  MeshCL();
  ~MeshCL();

  void initSize(int sizex, int sizey);
  inline Vector3CL &pos(int x, int y)   { return pvCoords[x+y*iSize[0]]; }
  void updateNormals();
  void render(float *col);
};

#endif // __util_gl_h_
