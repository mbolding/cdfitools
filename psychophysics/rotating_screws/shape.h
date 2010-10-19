//=========================================================================
//
//  shape.cpp
//  (rotating_screws project)
//
//  contains: ShapeCL base class for all shapes
//
//  (c) 2005 Jon K. Grossmann
//           jgrosman@uab.edu
//                 
//  History
//  -------
//  2006 Feb 08 - JKG - created.
//
//=========================================================================

#ifndef __SHAPE_H__
#define __SHAPE_H__

#define MAX_PROPS 64

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

using namespace std;

//-------------------------------------------------------------------------
//
//  MenuCL - renders a list of items, one of which is the currently selected
//           item, and each of which have actions when selected
//
//-------------------------------------------------------------------------

class MenuCL
{
public:
   struct ItemCL
   {
     int iSkip;     // set to true when should skip because disabled or because is a separator
     string sText;  // text of the menu item
     int iSelected; // true when this is the currently selected item on a menu
     int bAlwaysSetLabel;  // whether this menu item should always 'setlabel' whenever anything else on the menu changes
     int iColor;

     ItemCL() : iSelected(0), iSkip(0), bAlwaysSetLabel(0), iColor(0) {}
     ItemCL(const char *sz) : sText(sz), iSelected(0), iSkip(0), bAlwaysSetLabel(0), iColor(0) {}
     ItemCL(const string &s) : sText(s), iSelected(0), iSkip(0), bAlwaysSetLabel(0), iColor(0) {}
     virtual string text()   { return sText; }
     virtual void   action() {}           // called when menu item selected (e.g. when enter key hit)
     virtual void   delta(float dir) {}   // called when left or right arrow key hit
     virtual void   focus(int f)          // called when menu gains (or loses) focus (when it is displayed)
     {
       highlight(0);
     }
     virtual void   highlight(int h)      // called when this item is highlighted (but not yet selected)
     {
       iSelected = h;
     }
     virtual void   glut_key(unsigned char key) {}
     virtual void   glut_key_special(int key)   {}
     virtual int    skip()                      { return iSkip; }
     virtual int    alwaysSetLabel()            { return bAlwaysSetLabel; }
     virtual int    alwaysSetLabel(int set)     { return (bAlwaysSetLabel=set); }
     virtual int    color()                     { return iColor; }
     virtual int    color(int set)              { return (iColor=set); }

     virtual void   setLabel() {} // this was virtualized and added here as an afterthought, after adding alwayssetlabel and noticing setlabel wasn't even here
   };

protected:
  vector<ItemCL*>   vItems;
  int               iSelected;
  MenuCL           *opParent;
  string            sTitle;

public:
  MenuCL();
  MenuCL(const char *sz);
  MenuCL(const string &s);

  void appendItem(ItemCL *item);
  void renderMenu(float x, float y);  
  void select();                     // call action for current menu item
  void delta(float dir);             // called when left or right arrow key hit
  MenuCL *parentMenu()               { return opParent; }
  virtual void focus(int f);         // called when menu displayed, calls focus() for each menu item
  void clear();                      // clear the menu (delete all menu items)
  void   setTitle(const string &s)   { sTitle = s; }
  string getTitle()                  { return sTitle; }
  virtual void callback(int msg)     {}
  void glut_key_inst(unsigned char key)
  {
    if (iSelected < vItems.size())
      vItems[iSelected]->glut_key(key);
  }
  void glut_key_special_inst(int key)
  {
    if (iSelected < vItems.size())
      vItems[iSelected]->glut_key_special(key);
  }

  void changeSelected(int dir)  // change highlighted menu option
  {
    iSelected += dir;
    if (iSelected < 0)
      iSelected = vItems.size() - 1;
    else if (iSelected >= vItems.size())
      iSelected = 0;

    //
    //  Warning: possible infinite recursion if have menu with all blank items
    //  CHANGED: now uses skip() instead of blank item to determine who to skip, still
    //  infinite recursion problem if all items are 'skipped'
    //
    if (iSelected < vItems.size())
      //if (vItems[iSelected]->text() == "")
      if (vItems[iSelected]->skip())
        changeSelected(dir!=0 ? dir : 1); // BUG FIX: if dir is zero then this recursively calls changeselected with obvious stack overflow, infinite recursion problem is all items 'skipped' still not fixed though

    for (int i=0; i<vItems.size(); i++)
      vItems[i]->highlight(i==iSelected);
  }

