/**********************************************************/
/* Windows 95/98/NT/2000/XP experiment C support          */
/* For use with Version 2.0 of EyeLink Windows API        */
/*                                                        */
/*      (c) 1997-2002 by SR Research Ltd.                 */
/* For non-commercial use by Eyelink licencees only       */
/*                                                        */
/* THIS FILE: w32_text_bitmap.c                           */
/* CONTENTS:                                              */
/* - copy bitmaps to display                              */
/* - create new, bank bitmap                              */
/*                                                        */
/*                                                        */
/* CHANGES FOR Windows 2000/XP, EyeLink 2.0:              */
/* - change header file to w32_exptsppt2.h                */
/* - add GdiFlush after drawing poerations                */
/**********************************************************/
 
#include <windows.h>
#include <windowsx.h>

#include "eyelink.h"
#include "w32_exptsppt2.h"    // CHANGED FOR DLL VERSION 2.0
#include "w32_demo.h"


/************ DISPLAY ENTIRE BITMAP **************/

         // Copy a DDB bitmap to the display 
         // Top-left placed at (x,y)
         // Speed depends on video mode and graphics card
void display_bitmap(HWND hwnd, HBITMAP hbm, int x, int y)
{
  BITMAP bm;        // Bitmap data structure
  HDC hdc;          // Display context (to create memory context)
  HDC mdc;          // Memory context to draw bitmap in
  HBITMAP obm;      // old GDI bitmap

  hdc = GetDC(hwnd);                     // Display DC 
  mdc = CreateCompatibleDC(hdc);         // Memory DC
  obm = SelectObject(mdc, hbm);          // Select bitmap into memory DC
  
  GetObject(hbm, sizeof(BITMAP), &bm);   // Get data on bitmap
                                         // Use BitBlt to copy to display 
  BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, mdc, 0, 0, SRCCOPY);
  
  GdiFlush();   // ADDED for Windows 2000/XP: Forces drawing to be immediate

  SelectBitmap(mdc, obm);                // Deselect bitmap
  DeleteDC(mdc);                         // get rid of memory DC
  ReleaseDC(hwnd, hdc);
}


/************* DISPLAY PART OF BITMAP **********/

        // Copies a rectangular area of a bitmap to the display
        // Copies rectangle <xs1, ys1, xs2, ys2> of DDB <hbm> 
        // Places top-left corner at <xd, yd> 
        // for maximum drawing speed, xd=xs1 and yd=ys1 (aligned copy)
        // <hbm> is source bitmap, <hwnd> is the window to draw to
void display_rect_bitmap(HWND hwnd, HBITMAP hbm, int xd, int yd, 
                                                 int xs1, int ys1, int xs2, int ys2)
{
  HDC hdc;
  HDC mdc;
  HBITMAP obm;
  int h, w;

  if(!hwnd || !hbm) return;
  w = xs2 - xs1 + 1;
  h = ys2 - ys1 + 1;

  hdc = GetDC(hwnd);                // display drawing context
  mdc = CreateCompatibleDC(hdc);    // memory context
  obm = SelectObject(mdc, hbm);     // select bitmap
  
  BitBlt(hdc, xd, yd, w, h, mdc, xs1, ys1, SRCCOPY);   // copy relevant part to display

  GdiFlush();   // ADDED for Windows 2000/XP: Forces drawing to be immediate

  SelectBitmap(mdc, obm);           // release GDI resources
  DeleteDC(mdc);
  ReleaseDC(hwnd, hdc);
}


/************ CREATE BLANK BITMAP ***********/

      // Creates a blank, full-screen sized bitmap, cleared to <bgcolor>
HBITMAP blank_bitmap(COLORREF bgcolor)
{
  HDC hdc;
  HBITMAP hbm;
  HDC mdc;
  HBRUSH oBrush;
  HBITMAP obm;

  hdc = GetDC(NULL);                 
  mdc = CreateCompatibleDC(hdc);     // create display-compatible memory context
  hbm = CreateCompatibleBitmap(hdc, SCRWIDTH, SCRHEIGHT);
  obm = SelectObject(mdc, hbm);      // create DDB bitmap, select into context
                                     
  oBrush = SelectObject(mdc, CreateSolidBrush(bgcolor | 0x02000000L));  // brush to fill with 
  PatBlt(mdc, 0, 0, SCRWIDTH, SCRHEIGHT, PATCOPY);
  DeleteObject(SelectObject(mdc, oBrush));

  GdiFlush();   // ADDED for Windows 2000/XP: Forces drawing to be immediate

  SelectBitmap(mdc, obm);     // Release the GDI resources
  DeleteDC(mdc);
  ReleaseDC(NULL, hdc);
  return hbm;                 // Return the new bitmap
}

