//=========================================================================
//
//  rotating_screws
//
//  stimuli for fMRI experiments - rotating or not rotating screwahedrons
//  defined random dots, shaded solids, or silhouette, etc.
//
//  (c) 2005 Jon K. Grossmann
//           jgrosman@uab.edu
//                 
//  History
//  -------
//  2005 Mar 24 - JKG - created.
//  2008 Dec    - MSB - modified.
//
//=========================================================================

//#define LINUX 1
//#define SAM 0
#define FULLSCREEN 1

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef LINUX
#include <GLUT/glut.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#else
#include "glut.h"      // (this should be be moved to standard loc instead of local dir)
#include <windows.h>   // for findfirstfile, findnextfile
#include <algorithm>   // for sort in windows
#define M_PI 3.1415927
double log2(double x)
{
  return log10(x) / log10(2.0);
}
#endif

#include <time.h>      // for _strdate, _strtime
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
//#include <math.h>
#include <sys/types.h>
#include "util.h"
#include "shape.h"

using namespace std;  

//-------------------------------------------------------------------------
//
//  Defines/Globals
//
//-------------------------------------------------------------------------

double TWO_PI = (M_PI*2.0);
double DEG2RAD = (M_PI/180.0);
int    iWinSize[2];           // size of window, chosen by enter game mode

int    bSam = 0;              // when true, we are running as slave to Sam's program,
                              // or testing how display will look in slave mode
int    bVerbose = 0;          // turn on with -v on cmd line to get extra messages

#ifdef SAM

#include "gl.h"   // include for GXDRAW structure used for extern var "figures" below

#define NFIGS 8               // from Sam's gl.h
extern int tswgl[NFIGS];      // tswgl[0] == 1 means clear screen (don't draw),
                              // tswgl[0]  > 1 means draw
extern int blockselect;       // indicates which block should show; zero based
                              // these are linked in global3.o, from /usr2/projx

extern GXDRAW figures[8][3][2];

// x,y position offsets coming from Sam's program, apply to both eyes
// (only supports yoked eye movements, not eye movements in depth)
double gxpos = 0.0, gypos = 0.0;

#endif  // end: ifdef SAM

#define ID_SHAPE_MGR            100  // shape manager properties are also written to scene files
#define ID_SCREW                101
#define ID_FIX_CROSS            102
#define ID_QUARTETS             103
#define ID_ANNULUS              104
#define ID_RANDOM_DOTS          105
#define ID_MT_LOCALIZER         106
#define ID_POLAR_CHECKERBOARD   107
#define ID_RECT                 108

#define ID_YES                 1001
#define ID_NO                  1002

//
//  random number in range [0..1]
//
#define MY_RAND_MAX 10000   // windows rand_max way below 1,000,000, lowered to 10,000
double randNorm()
{
  return (double)(rand() % MY_RAND_MAX) / (MY_RAND_MAX - 1.0);
}

//-------------------------------------------------------------------------
//
//  MenuCL - renders a list of items, one of which is the currently selected
//           item, and each of which have actions when selected
//
//-------------------------------------------------------------------------
// moved to shape.h


struct SeparatorItemCL : public MenuCL::ItemCL
{
  SeparatorItemCL() : MenuCL::ItemCL() { iSkip = 1; }
  SeparatorItemCL(const char *pchar) : MenuCL::ItemCL(pchar) { iSkip = 1; }
};


MenuCL *MenuCL::pMainMenu = NULL;
MenuCL *MenuCL::pCurrentMenu = NULL;
void MenuCL::renderCurrentMenu(int x, int y)
{
  if (currentMenu())
    currentMenu()->renderMenu(x, y);
}

MenuCL::MenuCL() :
  iSelected(0),
  opParent(0L)
{
}

MenuCL::MenuCL(const char *sz) :
  iSelected(0),
  opParent(0L),
  sTitle(sz)
{
}

MenuCL::MenuCL(const string &s) :
  iSelected(0),
  opParent(0L),
  sTitle(s)
{
}

void MenuCL::appendItem(ItemCL *item)
{
  vItems.push_back(item);
  //vActions.push_back(action);
}

void MenuCL::clear()
{
  for (vector<ItemCL*>::iterator it=vItems.begin();
       it!=vItems.end(); it++)
    delete (*it);  // TODO: make sure all menu items' destructors delete memory they're managing
  vItems.clear();
}

void MenuCL::renderMenu(float x, float y)
{
  int i,j;
  glDisable(GL_LIGHTING);
  glPushMatrix();
  glColor3f(0.0, 1.0, 0.0);
  glRasterPos2f(x, y+18);
  for (j=0; j<sTitle.length(); j++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, sTitle[j]);

  for (i=0; i<vItems.size(); i++)
  {
    if (i == iSelected)
      glColor3f(1.0, 1.0, 0.0);
    else if (vItems[i]->color() == 1)
      glColor3f(0.0, 0.75, 0.75);
    else
      glColor3f(0.25, 0.25, 1.0);

    glRasterPos2f(x, y-i*16);

    const string &txt = vItems[i]->text();
    for (j=0; j<txt.length(); j++)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, txt[j]);
  }
  glPopMatrix();
}

void MenuCL::select()
{
  if (iSelected < vItems.size())
    vItems[iSelected]->action();
}

void MenuCL::delta(float dir)
{
  if (iSelected < vItems.size())
    vItems[iSelected]->delta(dir);

  // update menu items that should always be updated, like
  // the experiment clock
  for (vector<ItemCL*>::iterator it=vItems.begin();
       it != vItems.end(); it++)
    if ((*it)->alwaysSetLabel())
      (*it)->setLabel();
}

void MenuCL::focus(int f)  // 0==lose focus; 1==gain focus
{
  for (vector<ItemCL*>::iterator it = vItems.begin();
       it != vItems.end(); it++)
    (*it)->focus(f);
}


//
//  Menu used to display a simple message
//

struct MessageMenuCL : public MenuCL
{
  MessageMenuCL() : MenuCL() {}
  static MessageMenuCL *pInst;
  static MessageMenuCL *menu(char *title)
  {
    if (!pInst)
      pInst = new MessageMenuCL();
    pInst->setTitle(title);
    return pInst;
  }
};
MessageMenuCL *MessageMenuCL::pInst = 0L;

//-------------------------------------------------------------------------
//
//  ShapeDefCL - create one definition class for each shape type
//               static members provide interface to create classes based on id
//
//-------------------------------------------------------------------------

class ShapeCL;  // fwd decl for use in ShapeInterface and ShapeDefBaseCL

class ShapeDefBaseCL
{
protected:
  int    iId;
  string sName;
public:
  const int    id()     { return iId; }
  const string nameOf() { return sName; }
  virtual ShapeCL *create() = 0;
};

class ShapeInterface
{
public:
  static vector<ShapeDefBaseCL*> vDefs;
  static ShapeCL      *createShapeWithId(int id);
  static const string  nameOfShapeWithId(int id);
};

vector<ShapeDefBaseCL*> ShapeInterface::vDefs;

void ShapeCL::initTitle()
{
  char sznum[16];
  sprintf(sznum, "%d", iShapeNo);
  sName = ShapeInterface::nameOfShapeWithId(id()) + " (" + sznum + ")";
}

template <class SHAPECL>
class ShapeDefCL : public ShapeDefBaseCL
{
public:
  ShapeDefCL(int id, string name);
  ShapeCL *create() 
  { 
    ShapeCL *p = new SHAPECL();
    //p->initTitle();
    return p;
  }
};

template <class SHAPECL>
ShapeDefCL<SHAPECL>::ShapeDefCL(int id, string name)
{
  iId = id;
  sName = name;
  ShapeInterface::vDefs.push_back(this);
}


const string ShapeInterface::nameOfShapeWithId(int id)
{
  for (vector<ShapeDefBaseCL*>::iterator it=vDefs.begin();
       it != vDefs.end(); it++)
    if ((*it)->id() == id)
      return (*it)->nameOf();
  return string("name of shape not found");
}

//-------------------------------------------------------------------------
//
//  ShapeCL - Shapes base class
//
//-------------------------------------------------------------------------
// moved to shape.h

struct ShapePropertyItemCL;   // fwd decl for return type of ShapeCL::menuItem()
enum {DISP_DEFAULT, DISP_DOUBLE2=DISP_DEFAULT, DISP_BINARY8};  // this should be in shapepropertyitemcl definition


//
//  createShapeWithId needs shapecl def to see initTitle() for certain compilers (mac)
//
ShapeCL *ShapeInterface::createShapeWithId(int id)
{
  for (vector<ShapeDefBaseCL*>::iterator it=vDefs.begin();
       it != vDefs.end(); it++)
    if ((*it)->id() == id)
    {
      ShapeCL *p = (*it)->create();
      p->initTitle();
      return p;
    }
  std::cerr << "Warning: request to create shape for unknown id (shapeinterface::createshapewithid(" << id << "))\n";
  return 0L;
}

int ShapeCL::iShapeNoCtr = 1;


//
//  ShapeCompFunctor : used to sort shapes into order for rendering: opaque shapes first, then transparent shapes
//                     and finally shapes that should never be occluded
//
struct ShapeCompFunctor
{
  bool operator()(const ShapeCL *s1, const ShapeCL *s2) const
  {
    return s1->order() < s2->order();
  }
};

//-------------------------------------------------------------------------
//
//  MTLocalizerShapeCL - encapsulates two randomdots stimuli and an annulus
//
//-------------------------------------------------------------------------

class MTLocalizerShapeCL : public ShapeCL
{
protected:
  void initializePoints() {}
  //RandomDotsShapeCL *pInner, *pOuter;
  //AnnulusShapeCL    *pDisk;

public:
  MTLocalizerShapeCL();
  virtual void update(int id);
  virtual void updateAt(double dt) { ShapeCL::updateAt(dt); }
  virtual void synchronize() { ShapeCL::synchronize(); }
  virtual void render(int eye) {}
  virtual MTLocalizerShapeCL *duplicate()
  {
    MTLocalizerShapeCL *p = new MTLocalizerShapeCL();
    p->copyPropertiesFrom(this);
    p->incrementProperty(PROP_X, 2);
    p->incrementProperty(PROP_Y, 2);
    return p;
  }
  virtual int id() { return ID_MT_LOCALIZER; }
  enum {PROP_INNER_RADIUS=PROP_next, PROP_OUTER_RADIUS, PROP_ORIENTATION, PROP_ORIENTATION_DIFFERENCE,
        PROP_DOTSIZE, PROP_NUM_DOTS_CENTER, PROP_NUM_DOTS_SURROUND, PROP_VELOCITY,
        PROP_SWITCH_DIR_PERIOD, PROP_RANDOM_DIR_PERIOD};
  virtual int lastPropertyIdx() { return PROP_RANDOM_DIR_PERIOD; }
};

MTLocalizerShapeCL::MTLocalizerShapeCL() : ShapeCL()
{
  setProperty(PROP_OUTER_RADIUS, 200);  // better set outer before inner so inner doesn't clip to outer
  setProperty(PROP_INNER_RADIUS, 100);
  setProperty(PROP_ORIENTATION, 0);
  setProperty(PROP_ORIENTATION_DIFFERENCE, 0);
  setProperty(PROP_DOTSIZE, 2);
  setProperty(PROP_NUM_DOTS_CENTER, 100);
  setProperty(PROP_NUM_DOTS_SURROUND, 300);
  setProperty(PROP_VELOCITY, 50);
  setProperty(PROP_SWITCH_DIR_PERIOD, 0);
  setProperty(PROP_RANDOM_DIR_PERIOD, 0);

  pMenu = new MenuCL();
  menuItem(PROP_INNER_RADIUS,      "Inner Radius", prop(PROP_INNER_RADIUS), 4);
  menuItem(PROP_OUTER_RADIUS,      "Outer Radius", prop(PROP_OUTER_RADIUS), 4);
  menuItem(PROP_VELOCITY,          "Dot Velocity", prop(PROP_VELOCITY), 2);
  menuItem(PROP_ORIENTATION,       "Direction of Center",  prop(PROP_ORIENTATION), 2);
  menuItem(PROP_ORIENTATION_DIFFERENCE, "Center/Surround Orienation Difference", prop(PROP_ORIENTATION_DIFFERENCE), 2);
  menuItem(PROP_DOTSIZE,           "Dot Size", prop(PROP_DOTSIZE), 1);
  menuItem(PROP_NUM_DOTS_CENTER,   "Number of Dots in Center", prop(PROP_NUM_DOTS_CENTER), 4);
  menuItem(PROP_NUM_DOTS_SURROUND, "Number of Dots in Surround", prop(PROP_NUM_DOTS_SURROUND), 4);
  menuItem(PROP_SWITCH_DIR_PERIOD, "Switch Direction Period (s)", prop(PROP_SWITCH_DIR_PERIOD), 0.25);
  menuItem(PROP_RANDOM_DIR_PERIOD, "New Random Direction Period (s)", prop(PROP_RANDOM_DIR_PERIOD), 0.25);
}

void MTLocalizerShapeCL::update(int id)
{
  switch (id)
  {
    case PROP_INNER_RADIUS:
      if (prop(PROP_INNER_RADIUS) < 0.0)
        setPropertyOnly(PROP_INNER_RADIUS, 0.0);
      else if (prop(PROP_INNER_RADIUS) > prop(PROP_OUTER_RADIUS))
        setPropertyOnly(PROP_INNER_RADIUS, prop(PROP_OUTER_RADIUS));
      initializePoints();
      break;
    case PROP_OUTER_RADIUS:
      if (prop(PROP_OUTER_RADIUS) < prop(PROP_INNER_RADIUS))
        setPropertyOnly(PROP_OUTER_RADIUS, prop(PROP_INNER_RADIUS));
      initializePoints();
      break;
    case PROP_VELOCITY:

      break;
    default:
      ShapeCL::update(id);
  }
}

//-------------------------------------------------------------------------
//
//  RandomDotsShapeCL - Rectangular field of random dots
//
//-------------------------------------------------------------------------

class RandomDotsShapeCL : public ShapeCL
{
protected:
  int    iDirection;  // used with PROP_SWITCH_DIR_PERIOD
  double dTime, dDispJumpTime;
  int    iDispJumpState;  // used with PROP_DISP_JUMP_*
  vector<Vector3CL> vPoints;

  void initializePoints();

public:
  RandomDotsShapeCL();
  virtual void update(int id);
  virtual void updateAt(double dt);
  virtual void synchronize() { dTime = 0.0; dDispJumpTime = 0.0; 
                               iDirection = 0; iDispJumpState = 0; 
                               ShapeCL::synchronize(); 
                             }
  virtual void render(int eye);
  virtual RandomDotsShapeCL *duplicate()
  {
    RandomDotsShapeCL *p = new RandomDotsShapeCL();
    p->copyPropertiesFrom(this);
    p->incrementProperty(PROP_X, 2);
    p->incrementProperty(PROP_Y, 2);
    return p;
  }
  virtual int id() { return ID_RANDOM_DOTS; }
  enum {PROP_DOTSIZE=PROP_next, PROP_NUM_DOTS, PROP_VELOCITY, PROP_DIRECTION, PROP_PPATHCONTINUE,
        PROP_WIDTH, PROP_HEIGHT, PROP_SWITCH_DIR_PERIOD, PROP_ROT_VEL, PROP_EXP_VEL, PROP_SHAPE,
        PROP_DISP_JUMP_AMP, PROP_DISP_JUMP_PERIOD, PROP_DISP_JUMP_OFFSET};
  virtual int lastPropertyIdx() { return PROP_DISP_JUMP_OFFSET; }

  inline void createPoint(Vector3CL &pt)
  {
    if ((int)prop(PROP_SHAPE) == 0)
    {
      pt.setValue(randNorm()-0.5, randNorm()-0.5, 0.0);
    }
    else
    {
      double dist = sqrt(randNorm()) / 2.0;
      double angl = randNorm() * TWO_PI;
      pt.setValue(dist*cos(angl), dist*sin(angl), 0.0);
    }
  }
};

RandomDotsShapeCL::RandomDotsShapeCL() : ShapeCL(), dTime(0.0), iDirection(0), 
                                                    dDispJumpTime(0.0), iDispJumpState(0)
{
  setProperty(PROP_DOTSIZE, 2.0);
  setProperty(PROP_NUM_DOTS, 500);
  setProperty(PROP_VELOCITY, 0);
  setProperty(PROP_DIRECTION, 0);
  setProperty(PROP_PPATHCONTINUE, 1.0);
  setProperty(PROP_WIDTH, 640);
  setProperty(PROP_HEIGHT, 512);
  setProperty(PROP_SWITCH_DIR_PERIOD, 0);
  setProperty(PROP_ROT_VEL, 0);
  setProperty(PROP_EXP_VEL, 0);
  setProperty(PROP_SHAPE, 0);
  setProperty(PROP_DISP_JUMP_AMP, 0);
  setProperty(PROP_DISP_JUMP_PERIOD, 0);
  setProperty(PROP_DISP_JUMP_OFFSET, 0);

  pMenu = new MenuCL(name());
  menuItem(PROP_SHAPE,      "Shape (0=Rect 1=Ellipse)", prop(PROP_SHAPE), 1);
  menuItem(PROP_WIDTH,      "Width",             prop(PROP_WIDTH),       4);
  menuItem(PROP_HEIGHT,     "Height",            prop(PROP_HEIGHT),      4);
  menuItem(PROP_DOTSIZE,    "Dot Size",          prop(PROP_DOTSIZE),     1);
  menuItem(PROP_NUM_DOTS,   "Number of Dots",    prop(PROP_NUM_DOTS),    4);
  menuItem(PROP_VELOCITY,   "Dot Speed",         prop(PROP_VELOCITY),    2);
  menuItem(PROP_DIRECTION,  "Dot Direction",     prop(PROP_DIRECTION),   2);
  menuItem(PROP_EXP_VEL,    "Expansion Speed",   prop(PROP_EXP_VEL),     2);
  menuItem(PROP_ROT_VEL,    "Rotation Speed",    prop(PROP_ROT_VEL),     2);
  menuItem(PROP_PPATHCONTINUE, "P(path continue)", prop(PROP_PPATHCONTINUE), 0.02);
  menuItem(PROP_SWITCH_DIR_PERIOD, "Switch Direction Period (s)", prop(PROP_SWITCH_DIR_PERIOD), 0.25);
  menuItem(PROP_DISP_JUMP_PERIOD, "Disparity Jump Period (s)", prop(PROP_DISP_JUMP_PERIOD), 0.1);
  menuItem(PROP_DISP_JUMP_AMP, "Disparity Jump Amplitude (pixels)", prop(PROP_DISP_JUMP_AMP), 1);
  menuItem(PROP_DISP_JUMP_OFFSET, "Disparity Jump Offset (pixels)", prop(PROP_DISP_JUMP_OFFSET), 1);
  ShapeCL::appendMenuItems();  
}

void RandomDotsShapeCL::update(int id)
{
  switch (id)
  {
    case PROP_WIDTH:
    case PROP_HEIGHT:
      break;
    case PROP_NUM_DOTS:
      if (prop(id) < 0) setPropertyOnly(id, 0);
      initializePoints();
      break;
    case PROP_DISP_JUMP_PERIOD:
      if (prop(id) < 0.0) setProperty(id, 0);
      dDispJumpTime = 0.0;
      break;
    case PROP_SWITCH_DIR_PERIOD:
      if (prop(id) < 0.0) setProperty(id, 0);
      dTime = 0.0;
      break;
    case PROP_PPATHCONTINUE:
      if (prop(id) < 0.0) setPropertyOnly(id, 0.0);
      else if (prop(id) > 1.0) setPropertyOnly(id, 1.0);
      break;
    case PROP_DIRECTION:
      if (prop(id) > 360) 
        setProperty(id, prop(id)-360.0);
      else if (prop(id) < 0.0) 
        setProperty(id, prop(id)+360.0);
      break;
    case PROP_SHAPE:
      if (prop(id) < 0)
        setProperty(id, 1);
      else if (prop(id) > 1)
        setProperty(id, 0);
      vPoints.resize(0); 
      break;
    default:
      ShapeCL::update(id);
  }
}