  //
  //  static interface
  //
protected:
  static MenuCL *pCurrentMenu;  // let whoever change the current menu
public:
  static MenuCL *pMainMenu;
  static MenuCL *currentMenu()          { return pCurrentMenu; }
  // careful: the current menu can be set to NULL:
  static MenuCL *currentMenu(MenuCL *m) 
  {
    if (pCurrentMenu)
      pCurrentMenu->focus(0);
    if (m) 
    {
      m->focus(1);          // let menu know it has focus
      m->changeSelected(0); // don't change the current item, but notify the item that is has focus
    }
    return (pCurrentMenu = m); 
  }
  static void renderCurrentMenu(int x, int y);
  static void glut_key(unsigned char key);
  static void glut_key_special(int key);
};

//-------------------------------------------------------------------------
//
//  ShapeCL - Shapes base class
//
//-------------------------------------------------------------------------
struct ShapePropertyItemCL;   // fwd decl for return type of ShapeCL::menuItem()

class ShapeCL
{
protected:
  int    iId;
  int    bHighlight;
  double fProps[MAX_PROPS]; // indexed by PROP_ enumerated labels
  float  fRGBA[4];          // rgb colors managed by each specific shape because PROP_s different for each
  int    iOrder;            // set to 0 if opaque, 1 if transparent, 2 if always on top
  float  fSaveA;
  double dBlinkClock;
  int    bBlinkOn;
  double dRampClock;
  string sName;
  int    iShapeNo;
  static int iShapeNoCtr;

  MenuCL *pMenu;

  ShapePropertyItemCL *menuItem(int id, char *sz, float val, float del);

public:
  ShapeCL();
  virtual ~ShapeCL();
  virtual void    updateAt(double seconds);
  virtual void synchronize();
  virtual void    render(int eye)       {}
  virtual MenuCL *getMenu()      { if (pMenu) pMenu->setTitle(name()); return pMenu; }
  void highlight(int yn)         { bHighlight = yn; }
  int  isHighlighted()           { return bHighlight; }
  virtual int     id()           { return 0; }
  const string   &name()         { return sName; }
  void initTitle();
  virtual void    setOpacity(float val)     { fRGBA[3] = val; }
  virtual void    tempSetOpacity(float val) { fSaveA = fRGBA[3]; setOpacity(val); }
  virtual void    restoreOpacity()          { setOpacity(fSaveA); }
  inline  float   opacity()                 { return fRGBA[3]; }

  //
  // RAMPED (boolean) when true indicates ramp this stimulus at start of each block using ShapeMgr::RAMP_PERIOD
  // RAMP_REPEAT is how often to turn off the stimulus, then ramp it back on in ShapeCL::RAMP_PERIOD seconds
  // Keep in mind that ShapeCL::RAMP_PERIOD is different from ShapeMgr::RAMP_PERIOD 
  //
  // PROP_COLOR_TRACK_BG (boolean) means always (whenever synchronized) make the shapes color
  // equal to the currrent background color
  //
  enum { PROP_BLOCK, PROP_RED, PROP_GREEN, PROP_BLUE, PROP_X, PROP_Y, PROP_Z, PROP_EYE, PROP_RAMPED, 
         PROP_BLINK_FREQ, PROP_ALWAYS_ON, PROP_RAMP_REPEAT, PROP_RAMP_PERIOD, PROP_RAMP_UPORDOWN, 
         PROP_BLINK_DUTY_CYCLE, PROP_BLINK_INITIAL_PHASE, PROP_COLOR_TRACK_BG, 
         PROP_STENCIL, PROP_next=20 };
  virtual int lastPropertyIdx() { return PROP_STENCIL; }

  // setProperty, incrementProperty, and prop are the only functions that should access fProps[] directly
  void  setProperty(int propno, double value)         { fProps[propno] = value;  update(propno); }
  void  setPropertyOnly(int propno, double value)     { fProps[propno] = value; }
  void  incrementProperty(int propno, double delta)   { fProps[propno] += delta; update(propno); }
  inline double prop(int propno) const                { return fProps[propno]; }

  inline int order() const { return iOrder; }
  virtual ShapeCL *duplicate() { return 0L; }  // should be pure virtual ?
  void copyPropertiesFrom(ShapeCL *p);

  virtual void update(int id);

  //
  //  these properties are common to all stimuli, these interface elements are therefore also
  //  common to all stimulus edit menus
  //
  void appendMenuItems();
};


#endif
