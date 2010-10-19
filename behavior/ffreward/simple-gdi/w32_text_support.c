/*****************************************************/
/* Windows 95/98/NT/2000/XP experiment C support     */
/* For use with Version 2.0 of EyeLink Windows API   */
/*                                                   */
/*      (c) 1997-2002 by SR Research Ltd.            */
/* For non-commercial use by Eyelink licencees only  */
/*                                                   */
/* THIS FILE: w32_text_support.c                     */
/* CONTENTS: Font support, Draw text to display      */
/*                                                   */
/*                                                   */
/* CHANGES FOR Windows 2000/XP, EyeLink 2.0:         */
/* - change header file to w32_exptsppt2.h           */
/* - add GdiFlush after drawing poerations           */
/*****************************************************/
 
#include <windows.h>
#include <windowsx.h>

#include "gdi_expt.h"
#include "w32_demo.h"

#include <stdio.h>    // for vsprintf() function
#include <stdarg.h>


/************* GRAPHICAL TEXT *************/

        // This code supports Windows fonts.
        // Fonts are created, then used until a new font is required.
        // By not creating a font each time we draw,
        // characters are cached by Windows and drawing is faster

HFONT current_font = NULL;  // The currently-allocated font

         // Create a new font, given face name, character height, and whether it's boldface
         // Also releases any previously created font
HFONT get_new_font(char *name, int size, int bold)
{
  LOGFONT lf;     // Logical font definition

  if(current_font) 
    DeleteObject(current_font);  // Delete any cached font
  current_font = NULL;

  memset(&lf, 0, sizeof(lf));     // Fill in the LOGFONT structure
  lf.lfHeight = -size;            // Height in pixels
  lf.lfWeight = bold ? 700 : 0;   // Boldface weight
  strcpy(lf.lfFaceName, name);    // Face name

  current_font = CreateFontIndirect(&lf);  // Create the font
  return current_font;                     // will return NULL if creation failed
}

         // release any cached font at end of program
void release_font(void)
{
  if(current_font) 
    DeleteObject(current_font);
  current_font = NULL;
}

    
        // Print text using cached font
        // Accepts formatting just like printf()
        // Font is drawn in <fg> color
        // Background is cleared to <bg> color (-1 to NOT clear background)
        // Top-left of text is located at <x>,<y> 
        // If <center> is nonzero, text is centered on <x>,<y> 
void graphic_printf(COLORREF fg, COLORREF bg, int center, int x, int y, char *fmt, ...)
{
  va_list argptr;
  unsigned char msgbuf[200];
  SIZE sze;
  RECT r;
  HDC hdc;

  va_start(argptr, fmt);          // Create the text string, similar to printf()
  vsprintf((char *) msgbuf, fmt, argptr);
  va_end(argptr);

  if(!full_screen_window) return;                    // make sure window still exists
  hdc = GetDC(full_screen_window);                   // get drawing context
  if(current_font) SelectObject(hdc, current_font);  // set current font
  
  SetBkMode(hdc, ((INT32)bg)==-1 ? TRANSPARENT : OPAQUE);     // set background color and mode
  if(((INT32)bg) != -1) SetBkColor(hdc, bg | 0x02000000L);    
  SetTextColor(hdc, fg | 0x02000000L);                        // Set foreground text color
  GetTextExtentPoint(hdc, msgbuf, strlen(msgbuf), &sze);      // Size of text
  if(!center)
    {
      r.top = y;                 // compute rectangle if not centered
      r.bottom = y + sze.cy;
      r.left = x;
      r.right = x + sze.cx;
    }
  else
    {
      r.top = y - sze.cy/2;      // compute rectangle if centered
      r.bottom = r.top + sze.cy;
      r.left = x - sze.cx/2;
      r.right = r.left + sze.cx;
    }

  DrawText(hdc, msgbuf, -1, &r, DT_LEFT|DT_TOP);   // Draw the text

  GdiFlush();   // ADDED FOR Windows 2000/XP: forces drawing to be done immediately

  SelectObject(hdc, GetStockObject(SYSTEM_FONT));  // deselect the font
  ReleaseDC(full_screen_window, hdc);              // release drawing context
}