void RandomDotsShapeCL::updateAt(double dt)
{
  ShapeCL::updateAt(dt);
  //if (prop(PROP_VELOCITY) == 0.0)
  //  return;

  double dExp = prop(PROP_EXP_VEL);
  double dRot = prop(PROP_ROT_VEL);
  int iExpRot = ((dExp != 0.0) || (dRot != 0.0));
  double dR, dTheta;

  int iSwitch = (prop(PROP_SWITCH_DIR_PERIOD) > 0.0);
  if (iSwitch) {
    dTime += dt;
    if (dTime > prop(PROP_SWITCH_DIR_PERIOD)) {
      dTime -= prop(PROP_SWITCH_DIR_PERIOD);
      iDirection = 1 - iDirection;
      //setProperty(PROP_DIRECTION, prop(PROP_DIRECTION)+180.0);
    }
  }
  iSwitch = (prop(PROP_DISP_JUMP_PERIOD) > 0.0);
  if (iSwitch) {
    dDispJumpTime += dt;
    if (dDispJumpTime > prop(PROP_DISP_JUMP_PERIOD)) {
      dDispJumpTime -= prop(PROP_DISP_JUMP_PERIOD);
      iDispJumpState = !iDispJumpState;
    }
  }

  int iRnd = (prop(PROP_PPATHCONTINUE) < 1.0);

  float dx = cos((iDirection*180.0+prop(PROP_DIRECTION))*DEG2RAD) * prop(PROP_VELOCITY) * dt / prop(PROP_WIDTH);
  float dy = sin((iDirection*180.0+prop(PROP_DIRECTION))*DEG2RAD) * prop(PROP_VELOCITY) * dt / prop(PROP_HEIGHT);

  double mdir = iDirection * -2 + 1;

  for (vector<Vector3CL>::iterator it=vPoints.begin();
       it!=vPoints.end(); it++)
  {
    if (iRnd && (pow(randNorm(),2) > prop(PROP_PPATHCONTINUE)))
    {
      //(*it).setValue(randNorm()-0.5, randNorm()-0.5, 0.0);
      createPoint((*it));
    }
    else
    {
      if (iExpRot)
      {
        dR = sqrt((*it)[0] * (*it)[0] + (*it)[1] * (*it)[1]);
        dTheta = atan2((*it)[1], (*it)[0]);
        dTheta += 10 * mdir*prop(PROP_ROT_VEL) * dt / (prop(PROP_WIDTH)+prop(PROP_HEIGHT));
        dR += dR * mdir*prop(PROP_EXP_VEL) * dt / (prop(PROP_WIDTH)+prop(PROP_HEIGHT));
        (*it)[0] = dx + dR * cos(dTheta);
        (*it)[1] = dy + dR * sin(dTheta);
        if (((int)prop(PROP_SHAPE)==0 && fabs((*it)[0]) > 0.5 || fabs((*it)[1]) > 0.5) ||
            ((int)prop(PROP_SHAPE)==1 && sqrt(pow((*it)[0],2)+pow((*it)[1],2)) > 0.5))
	{
	  // (*it).setValue(randNorm()-0.5, randNorm()-0.5, 0.0);
          createPoint((*it));
	}
      }
      else
      {
        (*it)[0] += dx;
        (*it)[1] += dy;
        // bounds checking and wrapping:
        if ((int)prop(PROP_SHAPE) == 0)
	{
          if ((*it)[0] > 0.5) (*it)[0] -= 1.0;
          else if ((*it)[0] < -0.5) (*it)[0] += 1.0;
          if ((*it)[1] > 0.5) (*it)[1] -= 1.0;
          else if ((*it)[1] < -0.5) (*it)[1] += 1.0;
	}
        else
	{
          if (sqrt(pow((*it)[0],2)+pow((*it)[1],2)) > 0.5)
	  {
            double angl = atan2((*it)[1],(*it)[0]) + M_PI;
            (*it)[0] = cosf(angl) / 2.0;
            (*it)[1] = sinf(angl) / 2.0;
	  }
	}
      }
    }
  }
}

void RandomDotsShapeCL::render(int eye)
{
  if (bBlinkOn == 0)
    return;

  if (prop(PROP_NUM_DOTS) <= 0)
    return;
  if (vPoints.size() == 0)
    initializePoints();

  float xdisp = 0.0;  // disparity
  if (eye < 3 && prop(PROP_DISP_JUMP_PERIOD) > 0.0) {
    xdisp = prop(PROP_DISP_JUMP_OFFSET) + prop(PROP_DISP_JUMP_AMP) * (iDispJumpState*2.0-1.0);
    xdisp /= 2.0;
    if (eye == 1) xdisp *= -1;
  }

  glDisable(GL_LIGHTING);
  glColor4fv(fRGBA);
  glPointSize(prop(PROP_DOTSIZE));

  glPushMatrix();
  glTranslatef(xdisp+prop(PROP_X), prop(PROP_Y), prop(PROP_Z));
  glScaled(prop(PROP_WIDTH), prop(PROP_HEIGHT), 0.0);
  glBegin(GL_POINTS);
  for (int i=0; i<prop(PROP_NUM_DOTS); /*vPoints.size();*/ i++)
    glVertex3fv(vPoints[i].raw());
  glEnd();
  glPopMatrix();

  glEnable(GL_LIGHTING);

  //  glEnableClientState(GL_VERTEX_ARRAY);
  //  glVertexPointer(3, GL_FLOAT, 0, vPoints[0].raw());
  //  glDrawElements(GL_POINTS, vPoints.size(), ????????????????
  //  glDisableClientState(GL_VERTEX_ARRAY);
}

void RandomDotsShapeCL::initializePoints()
{
  int ptsnow = vPoints.size();
  vPoints.resize((int)prop(PROP_NUM_DOTS)); 

  if ((int)prop(PROP_SHAPE) == 0)
  {
    // rectangle
    for (int i=ptsnow; i<prop(PROP_NUM_DOTS); i++)
      vPoints[i].setValue(randNorm()-0.5, randNorm()-0.5, 0.0);
  }
  else                            
  {
    // ellipse
    for (int i=ptsnow; i<prop(PROP_NUM_DOTS); i++)
    {
      double dist = sqrt(randNorm()) / 2.0;
      double angl = randNorm() * TWO_PI;
      vPoints[i].setValue(dist*cos(angl), dist*sin(angl), 0.0);
    }
  }
}


//vPoints.reserve((int)prop(PROP_NUM_DOTS));
//vPoints.push_back(Vector3CL(randNorm()-0.5, randNorm()-0.5, 0.0));

//-------------------------------------------------------------------------
//
//  PolarCheckerboardShapeCL - counterphasing, rotating checkerboard wedges
//
//-------------------------------------------------------------------------

class PolarCheckerboardShapeCL : public ShapeCL
{
protected:
  double dAngle;             // current angle of rotating wedge
  double dExpState;          // current expansion state; [0..1] (?)
  double dCounterphaseTime;  // clock to keep track of counterphasing state
  double dExpDirTime;
  int    iExpDir;

  void drawShape();
  void drawShapeExp();
  void drawStrip(double r, double r2, double a, double a2);
  void drawCircularStrip(double r, double r2, int &color);

public:
  PolarCheckerboardShapeCL();
  virtual void update(int id);
  virtual void updateAt(double dt);
  virtual void synchronize() 
  { 
    iExpDir = 0;
    dExpDirTime = 0.0;
    dAngle = prop(PROP_INITIAL_PHASE);
    dExpState = 0.0; //prop(PROP_INITIAL_PHASE) / 360.0;
    dCounterphaseTime = 0.0;
    ShapeCL::synchronize(); 
  }
  virtual void render(int eye);
  virtual PolarCheckerboardShapeCL *duplicate()
  {
    PolarCheckerboardShapeCL *p = new PolarCheckerboardShapeCL();
    p->copyPropertiesFrom(this);
    p->incrementProperty(PROP_X, 2);
    p->incrementProperty(PROP_Y, 2);
    return p;
  }
  virtual int id() { return ID_POLAR_CHECKERBOARD; }
  enum {PROP_RADIUS=PROP_next, PROP_THICKNESS, PROP_WEDGE_SIZE, PROP_RADIAL_CYCLES,
        PROP_CIRCULAR_CYCLES, PROP_MAG_FACTOR, PROP_ROT_SPEED, PROP_INITIAL_PHASE,
        PROP_BOWTIE, PROP_EXP_SPEED, PROP_MAX_RADIUS, PROP_RESOLUTION,
        PROP_MIN_RADIUS, PROP_MAG_FACT_THICK, PROP_EXP_DIR_PERIOD, PROP_COUNTERPHASE_PERIOD};
  virtual int lastPropertyIdx() { return PROP_COUNTERPHASE_PERIOD; }
};

#define MIN_MAG_FACTOR 0.0 

PolarCheckerboardShapeCL::PolarCheckerboardShapeCL() : ShapeCL(),
  dAngle(0.0), dCounterphaseTime(0.0), dExpState(0.0),
  dExpDirTime(0.0), iExpDir(0)
{
  setProperty(PROP_RADIUS, 450);
  setProperty(PROP_THICKNESS, 425);
  setProperty(PROP_WEDGE_SIZE, 45);
  setProperty(PROP_RADIAL_CYCLES, 9);
  setProperty(PROP_CIRCULAR_CYCLES, 7);
  setProperty(PROP_MAG_FACTOR, 1.0);
  setProperty(PROP_COUNTERPHASE_PERIOD, 0.0);
  setProperty(PROP_ROT_SPEED, 0.0);
  setProperty(PROP_INITIAL_PHASE, 0.0);
  setProperty(PROP_BOWTIE, 0.0);
  setProperty(PROP_EXP_SPEED, 0.0);
  setProperty(PROP_MAX_RADIUS, 500);
  setProperty(PROP_MIN_RADIUS, 25);
  setProperty(PROP_RESOLUTION, 1);
  setProperty(PROP_EXP_DIR_PERIOD, 0.0);
  //  setProperty(PROP_MAG_FACT_THICK, 0); // if true, mag factor applies to whole thickness, not cells within wedge or ring

  pMenu = new MenuCL(name());
  menuItem(PROP_RADIUS,              "Outer Radius",         prop(PROP_RADIUS), 2);
  menuItem(PROP_THICKNESS,           "Thickness",            prop(PROP_THICKNESS), 2);
  menuItem(PROP_WEDGE_SIZE,          "Wedge Size (deg)",     prop(PROP_WEDGE_SIZE), 2);
  menuItem(PROP_BOWTIE,              "Bowtie (On/Off)",      prop(PROP_BOWTIE), 1);
  menuItem(PROP_RESOLUTION,          "Resolution",           prop(PROP_RESOLUTION), 1);
  pMenu->appendItem(new SeparatorItemCL());
  menuItem(PROP_RADIAL_CYCLES,       "Radial Cycles",        prop(PROP_RADIAL_CYCLES), 1);
  menuItem(PROP_CIRCULAR_CYCLES,     "Circular Cycles",      prop(PROP_CIRCULAR_CYCLES), 1);
  menuItem(PROP_COUNTERPHASE_PERIOD, "Counterphasing (full) Period (s)", prop(PROP_COUNTERPHASE_PERIOD), 0.125);
  menuItem(PROP_MAG_FACTOR,          "Magnification Factor", prop(PROP_MAG_FACTOR), 0.125);
  //  menuItem(PROP_MAG_FACT_THICK,      "Apply mag factor to whole ring (on/off)", prop(PROP_MAG_FACT_THICK), 1);
  pMenu->appendItem(new SeparatorItemCL());
  menuItem(PROP_ROT_SPEED,           "Rotation Speed (deg/s)",  prop(PROP_ROT_SPEED), 2);
  menuItem(PROP_INITIAL_PHASE,       "Initial Rotation Phase (deg)", prop(PROP_INITIAL_PHASE), 2);
  menuItem(PROP_EXP_SPEED,           "Expansion Speed (pct/s)", prop(PROP_EXP_SPEED), 0.02);
  menuItem(PROP_MAX_RADIUS,          "Expansion Max Radius", prop(PROP_MAX_RADIUS), 2);
  menuItem(PROP_MIN_RADIUS,          "Expansion Min Radius", prop(PROP_MIN_RADIUS), 2);
  menuItem(PROP_EXP_DIR_PERIOD,      "Expansion Reversal Period (s)", prop(PROP_EXP_DIR_PERIOD), 0.125);
  ShapeCL::appendMenuItems();
}

void PolarCheckerboardShapeCL::render(int eye)
{
  if (bBlinkOn == 0)
    return;

  glDisable(GL_LIGHTING);

  glTranslatef(prop(PROP_X), prop(PROP_Y), prop(PROP_Z));
  glRotatef(dAngle, 0.0, 0.0, 1.0);

  drawShape();

  if (prop(PROP_BOWTIE) > 0.0)  // if bowtie, rotate 180 deg and draw again
  {
    glRotatef(180.0, 0.0, 0.0, 1.0);
    drawShape();
  }

  glEnable(GL_LIGHTING);
}

void PolarCheckerboardShapeCL::drawShapeExp()
{
  double dMagFactor = prop(PROP_MAG_FACTOR);
  double dDist = prop(PROP_MAX_RADIUS) - prop(PROP_MIN_RADIUS);
  double rStart = prop(PROP_MIN_RADIUS) + dExpState * dDist;
  double dThick = prop(PROP_THICKNESS) / prop(PROP_RADIAL_CYCLES);

  double r = rStart;
  double r2;

  int color = 0;
  if (dCounterphaseTime > prop(PROP_COUNTERPHASE_PERIOD)/2.0)
    color = 1;

  for (int rs=0; rs<prop(PROP_RADIAL_CYCLES); rs++)
  {
    r2 = r + dThick * pow(r/prop(PROP_MAX_RADIUS), dMagFactor);

    if (r2 > prop(PROP_MAX_RADIUS))
    {
      int savecol = color;
      drawCircularStrip(r, prop(PROP_MAX_RADIUS), color);
      color = savecol;
      double pctover = (r2 - prop(PROP_MAX_RADIUS)) / (r2-r);
      r = prop(PROP_MIN_RADIUS);
      r2 = r + pctover * dThick * pow(r/prop(PROP_MAX_RADIUS), dMagFactor);
      drawCircularStrip(r, r2, color);
    }
    else
      drawCircularStrip(r, r2, color);

    if (!(((int)prop(PROP_CIRCULAR_CYCLES)) % 2))
      color = !color;

    r = r2;
  }
}

void PolarCheckerboardShapeCL::drawShape()
{
  if (prop(PROP_EXP_SPEED) != 0.0)
  {
    drawShapeExp();
    return;
  }

  double rStart = prop(PROP_RADIUS) - prop(PROP_THICKNESS);
  double dThick = prop(PROP_THICKNESS);
  double dMagFactor = prop(PROP_MAG_FACTOR);
  double r = rStart;
  double r2;

  int color = 0;
  if (dCounterphaseTime > prop(PROP_COUNTERPHASE_PERIOD)/2.0)
    color = 1;

  for (int rs=0; rs<prop(PROP_RADIAL_CYCLES); rs++)
  {
    r2 = rStart + dThick * pow((rs+1.0)/prop(PROP_RADIAL_CYCLES), dMagFactor);

    drawCircularStrip(r, r2, color);

    if (!(((int)prop(PROP_CIRCULAR_CYCLES)) % 2))
      color = !color;

    r = r2;
  }
}

void PolarCheckerboardShapeCL::drawCircularStrip(double r, double r2, int &color)
{
  double A = prop(PROP_WEDGE_SIZE) * DEG2RAD / 2.0;
  double a = -A;
  double da = 2.0*A/prop(PROP_CIRCULAR_CYCLES);
  double a2 = a+da;

  for (int cs=0; cs<prop(PROP_CIRCULAR_CYCLES); cs++)
  {
    if (color)
      glColor4fv(fRGBA);
    else
    glColor3f(0.0, 0.0, 0.0);
    drawStrip(r, r2, a, a2);
    a = a2;
    a2 += da;
    color = !color;
  }
}

void PolarCheckerboardShapeCL::drawStrip(double r, double r2, double a, double a2)
{
  double angstep = (a2 - a) / prop(PROP_RESOLUTION);
  glBegin(GL_QUAD_STRIP);
  for (int i=0; i<=(int)prop(PROP_RESOLUTION); i++)
  {
    glVertex2f(r*cos(a), r*sin(a));
    glVertex2f(r2*cos(a),r2*sin(a));
    a += angstep;
  }
  glEnd();
}

void PolarCheckerboardShapeCL::updateAt(double dt)
{
  dAngle += (dt * prop(PROP_ROT_SPEED));
  if (dAngle > 360.0)
    dAngle -= 360.0;
  else if (dAngle < 0.0)
    dAngle += 360.0;

  if (prop(PROP_COUNTERPHASE_PERIOD) > 0.0)
  {
    dCounterphaseTime += dt;
    if (dCounterphaseTime > prop(PROP_COUNTERPHASE_PERIOD))
      dCounterphaseTime -= prop(PROP_COUNTERPHASE_PERIOD);
  }

  if (prop(PROP_EXP_DIR_PERIOD) > 0.0)
  {
    dExpDirTime += dt;
    if (dExpDirTime > prop(PROP_EXP_DIR_PERIOD))
    {
      dExpDirTime -= prop(PROP_EXP_DIR_PERIOD);
      iExpDir = 1 - iExpDir;
    }
  }

  if (prop(PROP_EXP_SPEED) != 0.0)
  {
    dExpState += (prop(PROP_EXP_SPEED) * dt * (iExpDir * -2.0 + 1.0));
    if (dExpState > 1.0) 
      dExpState -= 1.0;
    else if (dExpState < 0.0)
      dExpState += 1.0;
  }

  ShapeCL::updateAt(dt);
}

void PolarCheckerboardShapeCL::update(int id)
{
  switch (id)
  {
    case PROP_BOWTIE:
      //case PROP_MAG_FACT_THICK:
      if (prop(id) < 0.0)
        setProperty(id, 1.0);
      else if (prop(id) > 1.0)
        setProperty(id, 0.0);
      break;
    case PROP_INITIAL_PHASE:
      if (prop(id) < 0.0)
        setProperty(id, 0.0);
      else if (prop(id) > 360.0)
        setProperty(id, 360.0);
      if (prop(PROP_ROT_SPEED) == 0.0)
        dAngle = prop(id);    
      break;
    case PROP_MAX_RADIUS:
      if (prop(PROP_MAX_RADIUS) < prop(PROP_MIN_RADIUS))
        setProperty(PROP_MAX_RADIUS, prop(PROP_MIN_RADIUS));
      break;
    case PROP_MIN_RADIUS:
      if (prop(PROP_MIN_RADIUS) < 0.0)
        setProperty(PROP_MIN_RADIUS, 0.0);
      else if (prop(PROP_MIN_RADIUS) > prop(PROP_MAX_RADIUS))
        setProperty(PROP_MIN_RADIUS, prop(PROP_MAX_RADIUS));
      break;
    case PROP_RADIUS:
      if (prop(PROP_RADIUS) < prop(PROP_THICKNESS))
	// if reading from file, then radius (1st prop) might be smaller than thickness,
        // so should really adjust thickness instead, or will not be able to set radius
	// from a scene file (this was a bug) :
	//  setProperty(id, prop(PROP_THICKNESS));
        setProperty(PROP_THICKNESS, prop(PROP_RADIUS));
      break;
    case PROP_THICKNESS:
      if (prop(id) < 0.0)
        setProperty(id, 0.0);
      else if (prop(id) > prop(PROP_RADIUS))
        setProperty(id, prop(PROP_RADIUS));
      break;
    case PROP_WEDGE_SIZE:
      if (prop(id) < 0.0)
        setProperty(id, 0.0);
      else if (prop(id) > 360.0)
        setProperty(id, 360.0);
      break;
    case PROP_RESOLUTION:
    case PROP_RADIAL_CYCLES:
    case PROP_CIRCULAR_CYCLES:
      if (prop(id) < 1.0)
        setProperty(id, 1.0);
      break;
    case PROP_MAG_FACTOR:
      if (prop(id) < MIN_MAG_FACTOR)
        setProperty(id, MIN_MAG_FACTOR);
      break;
    case PROP_COUNTERPHASE_PERIOD:
    case PROP_EXP_DIR_PERIOD:
      if (prop(id) < 0.0)
        setProperty(id, 0.0);
      break;
    case PROP_EXP_SPEED:
      if (prop(id) == 0.0)
      {
        dExpState = 0.0;
        //setProperty(PROP_MAG_FACT_THICK, 0.0);
      }
      break;
    case PROP_ROT_SPEED:
      break;
    default:
      ShapeCL::update(id);
      break;
  }
}

