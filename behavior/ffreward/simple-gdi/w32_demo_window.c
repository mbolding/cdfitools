/********************************************************/
/* Windows 95/98/NT/2000/XP experiment C support        */
/* For use with Version 2.0 of EyeLink Windows API      */
/*                                                      */
/*      (c) 1997-2002 by SR Research Ltd.               */
/* For non-commercial use by Eyelink licencees only     */
/*                                                      */
/* THIS FILE: w32_demo_window.c                         */
/* CONTENTS:                                            */
/* - Create a full-screen window                        */ 
/* - clear the window                                   */
/* - minimum required GUI support for window            */
/* - tuned for exclusive use of the desktop             */
/*                                                      */
/*                                                      */
/* CHANGES FOR Windows 2000/XP, EyeLink 2.0:            */
/* - change header file to w32_exptsppt2.h              */
/* - add GdiFlush after drawing poerations              */
/* - ADD GdiSetBatchLimit(1) FOR IMMEDIATE DRAWING      */
/* - ADD pump_delay() where required so GUI can redraw. */
/********************************************************/
 
#include <windows.h>
#include <windowsx.h>

#include "gdi_expt.h"
#include "w32_demo.h"


HWND full_screen_window = NULL;        // handle to our window
int full_screen_window_active = 0;     // set if window is currently active

static int window_drawn = 0;           // indicates first WM_PAINT message received
static HCURSOR arrow_cursor;           // the arrow cursor

/*************** CREATE FULL-SCREEN WINDOW **********/

        // Create and draw our full-screen window 
        // This window is used for all experiment graphics
int make_full_screen_window(HINSTANCE hInstance)
{
  WNDCLASS wc;   // the window class definition
  UINT32 t;      // timer variable
  
  arrow_cursor = LoadCursor( NULL, IDC_ARROW );        // load the default cursor

                              // set up and register window class 
  wc.style = CS_HREDRAW | CS_VREDRAW ;                 // force redraw if redisplaying
  wc.lpfnWndProc = (WNDPROC)full_screen_window_proc;   // the window message processing function
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;                            // our application instance
  wc.hIcon = LoadIcon(hInstance, "eyelink_icon");      // our icon
  wc.hCursor = arrow_cursor;                           // the default cursor
  wc.hbrBackground = NULL;                             // let the message handler clear background
  wc.lpszMenuName = NULL;                              // no menu
  wc.lpszClassName = "Fullscreen";                     // the identifier for this window type

  RegisterClass(&wc);           // Create the class
                              
  full_screen_window =          // Create the window
      CreateWindowEx(WS_EX_APPWINDOW,                // We want to see it in desktop bar
                     "Fullscreen",                   // Class of window 
                     "Eyelink Demo Application",     // and name of application
                     WS_POPUP,                       // DON'T use WS_MAXIMIZE, it repaints twice!
                     0, 0,  
                     GetSystemMetrics(SM_CXSCREEN),  // Set initial size to maximize,
                     GetSystemMetrics(SM_CYSCREEN),  // otherwise repaint can trash graphics
                     NULL, NULL, hInstance, NULL );  // our application instance

  if(!full_screen_window) return -1;       // exit if failed to create

  window_drawn = 0;                                   
  ShowWindow(full_screen_window, SW_SHOW);   // draw the window
  UpdateWindow(full_screen_window);

                         // CRITICAL Windows 2000/XP CODE!!!  
  GdiSetBatchLimit(1);   // FORCES IMMEDIATE DRAWING FROM NOW ON

  t = current_time();                  // Wait for WM_PAINT to draw the window
  while(t+2000 > current_time())       // If WM_PAINT is delayed, our graphics could be erased
    {
      if(getkey() == TERMINATE_KEY) break;  // getkey() also passes messages to window
      if(window_drawn) break;               // wait for WM_PAINT to be processed
    }
  pump_delay(500);        // Windows XP seems to need  time to remove taskbar
  return 0;                            // success! 
}


          // Ensure the window is closed before exiting
