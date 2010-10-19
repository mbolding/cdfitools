/*****************************************************/
/* Windows 95/98/NT/2000/XP sample experiment in C   */
/* For use with Version 2.0 of EyeLink Windows API   */
/*                                                   */
/*      (c) 1997-2002 by SR Research Ltd.            */
/* For non-commercial use by Eyelink licencees only  */
/*                                                   */
/* THIS FILE: w32_demo.h     (all templates)         */
/* CONTENTS:                                         */
/* - Function and variable declarations              */
/*   to link C files used in experiments             */
/* - contains declarations for all templates         */  
/*                                                   */
/*                                                   */
/* CHANGES FOR Windows 2000/XP, EyeLink 2.0:         */
/* - added is_eyelink2 ( 1 if EyeLink II tracker)    */
/*****************************************************/

          // These colors are used to draw the target and camera display
          // The background color is used to clear the display
          // for calibration, drift correction, and after recording is interrupted.
extern COLORREF target_foreground_color;  /* target display color */
extern COLORREF target_background_color;  /* background color */

/*************** CREATE FULL-SCREEN WINDOW **********/

extern HWND full_screen_window;          // window handle
extern int full_screen_window_active;    // set if we are topmost window

        // Create the full screen window
        // Ensure it is maximized, and no WM_PAINT messages are pending
        // Returns 0 if OK, else couldn't create
int make_full_screen_window(HINSTANCE hInstance);

        // Destroy or clean up window  
void close_full_screen_window(void);

        // Clear the window
void clear_full_screen_window(COLORREF c);

       // Handles all messages for full-scren experiment window
       // Repaint is handled by simply erasing the display: 
       // this should only happen if ALT-ESC used to switch apps 
       // or after dialog boxes (i.e. EDF file transfer) are erased.
       // NOTE: during calibration and drift correction, most messages are intercepted by EXPTSPPT
LRESULT CALLBACK full_screen_window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

/*********** BITMAP DRAWING AND COPYING *********/

               // Draw grid of letters on local display, 
               // Draw boxes for each letter on EyeLink screen
               // Creates memory bitmap 
               // After you're done, delete with DeleteObject(hBitmap)
HBITMAP draw_grid_to_bitmap(void);


       // Draws a grid of letters to a DDB bitmap                
       // This can be rapidly copied to the display
       // Also draw boxes to the EyeLink tracker display for feedback
	   // Save the segmentation information in the specified files.
       // Returns: Handle of bitmap
HBITMAP draw_grid_to_bitmap_segment(char *filename, char*path, int dotrack);


               // Copies bitmap to display 
               // Places top-left at (x,y)
void display_bitmap(HWND hwnd, HBITMAP hbm, int x, int y);


/************* FONT DRAWING SUPPORT ***********/

extern HFONT current_font;

              // Create a font, cache in current_font
HFONT get_new_font(char *name, int size, int bold);

                  // Release the font
void release_font(void);

		// Uses printf() formatting
		// Prints starting at x, y position
		// Uses current font, size, foreground color, bkgr. color & clr mode
                // <bg> is -1 to NOT clear background 
void graphic_printf(COLORREF fg, COLORREF bg, int center, int x, int y, char *fmt, ...);

/********** DRAW FORMATTED TEXT ***********/

       // Draw text within margins, left-justified, with word-wrap.
       // Draw to <hdc> context, so it supports display and bitmaps   
       // (bitmap must already be created and selected into context).
       // Draw boxes for each word on EyeLink tracker display if <dotrack> set
       // Use font selected with get_new_font(), and draws it in <color>
       // RECT <margins> sets margins, and <lspace> sets pixels between lines
void draw_text_box(HDC hdc, char *txt, COLORREF color, RECT margins, int lspace, int dotrack);

/********** DRAW FORMATTED TEXT AND SAVE SEGMENTATION ***********/

       // Draw text within margins, left-justified, with word-wrap.
       // Draw to <hdc> context, so it supports display and bitmaps   
       // (bitmap must already be created and selected into context).
       // Draw boxes for each word on EyeLink tracker display if <dotrack> set
       // Use font selected with get_new_font(), and draws it in <color>
       // RECT <margins> sets margins, and <lspace> sets pixels between lines
	   // Save the segmentation info into the text. 
	   // Currently, the bottom edge of the line N overlaps with the top edge of line N+1
	   // In each line, the left edge of word N overlaps with the right edge of word N+1
	   // Extra space (the width of letter 'M') is added to the the beginning and ending segment of each line 
void draw_text_box_segment(HDC hdc, char *txt, COLORREF color, 
		RECT margins, int lspace, int dotrack, char *filename);

/************ CREATE BITMAP WITH FORMATTED TEXT ***********/
      // Draw text to full-screen sized bitmap, cleared to <bgcolor>
      // Draw boxes for each word on EyeLink tracker display if <dotrack> set
      // Use font selected with get_new_font(), and draws it in <fgcolor>
      // RECT <margins> sets margins, and <lspace> sets pixels between lines