//-------------------------------------------------------------------------
//
//  AnnulusShapeCL - Annulus with a gap
//
//-------------------------------------------------------------------------

class AnnulusShapeCL : public ShapeCL
{
protected:
  double dTime;        // for tracking landolt c period 
  double dTravelTime;  // for tracking movement using the prop_mvmt_* properties
  double dTravelPct;   // based on dTravelTime
  double dExpPct;      // for expanding bullseyes

public:
  AnnulusShapeCL();

  virtual void update(int id);
  virtual void updateAt(double dt);
  virtual void synchronize()  
  { 
    dTime = 0.0; dTravelTime = 0.0; ShapeCL::synchronize(); dExpPct = 0.0; 
  }
  virtual void render(int eye);
  virtual AnnulusShapeCL *duplicate()
  {
    AnnulusShapeCL *p = new AnnulusShapeCL();
    p->copyPropertiesFrom(this);
    p->incrementProperty(PROP_X, 2);
    p->incrementProperty(PROP_Y, 2);
    return p;
  }
  virtual int id() { return ID_ANNULUS; }
  enum {PROP_RADIUS=PROP_next, PROP_THICKNESS, PROP_ORIENTATION, PROP_GAP,
        PROP_POINTS_AROUND, PROP_LANDOLTC_PERIOD, PROP_MVMT_DIRECTION, 
        PROP_MVMT_MAGNITUDE, PROP_MVMT_PERIOD, PROP_NUM_RINGS, PROP_EXP_SPEED};
  virtual int lastPropertyIdx() { return PROP_EXP_SPEED; }
};

AnnulusShapeCL::AnnulusShapeCL() : ShapeCL(), 
  dTime(0.0), dTravelTime(0.0), dTravelPct(0.0), dExpPct(0.0)
{
  setProperty(PROP_RADIUS, 50);
  setProperty(PROP_THICKNESS, 25);
  setProperty(PROP_ORIENTATION, 0);
  setProperty(PROP_GAP, 0);
  setProperty(PROP_POINTS_AROUND, 60);
  setProperty(PROP_LANDOLTC_PERIOD, 0.0);
  setProperty(PROP_MVMT_DIRECTION, 0.0);
  setProperty(PROP_MVMT_MAGNITUDE, 0.0);
  setProperty(PROP_MVMT_PERIOD, 4.0);    // period to move from start->end, not start->end->start
  setProperty(PROP_NUM_RINGS, 1);
  setProperty(PROP_EXP_SPEED, 0);

  pMenu = new MenuCL(name());
  menuItem(PROP_RADIUS,      "Radius",            prop(PROP_RADIUS),      2);
  menuItem(PROP_THICKNESS,   "Thickness",         prop(PROP_THICKNESS),   2);
  menuItem(PROP_ORIENTATION, "Orientation",       prop(PROP_ORIENTATION), 2);
  menuItem(PROP_NUM_RINGS,   "Number of Rings",   prop(PROP_NUM_RINGS),   1);
  menuItem(PROP_EXP_SPEED,   "Expansion Speed (cyc/s)", prop(PROP_EXP_SPEED), 0.05);
  menuItem(PROP_GAP,         "Gap",               prop(PROP_GAP),         2);
  menuItem(PROP_POINTS_AROUND, "points around",   prop(PROP_POINTS_AROUND), 2);
  menuItem(PROP_LANDOLTC_PERIOD, "Landolt C Change Period", prop(PROP_LANDOLTC_PERIOD),  0.1);
  menuItem(PROP_MVMT_DIRECTION,  "Movement Direction",      prop(PROP_MVMT_DIRECTION),   1.0);
  menuItem(PROP_MVMT_MAGNITUDE,  "Movement Magnitude",      prop(PROP_MVMT_MAGNITUDE),   2.0);
  menuItem(PROP_MVMT_PERIOD,     "Movement Period",         prop(PROP_MVMT_PERIOD),      0.5);
  ShapeCL::appendMenuItems();
}

void AnnulusShapeCL::update(int id)
{
  switch (id)
  {
    case PROP_ORIENTATION:
    case PROP_MVMT_DIRECTION:
    {
      if (prop(id) > 360)
        setProperty(id, prop(id)-360);
      else if (prop(id) < 0)
        setProperty(id, prop(id)+360);
      break;
    }

    case PROP_THICKNESS:
    {
      if (prop(PROP_THICKNESS) > prop(PROP_RADIUS))
        setProperty(PROP_THICKNESS, prop(PROP_RADIUS));
      break;
    }

    case PROP_GAP:
    {
      if (prop(PROP_GAP) < 0.0)
        setProperty(PROP_GAP, 0.0);
      else if (prop(PROP_GAP) > 360.0)
        setProperty(PROP_GAP, 360.0);
      break;
    }

    case PROP_POINTS_AROUND:
    {
      if (prop(PROP_POINTS_AROUND) < 3)
        setProperty(PROP_POINTS_AROUND, 3);
      break;
    }

    case PROP_RADIUS:
    {
      if (prop(PROP_RADIUS) < 0.0)
        setProperty(PROP_RADIUS, 0.0);
     // if (prop(PROP_THICKNESS) > prop(PROP_RADIUS))
     //   setProperty(PROP_THICKNESS, prop(PROP_RADIUS));
      break;
    }

    case PROP_NUM_RINGS:
    {
      if (prop(id) < 1)
        setProperty(id, 1);
      break;
    }

    case PROP_LANDOLTC_PERIOD:
    case PROP_MVMT_PERIOD:
    case PROP_MVMT_MAGNITUDE:
    {
      if (prop(id) < 0.0)
        setProperty(id, 0.0);
      break;
    }

    default:
      ShapeCL::update(id);
  }
}

void AnnulusShapeCL::updateAt(double dt)
{
  ShapeCL::updateAt(dt);

  dExpPct += (dt * prop(PROP_NUM_RINGS) * prop(PROP_EXP_SPEED));
  if (dExpPct > 1.0)
    dExpPct -= 1.0;

  dTime += dt;
  if (dTime > prop(PROP_LANDOLTC_PERIOD))
  {
    dTime -= prop(PROP_LANDOLTC_PERIOD);
    if (prop(PROP_LANDOLTC_PERIOD) > 0.0)
    {
      setProperty(PROP_ORIENTATION, prop(PROP_ORIENTATION)+((rand()%3)+1)*90.0);
    }
  }

  dTravelTime += dt;
  if (dTravelTime > (prop(PROP_MVMT_PERIOD)*2.0))
    dTravelTime -= (prop(PROP_MVMT_PERIOD)*2.0);
  dTravelPct = dTravelTime / (prop(PROP_MVMT_PERIOD)*2.0);
  if (dTravelPct > 0.5)
    dTravelPct = 1.0 - dTravelPct;
  dTravelPct *= 2.0;
  dTravelPct -= 0.5;
}

void AnnulusShapeCL::render(int eye)
{
  if (bBlinkOn == 0)
    return;

  double angleDelta = DEG2RAD * (360.0 - prop(PROP_GAP)) / prop(PROP_POINTS_AROUND);
  double innerRadius = prop(PROP_RADIUS) - prop(PROP_THICKNESS);

  glDisable(GL_LIGHTING);
  glColor4fv(fRGBA);

  glPushMatrix();
    glTranslatef(prop(PROP_X)+dTravelPct*cos(prop(PROP_MVMT_DIRECTION)*DEG2RAD)*prop(PROP_MVMT_MAGNITUDE), 
                 prop(PROP_Y)+dTravelPct*sin(prop(PROP_MVMT_DIRECTION)*DEG2RAD)*prop(PROP_MVMT_MAGNITUDE), 
                 prop(PROP_Z));

  for (int r=0; r<(int)prop(PROP_NUM_RINGS); r++)
  {
    double r0   = (dExpPct + r) * 2.0 * prop(PROP_THICKNESS);
    double angleStart = DEG2RAD * (prop(PROP_ORIENTATION) + prop(PROP_GAP) / 2.0);
    glBegin(GL_QUAD_STRIP);
    for (int i=0; i<=prop(PROP_POINTS_AROUND); i++)
    {
      double cang = cos(angleStart);
      double sang = sin(angleStart);
      glVertex3f((r0 + prop(PROP_RADIUS))*cang, (r0 + prop(PROP_RADIUS))*sang, 0.0);
      glVertex3f((r0 + innerRadius)*cang, (r0 + innerRadius)*sang, 0.0);
      angleStart += angleDelta;
    }
    glEnd();
  }

  glPopMatrix();

  glEnable(GL_LIGHTING);
}

//-------------------------------------------------------------------------
//
//  RectShapeCL - simple rectangle
//
//-------------------------------------------------------------------------

class RectShapeCL : public ShapeCL
{
protected:
  double dHorizPhaseAdv;

public:
  RectShapeCL();

  virtual void update(int id);
  virtual void updateAt(double dt);
  virtual void render(int eye);
  virtual void synchronize()  
  { 
    dHorizPhaseAdv = prop(PROP_INIT_HORIZ_PHASE_ADV); ShapeCL::synchronize(); 
  }
  virtual RectShapeCL *duplicate()
  {
    RectShapeCL *p = new RectShapeCL();
    p->copyPropertiesFrom(this);
    p->incrementProperty(PROP_X, 2);
    p->incrementProperty(PROP_Y, 2);
    return p;
  }

  virtual int id() { return ID_RECT; }
  enum {PROP_WIDTH=PROP_next, PROP_HEIGHT, PROP_ROWS, PROP_COLS, PROP_V_GAP, PROP_H_GAP, 
        PROP_HORIZ_PHASE_ADV, PROP_INIT_HORIZ_PHASE_ADV };
  virtual int lastPropertyIdx() { return PROP_HORIZ_PHASE_ADV; }
};

RectShapeCL::RectShapeCL() : ShapeCL(), dHorizPhaseAdv(0.0)
{
  setProperty(PROP_WIDTH, 100);
  setProperty(PROP_HEIGHT, 100);
  setProperty(PROP_ROWS, 1);
  setProperty(PROP_COLS, 1);
  setProperty(PROP_V_GAP, 100);
  setProperty(PROP_H_GAP, 100);
  setProperty(PROP_HORIZ_PHASE_ADV, 0);
  setProperty(PROP_INIT_HORIZ_PHASE_ADV, 0);

  pMenu = new MenuCL(name());
  menuItem(PROP_WIDTH,   "Width",   prop(PROP_WIDTH),    2);
  menuItem(PROP_HEIGHT,  "Height",  prop(PROP_HEIGHT),   2);
  menuItem(PROP_ROWS,    "Rows",    prop(PROP_ROWS),     1);
  menuItem(PROP_COLS,    "Columns",    prop(PROP_COLS),     1);
  menuItem(PROP_V_GAP,   "Vertical Gap", prop(PROP_V_GAP), 2);
  menuItem(PROP_H_GAP,   "Horizontal Gap", prop(PROP_H_GAP), 2);
  menuItem(PROP_HORIZ_PHASE_ADV, "Horiz Phase Advance", prop(PROP_HORIZ_PHASE_ADV), 1);
  menuItem(PROP_INIT_HORIZ_PHASE_ADV, "Initial Horiz Phase Adv", prop(PROP_INIT_HORIZ_PHASE_ADV), 1);
  ShapeCL::appendMenuItems();
}

void RectShapeCL::updateAt(double dt)
{
  dHorizPhaseAdv += (dt * prop(PROP_HORIZ_PHASE_ADV));
  if (dHorizPhaseAdv > 360.0)
    dHorizPhaseAdv -= 360.0;
  else if (dHorizPhaseAdv < -360.0)
    dHorizPhaseAdv += 360.0;
}

void RectShapeCL::render(int eye)
{
  if (bBlinkOn == 0)
    return;

  float z = prop(PROP_Z);
  float w = prop(PROP_WIDTH);
  float h = prop(PROP_HEIGHT);

  float dx = w + prop(PROP_H_GAP);
  float dy = h + prop(PROP_V_GAP);

  float tw = (int)prop(PROP_COLS) * prop(PROP_WIDTH) + ((int)prop(PROP_COLS) - 1) * prop(PROP_H_GAP);
  float th = (int)prop(PROP_ROWS) * prop(PROP_HEIGHT) + ((int)prop(PROP_ROWS) - 1) * prop(PROP_V_GAP);

  float x = prop(PROP_X) - tw/2.0;
  float y = prop(PROP_Y) - th/2.0;
  float phasex = dx * dHorizPhaseAdv / 360.0;

  glDisable(GL_LIGHTING);

  glColor4fv(fRGBA);
  glBegin(GL_QUADS);

  for (int r=0; r<(int)prop(PROP_ROWS); r++)
  {
    x = prop(PROP_X) - tw/2.0 + phasex;
    for (int c=0; c<(int)prop(PROP_COLS); c++)
    {
      glVertex3f(x,y,z);
      glVertex3f(x+w,y,z);
      glVertex3f(x+w,y+h,z);
      glVertex3f(x,y+h,z);
      x += dx;
    }
    y += dy;
  }
  glEnd();
}

void RectShapeCL::update(int id)
{
  switch (id)
  {
    case PROP_WIDTH:
    case PROP_HEIGHT:
      if (prop(id) < 0.0) setProperty(id, 0.0);
      break;

    case PROP_HORIZ_PHASE_ADV:
    case PROP_INIT_HORIZ_PHASE_ADV:
      break;

    default:
      ShapeCL::update(id);
  }
}

//-------------------------------------------------------------------------
//
//  FixShapeCL - fixation point or cross
//
//-------------------------------------------------------------------------

class FixShapeCL : public ShapeCL
{
protected:

public:
  FixShapeCL();

  virtual void update(int id);
  virtual void render(int eye);
  virtual FixShapeCL *duplicate()
  {
    FixShapeCL *p = new FixShapeCL();
    p->copyPropertiesFrom(this);
    p->incrementProperty(PROP_X, 2);
    p->incrementProperty(PROP_Y, 2);
    return p;
  }

  virtual int id() { return ID_FIX_CROSS; }
  enum {PROP_SIZE=PROP_next, PROP_THICKNESS, PROP_ONTOP};
  virtual int lastPropertyIdx() { return PROP_ONTOP; }
};

FixShapeCL::FixShapeCL() : ShapeCL()
{
  iOrder = 1;  // this should cause fix cross to be rendered after other opaque shapes

  setProperty(PROP_SIZE, 12);
  setProperty(PROP_THICKNESS, 3);
  setProperty(PROP_ONTOP, 1);

  pMenu = new MenuCL(name());
  menuItem(PROP_SIZE,       "Size",              prop(PROP_SIZE),        1);
  menuItem(PROP_THICKNESS,  "Thickness",         prop(PROP_THICKNESS),   1);
  menuItem(PROP_ONTOP,      "Always on top",     prop(PROP_ONTOP),       1);
  ShapeCL::appendMenuItems();
}

void FixShapeCL::render(int eye)
{
  if (bBlinkOn == 0)
    return;

  float x = prop(PROP_X);
  float y = prop(PROP_Y);
  float z = prop(PROP_Z);
  float w = prop(PROP_THICKNESS);
  float h = prop(PROP_SIZE);

  glDisable(GL_LIGHTING);
  if (prop(PROP_ONTOP) == 1)
    glDisable(GL_DEPTH_TEST);

  glColor4fv(fRGBA);
  glBegin(GL_QUADS);
    glVertex3f(x-w,y-h,z);
    glVertex3f(x+w,y-h,z);
    glVertex3f(x+w,y+h,z);
    glVertex3f(x-w,y+h,z);
    glVertex3f(x-h,y-w,z);
    glVertex3f(x+h,y-w,z);
    glVertex3f(x+h,y+w,z);
    glVertex3f(x-h,y+w,z);
  glEnd();

  glEnable(GL_DEPTH_TEST);
}

void FixShapeCL::update(int id)
{
  switch (id)
  {
    case PROP_ONTOP:
      if (prop(PROP_ONTOP) < 0)
        setProperty(PROP_ONTOP, 1);
      else if (prop(PROP_ONTOP) > 1)
        setProperty(PROP_ONTOP, 0);
      break;

    case PROP_SIZE:
    case PROP_THICKNESS:
      break;

    default:
      ShapeCL::update(id);
  }
}

//-------------------------------------------------------------------------
//
//  QuartetShapeCL - an array of quartets stimulus
//
//-------------------------------------------------------------------------

class QuartetShapeCL : public ShapeCL
{
protected:
  double dTime; 
  double dPeriod;

public:
  QuartetShapeCL();

  virtual void updateAt(double seconds);
  virtual void update(int id);
  virtual void synchronize()  { dTime = 0.0; ShapeCL::synchronize(); }
  virtual void render(int eye);
  virtual QuartetShapeCL *duplicate()
  {
    QuartetShapeCL *p = new QuartetShapeCL();
    p->copyPropertiesFrom(this);
    p->incrementProperty(PROP_X, 2);
    p->incrementProperty(PROP_Y, 2);
    return p;
  }

  virtual int id() { return ID_QUARTETS; }
  //
  //  notes on properies:
  //  PROP_UNAMBIGUOUS : if true, then dots jump from side to side instead of from opp corners
  //  PROP_SYNCHRONOUS : if true, then all dots on for regular time and all off for same time
  //  PROP_CONTINUOUS  : if true, move dots back in forth (smooth motion instead of AM)
  //
  enum {PROP_WIDTH=PROP_next, PROP_HEIGHT, PROP_DOTSIZE, PROP_ROWS, PROP_COLS, PROP_CENTRAL_DOT, PROP_FREQUENCY,
        PROP_SPACING, PROP_ORIENTATION, PROP_UNAMBIGUOUS, PROP_SYNCHRONOUS, PROP_CONTINUOUS, PROP_ISI};
  virtual int lastPropertyIdx() { return PROP_ISI; }
};

QuartetShapeCL::QuartetShapeCL() : ShapeCL(), 
  dTime(0.0),
  dPeriod(0.0)
{
  setProperty(PROP_WIDTH, 50);
  setProperty(PROP_HEIGHT, 50);
  setProperty(PROP_DOTSIZE, 3);
  setProperty(PROP_ROWS, 1);
  setProperty(PROP_COLS, 1);
  setProperty(PROP_CENTRAL_DOT, 1);  // boolean, whether central dot on or off
  setProperty(PROP_FREQUENCY, 2);
  setProperty(PROP_SPACING, 150);
  setProperty(PROP_ORIENTATION, 0);
  setProperty(PROP_UNAMBIGUOUS, 0);
  setProperty(PROP_SYNCHRONOUS, 0);
  setProperty(PROP_CONTINUOUS, 0);
  setProperty(PROP_ISI, 0);

  pMenu = new MenuCL(name());
  menuItem(PROP_WIDTH,       "Width",     prop(PROP_WIDTH),                2);
  menuItem(PROP_HEIGHT,      "Height",    prop(PROP_HEIGHT),               2);
  menuItem(PROP_FREQUENCY,   "Frequency (Hz)", prop(PROP_FREQUENCY),            0.1);
  menuItem(PROP_ISI,         "ISI (ms)",       prop(PROP_ISI),                  2);
  menuItem(PROP_DOTSIZE,     "Dot size (px)",               prop(PROP_DOTSIZE),     1);
  menuItem(PROP_CENTRAL_DOT, "Central dot on/off",     prop(PROP_CENTRAL_DOT), 1);
  menuItem(PROP_ROWS,        "Rows in array",          prop(PROP_ROWS),        1);
  menuItem(PROP_COLS,        "Columns in array",       prop(PROP_COLS),        1);
  menuItem(PROP_SPACING,     "Array size per col/row", prop(PROP_SPACING),     1);
  menuItem(PROP_ORIENTATION, "Orientation (deg)",      prop(PROP_ORIENTATION), 2);
  menuItem(PROP_UNAMBIGUOUS, "Unambiguous motion",     prop(PROP_UNAMBIGUOUS), 1);
  menuItem(PROP_SYNCHRONOUS, "Synchronous flashing",   prop(PROP_SYNCHRONOUS), 1);
  menuItem(PROP_CONTINUOUS,  "Continuous motion",      prop(PROP_CONTINUOUS),  1);
  ShapeCL::appendMenuItems();
}

