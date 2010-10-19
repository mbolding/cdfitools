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

#include "shape.h"

ShapeCL::~ShapeCL() 
{ 
  if (pMenu) delete pMenu; 
}

void ShapeCL::updateAt(double seconds) 
{
    if (prop(PROP_BLINK_FREQ) > 0.0)
    {
      dBlinkClock += seconds;
      double period = 1.0/prop(PROP_BLINK_FREQ);
      if (dBlinkClock > period)
      {
        dBlinkClock -= period;
      }
      bBlinkOn = 1;
      if (dBlinkClock > period*prop(PROP_BLINK_DUTY_CYCLE))
        bBlinkOn = 0;
    }
    if (prop(PROP_RAMP_REPEAT) > 0.0)
    {
      dRampClock += seconds;
      if (dRampClock > prop(PROP_RAMP_REPEAT))
      {
        dRampClock -= prop(PROP_RAMP_REPEAT);
      }
      if (prop(PROP_RAMP_UPORDOWN) == 0)
      {
        if (prop(PROP_RAMP_PERIOD) > 0.0 && dRampClock < prop(PROP_RAMP_PERIOD))
          setOpacity(dRampClock / prop(PROP_RAMP_PERIOD));
        else
          setOpacity(1.0);
      }
      else
      {
        if (prop(PROP_RAMP_PERIOD) > 0.0 && dRampClock < prop(PROP_RAMP_PERIOD))
          setOpacity(1.0 - dRampClock / prop(PROP_RAMP_PERIOD));
        else
          setOpacity(0.0);
      }
    }
}

void ShapeCL::copyPropertiesFrom(ShapeCL *p)
{
    for (int i=0; i<MAX_PROPS; i++)
      setProperty(i, p->prop(i));
}

void ShapeCL::update(int id)
{
    switch (id)
    {
      case PROP_STENCIL:
      {
        if (prop(id) < 0.0) setProperty(id, 0.0);
        else if (prop(id) > 3.0) setProperty(id, 3.0);
        break;
      }
      case PROP_BLINK_FREQ:
      {
        if (prop(id) < 0.0)
	{
          setProperty(id, 0.0);
          bBlinkOn = 1;
          dBlinkClock = 0.0;
	}
	break;
      }
      // numbers between 0.0 and 1.0:
      case PROP_BLINK_DUTY_CYCLE:
      case PROP_BLINK_INITIAL_PHASE:
      {
        if (prop(id) < 0.0)
          setProperty(id, 0.0);
        else if (prop(id) > 1.0)
          setProperty(id, 1.0);
        break;
      }
      // properties that must be >= 0.0
      case PROP_RAMP_REPEAT:
      case PROP_RAMP_PERIOD:
      {
        if (prop(id) < 0.0)
          setProperty(id, 0.0);
	break;        
      }
      // allow least sig 8 bits to be set, bit n [1..8] on 
      // means turn stimulus on for block n
      case PROP_BLOCK:
      {
	if (prop(id) < 0)
          setProperty(id, 0);
        else if (prop(id) > 255)
          setProperty(id, 255);
        break;
      }
      // properties that should be integers in range [1..3]:
      case PROP_EYE:
      {
        if (prop(id) < 1)
          setProperty(id, 3);
        else if (prop(id) > 3)
          setProperty(id, 1);
        break;
      }
      // properties than can be 0 or 1 or 2
      case PROP_RAMPED:
      {
        if (prop(id) < 0)
          setProperty(id, 2);
        else if (prop(id) > 2)
          setProperty(id, 0);
        break;
      }
      // properties that are 0 or 1 (e.g. on/off properties)
      case PROP_ALWAYS_ON:
      case PROP_RAMP_UPORDOWN:
      case PROP_COLOR_TRACK_BG:
      {
        if (prop(id) < 0)
          setProperty(id, 1);
        else if (prop(id) > 1) 
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
          fRGBA[i-PROP_RED] = prop(i) / 255.0;
        }
        break;
      }

    }
}



ShapeCL::ShapeCL() : bHighlight(0), pMenu(0L), iOrder(0), fSaveA(0.0), bBlinkOn(1), dBlinkClock(0.0), sName("[error]")
{ 
  iShapeNo = iShapeNoCtr;
  iShapeNoCtr++;

  for (int i=0; i<MAX_PROPS /*PROP_next*/; i++) fProps[i] = 0.0;

  fRGBA[0] = fRGBA[1] = fRGBA[2] = fRGBA[3] = 1.0;   
  setProperty(PROP_BLOCK, 3);
  setProperty(PROP_EYE,   3);
  setProperty(PROP_RAMPED, 0);
  setProperty(PROP_RAMP_REPEAT, 0.0);
  setProperty(PROP_RAMP_PERIOD, 0.0);
  setProperty(PROP_RAMP_UPORDOWN, 0);
  setProperty(PROP_RED, 255);
  setProperty(PROP_GREEN, 255);
  setProperty(PROP_BLUE, 255);
  setProperty(PROP_COLOR_TRACK_BG, 0);
  setProperty(PROP_X, 0);
  setProperty(PROP_Y, 0);
  setProperty(PROP_Z, 0);
  setProperty(PROP_BLINK_FREQ, 0.0);
  setProperty(PROP_BLINK_DUTY_CYCLE, 0.5);
  setProperty(PROP_BLINK_INITIAL_PHASE, 0.0);
  setProperty(PROP_ALWAYS_ON, 0);
  setProperty(PROP_STENCIL, 0);
}