HBITMAP text_bitmap(char *txt, COLORREF fgcolor, COLORREF bgcolor, 
                               RECT margins, int lspace, int dotrack);

/************ CREATE BITMAP WITH FORMATTED TEXT AND SAVE SEGMENTATION INFO ***********/

      // Draw text to full-screen sized bitmap, cleared to <bgcolor>
      // Draw boxes for each word on EyeLink tracker display if <dotrack> set
      // Use font selected with get_new_font(), and draws it in <fgcolor>
      // RECT <margins> sets margins, and <lspace> sets pixels between lines
	  // <dottrack> determines whether outlines of the word segment is displayed over the tracker PC.
	  // Splice the path and filename to record the segmentation information
	  // If the segmentation file already exists, it will not overwritten the file 
	  // if <sv_options> is set as SV_NOREPLACE
HBITMAP text_bitmap_segment(char *txt, COLORREF fgcolor, COLORREF bgcolor, 
                  RECT margins, int lspace, int dotrack, char *fname, char *path, INT16 sv_options);

/************ CREATE BLANK BITMAP ***********/

      // Creates a blank, full-screen sized bitmap, cleared to <bgcolor>
HBITMAP blank_bitmap(COLORREF bgcolor);


/************* FAST BITMAP SECTION COPY **********/

        // Copies a rectangular area of a bitmap to the display
        // Copies rectangle <xs1, ys1, xs2, ys2> of DDB <hbm> 
        // Places top-left corner at <xd, yd> 
        // for maximum drawing speed, xd=xs1 and yd=ys1 (aligned copy)
        // <hbm> is source bitmap, <hwnd> is the window to draw to
void display_rect_bitmap(HWND hwnd, HBITMAP hbm, int xd, int yd, 
                                                 int xs1, int ys1, int xs2, int ys2);

/*********** LOAD IMAGE FILE, CREATE BITMAP ************/

           // Creates DDB bitmap from loaded image file
           // if <keepsize>, makes bitmap the same size as image file
           // otherwise, creates a full-screen sized bitmap
           // If <dx, dy> are not 0, will resize image to those dimensions
           // If resized image won't fill display, 
           // it clears the bitmap to <bgcolor> and centers the image in it
           // NOTE: intermediate bitmap is 24-bit color, 
           // so won't work well in 256-color display modes

HBITMAP image_file_bitmap(char *fname, int keepsize, int dx, int dy, COLORREF bgcolor);


/************* EXPERIMENT TRIALS ************/

      // Several types of trials are demonstrated
      // Some support multiple displays 
      // <trial> is trial number
      // <type> selects the stimulus: 0 for text, 1 for picture images, 2 for grid of letter
      // <trialid> sets the contents of the TRIALID message
      // <time_limit> is the maximum time the stimuli are displayed 

    // NOTE: TRIALID AND TITLE MUST HAVE BEEN SET BEFORE CALLING THESE FUNCTIONS!
    // FAILURE TO INCLUDE THESE MAY CAUSE INCOMPATIBILITIES WITH ANALYSIS SOFTWARE!

                // Simple trial, displays a single line of text
int simple_recording_trial(char *text, UINT32 time_limit);

                // Trial using bitmap copying
                // Caller must create and destroy the bitmap
int bitmap_recording_trial(HBITMAP bitmap, UINT32 time_limit);

                // trial with real-time gaze cursor
                // Caller must create and destroy the bitmap
int realtime_data_trial(HBITMAP bitmap, UINT32 time_limit);

      // Run gaze-contingent window trial
      // <fgbm> is bitmap to display within window
      // <bgbm> is bitmap to display outside window
      // <wwidth, wheight> is size of window in pixels
      // <mask> flags whether to treat window as a mask
      // <time_limit> is the maximum time the stimuli are displayed 
int gc_window_trial(HBITMAP fgbm, HBITMAP bgbm, 
                    int wwidth, int wheight, int mask, UINT32 time_limit);

                // trial with gaze control
                // this trial is assumend to draw its own bitmap, 
                // which matches the control regions
int gaze_control_trial(UINT32 time_limit);

	// Plays back last trial data
	// Prints white "F" for fixations
	// Connects samples with black line
int playback_trial(void);


/************ TRIAL SEQUENCING *********/

	/* This code sequences trials within a block. */
	/* It calls do_xxx_trial() to execute a trial, then interperts result code. */
	/* It places a result message in the EDF file */
int run_trials(void);

    // Each type of experiment has its own function which executes trials by number.
    // For each trial, it must:
         // - set title, TRIALID
         // - Create bitmaps and EyeLink display graphics
         // - Check for errors in creating bitmaps
         // - Run the trial recording loop
         // - Delete bitmaps
         // - Return any error code

int do_simple_trial(int num); 

int do_text_trial(int num);

int do_picture_trial(int num);

int do_data_trial(int num);

int do_control_trial(int num);

int do_gcwindow_trial(int num);

extern int is_eyelink2;
extern DISPLAYINFO dispinfo;