void QuartetShapeCL::update(int id)
{
  switch (id)
  {
    case PROP_WIDTH:
    case PROP_HEIGHT:
    case PROP_DOTSIZE:
    case PROP_SPACING:
      break;

    case PROP_ROWS:
      if (prop(PROP_ROWS) < 1)
        setProperty(PROP_ROWS, 1);  // reminder: setProperty will call this function again; add setPropOnly fn?
      break;
    case PROP_COLS:
      if (prop(PROP_COLS) < 1)
        setProperty(PROP_COLS, 1);
      break;

    case PROP_CENTRAL_DOT:
    case PROP_UNAMBIGUOUS:
    case PROP_SYNCHRONOUS:
    case PROP_CONTINUOUS:
    {
      if (prop(id) < 0)
        setProperty(id, 1);
      else if (prop(id) > 1)
        setProperty(id, 0);
      break;
    }

    case PROP_ORIENTATION:
    {
      if (prop(id) > 360)
        setProperty(id, prop(id)-360);
      else if (prop(id) < 0)
        setProperty(id, prop(id)+360);
      break;
    }
    case PROP_ISI:
    {
      if (prop(id) < 0) setProperty(id, 0.0);
      break;
    }
    case PROP_FREQUENCY:
      dTime = 0.0;
      if (prop(PROP_FREQUENCY) < 0.0)
        setProperty(PROP_FREQUENCY, 0.0);
      if (prop(PROP_FREQUENCY) != 0.0)
        dPeriod = 1.0 / prop(PROP_FREQUENCY);
      else
        dPeriod = 1.0;
      break;

    default:
      ShapeCL::update(id);
  }
}

void QuartetShapeCL::updateAt(double dt)
{
  ShapeCL::updateAt(dt);

  dTime += dt;
  if (dTime > dPeriod)
    dTime -= dPeriod;
}

void QuartetShapeCL::render(int eye)
{
  if (bBlinkOn == 0)
    return;

  bool bUnambig = (prop(PROP_UNAMBIGUOUS) != 0.0);
  bool bSync    = (prop(PROP_SYNCHRONOUS) != 0.0);
  bool bContin  = (prop(PROP_CONTINUOUS) != 0.0);
  bool bOn      = false; // if sync, then only draw when On

  float w = prop(PROP_WIDTH) / 2.0;
  float h = prop(PROP_HEIGHT) / 2.0;
  float dist = bContin ? (h * 2.0) : 0.0;  // for continuous motion
  double dISItime = dTime;

  if (dTime > (dPeriod/2.0)) {
    bOn = true;
    h *= -1;
    dist *= -1 * ((dTime / (dPeriod/2.0)) - 1.0);
    dISItime -= (dPeriod/2.0);
  }
  else dist *= (dTime / (dPeriod/2.0));

  // don't draw anything except center dots during interstimulus interval
  bool bISIon = (!bSync && !bContin && dISItime < (prop(PROP_ISI)/1000.0));

  float dx = prop(PROP_SPACING); 
  float dy = prop(PROP_SPACING);

  glDisable(GL_LIGHTING);
  glColor4fv(fRGBA); 
  glPointSize(prop(PROP_DOTSIZE));

  float z = prop(PROP_Z);
  float x = prop(PROP_X) - dx * (prop(PROP_COLS)-1) / 2.0;
  for (int c=0; c<prop(PROP_COLS); c++)
  {
    float y = prop(PROP_Y) - dy * (prop(PROP_ROWS)-1) / 2.0;
    for (int r=0; r<prop(PROP_ROWS); r++)
    {
      glPushMatrix();
      glTranslatef(x, y, 0);
      glRotatef(prop(PROP_ORIENTATION), 0, 0, 1);
      glBegin(GL_POINTS);

      if (bSync)
      {
        if (bOn) {
        glVertex3f( w,  h, z);
        glVertex3f( w, -h, z);
        glVertex3f(-w,  h, z);
        glVertex3f(-w, -h, z); }
      }
      else if (!bISIon)
      {
        glVertex3f(w, h-dist, z);
        if (bUnambig) glVertex3f(-w, h-dist, z); else glVertex3f(-w, -h+dist, z);
      }
      if (prop(PROP_CENTRAL_DOT) == 1)
        glVertex3f(0, 0, z);

      glEnd();
      glPopMatrix();
      y += dy;
    }
    x += dx;
  }

  glEnable(GL_LIGHTING);
}

//-------------------------------------------------------------------------
//
//  ScrewShapeCL - Encapsulation of Screw Shape (screwahedron) Geometry (only)
//
//-------------------------------------------------------------------------

class ScrewShapeCL : public ShapeCL
{
protected:
  vector<Vector3CL> vPoints;         // for random collections of points 

  vector<Vector3CL> vCoords;         // mesh coords
  inline Vector3CL &mesh(int ang, int hgt)   { return vCoords[ang+hgt*(int)prop(PROP_POINTS_AROUND)]; }
  vector<Vector3CL> vNormals;        // per vertex mesh normals
  vector<int>       vVI;             // vertex indices for quad strips
  float             fRGBA2[4];        // rgb colors for dots
  int               iDirection;      // used to keep track of rpm when reversing period > 0
  double            dTime;           // used in conjunction with iDirection and PROP_SWITCH_RPM_PERIOD

public:
  inline int pointsAround() const { return (int)prop(PROP_POINTS_AROUND); }
  inline int pointsHigh()   const { return (int)prop(PROP_POINTS_HIGH); }

  ScrewShapeCL();

  virtual void updateAt(double seconds);
  virtual void update(int id);
  virtual void synchronize()  
  { 
    setProperty(PROP_PHASE, 0.0); 
    initializeRandomPoints(1);
    dTime = 0.0;
    iDirection = 0;
    ShapeCL::synchronize();
  }
  virtual int  id() { return ID_SCREW; }
  virtual void setOpacity(float val) { fRGBA[3] = (fRGBA2[3] = val); }

  // - make sure fProps has enought room for all properties
  // - only use setProperty and prop to set/retreive values, don't access fProps directly
  enum {PROP_RADIUS=PROP_next, PROP_HEIGHT, PROP_WIGGLES, PROP_PITCH, PROP_AMPLITUDE, PROP_PHASE,
        PROP_DOTSIZE, PROP_RPM, PROP_SILHOUETTE, PROP_AXISPITCH,
        PROP_KD_OR_MESH, PROP_NUM_DOTS, PROP_POINTS_AROUND, PROP_POINTS_HIGH,
        PROP_RED2, PROP_GREEN2, PROP_BLUE2, PROP_DOTS_TRANSLATE, PROP_AXISYAW,
        PROP_SWITCH_RPM_PERIOD, PROP_PPATHCONTINUE, PROP_DENSITY_COMPENSATED};
  virtual int lastPropertyIdx() { return PROP_DENSITY_COMPENSATED; }

  //
  //  calculate the point on the surface of the screw at angle [0..1] and height [0..1]
  //  (angle is stretched to [0..2*pi] inside)
  //
  inline void calcCoordinateAt(double angle, double height, Vector3CL &pt, double dR=0.0) 
  {
    assert(angle >= 0.0 && angle <= 1.0);   // out of bounds may not cause an error, but would be confusing
    assert(height >= 0.0 && height <= 1.0);

    //
    //  if density compensated is true, then make density of projection of dots uniform
    //
    if (prop(PROP_DENSITY_COMPENSATED))
    {
      angle = acos(angle*2.0 - 1);
      if (rand() % 2)
        angle += M_PI;
      angle += (prop(PROP_PHASE) * DEG2RAD);
    }
    else
      angle *= (2.0*M_PI);
    // twopi cancels on top and bottom of timesaround, should mult radius by 2pi for circumference,
    // and mult numerator when used to calculate phase advance based on height when calculating xy below
    double timesaround = prop(PROP_HEIGHT) / (prop(PROP_RADIUS) * tan(prop(PROP_PITCH)*M_PI/180.0));
    //double xy = prop(PROP_RADIUS) * (1.0 + prop(PROP_AMPLITUDE) * cos(prop(PROP_WIGGLES)*angle + prop(PROP_PITCH)*height)); 
    double xy = (dR + prop(PROP_RADIUS)) * (1.0 + prop(PROP_AMPLITUDE) * cos(prop(PROP_WIGGLES)*(angle + timesaround*height))); 
    pt[0] = cos(angle) * xy;
    pt[2] = sin(angle) * xy;
    pt[1] = (height - 0.5) * prop(PROP_HEIGHT);
  }

  //
  //  create[Mesh|RandomPoints] : generate points for object, but external routine must 
  //  render either the surface or points using api of choice such as opengl or open inventor
  //
  //  create a 2D mesh of points for rendering a surface
  void initializeMesh();
  //  choose a bunch of random points on the surface
  void initializeRandomPoints(int bAll=0);  // if bAll true, recalc all points
};

void rotateXZ(Vector3CL &P, double ang)  // rotate point about Y axis 'ang' radians
{
  double newX = P[0]*cos(ang) + P[2]*sin(ang);
  P[2] = -P[0]*sin(ang) + P[2]*cos(ang);
  P[0] = newX;
}

void ScrewShapeCL::updateAt(double seconds)
{
  ShapeCL::updateAt(seconds);

  if (prop(PROP_SWITCH_RPM_PERIOD) > 0.0)
  {
    dTime += seconds;
    if (dTime > prop(PROP_SWITCH_RPM_PERIOD))
    {
      dTime -= prop(PROP_SWITCH_RPM_PERIOD);
      iDirection = 1 - iDirection;
    }
  }

  float fRev = (iDirection ? -1.0 : 1.0);

  if (prop(PROP_DOTS_TRANSLATE) == 1)
  {
    double dh = fRev*prop(PROP_RPM) * tan(prop(PROP_PITCH)*DEG2RAD) * prop(PROP_RADIUS)*TWO_PI * seconds / 60;
    double angleoff = prop(PROP_HEIGHT) / (prop(PROP_RADIUS) * tan(prop(PROP_PITCH)*DEG2RAD));

    for (int i=0; i<vPoints.size(); i++)
    {
      vPoints[i][1] -= dh;
      if (vPoints[i][1] > (prop(PROP_HEIGHT) / 2.0)) 
      { 
        vPoints[i][1] -= prop(PROP_HEIGHT);
        rotateXZ(vPoints[i], -angleoff);
      }
      else if (vPoints[i][1] < (-prop(PROP_HEIGHT) / 2.0))
      { 
        vPoints[i][1] += prop(PROP_HEIGHT);
        rotateXZ(vPoints[i], angleoff);
      }
    }
  }

  //setProperty(PROP_PHASE, prop(PROP_PHASE) + (TWO_PI * prop(PROP_RPM) * seconds));
  setProperty(PROP_PHASE, prop(PROP_PHASE) + (fRev*prop(PROP_RPM) * seconds * 6.0));

  if (prop(PROP_PPATHCONTINUE) < 1.0)
  {
    for (int i=0; i<vPoints.size(); i++)
      if (pow(randNorm(),2) > prop(PROP_PPATHCONTINUE))
        calcCoordinateAt(randNorm(), randNorm(), vPoints[i], prop(PROP_DOTSIZE)/2.0);
  }
}

ScrewShapeCL::ScrewShapeCL() :
  iDirection(0),
  dTime(0.0),
  ShapeCL()
{
  fRGBA[0] = fRGBA[1] = fRGBA[2] = fRGBA[3] = 1.0;
  setProperty(PROP_RADIUS, 150);
  setProperty(PROP_HEIGHT, 300);
  setProperty(PROP_WIGGLES, 3);
  setProperty(PROP_PITCH, 45);
  setProperty(PROP_AMPLITUDE, 0.25);
  setProperty(PROP_PHASE, 0.0);  // rotation angle updated by rpm
  setProperty(PROP_DOTSIZE, 3.0);
  setProperty(PROP_RPM, 20);
  setProperty(PROP_SILHOUETTE, 0);
  setProperty(PROP_AXISPITCH, 0);
  setProperty(PROP_AXISYAW, 0);
  setProperty(PROP_KD_OR_MESH, 0);
  setProperty(PROP_NUM_DOTS, 500);
  setProperty(PROP_POINTS_AROUND, 64);
  setProperty(PROP_POINTS_HIGH, 32);
  setProperty(PROP_RED2, 255);
  setProperty(PROP_GREEN2, 255);
  setProperty(PROP_BLUE2, 255);
  setProperty(PROP_DOTS_TRANSLATE, 0);
  setProperty(PROP_SWITCH_RPM_PERIOD, 0);
  setProperty(PROP_PPATHCONTINUE, 1.0);
  setProperty(PROP_DENSITY_COMPENSATED, 0.0);
}

void ScrewShapeCL::update(int id)
{
  switch (id)
  {
    case PROP_RADIUS:
    case PROP_HEIGHT:
    case PROP_WIGGLES:
    case PROP_PITCH:
    case PROP_AMPLITUDE:
    case PROP_POINTS_AROUND:
    case PROP_POINTS_HIGH:
      if (pointsAround()>0 && pointsHigh()>0)
        initializeMesh();
      if (vPoints.size() > 0)
        initializeRandomPoints(1);
      break;

    case PROP_NUM_DOTS:
      if (prop(PROP_NUM_DOTS) < 0)
        setProperty(PROP_NUM_DOTS, 0);
      initializeRandomPoints();
      break;

    //
    //  on/off, 0 or 1 properties
    //
    case PROP_DOTS_TRANSLATE:
      synchronize();
    case PROP_SILHOUETTE:
    case PROP_DENSITY_COMPENSATED:
      if (prop(id) < 0)
        setProperty(id, 1);
      else if (prop(id) > 1)
        setProperty(id, 0);
      break;

    case PROP_KD_OR_MESH:
      if (prop(PROP_KD_OR_MESH) < 0)
        setProperty(PROP_KD_OR_MESH, 2);
      else if (prop(PROP_KD_OR_MESH) > 2)
        setProperty(PROP_KD_OR_MESH, 0);
      break;

    case PROP_SWITCH_RPM_PERIOD:
      if (prop(id) < 0.0) setProperty(id, 0);
      dTime = 0.0;
      break;

    case PROP_PHASE:
    case PROP_DOTSIZE:
    case PROP_RPM:
    case PROP_X:
    case PROP_Y:
    case PROP_AXISPITCH:
    case PROP_AXISYAW:
      // (do nothing)
      break;

    case PROP_PPATHCONTINUE:
      if (prop(id) < 0.0) setPropertyOnly(id, 0.0);
      else if (prop(id) > 1.0) setPropertyOnly(id, 1.0);
      break;

    case PROP_RED2:
    case PROP_GREEN2:
    case PROP_BLUE2:
    {
      for (int i=PROP_RED2; i<=PROP_BLUE2; i++)
      {
        if (prop(i) < 0)
          setProperty(i, 0);
        else if (prop(i) > 255)
          setProperty(i, 255);
        fRGBA2[i-PROP_RED2] = prop(i) / 255.0;
      }
      break;
    }

    default:
      ShapeCL::update(id);
  }
}

void ScrewShapeCL::initializeMesh()
{
  if (pointsHigh() <= 2 || pointsAround() <= 2)
    return;

  vCoords.resize(pointsAround()*pointsHigh()); 
  vNormals.resize(pointsAround()*pointsHigh());  // per vertex
  vVI.resize(pointsAround()*2*(pointsHigh()-1)); // pointsHigh-1 # strips (w/ 2*pointsAround pts each)

  //
  //  initialize vVI for indexed quad strips
  //
  int idx = 0, y, x;
  for (y=0; y<(pointsHigh()-1); y++)
    for (x=0; x<pointsAround(); x++)
    {
      vVI[idx++] = x+y*pointsAround();
      vVI[idx++] = x+(y+1)*pointsAround();
    }
  //
  //  create the mesh
  //
  idx = 0;
  for (y=0; y<pointsHigh(); y++)
    for (x=0; x<pointsAround(); x++)
      calcCoordinateAt((double)x/(pointsAround()-1), (double)y/(pointsHigh()-1), vCoords[idx++]);
  //
  //  calculate per-face normals (temporary)
  //
  Vector3CL v2;
  int sizex1 = pointsAround()-1;
  vector<Vector3CL> vFaceNorm;
  vFaceNorm.resize((pointsAround()-1)*(pointsHigh()-1));
  for (x=0; x<pointsAround()-1; x++)
    for (y=0; y<pointsHigh()-1; y++)
    {
      idx = x+y*sizex1;
      vFaceNorm[idx] = mesh(x+1,y);
      vFaceNorm[idx] -= mesh(x,y);
      v2 = mesh(x+1,y+1);
      v2 -= mesh(x+1,y);
      vFaceNorm[idx].cross(v2);
      vFaceNorm[idx].normalize();
    }
  //
  //  calculate averaged vertex normals using face normals
  //
  idx = 0;
  for (y=0; y<pointsHigh(); y++)
    for (x=0; x<pointsAround(); x++)
    {
      vNormals[idx].clear();

      //
      //  mesh is assumed to wrap around in points around direction so that there is no seam
      //  i.e., a cylindrical shape is assumed
      //  remember that there are iPointsHigh*iPointsAround vertex normals, but only
      //  (iPointsHigh-1)*(iPointsAround-1) face normals
      //
      if (y < (pointsHigh()-1))
        if (x < (pointsAround()-1))
          vNormals[idx] += vFaceNorm[x+y*sizex1];
        else
          vNormals[idx] += vFaceNorm[0+y*sizex1];

      if (y > 0)
        if (x < (pointsAround()-1))
          vNormals[idx] += vFaceNorm[x+(y-1)*sizex1];
        else
          vNormals[idx] += vFaceNorm[0+(y-1)*sizex1];

      if (y > 0)
        if (x > 0)
          vNormals[idx] += vFaceNorm[x-1+(y-1)*sizex1];
        else
          vNormals[idx] += vFaceNorm[pointsAround()-2+(y-1)*sizex1];

      if (y < (pointsHigh()-1))
        if (x > 0)
          vNormals[idx] += vFaceNorm[x-1+y*sizex1];
        else
          vNormals[idx] += vFaceNorm[pointsAround()-2+y*sizex1];

      vNormals[idx].normalize();
      vNormals[idx] *= -1;
      idx++;
    }
}

void ScrewShapeCL::initializeRandomPoints(int bAll)
{
  int ptsnow = vPoints.size();
  vPoints.resize((int)prop(PROP_NUM_DOTS));

  for (int i=(bAll ? 0 : ptsnow); i<(int)prop(PROP_NUM_DOTS); i++)
  {
    double ang = /* 2.0*M_PI* */randNorm();
    double h   = randNorm();
    calcCoordinateAt(ang, h, vPoints[i], prop(PROP_DOTSIZE)/2.0);
  }
}

//-------------------------------------------------------------------------
//
//  GLScrewInterfaceGL - interface to ScrewShapeCL for opengl/glut
//
//  (a different interface class should be provided for open inventor or other api)
//
//-------------------------------------------------------------------------

class GLScrewInterfaceCL : public ScrewShapeCL
{
public:
  GLScrewInterfaceCL();

  void renderKD();    // render kinetic dot version of screw
  void renderMesh();  // render surface mesh version of screw
  virtual void render(int eye);
  virtual GLScrewInterfaceCL *duplicate()
  {
    GLScrewInterfaceCL *p = new GLScrewInterfaceCL();
    p->copyPropertiesFrom(this);
    p->incrementProperty(PROP_X, 2);
    p->incrementProperty(PROP_Y, 2);
    return p;
  }
};

