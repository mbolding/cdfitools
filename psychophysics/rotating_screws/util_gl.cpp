//**********************************************************************
//
//  MeshCL
//
//  (c) 2005 Jon K. Grossmann
//           jgrosman@uab.edu
//
//**********************************************************************

#include <GLUT/glut.h>
#include "util_gl.h"

MeshCL::MeshCL()
{
  pvCoords    = NULL;
  pvNormals   = NULL;
  pvFaceNorms = NULL;
  ipVI        = NULL;
  iSize[0] = iSize[1] = 0;
}

MeshCL::~MeshCL()
{
  if (pvCoords)
    free(pvCoords);
  if (pvNormals)
    free(pvNormals);
  if (pvFaceNorms)
    free(pvFaceNorms);
  if (ipVI)
    free(ipVI);
}

void MeshCL::initSize(int sizex, int sizey)
{
  if ((sizex*sizey) > (iSize[0]*iSize[1]))
  {
    pvCoords    = (Vector3CL*)realloc(pvCoords,    sizex*sizey*3*sizeof(float));
    pvNormals   = (Vector3CL*)realloc(pvNormals,   sizex*sizey*3*sizeof(float));
    pvFaceNorms = (Vector3CL*)realloc(pvFaceNorms, (sizex-1)*(sizey-1)*3*sizeof(float));
    ipVI        = (unsigned int*)realloc(ipVI, sizex*2*(sizey-1)*sizeof(unsigned int));
  }
  iSize[0] = sizex;
  iSize[1] = sizey;

  int index = 0;
  for (int y=0; y<(iSize[1]-1); y++)
    for (int x=0; x<iSize[0]; x++)
    {
      ipVI[index++] = x+y*iSize[0];
      ipVI[index++] = x+(y+1)*iSize[0];
    }
}

void MeshCL::updateNormals()
{
  int sizex1 = iSize[0]-1;
  int x,y,index;
  Vector3CL v2;
  // Calc Face Normals
  for (x=0; x<iSize[0]-2; x++)
    for (y=0; y<iSize[1]-1; y++)
    {
      index = x+y*sizex1;
      pvFaceNorms[index] = pos(x+1,y);
      pvFaceNorms[index] -= pos(x,y);
      v2 = pos(x+1, y+1);
      v2 -= pos(x+1,y);
      pvFaceNorms[index].cross(v2);
      pvFaceNorms[index].normalize();
    }
  x = iSize[0] - 2; // (should be anyway but make sure)
  // if mesh collapses to point here, then need diff vecs for normals to make
  // not using 0 length in cross product
  // (may need to also do this for meshes that converge to a point in other dim)
  for (y=0; y<iSize[1]-1; y++)
    {
      index = x+y*sizex1;
      pvFaceNorms[index] = pos(x,y);
      pvFaceNorms[index] -= pos(x,y+1);
      v2 = pos(x+1, y);
      v2 -= pos(x,y);
      pvFaceNorms[index].cross(v2);
      pvFaceNorms[index].normalize();
    }

  // Calc Averaged Vertex Normals
  index = 0;
  for (y=0; y<iSize[1]; y++)
    for (x=0; x<iSize[0]; x++)
    {
      pvNormals[index].clear();
      if (x < (iSize[0]-1) && y < (iSize[1]-1))
        pvNormals[index] += pvFaceNorms[x+y*sizex1];
      if (x > 0 && y > 0)
        pvNormals[index] += pvFaceNorms[x-1+(y-1)*sizex1];
      if (x > 0 && y < (iSize[1]-1))
        pvNormals[index] += pvFaceNorms[x-1+y*sizex1];
      if (y > 0 && x < (iSize[0]-1))
        pvNormals[index] += pvFaceNorms[x+(y-1)*sizex1];
      pvNormals[index].normalize();
      pvNormals[index] *= -1;
      index++;
    }
}

void MeshCL::render(float *col)
{
  if (!iSize[0] || !iSize[1])
    return;

  glEnable(GL_LIGHTING);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);  // polygons lit one side by default
  float w[3] = {1,1,1};
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, w);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  glVertexPointer(3, GL_FLOAT, 0, pvCoords[0].raw());
  glNormalPointer(GL_FLOAT, 0, pvNormals[0].raw());
  for (int i=0; i<(iSize[1]-1); i++)
    glDrawElements(GL_QUAD_STRIP, iSize[0]*2, GL_UNSIGNED_INT, &ipVI[i*iSize[0]*2]);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
}