void close_full_screen_window(void)
{
  release_font();                  // release any cached font
  if(full_screen_window)           // if window is still open:
    {
      close_expt_graphics();              // release graphics in DLL
      terminal_break(1);                  // break out of any loops
      DestroyWindow(full_screen_window);  // close window
      GdiFlush();                         // Win 2000/XP: force immediate drawing 
    }
  pump_delay(500);                    // Windows 2000/XP: Need to give GUI time to clean up
  full_screen_window = NULL;          // mark window as deleted
}


        // Clear the window background to a color
void clear_full_screen_window(COLORREF color)
{
  RECT rc;   
  HDC hdc;  
  HBRUSH oBrush;

  if(!full_screen_window) return;           // make sure window is still open!
  hdc = GetDC(full_screen_window);          // get drawing context
  GetWindowRect(full_screen_window, &rc);   // get window size

  oBrush = SelectObject(hdc, CreateSolidBrush(color|0x02000000L));  // brush to fill with 
  PatBlt(hdc, rc.left, rc.top, 
         rc.right-rc.left+1, rc.bottom-rc.top+1, PATCOPY);  // Fill window plus frame area

  GdiFlush();
  DeleteObject(SelectObject(hdc, oBrush));  // Clean up GDI resources
  ReleaseDC(full_screen_window, hdc);
}

       // Handles all messages for full-scren experiment window
       // Repaint is handled by simply erasing affected area: 
       // this should only happen if ALT-ESC used to switch apps 
       // or after dialog boxes (i.e. EDF file transfer) are erased.
       // NOTE: during calibration and drift correction, most messages are intercepted by EXPTSPPT
LRESULT CALLBACK full_screen_window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{                     
  PAINTSTRUCT ps;

  if(hwnd==NULL) return FALSE;  // could happen if full_screen_window already closed

  switch (message)
    {
      case WM_KEYDOWN:          // process key messages: these can be accessed by getkey()
      case WM_CHAR:
        process_key_messages(hwnd, message, wparam, lparam);   
        break;

      case WM_ACTIVATE:         // Window is activated or deactivated
        full_screen_window_active = LOWORD(wparam) != WA_INACTIVE;
        if(full_screen_window_active)
          {                     // newly active: make sure it's fully visible
            ShowWindow(full_screen_window, SW_SHOWMAXIMIZED );
            UpdateWindow(full_screen_window);
          }
        break;

      case WM_CLOSE:      // If ALT-F4 pressed, force program to close 
        PostQuitMessage(0);
        terminal_break(1);
	break;

      case WM_DESTROY:     // Window being closed by ALT-F4
         PostQuitMessage( 0 );
         full_screen_window = NULL;  // mark window as deleted
         terminal_break(1);          // break out of loops
        break;

      case WM_QUIT:        // Need to break out of any loops
        terminal_break(1); 
        break;

      case WM_SETCURSOR:   // Cursor moving into window area
	if(full_screen_window_active) 
          SetCursor(NULL);          // If covering full screen, hide it
        else 
          SetCursor(arrow_cursor);  // Otherwise, keep cursor visible
	return TRUE;

      case WM_PAINT:                // Redraw window (after parts are unobscured by other windows)
        {                           // Only happens after dialog boxes erased, or ALT-TAB
          HDC hdc = BeginPaint(hwnd, &ps);
          HBRUSH oBrush;
                                     // create a brush to fill with 
          oBrush = SelectObject(hdc, CreateSolidBrush(target_background_color|0x02000000L));
          PatBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,     // Fill window plus frame area
                      ps.rcPaint.right-ps.rcPaint.left+1, 
                      ps.rcPaint.bottom-ps.rcPaint.top+1, PATCOPY); 
          GdiFlush();
          DeleteObject(SelectObject(hdc, oBrush));         // Clean up GDI resources
          window_drawn ++;          // mark window as drawn the first time
          EndPaint(hwnd, &ps);    
          return TRUE;
        }

      case WM_ERASEBKGND:  
        return TRUE;     // don't clear background, let WM_PAINT handle it           
         
    }                               // Let Windows handle all other messages
  return DefWindowProc(hwnd, message, wparam, lparam);
}