GLScrewInterfaceCL::GLScrewInterfaceCL() :
  ScrewShapeCL()
{
  pMenu = new MenuCL(name());
  menuItem(PROP_KD_OR_MESH, "Solid or KD or Both", prop(PROP_KD_OR_MESH), 1);
  menuItem(PROP_RADIUS,     "Radius",            prop(PROP_RADIUS),      5.0);
  menuItem(PROP_HEIGHT,     "Height",            prop(PROP_HEIGHT),      5.0);
  menuItem(PROP_WIGGLES,    "Number of Wiggles", prop(PROP_WIGGLES),     1.0);
  menuItem(PROP_AMPLITUDE,  "Thread Amplitude",  prop(PROP_AMPLITUDE),   0.01);
  menuItem(PROP_PITCH,      "Thread Pitch",      prop(PROP_PITCH),       1.0);
  menuItem(PROP_AXISPITCH,  "Axis Pitch",        prop(PROP_AXISPITCH),   1);
  menuItem(PROP_AXISYAW,    "Axis Yaw",          prop(PROP_AXISYAW),     1);
  menuItem(PROP_RPM,        "RPM",               prop(PROP_RPM),         1);
  menuItem(PROP_SWITCH_RPM_PERIOD, "Switch RPM Period", prop(PROP_SWITCH_RPM_PERIOD), 0.25);
  menuItem(PROP_SILHOUETTE, "silhouette",        prop(PROP_SILHOUETTE),  1);
  menuItem(PROP_POINTS_AROUND, "points around",  prop(PROP_POINTS_AROUND), 1);
  menuItem(PROP_POINTS_HIGH,   "points high",    prop(PROP_POINTS_HIGH), 1);
  pMenu->appendItem(new SeparatorItemCL());
  menuItem(PROP_NUM_DOTS,   "number of dots",    prop(PROP_NUM_DOTS),    4);
  menuItem(PROP_PPATHCONTINUE, "p(path continue)", prop(PROP_PPATHCONTINUE), 0.02);
  menuItem(PROP_DENSITY_COMPENSATED, "density compensated", prop(PROP_DENSITY_COMPENSATED), 1);
  menuItem(PROP_DOTSIZE,    "dot size",          prop(PROP_DOTSIZE),     1);
  menuItem(PROP_RED2,      "Dot Red",            prop(PROP_RED2),         2);
  menuItem(PROP_GREEN2,    "Dot Green",          prop(PROP_GREEN2),       2);
  menuItem(PROP_BLUE2,     "Dot Blue",           prop(PROP_BLUE2),        2);
  menuItem(PROP_DOTS_TRANSLATE, "Dots Rotate or Translate", prop(PROP_DOTS_TRANSLATE), 1);
  pMenu->appendItem(new SeparatorItemCL());
  ShapeCL::appendMenuItems();
}

void GLScrewInterfaceCL::render(int eye)
{
  if (bBlinkOn == 0)
    return;

  if (prop(PROP_KD_OR_MESH) >= 1)
    renderKD();
  if (prop(PROP_KD_OR_MESH) == 0 || prop(PROP_KD_OR_MESH) == 2)
    renderMesh();
}

void GLScrewInterfaceCL::renderKD()
{
  if (prop(PROP_NUM_DOTS) <= 0)
    return;
  if (vPoints.size() == 0)
    initializeRandomPoints();

  glDisable(GL_LIGHTING);
  glColor3fv(fRGBA2); //fRGB2[0], fRGB2[1], fRGB2[2]);
  glPushMatrix();
  {
    glTranslatef(prop(PROP_X), prop(PROP_Y), prop(PROP_Z));
    glRotatef(prop(PROP_AXISYAW), 0.0, 1.0, 0.0);
    glRotatef(prop(PROP_AXISPITCH), 1.0, 0.0, 0.0);

    if (prop(PROP_DOTS_TRANSLATE) == 0)  // if rotating dots, not translating
      glRotatef(prop(ScrewShapeCL::PROP_PHASE), 0.0, 1.0, 0.0);

    glPointSize(prop(ScrewShapeCL::PROP_DOTSIZE));
    glBegin(GL_POINTS);
      for (vector<Vector3CL>::iterator it=vPoints.begin();
           it!=vPoints.end(); it++)
      {
        glVertex3fv((*it).raw());
      }
    glEnd();
  }
  glPopMatrix();
}

void GLScrewInterfaceCL::renderMesh()
{
  if (!pointsAround() || !pointsHigh())
    return;
  if (vCoords.size() == 0)
    initializeMesh();

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // put this in for wireframe

  if (prop(PROP_SILHOUETTE) == 1)
    glDisable(GL_LIGHTING);
  else
    glEnable(GL_LIGHTING);  // disable lighting for silhouette

  glShadeModel(GL_SMOOTH);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);    // polygons lit one side by default
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fRGBA);  // for rendering with lighting
  glColor4fv(fRGBA);         // for rendering without lighting (base color - silhouette)

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  glVertexPointer(3, GL_FLOAT, 0, vCoords[0].raw());
  glNormalPointer(GL_FLOAT, 0, vNormals[0].raw());

  glPushMatrix();
  {
    glTranslatef(prop(PROP_X), prop(PROP_Y), prop(PROP_Z));
    glRotatef(prop(PROP_AXISYAW), 0.0, 1.0, 0.0);
    glRotatef(prop(PROP_AXISPITCH), 1.0, 0.0, 0.0);
    glRotatef(prop(ScrewShapeCL::PROP_PHASE), 0.0, 1.0, 0.0);
    for (int i=0; i<(pointsHigh()-1); i++)
      glDrawElements(GL_QUAD_STRIP, pointsAround()*2, GL_UNSIGNED_INT, &vVI[i*pointsAround()*2]);
  }
  glPopMatrix();

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);  
}


//=========================================================================
//
//  ShapeMgrCL
//
//  singleton manager of all instantiated shapes and file I/O
//
//=========================================================================

struct MenuCallbackItem : public MenuCL::ItemCL
{
  int iMsg;
  MenuCL *pMenu;
  MenuCallbackItem(MenuCL *m, int id, char *sz) : MenuCL::ItemCL(sz), iMsg(id), pMenu(m) {}
  virtual void action()
  {
    pMenu->callback(iMsg);
  }
};

struct DelConfirmMenuCL : public MenuCL
{
  ShapeCL *pShape;
  void setShape(ShapeCL *p) 
  { 
    pShape = p; 
  }
  DelConfirmMenuCL() : MenuCL(), pShape(0L) 
  {
    setTitle("Delete this shape ?");
    appendItem(new MenuCallbackItem(this, ID_YES, "Yes"));
    appendItem(new MenuCallbackItem(this, ID_NO, "No"));
  }
  virtual void callback(int msg);
};

struct ExitConfirmMenuCL : public MenuCL
{
  ExitConfirmMenuCL() : MenuCL()
  {
    setTitle("Exit without saving changes ?");
    appendItem(new MenuCallbackItem(this, ID_YES, "Yes"));
    appendItem(new MenuCallbackItem(this, ID_NO, "No"));
  }
  virtual void callback(int msg)
  {
    if (msg == ID_YES)
      exit(0);
    MenuCL::currentMenu(0L);
  }

  static ExitConfirmMenuCL *pInst;
  static ExitConfirmMenuCL *inst()
  {
    if (!pInst)
      pInst = new ExitConfirmMenuCL();
    return pInst;
  }
};
ExitConfirmMenuCL *ExitConfirmMenuCL::pInst = 0L;

//-------------------------------------------------------------------------
//
//  StringEntryItemCL - string input widget
//
//-------------------------------------------------------------------------

struct StringEntryItemCL : public MenuCL::ItemCL
{
  int iMinSize; // minimum size of item, used to set text that isn't deleted at start of string
  StringEntryItemCL(char *sz, int minsize=0) : MenuCL::ItemCL(sz), iMinSize(minsize) {}
  virtual void glut_key(unsigned char key)
  {
    if (key == 8)
    {
      if (sText.size() > iMinSize)
        sText.erase(sText.size()-1);
    }
    else
      sText += key;
  }
  virtual void glut_key_special(int key)
  {
  }  
};

//-------------------------------------------------------------------------
//
//    ShapeMgrCL
//
//-------------------------------------------------------------------------

class ShapeMgrCL
{
protected:
  static ShapeMgrCL *pInstance;

  string             sFilename;            // set to name of file when scene is saved or opened from exist file
  int                bIsDirty;             // true when changes not saved
  vector<ShapeCL*>   vShapes;              // container of instantiated shapes
  MenuCL            *opEditShapeMenu, *opDeleteShapeMenu, *opDuplicateShapeMenu;
  MenuCL            *opBlockDesignMenu;
  DelConfirmMenuCL  *opDeleteShapeConfirmMenu;
  //  int              iHighlightCtr;        // used to control flashing of highlighted shapes
  double fProps[MAX_PROPS];  // indexed by PROP_ enumerated labels
  double dTime;
  double dExperimentClock;   // experiment countdown clock
  float  fRGB[3];            // background (gl clear) color
  double dRampTime;          // when ramping between blocks, how far are we into the ramp? (in seconds)
  double dFrameDelayClock;   // min time btwn frames, see property PROP_MIN_FRAME_DELAY (s)
  double dDummyScanClock;    // countdown to start of experiment to allow for dummy scans
  double dEyeSwitchClock;    // countdown to next eye exchange

  int    bRunning;  // true when running an experiment
  int    bHolding;  // true when waiting to run an experiment, or when just should not render any shapes
  unsigned int iBlock, iNextBlock;  // iBlock is the current block, iNextBlock is used to keep track of which
                                    // block is next after an inter-block delay, because iBlock is set to zero
                                    // during these 'blank' intervals
  ShapeMgrCL();
  void renderAllEye(int eye, int onlyone, unsigned int block);

public:
  void     manageShape(ShapeCL *s);
  ShapeCL *createShape(int id);
  ShapeCL *duplicateShape(ShapeCL *s);
  void     deleteShape(ShapeCL *s);
  void     deleteAll();
  void     openFile(const string &sFN, int bAdding=0);
  string   getFilename() { return sFilename; }
  void     synchronizeAll();
  void     swapBlocks();
  void     swapEyes();
  double   initializeExperimentClock();
  double   getExperimentClock() { return dExperimentClock; }

  int      getFirstBlockNo();
  int      getNextBlockNo();

  inline int  isRunning() { return bRunning; }

  int      prepareToStartExperiment();  // return > 0 on error
  void     startExperiment(int bData=1);
  void     abortExperiment();

  StringEntryItemCL *pSubjectName;
  string   getSubjectName() 
  {
    string name(pSubjectName->text(), 14);
    return name;
  }
  float getBGcolor(int i) { return fRGB[i]; }

  //
  // saveScene checks to see if need to prompt user to overwrite an existing file
  //
  void saveScene(const string &sFN);
  //
  //  saveSceneOverwrite assumes it is ok to write to the specified file (an existing file is overwritten)
  //
  void saveSceneOverwrite(const string &sFN);

  void updateAll(double dt);
  void renderAll();

  inline void dirty(int s)    { bIsDirty = s; }
  inline int  isDirty() const { return bIsDirty; }

  void  setProperty(int propno, double value)         { fProps[propno] = value;  update(propno); }
  void  incrementProperty(int propno, double delta)   { fProps[propno] += delta; update(propno); }
  inline double prop(int propno) const                { return fProps[propno]; }
  void  update(int id)
  {
    switch (id)
    {
      case PROP_VIEW_BLOCK:
      {
        if (prop(PROP_VIEW_BLOCK) < 0)
          setProperty(PROP_VIEW_BLOCK, 0);
        else if (prop(PROP_VIEW_BLOCK) > 128)
          setProperty(PROP_VIEW_BLOCK, 128);
        break;
      }
      case PROP_RAMP_PERIOD:
      case PROP_EYE_SWITCH_PERIOD:
      case PROP_MIN_FRAME_DELAY:
      {
        if (prop(id) < 0.0)
          setProperty(id, 0.0);
        break;
      }
      case PROP_BLOCK_PERIOD:
      {
        if (prop(PROP_BLOCK_PERIOD) < 1.0)
          setProperty(PROP_BLOCK_PERIOD, 1.0);
        break;
      }
      case PROP_DUMMY_TIME:
      case PROP_INTERBLOCK_DELAY:
      {
        if (prop(id) < 0.0)
          setProperty(id, 0.0);
        break;
      }
      case PROP_NUM_CYCLES:
      {
        if (prop(PROP_NUM_CYCLES) < 0.5)
          setProperty(PROP_NUM_CYCLES, 1.0);
        break;
      }
      case PROP_ANAGLYPH_MODE:
      {
        if (prop(id) < 0)
          setProperty(id, 3);
        else if (prop(id) > 3)
          setProperty(id, 0);
        break;
      }
      case PROP_RED:
      case PROP_GREEN:
      case PROP_BLUE:
      {
        for (int i=PROP_RED; i<=PROP_BLUE; i++)
        {
          if (prop(i) < 0)
            setProperty(i, 0);
          else if (prop(i) > 255)
            setProperty(i, 255);
          fRGB[i-PROP_RED] = prop(i) / 255.0;
        }
        glClearColor(fRGB[0],fRGB[1],fRGB[2],0.0);
        break;
      }
      case PROP_BLOCK_ORDER:
      {
        if (prop(id) < 1) setProperty(id, 1); else if (prop(id) > 2) setProperty(id, 2);
        break;
      }
      case PROP_BLOCK_1:
      case PROP_BLOCK_2:
      case PROP_BLOCK_3:
      case PROP_BLOCK_4:
      case PROP_BLOCK_5:
      case PROP_BLOCK_6:
      case PROP_BLOCK_7:
      case PROP_BLOCK_8:
      {
        if (prop(id) < 0) setProperty(id, 0);
        break;
      }
    }
  }

  MenuCL *getEditShapeMenu(int showall=0);  // update and return the menu of instantiated shapes avail for editing
  MenuCL *getDeleteShapeMenu();
  MenuCL *getDuplicateShapeMenu();
  MenuCL *getBlockDesignMenu();
  DelConfirmMenuCL *getDeleteShapeConfirmMenu(); 

  static ShapeMgrCL *inst();

  enum {PROP_VIEW_BLOCK, PROP_BLOCK_PERIOD, PROP_NUM_CYCLES, PROP_RED, PROP_GREEN, PROP_BLUE, 
        PROP_ANAGLYPH_MODE, PROP_RAMP_PERIOD, PROP_INTERBLOCK_DELAY, PROP_DUMMY_TIME,
        PROP_EYE_SWITCH_PERIOD, PROP_BLOCK_ORDER, PROP_BLOCK_1, PROP_BLOCK_2, PROP_BLOCK_3,
        PROP_BLOCK_4, PROP_BLOCK_5, PROP_BLOCK_6, PROP_BLOCK_7, PROP_BLOCK_8, PROP_MIN_FRAME_DELAY};
  int lastPropertyIdx() { return PROP_MIN_FRAME_DELAY; }

#ifdef SAM
  int iSamMode;
  void SamMode(int iset)
  {
    iSamMode = iset;
    if (iSamMode)
    {
      if (ShapeMgrCL::inst()->prepareToStartExperiment() > 0)
      {
        iSamMode = 0;
      }
    }
  }

  void updateFixPos()
  {
    gxpos = figures[1][0][0].x;
    gypos = figures[1][0][0].y;
    if (tswgl[0] == 4) {
      gxpos += figures[2][0][0].x;
      gypos += figures[2][0][0].y;
    }
    gxpos *= (800.0 / 30.0);
    gypos *= (800.0 / 30.0);
    //for (vector<ShapeCL*>::iterator it=vShapes.begin(); it!=vShapes.end(); it++)
      //if ((*it)->id() == ID_FIX_CROSS)
      //{
      //  (*it)->setProperty(ShapeCL::PROP_X, xpos * 800.0/30.0);
      //  (*it)->setProperty(ShapeCL::PROP_Y, ypos * 800.0/30.0);
      //}
  }
#endif
};

int ShapeMgrCL::getFirstBlockNo()
{
  switch ((int)prop(PROP_BLOCK_ORDER))
  {
    case 1:
      for (int i=0; i<8; i++)
        if (prop(i+PROP_BLOCK_1) > 0.0)
          return (int)pow(2.0,i);
      return 0;
    case 2:
      return getNextBlockNo();
  }  
  return 0;
}

//
//  get next block number based on PROP_BLOCK_ORDER:
//  if ==1, go through block in order, using PROP_BLOCK_x
//  values that are >0. if ==2, then randomly choose the
//  next block (with replacement) based on the relative
//  block frequencies stored in the PROP_BLOCK_x values
//
//  TODO: this is more complicated than it needs to be:
//  iBlock is a bit mask, i.e. the on bit position defines
//  the current block, not the numeric value of iBlock,
//  and PROP_BLOCK_1 to _8 run consecutively, but are not
//  zero or one based....
int ShapeMgrCL::getNextBlockNo()
{
  switch ((int)prop(PROP_BLOCK_ORDER))
  {
    case 1:  // run blocks in order
    { 
      int b = 1;
      if (iBlock > 0) b = (int)log2(iBlock) + 1;
      int i = b + 1;  // i and b are **1** based, not zero
      for (; i<=8; i++)
        if (prop(i-1+PROP_BLOCK_1) > 0.0)
          return (int)pow(2.0,i-1);
      for (i=1; i<=b; i++)
        if (prop(i-1+PROP_BLOCK_1) > 0.0)
          return (int)pow(2.0,i-1);
      return 0;
      break;
    }
    case 2:  // randomly choose blocks
    {
      int i;
      double dW = 0.0;
      for (i=PROP_BLOCK_1; i<=PROP_BLOCK_8; i++)
        dW += prop(i);
      double r = randNorm() * dW;
      dW = 0;
      for (i=PROP_BLOCK_1; i<=PROP_BLOCK_8; i++)
      {
        dW += prop(i);
        if (r <= dW && (prop(i) > 0.0))
          return (int)pow(2.0,i-PROP_BLOCK_1);
      }
      return 0;
      break;
    }
    default:  // randomly choose blocks
    {
      std::cerr << "ERROR: prop block order not a known value (" << (int)prop(PROP_BLOCK_ORDER) << ")\n";
      MenuCL::currentMenu(MessageMenuCL::menu("ERROR: prop_block_order has invalid value"));
      return 0;
      break;
    }
  }
  return 0;
}


//
//  glut_key moved here so it can see definition of shapemgrcl
//  glut_key_special moved here to keep it next to glut_key
//
void MenuCL::glut_key(unsigned char key)
{
  switch (key)
  {
    case 'q':  // force quit if hit 'q' and no menu present
      if (!currentMenu())
        exit(0);
      else
        currentMenu()->glut_key_inst(key);
      break;
    case 'v':  // toggle verbose mode
      if (!currentMenu())
        bVerbose = !bVerbose;
      else
        currentMenu()->glut_key_inst(key);
      break;
    //
    //  if not running, (and no menu active) keys '1' and '2' will switch to viewing that block,
    //  or '3' to viewing both blocks or '0' to turn off both blocks
    //  (this is a shortcut to the menu item that does the same)
    //
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    {
      if (!currentMenu())
      {
        if (!ShapeMgrCL::inst()->isRunning())
	{
          int imod = glutGetModifiers();
          int iblk = 0; if (key-'0') iblk = 1 << (key-'0'-1);
          if (imod != GLUT_ACTIVE_CTRL)
	    ShapeMgrCL::inst()->setProperty(ShapeMgrCL::PROP_VIEW_BLOCK, iblk);
          else
	    ShapeMgrCL::inst()->setProperty(ShapeMgrCL::PROP_VIEW_BLOCK, ((int)ShapeMgrCL::inst()->prop(ShapeMgrCL::PROP_VIEW_BLOCK)) ^ iblk);
	}
      }
      else
        currentMenu()->glut_key_inst(key);
      break;
    }
    case 27: // ESC
      if (currentMenu())
        currentMenu(currentMenu()->parentMenu());
      else
        currentMenu(pMainMenu);
      break;
    case 13: // enter
      if (currentMenu())
        currentMenu()->select();
      break;
    default:
      if (currentMenu())
        currentMenu()->glut_key_inst(key);
      break;
      //  std::cout << (int)key << std::endl;
  }
}
void MenuCL::glut_key_special(int key)
{
  float fmod = 1.0;
  int   imod = glutGetModifiers();
  if (imod == GLUT_ACTIVE_SHIFT)
    fmod = 4.0;
  else if (imod == GLUT_ACTIVE_CTRL)
    fmod = 0.25;

  switch (key)
  {
    case GLUT_KEY_UP:
      if (currentMenu())
        currentMenu()->changeSelected(-1);
      break;
    case GLUT_KEY_DOWN:
      if (currentMenu())
        currentMenu()->changeSelected(1);
      break;
    case GLUT_KEY_LEFT:
      if (currentMenu())
        currentMenu()->delta(-fmod);
      break;
    case GLUT_KEY_RIGHT:
      if (currentMenu())
        currentMenu()->delta(fmod);
      break;
    default:
      currentMenu()->glut_key_special_inst(key);
      break;
  }
}


struct OK2OverwriteMenuCL : public MenuCL
{
  string szSaveFilename;
  OK2OverwriteMenuCL() : MenuCL()
  {
    appendItem(new MenuCallbackItem(this, ID_YES, "OK"));
    appendItem(new MenuCallbackItem(this, ID_NO, "Cancel"));
  }
  virtual void callback(int msg)
  {
    if (msg == ID_YES)
    {
      ShapeMgrCL::inst()->saveSceneOverwrite(szSaveFilename);
      MenuCL::currentMenu(0L);
    }
    else
      MenuCL::currentMenu(0L);
  }
  static OK2OverwriteMenuCL *pInst;
  static OK2OverwriteMenuCL *inst(const string &sFN)
  {
    if (!pInst)
      pInst = new OK2OverwriteMenuCL();
    pInst->szSaveFilename = sFN;
    pInst->setTitle(string("File Exists. Overwrite file: ") + sFN + string(" ?"));
    return pInst;
  }
};
OK2OverwriteMenuCL *OK2OverwriteMenuCL::pInst = 0L;

void DelConfirmMenuCL::callback(int msg)
{
  if (msg == ID_YES)
  {
    ShapeMgrCL::inst()->deleteShape(pShape);
  }
  MenuCL::currentMenu(0L);
}

//-------------------------------------------------------------------------
//
//  Types of Menu Item
//
//-------------------------------------------------------------------------

struct TotalExpTimeItemCL : public MenuCL::ItemCL
{
  TotalExpTimeItemCL() : MenuCL::ItemCL() { iSkip = 1; color(1); alwaysSetLabel(1); setLabel(); }
  virtual void setLabel()
  {
    unsigned int clock = (unsigned int)ShapeMgrCL::inst()->initializeExperimentClock();
    
    char sz[128];
    sprintf(sz, "Exp Time (cycles * blocks * block_length):  %d:%02d", clock/60, clock%60);
    sText = sz;
  }
};

//
//  QuitItem menu item
//

struct QuitItemCL : public MenuCL::ItemCL  // menu item interface for quitting program
{
  QuitItemCL() : MenuCL::ItemCL("Quit") {}
  virtual void action() 
  { 
    if (ShapeMgrCL::inst()->isDirty())
      MenuCL::currentMenu(ExitConfirmMenuCL::inst());
    else
      exit(0); 
  }
};

//
//  Menu item that leads to a submenu
//

struct SubMenuItemCL : public MenuCL::ItemCL
{
  MenuCL *pSubmenu;
  SubMenuItemCL(char *sz, MenuCL *submenu) : MenuCL::ItemCL(sz), pSubmenu(submenu) {}
  SubMenuItemCL(const string &s, MenuCL *submenu) : MenuCL::ItemCL(s), pSubmenu(submenu) {}
  virtual void action()         { MenuCL::currentMenu(pSubmenu); }
  virtual void delta(float dir) { MenuCL::currentMenu(pSubmenu); }
};

struct ShapeSubMenuItemCL : public SubMenuItemCL
{
  ShapeCL *pShape;
  ShapeSubMenuItemCL(ShapeCL *s) : SubMenuItemCL(s->name(), s->getMenu()), pShape(s) {}
  virtual void highlight(int h)
  {
    iSelected = h;  // maybe this iselected variable isn't necessary...
    pShape->highlight(h);
  }
};

struct DupShapeSubMenuItemCL : public MenuCL::ItemCL
{
  ShapeCL *pShape;
  DupShapeSubMenuItemCL(ShapeCL *s) : MenuCL::ItemCL(s->name()), pShape(s) {}
  virtual void highlight(int h)
  {
    pShape->highlight(h);
  }
  virtual void action()
  {
    ShapeMgrCL::inst()->duplicateShape(pShape);
    MenuCL::currentMenu(0L);
  }
};

struct ShapeDelMenuItemCL : public MenuCL::ItemCL
{
  ShapeCL *pShape;
  ShapeDelMenuItemCL(ShapeCL *s) : MenuCL::ItemCL(s->name()), pShape(s) {}
  virtual void highlight(int h)
  {
    pShape->highlight(h);
  }
  virtual void action()
  {
    DelConfirmMenuCL *delmenu = ShapeMgrCL::inst()->getDeleteShapeConfirmMenu();
    delmenu->setShape(pShape);
    MenuCL::currentMenu(delmenu);
  }
};

struct NewShapeMenuItemCL : public MenuCL::ItemCL
{
  int iId; 
  NewShapeMenuItemCL(int shapeid) : MenuCL::ItemCL(ShapeInterface::nameOfShapeWithId(shapeid)), iId(shapeid) {}
  virtual void action()         
  { 
    ShapeMgrCL::inst()->createShape(iId);  
    MenuCL::currentMenu(0L); 
  }
};

struct DuplicateShapeMenuItemCL : public MenuCL::ItemCL
{
  DuplicateShapeMenuItemCL() : MenuCL::ItemCL("Clone Shape") {}
  virtual void action()
  {
    MenuCL::currentMenu(ShapeMgrCL::inst()->getDuplicateShapeMenu());
  }
};

struct EditShapeMenuItemCL : public MenuCL::ItemCL
{
  EditShapeMenuItemCL() : MenuCL::ItemCL("Edit Shape (choose from blocks on)") {}
  virtual void action()
  {
    MenuCL::currentMenu(ShapeMgrCL::inst()->getEditShapeMenu());
  }
};

struct EditAllShapesMenuItemCL : public MenuCL::ItemCL
{
  EditAllShapesMenuItemCL() : MenuCL::ItemCL("Edit Shape (choose from all)") {}
  virtual void action()
  {
    MenuCL::currentMenu(ShapeMgrCL::inst()->getEditShapeMenu(1));
  }
};

struct DeleteShapeMenuItemCL : public MenuCL::ItemCL
{
  DeleteShapeMenuItemCL() : MenuCL::ItemCL("Delete Shape") {}
  virtual void action()
  {
    MenuCL::currentMenu(ShapeMgrCL::inst()->getDeleteShapeMenu()); 
  }
};


//
//  Save Scene Menu
//
struct SaveMenuCL : public MenuCL
{
  StringEntryItemCL *pFilenameItem;

  SaveMenuCL() : MenuCL()
  {
    setTitle("Save Scene in File:");
    appendItem(pFilenameItem = new StringEntryItemCL("scene.txt"));
    appendItem(new MenuCallbackItem(this, ID_YES, "OK"));
    appendItem(new MenuCallbackItem(this, ID_NO,  "Cancel"));
  }
  virtual void callback(int msg)
  {
    if (msg == ID_YES)
    {
      // save scene should also do something about the current menu
      ShapeMgrCL::inst()->saveScene(pFilenameItem->text());
      //ShapeMgrCL::inst()->dirty(0);
      //MenuCL::currentMenu(MessageMenuCL::menu("Save not implemented..."));
    }
    else
      MenuCL::currentMenu(0L);
  }
  virtual void focus(int f)
  {
    if (!f) return;
    pFilenameItem->sText = ShapeMgrCL::inst()->getFilename();
    if (pFilenameItem->sText == "")
      pFilenameItem->sText = "scene.txt";
  }

  static SaveMenuCL *pInst;
  static SaveMenuCL *inst()
  {
    if (!pInst)
      pInst = new SaveMenuCL();
    return pInst;
  }
};
SaveMenuCL *SaveMenuCL::pInst = 0L;

//
//  Clear dirty scene confirm menu
//

struct ClearConfirmMenuCL : public MenuCL
{
  ClearConfirmMenuCL() : MenuCL()
  {
    setTitle("Clear this unsaved scene ?");
    appendItem(new MenuCallbackItem(this, ID_YES, "Yes"));
    appendItem(new MenuCallbackItem(this, ID_NO, "No"));    
  }
  virtual void callback(int msg)
  {
    if (msg == ID_YES)
      ShapeMgrCL::inst()->deleteAll();
    MenuCL::currentMenu(0L);
  }

  static ClearConfirmMenuCL *pInst;
  static ClearConfirmMenuCL *inst()
  {
    if (!pInst) pInst = new ClearConfirmMenuCL();
    return pInst;
  }
};
ClearConfirmMenuCL *ClearConfirmMenuCL::pInst = 0L;

//
//  Run Experiment Go Menu
//
struct StartExperimentMenuST : public MenuCL
{
  StartExperimentMenuST() : MenuCL()
  {
    appendItem(new MenuCallbackItem(this, ID_YES, "go"));
  }
  //virtual void focus(int f)
  //{
  //  MenuCL::focus(f);
  //  if (!f)
  //    return;
  //  ShapeMgrCL::inst()->prepareToStartExperiment();
  //}
  virtual void callback(int msg)
  {
    MenuCL::currentMenu(0L);
    if (msg == ID_YES)
      ShapeMgrCL::inst()->startExperiment();
  }
  static MenuCL *pInst;
  static MenuCL *inst() {
    if (!pInst) pInst = new StartExperimentMenuST();
    return pInst;
  }
};
MenuCL *StartExperimentMenuST::pInst = 0L;

//
//  Main Menu is subclassed to provide focus override that stops exp if running
//
struct MainMenuCL : public MenuCL
{
  MainMenuCL() : MenuCL("Main Menu") {}
  virtual void focus(int f)
  {
    MenuCL::focus(f);
    if (f)
      ShapeMgrCL::inst()->abortExperiment();
  }
};

//
//  Clear Scene Menu
//

struct ClearMenuItemCL : public MenuCL::ItemCL
{
  ClearMenuItemCL() : MenuCL::ItemCL("Clear Scene") {}
  virtual void action()
  {
    if (ShapeMgrCL::inst()->isDirty())
      MenuCL::currentMenu(ClearConfirmMenuCL::inst());
    else
    {
      ShapeMgrCL::inst()->deleteAll();
      MenuCL::currentMenu(0L);
    }
  }
};

//
//  Open File menu items - each one represents a filename
//
struct OpenFileItemCL : public MenuCL::ItemCL
{
  int bAdding;  // 0==replacing, 1==adding, 2==replacing and preparing to run
  OpenFileItemCL(char *sz, int adding=0) : MenuCL::ItemCL(sz), bAdding(adding) {}
  virtual void action()
  {
    ShapeMgrCL::inst()->openFile(sText, bAdding);
    MenuCL::currentMenu((bAdding==2) ? StartExperimentMenuST::inst() : 0L);
    ShapeMgrCL::inst()->synchronizeAll();  // maybe openFile should always do this?
  }
};

//
//  Open Scene Menu
//
struct OpenMenuCL : public MenuCL
{
  int bAdding; // adding=0 means replacing scene, adding=1 means adding scene, adding=2 means replacing
               // scene and preparing to start experiment
  OpenMenuCL(int adding=0) : MenuCL(), bAdding(adding)
  {
    setTitle("Open Scene file:");
  }
  virtual void focus(int f)
  {
    if (!f) return;
    clear();
#ifdef LINUX
    DIR *d = opendir(".");  // hopefully always the current working dir
    if (!d) { std::cout << "could not opendir\n"; return; }
    dirent *entry;
    while (entry = readdir(d))
    {
      if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        appendItem(new OpenFileItemCL(entry->d_name, bAdding));
    }
#else
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    hFind = FindFirstFile("*", &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
      std::cerr << "FindFirstFile failed with error: " << GetLastError() << "\n";
      return;
    }
    if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, ".."))
      appendItem(new OpenFileItemCL(FindFileData.cFileName, bAdding));
    while (FindNextFile(hFind, &FindFileData) != 0)
      if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, ".."))
        appendItem(new OpenFileItemCL(FindFileData.cFileName, bAdding));
    FindClose(hFind);
#endif
  }
};


//-------------------------------------------------------------------------
//
//  ActionMenuItemCL : menu item that calls a functor
//
//-------------------------------------------------------------------------

template <typename T>
struct ActionMenuItemCL : public MenuCL::ItemCL
{
  const T oAction;
  ActionMenuItemCL(char *sz, const T &action_funct) : MenuCL::ItemCL(sz), oAction(action_funct) {}
  virtual void action() { oAction(); }
};

struct SynchronizeActionST
{
  void operator()() const
  {
    ShapeMgrCL::inst()->synchronizeAll();
    MenuCL::currentMenu(0L);
  }
};

struct SwapBlocksActionST
{
  void operator()() const
  {
    ShapeMgrCL::inst()->swapBlocks();
    MenuCL::currentMenu(0L);
  }
};

struct SwapEyesActionST
{
  void operator()() const
  {
    ShapeMgrCL::inst()->swapEyes();
  }
};

struct RunNowNoDataST
{
  void operator()() const
  {
    MenuCL::currentMenu(0L);
    if (ShapeMgrCL::inst()->prepareToStartExperiment() > 0)
      return;
    ShapeMgrCL::inst()->startExperiment(0);  // 0==don't write datafile
  }
};

struct PrepareStartST
{
  void operator()() const
  {
    if (ShapeMgrCL::inst()->prepareToStartExperiment() == 0)
      MenuCL::currentMenu(StartExperimentMenuST::inst());
  }
};

struct BlockDesignActionST
{
  void operator()() const
  {
    MenuCL::currentMenu(ShapeMgrCL::inst()->getBlockDesignMenu());
  }
};

#ifdef SAM
struct SamModeST
{
  void operator()() const
  {
    MenuCL::currentMenu(0L);
    ShapeMgrCL::inst()->SamMode(1);
  }
};
#else
struct SplitScreenST
{
  void operator()() const
  {
    bSam = !bSam;
  }
};
#endif

//-------------------------------------------------------------------------
//
//  ShapePropertyItemCL methods
//
//-------------------------------------------------------------------------

struct ShapeMgrPropertyItemCL : public MenuCL::ItemCL
{
  string sLabel;
  float fDelta;
  int iId;
  int iDispType;
  ShapeMgrPropertyItemCL(char *sz, int id, float del) :
    MenuCL::ItemCL(sz), sLabel(sz), fDelta(del), iId(id), iDispType(DISP_DOUBLE2) { setLabel(); }
  void setDisplayType(int set) { iDispType=set; }
  //
  //  gSetLabel is used by ShapeMgrPropertyItemCL::setLabel and 
  //  ShapePropertyItemCL::setLabel -- it just provides a common implementation of 
  //  formatting properties values on menus
  //
  static void gSetLabel(string &s, double val, const string &label, int format)
  {
    unsigned int i;
    char sz[128];
    char szcat[2];
    szcat[1] = 0;
    switch (format)
    {
      case DISP_BINARY8:
      {
        unsigned int ui = (unsigned int)val;
        sprintf(sz, "%s: (", label.c_str(), (int)ui);
        for (i=1, szcat[0]='1'; i<256; i*=2, szcat[0]++)
        {
          if (ui & i)
            strcat(sz, szcat);
          else
            strcat(sz, "-");
        }
        strcat(sz, ")");
        break;
      }
      default:
        sprintf(sz, "%s = %.3f", label.c_str(), (float)val);
        break;
    }
    s = sz;
  }


  virtual void delta(float dir)
  {
    ShapeMgrCL::inst()->incrementProperty(iId, fDelta*dir);
    setLabel();    
  }
  virtual void setLabel()
  {
    gSetLabel(sText, ShapeMgrCL::inst()->prop(iId), sLabel, iDispType);
    //char sz[128];
    //sprintf(sz, "%s = %.2f", sLabel.c_str(), (float)ShapeMgrCL::inst()->prop(iId));
    //sText = sz;
  }
  virtual void focus(int f)
  {
    if (f) setLabel();
  }
};

struct ShapePropertyItemCL : public MenuCL::ItemCL  // menu item interface for a property of a screw
{
  string sLabel;
  float fDelta;
  //myScrewShapeCL *pObject;
  ShapeCL *pObject;
  int iId;

  // iDispType should be one of DISP_ enumerated types:
  int iDispType;  // how to display the property, as double, binary, etc.
  void setDisplayType(int set) { iDispType=set; }

  ShapePropertyItemCL(ShapeCL *obj, int id, char *sz, float del=1.0) : 
    MenuCL::ItemCL(sz), pObject(obj), iId(id), fDelta(del), sLabel(sz), iDispType(DISP_DOUBLE2) { setLabel(); }

  virtual void delta(float dir);
  virtual void focus(int f);  // 0==lose focus; 1==fain focus
  void setLabel();
};

ShapePropertyItemCL *ShapeCL::menuItem(int id, char *sz, float val, float del)
{
  ShapePropertyItemCL *p = 0L;

  setProperty(id, val);
  if (pMenu)
  {
    p = new ShapePropertyItemCL(this, id, sz, del);
    pMenu->appendItem(p);
  }
  return p;
}

void ShapeCL::appendMenuItems()
{
    pMenu->appendItem(new SeparatorItemCL());
    MenuCL::ItemCL *item = new SeparatorItemCL("common shape properties:");
    item->color(1);
    pMenu->appendItem(item);
    menuItem(PROP_BLINK_FREQ,  "Blink Frequency",      prop(PROP_BLINK_FREQ),  0.1);
    menuItem(PROP_BLINK_INITIAL_PHASE, "Blink Initial Phase", prop(PROP_BLINK_INITIAL_PHASE), 0.01);
    menuItem(PROP_BLINK_DUTY_CYCLE, "Blink Duty Cycle (fraction ON)", prop(PROP_BLINK_DUTY_CYCLE), 0.01);
    menuItem(PROP_X,           "X position",           prop(PROP_X),           2);
    menuItem(PROP_Y,           "Y position",           prop(PROP_Y),           2);
    menuItem(PROP_Z,           "Z position",           prop(PROP_Z),           2);
    menuItem(PROP_RED,         "Red",                  prop(PROP_RED),         2);
    menuItem(PROP_GREEN,       "Green",                prop(PROP_GREEN),       2);
    menuItem(PROP_BLUE,        "Blue",                 prop(PROP_BLUE),        2);
    menuItem(PROP_COLOR_TRACK_BG, "Synchronize Color to BG Color (on/off)", prop(PROP_COLOR_TRACK_BG), 1);
    menuItem(PROP_RAMPED,      "Block-synced Ramp Up(1), Down(2), or off(0)",     prop(PROP_RAMPED),      1);
    menuItem(PROP_RAMP_REPEAT, "Ramp Repeat Time (non block-based)", prop(PROP_RAMP_REPEAT), 0.5);
    menuItem(PROP_RAMP_PERIOD, "Ramping Period (non block-based)",   prop(PROP_RAMP_PERIOD), 0.5);  
    menuItem(PROP_RAMP_UPORDOWN, "Ramp Up(0) or Down(1) (non block-based)", prop(PROP_RAMP_UPORDOWN), 1);
    menuItem(PROP_ALWAYS_ON,   "Always On",            prop(PROP_ALWAYS_ON),   1);
    menuItem(PROP_STENCIL,     "Stencil (1=set 2=test 3=-test)", prop(PROP_STENCIL), 1);
    menuItem(PROP_EYE,         "Eye (1=L 2=R 3=both)", prop(PROP_EYE),         1);
    menuItem(PROP_BLOCK,       "Block 1=A 2=B etc",  prop(PROP_BLOCK),       1)->setDisplayType(DISP_BINARY8);
}

//
//  ShapeCL::synchronize needs to know about ShapeMgrCL, so its definition is here
//
void ShapeCL::synchronize()
{ 
  dRampClock = 0.0; 
  dBlinkClock = 0.0; 
  if (prop(PROP_BLINK_FREQ) > 0.0)
    dBlinkClock = prop(PROP_BLINK_INITIAL_PHASE) / prop(PROP_BLINK_FREQ);

  if ((int)prop(PROP_COLOR_TRACK_BG))
  {
    for (int i=0; i<3; i++)
      fRGBA[i] = ShapeMgrCL::inst()->getBGcolor(i);
  }
}

void ShapePropertyItemCL::delta(float dir)
{ 
  pObject->incrementProperty(iId, fDelta*dir);
  ShapeMgrCL::inst()->dirty(1);
  setLabel();
}

void ShapePropertyItemCL::setLabel()
{
  ShapeMgrPropertyItemCL::gSetLabel(sText, pObject->prop(iId), sLabel, iDispType);
}

void ShapePropertyItemCL::focus(int f)
{
  if (f)
    setLabel();
}

//============================================================================
//
//  getTime returns time now in microseconds, ulPrevTime used to calculate dt
//
//============================================================================

unsigned long ulPrevTime = 0;

unsigned long getTime()
{
#ifdef LINUX
  timeval tv;
  gettimeofday(&tv, 0L);
  unsigned long usec = tv.tv_sec*1000000 + tv.tv_usec;
  return usec;
#else
  return glutGet(GLUT_ELAPSED_TIME) * 1000;
#endif
}

//-------------------------------------------------------------------------
//
//  DataCL - data file interface
//
//-------------------------------------------------------------------------

string date_time_stamp()
{
  string sDateTime = "Date_Time";
#ifdef LINUX
  FILE *fp = popen("/bin/date +%m-%d-%y_%H-%M-%S","r");
  if (fp)
  {
    char sz[64];
    fgets(sz, sizeof(sz), fp);
    sz[17] = 0;  // should be mm-dd-yy_hh_mm_ss
    sDateTime = sz;
    pclose(fp);
  }
#else
  char szdate[32], sztime[32];
  _strdate(szdate);
  _strtime(sztime);
  szdate[2] = szdate[5] = '-';
  sztime[2] = sztime[5] = '-';
  sDateTime = string(szdate) + string("_") + string(sztime);
#endif
  return sDateTime;
}

class DataCL
{
protected:
  static int bRecording;
  static string sFilename;
  static ofstream *pout;
  static unsigned long ulStartTime;

public:
  static int startFile(const string &prefix)
  {
    //int i;
    //char szdate[32], sztime[32];
    //_strdate(szdate);
    //_strtime(sztime);
    //szdate[2] = szdate[5] = '-';
    //sztime[2] = sztime[5] = '-';
    //sFilename = string(szdate) + string("_") + string(sztime) + string("_") + prefix + string(".txt");
    sFilename = date_time_stamp() + string("_") + prefix + string(".txt");

    bRecording = 1;

    if (!pout)
      pout = new ofstream();
    if (pout->is_open())
      pout->close();
    pout->open(sFilename.c_str(), ios::out);
    ulStartTime = getTime() / 1000;
    (*pout) << 0L << " \t" << 0L << std::endl;  // at time zero, output event 0 (start/end code)
    if (!(*pout))
      return 1;
    return 0;
  }
  static void record(int code)
  {
    if (!isRecording() || !pout || !pout->is_open()) 
      return;
    (*pout) << ((getTime()/1000)-ulStartTime) << " \t" << code << std::endl;
  }
  static void endFile()
  {
    bRecording = 0;
    if (!pout) return;
    if (pout->is_open()) pout->close();
  }
  static void abort()
  {
    if (!pout || !isRecording()) return;
    if (pout->is_open()) {
      record(-1);
      (*pout) << "ABORTED" << std::endl;
    }
    endFile();
  }
  static void recordingOff() { bRecording = 0; }
  static int  isRecording()  { return bRecording; }
};
string         DataCL::sFilename = "";
int            DataCL::bRecording = 0;
ofstream      *DataCL::pout = 0L;
unsigned long  DataCL::ulStartTime = 0L;

//-------------------------------------------------------------------------
//
//  ShapeMgrCL - member definitions
//
//-------------------------------------------------------------------------

ShapeMgrCL::ShapeMgrCL() :                           // (hide ctor)
  opEditShapeMenu(0L), 
  opDeleteShapeMenu(0L),
  opBlockDesignMenu(0L),
  opDeleteShapeConfirmMenu(0L),
  opDuplicateShapeMenu(0L),
  dTime(0.0),
  dExperimentClock(0.0),
  dRampTime(0.0),
  dDummyScanClock(0.0),
  dEyeSwitchClock(0.0),
  dFrameDelayClock(0.0),
  bRunning(0),
  bHolding(0),
  bIsDirty(0),
#ifdef SAM
  iSamMode(0),
#endif
  iNextBlock(0),
  iBlock(1)       /*, iHighlightCtr(0)*/  
{
  fRGB[0] = fRGB[1] = fRGB[2] = 0.0;
  for (int i=0; i<MAX_PROPS; i++) fProps[i] = 0.0;
  pSubjectName = new StringEntryItemCL("Subject Name: ", 14);

  setProperty(PROP_VIEW_BLOCK, 3);
  setProperty(PROP_BLOCK_PERIOD, 30);
  setProperty(PROP_RAMP_PERIOD, 0.0);
  setProperty(PROP_EYE_SWITCH_PERIOD, 0.0);
  setProperty(PROP_DUMMY_TIME, 0.0);
  setProperty(PROP_INTERBLOCK_DELAY, 0.0);
  setProperty(PROP_NUM_CYCLES, 4);
  setProperty(PROP_RED, 0);
  setProperty(PROP_GREEN, 0);
  setProperty(PROP_BLUE, 0);
  setProperty(PROP_BLOCK_ORDER, 1);
  setProperty(PROP_BLOCK_1, 1);
  setProperty(PROP_BLOCK_2, 1);
  setProperty(PROP_BLOCK_3, 0);
  setProperty(PROP_BLOCK_4, 0);
  setProperty(PROP_BLOCK_5, 0);
  setProperty(PROP_BLOCK_6, 0);
  setProperty(PROP_BLOCK_7, 0);
  setProperty(PROP_BLOCK_8, 0);
}



//
// saveScene checks to see if need to prompt user to overwrite an existing file
//
void ShapeMgrCL::saveScene(const string &sFN)
{
  if (vShapes.size() == 0)
  {
    MenuCL::currentMenu(MessageMenuCL::menu("Nothing to save"));
    return;
  }

  if (sFN != sFilename)
  {
    ifstream checkfile(sFN.c_str(), ios::in);
    if (checkfile)  // if file exists, and if this scene was not saved before with this name, make sure ok
    {
      MenuCL::currentMenu(OK2OverwriteMenuCL::inst(sFN));
      return;
    }
  }
  MenuCL::currentMenu(0L);
  saveSceneOverwrite(sFN);
}

//
//  saveSceneOverwrite assumes it is ok to write to the specified file (an existing file is overwritten)
//
void ShapeMgrCL::saveSceneOverwrite(const string &sFN)
{
  int i;
  ofstream outf(sFN.c_str(), ios::out|ios::trunc);
  if (!outf)
  {
    MenuCL::currentMenu(MessageMenuCL::menu("ERROR: Could not open output file."));
    return;
  }

  outf << ID_SHAPE_MGR << " " << lastPropertyIdx() << std::endl;
  for (i=0; i<=lastPropertyIdx(); i++)
    outf << prop(i) << std::endl;
  
  for (vector<ShapeCL*>::iterator it=vShapes.begin(); it!=vShapes.end(); it++)
  {
    outf << (*it)->id() << " " << (*it)->lastPropertyIdx() << std::endl;
    for (i=0; i<=(*it)->lastPropertyIdx(); i++)
    {
      outf << (*it)->prop(i) << std::endl;
    }
  }
  outf.close();
  sFilename = sFN;
  dirty(0);
}

void ShapeMgrCL::manageShape(ShapeCL *s)
{
  if (s == 0L)
  {
    std::cerr << "Error: attempt to manage null shape (shapemgrcl::manageshape)\n";
    return;
  }
  vShapes.push_back(s);
  std::sort(vShapes.begin(), vShapes.end(), ShapeCompFunctor());
  dirty(1);
}

void ShapeMgrCL::deleteShape(ShapeCL *s)
{
  // BUG? should iterate in reverse instead because
  // deleting something in the middle?  Maybe ok because
  // exiting right after deleting.
  for (vector<ShapeCL*>::iterator it=vShapes.begin();
       it!=vShapes.end(); it++)
    if ((*it) == s)
    {
      vShapes.erase(it);
      delete s;
      return;
    }
}

void ShapeMgrCL::deleteAll()
{
  for (vector<ShapeCL*>::iterator it=vShapes.begin();
       it!=vShapes.end(); it++)
    delete (*it);
  vShapes.clear();
  dirty(0);
  sFilename = "";
}

void ShapeMgrCL::synchronizeAll()
{
  for (vector<ShapeCL*>::iterator it=vShapes.begin();
       it!=vShapes.end(); it++)
    (*it)->synchronize();
}

void ShapeMgrCL::swapBlocks()
{
  for (vector<ShapeCL*>::iterator it=vShapes.begin();
       it!=vShapes.end(); it++)
  {
    if ((*it)->prop(ShapeCL::PROP_BLOCK) == 1)
      (*it)->setProperty(ShapeCL::PROP_BLOCK, 2);
    else if ((*it)->prop(ShapeCL::PROP_BLOCK) == 2)
      (*it)->setProperty(ShapeCL::PROP_BLOCK, 1);
  }
}

void ShapeMgrCL::swapEyes()
{
  for (vector<ShapeCL*>::iterator it=vShapes.begin();
       it!=vShapes.end(); it++)
  {
    if ((*it)->prop(ShapeCL::PROP_EYE) == 1)
      (*it)->setProperty(ShapeCL::PROP_EYE, 2);
    else if ((*it)->prop(ShapeCL::PROP_EYE) == 2)
      (*it)->setProperty(ShapeCL::PROP_EYE, 1);
  }
}

ShapeCL *ShapeMgrCL::duplicateShape(ShapeCL *s)
{
  ShapeCL *s2 = s->duplicate();
  s2->initTitle();
  manageShape(s2);
  return s2;
}

//
// bAdding in openFile: 0==replacing scene, 1==adding scene, 2==replacing and preparing to run
//
void ShapeMgrCL::openFile(const string &sFN, int bAdding)
{
  ifstream in(sFN.c_str());
  if (!in) return;

  if (bAdding != 1)  // if not adding, then replacing or replacing and preparing to run
    deleteAll();

  int id, num;
  double val;
  while (!in.eof())
  {
    id = 0;
    in >> id >> num;

    if (id == ID_SHAPE_MGR)
    {
      for (int i=0; i<=num; i++)
      {
        in >> val;
        setProperty(i, val);
      }
      continue;  // while (!in.eof())
    }

    ShapeCL *pS = 0L;
    if (id != 0)
      pS = createShape(id);
    if (!pS) 
      if (in.eof())
        break;
      else
        return;
    for (int i=0; i<=num; i++)
    {
      in >> val;
      pS->setProperty(i, val);
    }
  }
  dirty(0);
  sFilename = sFN;
}

ShapeCL *ShapeMgrCL::createShape(int id)
{
  /*
  switch (id)
  {
    case ID_SCREW:
    {
      GLScrewInterfaceCL *p = new GLScrewInterfaceCL();
      manageShape(p);
      return p;
    }
    case ID_FIX_CROSS:
    {
      FixShapeCL *p = new FixShapeCL();
      manageShape(p);
      return p;
    }
    case ID_QUARTETS:
    {
      QuartetShapeCL *p = new QuartetShapeCL();
      manageShape(p);
      return p;
    }
    case ID_ANNULUS:
    {
      AnnulusShapeCL *p = new AnnulusShapeCL();
      manageShape(p);
      return p;
    }
    case ID_RANDOM_DOTS:
    {
      RandomDotsShapeCL *p = new RandomDotsShapeCL();
      manageShape(p);
      return p;
    }
    case ID_POLAR_CHECKERBOARD:
    {
      PolarCheckerboardShapeCL *p = new PolarCheckerboardShapeCL();
      manageShape(p);
      return p;
    }
  }
  */
  ShapeCL *p = ShapeInterface::createShapeWithId(id);
  if (p)
    manageShape(p);
  return p;
}

void ShapeMgrCL::updateAll(double dt)
{
  dFrameDelayClock += dt;
  if (dFrameDelayClock > prop(PROP_MIN_FRAME_DELAY))
  {
    dt = dFrameDelayClock;
    dFrameDelayClock = 0.0;
  }
  else
    return;
#ifdef SAM
  if (bVerbose)
    cerr << "(twsgl0=" << tswgl[0] << ",blockselect=" << blockselect << ",iSamMode=" << iSamMode << ")\n";

  for (vector<ShapeCL*>::iterator it=vShapes.begin();
       it!=vShapes.end(); it++)
    (*it)->updateAt(dt);
  return;

  //if (iSamMode && ((unsigned int)tswgl[0])>1 && bRunning==0)
  //{
  //  ShapeMgrCL::inst()->startExperiment(0);
  //}
  //if (iSamMode && ((unsigned int)tswgl[0])<=1 && bRunning==1)
  //{
  //  ShapeMgrCL::inst()->prepareToStartExperiment();    
  //}
#endif

  if (bRunning)
  {
    if (dDummyScanClock > 0.0)   // dummy scan clock is also used to insert blanks between blocks
    {
      dDummyScanClock -= dt;
      if (dDummyScanClock <= 0.0)
        iBlock = iNextBlock;
      if (iBlock > 0)
        DataCL::record(iBlock*10);

      //
      //  new: if waiting for dummies or inter-block delay, keep updating
      //  stimuli because some animated stimuli are sometimes used in the delays
      //
      for (vector<ShapeCL*>::iterator it=vShapes.begin();
        it!=vShapes.end(); it++)
      (*it)->updateAt(dt);

      return;
    }

    if (prop(PROP_EYE_SWITCH_PERIOD) > 0.0)
    {
      dEyeSwitchClock -= dt;
      if (dEyeSwitchClock < 0.0)
      {
        dEyeSwitchClock += prop(PROP_EYE_SWITCH_PERIOD);
        swapEyes();  // swaps all eye-based stimuli between eyes
      }
    }

    dExperimentClock -= dt;
    if (dExperimentClock < 0.0)
    {
      bHolding = 0;
      bRunning = 0;
      iBlock = 0;
      DataCL::record(0);
      DataCL::endFile();
    }

    if (dRampTime > 0.0)
      dRampTime += dt;
    else if (dRampTime < 0.0)
      dRampTime -= dt;

    dTime += dt;
    double dOver = dTime - prop(PROP_BLOCK_PERIOD);
    //
    //  If end of block, choose next one
    //
    if (dOver > 0.0)
    {
      iBlock = getNextBlockNo();
      if (prop(PROP_RAMP_PERIOD) > 0.0)
        dRampTime = dOver;
      //if (iBlock == 1) 
      //{
      //  iBlock = 2; 
      //  if (prop(PROP_RAMP_PERIOD) > 0.0)
      //    dRampTime = dOver;
      //}
      //else if (iBlock == 2) 
      //{
      //  iBlock = 1;
      //  if (prop(PROP_RAMP_PERIOD) > 0.0)
      //    dRampTime = dOver;  // not times -1
      //}

      if (prop(PROP_INTERBLOCK_DELAY) > 0.0)
      {
        dDummyScanClock = prop(PROP_INTERBLOCK_DELAY);
        iNextBlock = iBlock;
        iBlock = 0;
      }

      if (iBlock > 0)
        DataCL::record(iBlock*10);
      dTime = dOver; 
    }

    // check if ramp time > ramp period here in case ramp time reset by end of block
    if (!bRunning || fabs(dRampTime) > prop(PROP_RAMP_PERIOD))
    {
      dRampTime = 0.0;
    }
  }

  for (vector<ShapeCL*>::iterator it=vShapes.begin();
       it!=vShapes.end(); it++)
    (*it)->updateAt(dt);
}


void ShapeMgrCL::abortExperiment()
{
#ifdef SAM
  iSamMode = 0;
#endif
  bHolding = 0;
  bRunning = 0;
  DataCL::abort();
}

int ShapeMgrCL::prepareToStartExperiment()
{
  if (vShapes.size() == 0)
  {
    MenuCL::currentMenu(MessageMenuCL::menu("Nothing to run"));
    return 1;
  }
  bHolding = 1;
  bRunning = 0;  // (should already be false)
  iBlock = 0;
  return 0;
}

double ShapeMgrCL::initializeExperimentClock()
{
  // TODO: experiment clock needs to be aware that there might be more than two blocks
  dExperimentClock = prop(PROP_NUM_CYCLES) * 2 * prop(PROP_BLOCK_PERIOD); // countdown clock for experiment
  return dExperimentClock;
}

void ShapeMgrCL::startExperiment(int bData)
{
  //iBlock = 1;
  iBlock = getFirstBlockNo();
  bHolding = 0;
  bRunning = 1;
  synchronizeAll();

  if (bData)
  {
    string prefix = sFilename;
    if (isDirty()) prefix += string("CHANGES");
    prefix += (string("_") + getSubjectName());
    if (DataCL::startFile(prefix))  // returns non-zero on error
       MenuCL::currentMenu(MessageMenuCL::menu("Warning: could not write to data file."));
    if (iBlock > 0)
       DataCL::record(iBlock*10);
  }
  else
    DataCL::recordingOff();

  initializeExperimentClock();  // this sets dExperimentClock

  dTime = 0.0;
  dRampTime = 0.0;
  dEyeSwitchClock = prop(PROP_EYE_SWITCH_PERIOD);
  dDummyScanClock = prop(PROP_DUMMY_TIME);
  dFrameDelayClock = 0.0;
  if (dDummyScanClock > 0.0)
  {
    iNextBlock = iBlock;
    iBlock = 0;
  }
}

void ShapeMgrCL::renderAll()
{
  if (bHolding && !bSam)
    return;
  //
  //  determine if one stimulus is 'highlighted' and so should be the only one displayed
  //
  int bOneOnly = 0;
  for (vector<ShapeCL*>::iterator it=vShapes.begin();
       it!=vShapes.end() && !bOneOnly; it++)
    if ((*it)->isHighlighted())
      bOneOnly = 1;  
  //
  //  when running use iBlock, otherwise use PROP_VIEW_BLOCK parameter
  //
  unsigned int block = iBlock;
  if (!bRunning)
    block = (unsigned int)prop(PROP_VIEW_BLOCK);
  //
  //  render all stimuli that should be on this block
  //
  if (!bSam)
  {
    if ((int)prop(PROP_ANAGLYPH_MODE) == 0)
    {
      renderAllEye(3, bOneOnly, block);
    }
    else
    {
      renderAllEye(1, bOneOnly, block);
      renderAllEye(2, bOneOnly, block);
    }
  }
  else  // if in Sam mode then need to draw scene for each eye
  {
#ifdef SAM
    // first update fix cross positions:
    updateFixPos();

    if (!iSamMode || (iSamMode && tswgl[0] > 1))
    {
      if (iSamMode)
        block = (int)pow(2.0, blockselect);

      glPushMatrix();
      glTranslated(gxpos, gypos, 0);

      glViewport(0, 0, -1 + iWinSize[0]/2, iWinSize[1]);
      renderAllEye(1, bOneOnly, block);
      glViewport(iWinSize[0]/2, 0, iWinSize[0]/2, iWinSize[1]);
      renderAllEye(2, bOneOnly, block);
      glViewport(0, 0, iWinSize[0], iWinSize[1]);

      glPopMatrix();
    }
#else
      glViewport(0, 0, -1 + iWinSize[0]/2, iWinSize[1]);
      renderAllEye(1, bOneOnly, block);
      glViewport(iWinSize[0]/2, 0, iWinSize[0]/2, iWinSize[1]);
      renderAllEye(2, bOneOnly, block);
      glViewport(0, 0, iWinSize[0], iWinSize[1]);
#endif
  }
}

//
// 1==left, 2==right, 3==both
//
void ShapeMgrCL::renderAllEye(int eye, int onlyone, unsigned int block)
{
  //
  //  if anaglyph mode
  //
  if (eye==1 && (int)prop(PROP_ANAGLYPH_MODE) > 0 && (int)prop(PROP_ANAGLYPH_MODE) < 3 )
      glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE); //red
  else if (eye==2)
  {
    glClear(GL_DEPTH_BUFFER_BIT);  // maybe this should only be done if sure we are in anaglyph mode
    if ((int)prop(PROP_ANAGLYPH_MODE) == 1)
      glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE); //green
    else if ((int)prop(PROP_ANAGLYPH_MODE) == 2)
      glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_TRUE); //blue
	else if ((int)prop(PROP_ANAGLYPH_MODE) == 3)
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE); //black
  }
  //
  //  render everything for this eye and this block
  //
  double rampState = 0.0;
  if (prop(PROP_RAMP_PERIOD) > 0.0)
    rampState = dRampTime / prop(PROP_RAMP_PERIOD);  // [-1..1]
  int ramping = (rampState != 0.0);
  for (vector<ShapeCL*>::iterator it=vShapes.begin();
       it!=vShapes.end(); it++)
  {
    if (onlyone && !(*it)->isHighlighted())
      continue;
    if (!onlyone && !((int)(*it)->prop(ShapeCL::PROP_BLOCK) & block) && !(int)(*it)->prop(ShapeCL::PROP_ALWAYS_ON))
      continue;

    if ((int)(*it)->prop(ShapeCL::PROP_EYE) & eye)  // if this shape should be drawn for this eye
    {
      // insert check for stimulus on/off ramping here
      // int blend = (ramping && (int)(*it)->prop(ShapeCL::PROP_RAMPED)==1);

      // blend==1 means ramp up (to visible, i.e. opaque), ==2 means ramp down (to invisible, i.e. transparent)
      int blend = (int)(*it)->prop(ShapeCL::PROP_RAMPED);
      
      // (I don't understand anymore how rampState (below) could be negative.  May be able to remove 
      // the check for rampState>0.0 if this is true upon further inspection.)
      if (ramping)
      {
        if (blend == 1)
          (*it)->tempSetOpacity((rampState > 0.0) ? rampState : (rampState+1));
        else if (blend == 2)
          (*it)->tempSetOpacity(1.0 - rampState);
      }

      if ((*it)->opacity() < 1.0)
      {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }

      // stencil check: n/a, write to buffer, or restrict drawing ?
      switch ((int)(*it)->prop(ShapeCL::PROP_STENCIL))
      {
	// optional: glPushAttrib(GL_ALL_ATTRIB_BITS); paired with glPopAttrib(); when done
        case 1: // write (1s) to stencil buffer 
          glClearStencil(0);
          glClear(GL_STENCIL_BUFFER_BIT);
          glEnable(GL_STENCIL_TEST);
          glStencilFunc(GL_ALWAYS, 1, 0xFFFF);
          glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	  break;
        case 2: // only write where stencil buffer set to 1
          glEnable(GL_STENCIL_TEST);
          glStencilFunc(GL_EQUAL, 1, 0xFFFF);
          glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	  break;
        case 3: // only write where stencil buffer still 0
          glEnable(GL_STENCIL_TEST);
          glStencilFunc(GL_NOTEQUAL, 1, 0xFFFF);
          glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	  break;
      }

      glPushMatrix();
      (*it)->render(eye);
      glPopMatrix();

      glDisable(GL_STENCIL_TEST); // always clear in case turned on above

      glDisable(GL_BLEND);
      if (ramping && blend)
        (*it)->restoreOpacity();
    }
  }

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

MenuCL *ShapeMgrCL::getDuplicateShapeMenu()
{
  if (!opDuplicateShapeMenu)
    opDuplicateShapeMenu = new MenuCL("Clone");
  opDuplicateShapeMenu->clear();
  for (vector<ShapeCL*>::iterator it=vShapes.begin();
       it!=vShapes.end(); it++)
    opDuplicateShapeMenu->appendItem(new DupShapeSubMenuItemCL((*it)));
  return opDuplicateShapeMenu;
}

MenuCL *ShapeMgrCL::getEditShapeMenu(int showall)
{
  if (!opEditShapeMenu)
    opEditShapeMenu = new MenuCL("Edit");

  opEditShapeMenu->clear();
  for (vector<ShapeCL*>::iterator it=vShapes.begin(); 
       it!=vShapes.end(); it++)
    // limit shapes in edit menu to ones currently ON
    if (showall || (((unsigned int)(*it)->prop(ShapeCL::PROP_BLOCK) & (unsigned int)ShapeMgrCL::inst()->prop(ShapeMgrCL::PROP_VIEW_BLOCK)) != 0))
      opEditShapeMenu->appendItem(new ShapeSubMenuItemCL((*it)));

  return opEditShapeMenu;
}

MenuCL *ShapeMgrCL::getBlockDesignMenu()
{
  if (!opBlockDesignMenu)
  {
    opBlockDesignMenu = new MenuCL("Block Design");
    opBlockDesignMenu->appendItem(new SeparatorItemCL());
    opBlockDesignMenu->appendItem(new ShapeMgrPropertyItemCL("1) Blocks Consecutive or 2) Randomly Chosen (with replacement)", ShapeMgrCL::PROP_BLOCK_ORDER, 1));
    opBlockDesignMenu->appendItem(new SeparatorItemCL());
    opBlockDesignMenu->appendItem(new SeparatorItemCL("If Blocks Consecutive, turn blocks on (>0) or off (==0) below:"));
    opBlockDesignMenu->appendItem(new SeparatorItemCL("If Blocks Random, describe relative frequencies of each block below:"));
    opBlockDesignMenu->appendItem(new ShapeMgrPropertyItemCL("Block 1", ShapeMgrCL::PROP_BLOCK_1, 1));
    opBlockDesignMenu->appendItem(new ShapeMgrPropertyItemCL("Block 2", ShapeMgrCL::PROP_BLOCK_2, 1));
    opBlockDesignMenu->appendItem(new ShapeMgrPropertyItemCL("Block 3", ShapeMgrCL::PROP_BLOCK_3, 1));
    opBlockDesignMenu->appendItem(new ShapeMgrPropertyItemCL("Block 4", ShapeMgrCL::PROP_BLOCK_4, 1));
    opBlockDesignMenu->appendItem(new ShapeMgrPropertyItemCL("Block 5", ShapeMgrCL::PROP_BLOCK_5, 1));
    opBlockDesignMenu->appendItem(new ShapeMgrPropertyItemCL("Block 6", ShapeMgrCL::PROP_BLOCK_6, 1));
    opBlockDesignMenu->appendItem(new ShapeMgrPropertyItemCL("Block 7", ShapeMgrCL::PROP_BLOCK_7, 1));
    opBlockDesignMenu->appendItem(new ShapeMgrPropertyItemCL("Block 8", ShapeMgrCL::PROP_BLOCK_8, 1));
  }

  return opBlockDesignMenu;
}

MenuCL *ShapeMgrCL::getDeleteShapeMenu()
{
  if (!opDeleteShapeMenu)
    opDeleteShapeMenu = new MenuCL("Delete");

  opDeleteShapeMenu->clear();
  for (vector<ShapeCL*>::iterator it=vShapes.begin();
       it!=vShapes.end(); it++)
    opDeleteShapeMenu->appendItem(new ShapeDelMenuItemCL((*it)));

  return opDeleteShapeMenu;
}

DelConfirmMenuCL *ShapeMgrCL::getDeleteShapeConfirmMenu()
{
  if (!opDeleteShapeConfirmMenu)
    opDeleteShapeConfirmMenu = new DelConfirmMenuCL();
  return opDeleteShapeConfirmMenu;
}

ShapeMgrCL *ShapeMgrCL::pInstance = 0L;

ShapeMgrCL *ShapeMgrCL::inst()
{
  if (!pInstance)
    pInstance = new ShapeMgrCL();
  return pInstance;
}

ShapeMgrCL *opShapes;

//-------------------------------------------------------------------------
//
//  ***  key_pressed, key_special, idle  ***
//
//-------------------------------------------------------------------------

void key_pressed(unsigned char key, int x, int y)
{
  MenuCL::glut_key(key);
}

void key_special(int key, int x, int y)
{
  MenuCL::glut_key_special(key);
}

void idle()
{
  glutPostRedisplay();
}

//-------------------------------------------------------------------------
//
//  ***  redraw  ***
//
//-------------------------------------------------------------------------

void redraw()
{
  // update animation only when ready to draw because idle is called too much
  unsigned long tnow = getTime();
  double dt = (tnow - ulPrevTime) / 1000000.0;
  ulPrevTime = tnow;

  opShapes->updateAll(dt);

  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  opShapes->renderAll();

  glDisable(GL_DEPTH_TEST);
  if (bSam)
  {
    MenuCL::renderCurrentMenu(-iWinSize[0]/2+45, 400);
    MenuCL::renderCurrentMenu(45, 400);
  }
  else
    MenuCL::renderCurrentMenu(-600, 400);

  glutSwapBuffers();
}

static void mouse(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN)
    DataCL::record(button+1);  // assuming that GLUT_LEFT_BUTTON starts at zero
}

static void reshape(int w, int h)
{
  iWinSize[0] = w;
  iWinSize[1] = h;
  glViewport(0,0,w,h);
}

//==============================================================
//
//  setupvars()
//  -  when running as slave to Sam's program, need to map shared
//     memory.  code extracted from gl.c (Sam's gl.c not OpenGL)
//
//==============================================================

#ifdef SAM

#include "clkisr.h"
#include <sys/mman.h>
#include <asm/page.h>
#include <sys/ioctl.h> // ioctl
#include <sys/types.h> // open
#include <sys/stat.h>  // open
#include <fcntl.h>     // open

int setupvars()
{
  extern char pagestart3[4096];
  extern char pageend3[4096];
  extern char pagetest3;
  static struct vmmapping mymap;

  int i;
  int clkisrfd;
  char *pagepointer;

  if ( (clkisrfd = open("/dev/clkisr", O_RDWR, 644)) < 0 )
  {
    cerr << "FATAL: setupvars: cannot open /dev/clkisr\n";
    exit(0);
  }

  for ( pagepointer = pagestart3, i = 0; pagepointer < pageend3; pagepointer += PAGE_SIZE, i++ )
  {
    mymap.address = (unsigned long)pagepointer;
    mymap.page = i;
    mymap.file = 2;
    ioctl(clkisrfd, CLKISR_FUNCTION_GLOBAL_VARS, &mymap);
  }

  ioctl(clkisrfd, CLKISR_FUNCTION_TEST, &mymap);

  if ( pagetest3 != 65 )
  {
    cerr << "FATAL: setupvars: pagetest3 != 65 but " << pagetest3 << " instead.\n";
    close(clkisrfd);
    exit(0);
  }

  close(clkisrfd);

  return 1;
}

#endif // ifdef SAM

//-------------------------------------------------------------------------
//
//  ***  main  ***
//
//-------------------------------------------------------------------------

int bLoadAndRun = 0;

int main(int argc, char **argv)
{
#ifdef SAM
  setupvars();
  bSam = 1;
#endif

  // initialization:
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);

  
  
  #ifdef FULLSCREEN
	glutEnterGameMode();  
  #else
	glutInitWindowSize(1024, 768);
	glutCreateWindow("rotating screws"); 
  #endif
  glutSetCursor(GLUT_CURSOR_NONE);
  

  // callbacks:
  glutDisplayFunc(redraw);
  glutKeyboardFunc(key_pressed);
  glutSpecialFunc(key_special);
  glutIdleFunc(idle);
  glutMouseFunc(mouse);
  glutReshapeFunc(reshape);

  //
  //  Make sure the gl context is defined (by glutInit calls above) before calling ShapeMgr::inst()
  //  for the first time, because glClearColor is called within the shapemgrcl ctor, which fails
  //  on mac w/o gl initialized first
  //
  opShapes = ShapeMgrCL::inst();
  new ShapeDefCL<GLScrewInterfaceCL>(ID_SCREW, "Screwahedron");  // don't use ScrewShapeCL
  new ShapeDefCL<RandomDotsShapeCL>(ID_RANDOM_DOTS, "Random Dots");
  new ShapeDefCL<AnnulusShapeCL>(ID_ANNULUS, "Annulus");
  new ShapeDefCL<PolarCheckerboardShapeCL>(ID_POLAR_CHECKERBOARD, "Polar Checkerboard");
  new ShapeDefCL<QuartetShapeCL>(ID_QUARTETS, "Quartets");
  new ShapeDefCL<RectShapeCL>(ID_RECT, "Rectangle");
  new ShapeDefCL<FixShapeCL>(ID_FIX_CROSS, "Fixation Cross");

  for (int i=1; i<argc; i++)
  {
    string sArg = argv[i];
    if (sArg == "-sam")
      bSam = 1;
    else if (sArg == "-v")
      bVerbose = 1;
    else if (sArg == "-load" || sArg == "-run")
    {
      i++;
      if (i >= argc) 
      {
	std::cerr << "Warning: specified -load on cmd line, but did not specify a file\n";
      }
      else
      {
        if (sArg == "-run")
          bLoadAndRun = 1;
        sArg = argv[i];
        ShapeMgrCL::inst()->openFile(sArg);
        ShapeMgrCL::inst()->synchronizeAll();  // maybe openFile should always do this?
      }
    }
#ifdef LINUX
    if (sArg[0] != '-')
      chdir(argv[1]);
#endif
  }

#ifdef LINUX
  setenv("__GL_SYNC_TO_VBLANK", "1", 1);
  //if (argc > 1 && argv[1][0] != '-') 
  //{
  //  chdir(argv[1]);
  //}
#endif

#ifndef LINUX
  //srand( (unsigned)time( NULL ) );
#endif

  // orthographic camera:
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //if (bSam)
  //  glOrtho(-1280, 1280, -512, 512, -1000, 1000);
  //else
    glOrtho(-640, 640, -512, 512, -1000, 1000);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // background color:
  glClearColor(0.0, 0.0, 0.0, 0.0);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glFrontFace(GL_CCW);
  float pfLightPos[4] = {0.0, 0.0, 1000.0, 0.0};
  glLightfv(GL_LIGHT0, GL_POSITION, pfLightPos);

  ulPrevTime = getTime();  // initialize start time

  MenuCL *pNewShapeMenu = new MenuCL("New");
  pNewShapeMenu->appendItem(new NewShapeMenuItemCL(ID_SCREW));
  pNewShapeMenu->appendItem(new NewShapeMenuItemCL(ID_RANDOM_DOTS));
  pNewShapeMenu->appendItem(new NewShapeMenuItemCL(ID_ANNULUS));
  pNewShapeMenu->appendItem(new NewShapeMenuItemCL(ID_POLAR_CHECKERBOARD));
  pNewShapeMenu->appendItem(new NewShapeMenuItemCL(ID_QUARTETS));
  pNewShapeMenu->appendItem(new NewShapeMenuItemCL(ID_RECT));
  pNewShapeMenu->appendItem(new NewShapeMenuItemCL(ID_FIX_CROSS));

  MenuCL *menu = new MainMenuCL();
  menu->appendItem(new SubMenuItemCL("New Shape",  pNewShapeMenu));
  menu->appendItem(new DuplicateShapeMenuItemCL());
  menu->appendItem(new EditShapeMenuItemCL());
  menu->appendItem(new EditAllShapesMenuItemCL());
  menu->appendItem(new DeleteShapeMenuItemCL());
  menu->appendItem(new SeparatorItemCL());
  menu->appendItem(new ClearMenuItemCL());
  menu->appendItem(new SubMenuItemCL("Save Scene", SaveMenuCL::inst()));
  menu->appendItem(new SubMenuItemCL("Open Scene", new OpenMenuCL()));
  menu->appendItem(new SubMenuItemCL("Add Scene", new OpenMenuCL(1)));
  menu->appendItem(new SeparatorItemCL());

  menu->appendItem(new TotalExpTimeItemCL());
  ShapeMgrPropertyItemCL *pitem = new ShapeMgrPropertyItemCL("View Block 1=A 2=B etc ", ShapeMgrCL::PROP_VIEW_BLOCK, 1);
  pitem->setDisplayType(DISP_BINARY8);
  menu->appendItem(pitem);
  menu->appendItem(new ShapeMgrPropertyItemCL("Block Period (s)", ShapeMgrCL::PROP_BLOCK_PERIOD, 2.0));
  menu->appendItem(new ShapeMgrPropertyItemCL("Number of Cycles", ShapeMgrCL::PROP_NUM_CYCLES, 1));
  menu->appendItem(new ShapeMgrPropertyItemCL("Delay between blocks (s)", ShapeMgrCL::PROP_INTERBLOCK_DELAY, 1));
  menu->appendItem(new ShapeMgrPropertyItemCL("Delay for Dummy Scans (s)", ShapeMgrCL::PROP_DUMMY_TIME, 1));
  menu->appendItem(new ActionMenuItemCL<BlockDesignActionST>("Block Design...", BlockDesignActionST()));
  menu->appendItem(new ShapeMgrPropertyItemCL("Eye Exchange Period (s)", ShapeMgrCL::PROP_EYE_SWITCH_PERIOD, 0.5));
  menu->appendItem(new ShapeMgrPropertyItemCL("Ramp time (s)", ShapeMgrCL::PROP_RAMP_PERIOD, 0.025));
  menu->appendItem(new ShapeMgrPropertyItemCL("Minimum frame delay (s)", ShapeMgrCL::PROP_MIN_FRAME_DELAY, 0.002));
  menu->appendItem(new SeparatorItemCL());

  menu->appendItem(new ShapeMgrPropertyItemCL("BG Red", ShapeMgrCL::PROP_RED, 2));
  menu->appendItem(new ShapeMgrPropertyItemCL("BG Green", ShapeMgrCL::PROP_GREEN, 2));
  menu->appendItem(new ShapeMgrPropertyItemCL("BG Blue", ShapeMgrCL::PROP_BLUE, 2));
  menu->appendItem(new SeparatorItemCL());

  menu->appendItem(new ActionMenuItemCL<SwapBlocksActionST>("Swap Blocks", SwapBlocksActionST()));
  menu->appendItem(new ActionMenuItemCL<SwapEyesActionST>("Swap Eyes", SwapEyesActionST()));
  menu->appendItem(new ActionMenuItemCL<SynchronizeActionST>("Synchronize Stimuli", SynchronizeActionST()));
  menu->appendItem(new ShapeMgrPropertyItemCL("Anaglyph mode", ShapeMgrCL::PROP_ANAGLYPH_MODE, 1));
#ifndef SAM
  menu->appendItem(new ActionMenuItemCL<SplitScreenST>("Toggle split screen", SplitScreenST()));
#endif
  menu->appendItem(new SeparatorItemCL());
#ifdef SAM
  menu->appendItem(new ActionMenuItemCL<SamModeST>("Enter slave to Sam mode", SamModeST()));
#else
  menu->appendItem(ShapeMgrCL::inst()->pSubjectName); 
  //menu->appendItem(new SubMenuItemCL("Prepare to Start Experiment", StartExperimentMenuST::inst()));
  menu->appendItem(new ActionMenuItemCL<PrepareStartST>("Prepare to Start Experiment", PrepareStartST()));
  menu->appendItem(new SubMenuItemCL("Open Scene and Prepare to Start", new OpenMenuCL(2)));
#endif
  menu->appendItem(new ActionMenuItemCL<RunNowNoDataST>("Run now, without recording data", RunNowNoDataST()));
  menu->appendItem(new SeparatorItemCL());
  menu->appendItem(new QuitItemCL());
  MenuCL::pMainMenu = menu;

  if (bLoadAndRun)
  {
    RunNowNoDataST orun;
    orun();
  }

  // run:
  glutMainLoop();
  return 0;
}
