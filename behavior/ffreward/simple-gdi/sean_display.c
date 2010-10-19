// vesion 0.22
// msb modified oct 2007 to load and use bitmap stimuli

/* choose how to run */
#define EYELINK
//#define TESTING

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "GL/glut.h"
//msb  use DevIL image library to load images for stimuli
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#if defined EYELINK
#include "sean_display.h"
#endif


#define TRUE 1 
#define FALSE 0
#define PI 3.1415926535897

/* default values */
static int winW = 1024;
static int winH = 800;			
int fullscreen = TRUE;
int bSafety_lock = 0;  // prevent multiple trigger

/* stimulus dimensions */
int width = 200;
int height = 200;
int height2 = 50; // text box
int space2 = 0;  // b/w text box & square
int thick = 5;    // lines in box
int space =15;    // b/w lines
int radius = 70;  // circle_in_box
int thick2 = 40;  // in the circle
int radius2 = 20;   // out of the box
int space3 = 20;  //b/w box & circle


  //contrast
double Color_back[3] = {0.5, 0.5, 0.5}; 
double Color_black[3] = {0.0, 0.0, 0}; 
double Color_white[3] = {1.0, 1.0, 1.0}; 
double Color_gray[3] = {0.75, 0.75, 0.75}; 
double Color_red[3] = {0.75, 0.0, 0.0}; 

//trials
int rotate_rect = 1;
int rotate_circle= 1;
int disc_pos = 1;
char Text[64];
int timeBoxOnly = 3000;
int timeBoxOnlyPost = 2000;
int timeTextOn = 2000;
int timeFixOn = 2000;
int timeSession;


int Rotate_rect[512];
int Rotate_circle[512];
int Disc_pos[512];
char TextPool[512][32];
int TimeBoxOnly[512];
int TimeBoxOnlyPost[512];
int TimeTextOn[512];
int TimeFixOn[512];
int TimeSession[512];

int trial_total = 0;
int trial=0;

/* RUN-TIME variables */

char File_config[264] = {"config.txt"};
GLuint stimulus = 111;
GLuint stim_rect = 112;
GLuint stim_frame = 113;
GLuint stim_circle = 114;
GLuint stim_disc = 115;

//msb load picture logos
GLuint image[4];
void load_logos(void)
{
	ilInit();
	iluInit();
	ilutRenderer(ILUT_OPENGL);
    image[0]=ilutGLLoadImage("mcd128.bmp");
	image[1]=ilutGLLoadImage("arb128.bmp");
	image[2]=ilutGLLoadImage("bk128.bmp");
	image[3]=ilutGLLoadImage("blank.bmp");
}


//SEAN
#if defined EYELINK
LPSTR sean_lpCmdLine;
int sean_nCmdShow;
#endif

/************************ load Stimulus ****************************/

void config_load(void){
  FILE *fRead;
  char String_input[256];
  char *locater;
  int line=0;
  if((fRead = fopen(File_config, "r")) == NULL ){
    printf("!!ERROR!! The file '%s' is not opened(load)\n", File_config);
  } 
 
  
  else{
    while(!feof(fRead)) {
      fgets(String_input, 256, fRead);
      locater = strchr(String_input, '#');
      if(locater != NULL) strcpy(locater, "\x0");
      printf("%s", String_input); 
      
      //colors
      if(strstr(String_input, "Gray")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%f %f %f", 
          &Color_gray[0], &Color_gray[1], &Color_gray[2]);
      }
      else if(strstr(String_input, "Back")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%f %f %f", 
          &Color_back[0], &Color_back[1], &Color_back[2]);
      }
      else if(strstr(String_input, "Black")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%f %f %f", 
          &Color_black[0], &Color_black[1], &Color_black[2]);
      }
      else if(strstr(String_input, "White")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%f %f %f", 
          &Color_white[0], &Color_white[1], &Color_white[2]);
      }
      //dimensions
      else if(strstr(String_input, "square")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%d %d %d %d", &width, &height, &thick, &space);
      }
      else if(strstr(String_input, "textbox")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%d %d", &height2, &space2);
      }
      else if(strstr(String_input, "disks")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%d %d %d %d", &radius, &thick2, &radius2, &space3);
      }
      else if(strstr(String_input, "timing")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%d %d %d %d", &timeBoxOnly, &timeBoxOnlyPost, &timeTextOn, &timeFixOn);
      }

      //not implemeted yet
      else if(strstr(String_input, "TRIALS")){
        while(!feof(fRead)) {
          fgets(String_input, 256, fRead);
          printf("%s", String_input); 
          
          if(strstr(String_input, "[")){
            sscanf(String_input, "%d %d %d %s [%d %d %d %d] %d", 
              &Disc_pos[line], &Rotate_rect[line], &Rotate_circle[line],  TextPool[line],
              &TimeBoxOnly[line], &TimeTextOn[line],&TimeBoxOnlyPost[line], &TimeFixOn[line], 
              &TimeSession[line]);

            while(1){
              locater = strchr(TextPool[line], '_');
              if(locater != NULL) *locater = ' ';
              else break;
            }
            line++;
          }
          else if(strstr(String_input, "TRIAL-END")){
            trial_total = line;
            break;
          }
        }
      }

      else if(strstr(String_input, "make")) break;
    }//while
    fclose(fRead);
    printf("\n %s loaded !!\n", File_config);
  }//else
  
}


void myReshape(GLsizei w, GLsizei h){
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);			
  glLoadIdentity();
  gluOrtho2D ((int)-w*0.5, (int)w*0.5, (int)-h*0.5, (int)h*0.5);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (fullscreen){
    glutFullScreen();
    //glTranslatef(-128.0, 128.0, 0.0);
    winW = 1024;
    winH = 768;
  }
  else {
    winW = w;
    winH = h;
  }
  glShadeModel (GL_FLAT);  //??  
  glutPostRedisplay();
}

/*********************************************************************/
//***************************** STIMULUS ****************************//


void make_stim_rect(void){
  int i, n, s;

  glNewList(stim_rect, GL_COMPILE); 

    glColor3dv(Color_black);
    glRectd(-width, -height, width, height);
    
    n = (int)(2*width/(space+thick));
    s = (int)(0.5*(2*width - (space*n + thick*(n+1))) +0.5*thick); 
    glColor3dv(Color_white);
    glLineWidth(thick); 
    glBegin(GL_LINES); 
    for(i=0;i<=n;i++){
      glVertex2i(i*(thick+space) -width +s, -height); 
      glVertex2i(i*(thick+space) -width +s, height);
    }
    glEnd();
  
  glEndList(); 
}

void make_stim_frame(void){

  glNewList(stim_frame, GL_COMPILE); 

    glColor3dv(Color_back);
    glRectd(0.5*width, -height, 1.5*width, height);
    glRectd(-0.5*width, -height, -1.5*width, height);
    glRectd(-width, 0.5*height, width,1.5*height);
    glRectd(-width, -0.5*height, width, -1.5*height);  
  glEndList(); 
}

void make_stim_circle(void){
  GLUquadricObj *Disk;
  double angle, half_length;

  glNewList(stim_circle, GL_COMPILE); 
    glColor3dv(Color_white);
    Disk = gluNewQuadric();
    gluDisk(Disk, 0, radius, 32, 1);

    glColor3dv(Color_black);
    angle = asin(0.5*thick2/radius) *360/PI *0.5;
    half_length = sqrt(radius*radius - 0.25*thick2*thick2);
    glRectd(-0.5*thick2, -half_length, 0.5*thick2, half_length);

    glColor3dv(Color_black);
    gluPartialDisk(Disk, 0, radius, 32, 1, -angle, 2*angle);
    gluPartialDisk(Disk, 0, radius, 32, 1, 180-angle, 2*angle);
    gluDeleteQuadric(Disk);

  glEndList(); 
}

//void *font = GLUT_BITMAP_TIMES_ROMAN_24; 
// msb modified oct 2007 to load and use bitmap stimuli
void put_text(void){
/*  char TextString[64];
  int i, string_length;
  float x, y;

  glColor3f(1.0, 0.0, 0.0);
  sprintf(TextString, "%s", Text);

  string_length = (int)strlen(TextString);

  //x = -0.5*width + 15;
  x = -0.5*(8.0*string_length) -8.0;  
  y = -0.5*height -space2 - height2 +0.5*(height2-24) +5;
  
  glRasterPos2f(x,y); 
  for(i=0;i<string_length;i++){
    glutBitmapCharacter(font, TextString[i]); */
  //msb put a bitmap instead of text
	int idx;
    if(strstr(Text,"Arbys"))
	{
		idx = 1;
	}
	else if(strstr(Text,"Burger"))
	{
		idx = 2;
	}
	else if(strstr(Text,"McDonalds"))
	{
		idx = 0;
	}
	else
	{
		idx = 3;
	}
	glBindTexture(GL_TEXTURE_2D, image[idx]);		// Select Our Logo Texture
	//glRectd(-0.5*width, -0.5*height-space2, 0.5*width, -0.5*height-space2-height2);
	
//	glBegin(GL_QUADS);							// Start Drawing A Textured Quad
//		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.1*width, -0.5*height-space2-height2,  0.0f);	// Bottom Left
//		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.1*width, -0.5*height-space2-height2,  0.0f);	// Bottom Right
//		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.1*width, -0.5*height-space2,          0.0f);	// Top Right
//		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.1*width, -0.5*height-space2,          0.0f);	// Top Left
//	glEnd();									// Done Drawing The Quad

  glBegin(GL_QUADS);							// Start Drawing A Textured Quad
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.1*width, -height2*0.5,  0.0f);	// Bottom Left
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.1*width, -height2*0.5,  0.0f);	// Bottom Right
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.1*width, height2*0.5,          0.0f);	// Top Right
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.1*width, height2*0.5,          0.0f);	// Top Left
	glEnd();
}


void make_stimulus(int bText){
  GLUquadricObj *Disk;

  make_stim_rect();
  make_stim_frame();
  make_stim_circle();

  glNewList(stimulus, GL_COMPILE); 
    if(bText!=2){
      glPushMatrix();
      glRotated(rotate_rect*45, 0, 0, 1);
      glCallList(stim_rect);
      glPopMatrix(); 
    
      glCallList(stim_frame);
      glPushMatrix();
      glRotated(rotate_circle*45, 0, 0, 1);
      glCallList(stim_circle);
      glPopMatrix(); 
    }
    glColor3dv(Color_white);
    Disk = gluNewQuadric();
    glPushMatrix();
    glTranslated((disc_pos-1)*(0.5*width+space3+radius2), (disc_pos%2)*(0.5*height+space3+radius2), 0);
    gluDisk(Disk, 0, radius2, 32, 1);
    glPopMatrix(); 
    gluDeleteQuadric(Disk);

    // glColor3dv(Color_white);
    // glRectd(-0.5*width, -0.5*height-space2, 0.5*width, -0.5*height-space2-height2);

    if(bText==1) put_text();
  glEndList(); 

#if defined EYELINK
  switch(bText){
  case 0:
    eyemsg_printf("[%3d] ENV-ON", trial);    
    break;
  case 1:
    eyemsg_printf("[%3d] TEXT-ON", trial);    
    break;
  case 2:
    eyemsg_printf("[%3d] ITI", trial);    
    break;
  }
#endif

  glutPostRedisplay();
}

void make_fixation(int none){
  glNewList(stimulus, GL_COMPILE); 
    glColor3dv(Color_black);
    glRectd(-10, -1, 10, 1);
    glRectd(-1, -10, 1, 10);
  glEndList();

  glutPostRedisplay();
}


void make_pause_screen(int none){
  glNewList(stimulus, GL_COMPILE); 
    glColor3dv(Color_red);
    glRectd(-10, -1, 10, 1);
    glRectd(-1, -10, 1, 10);
  glEndList();

  glutPostRedisplay();
}

void clean_screen(int bUnLock){
 
  char our_file_name[260] = "TEST";

  glNewList(stimulus, GL_COMPILE); 

  glEndList();
  glutPostRedisplay();

  if(bUnLock) bSafety_lock = 0;


  if (trial!=0){
    eyemsg_printf("TIMEOUT");    // message to log the timeout 
    //end_trial();                 // local function to stop recording
    stop_recording();
  }

  if(trial>=trial_total){

    receive_data_file(our_file_name, "", 0);  // transfer the file, ask for a local name
    exit(0); //return 0;
  }

}



#if defined TESTING
int sean_recording_trial(int none){
  
  bSafety_lock = 1;
  
  disc_pos = Disc_pos[trial];
  rotate_rect = Rotate_rect[trial];
  rotate_circle = Rotate_circle[trial];
  
  timeBoxOnly = TimeBoxOnly[trial];
  timeBoxOnlyPost = TimeBoxOnlyPost[trial];
  timeTextOn = TimeTextOn[trial];
  timeFixOn = TimeFixOn[trial];
  strcpy(Text, TextPool[trial]);
  //printf("text %s\n\n", Text);
  
  make_stimulus(0);
  glutTimerFunc(timeBoxOnly, make_stimulus, 1);  
  glutTimerFunc(timeBoxOnly+timeTextOn, make_stimulus, 2);  
  glutTimerFunc(timeBoxOnly+timeBoxOnlyPost+timeTextOn, make_fixation, 1);
  glutTimerFunc(timeBoxOnly+timeBoxOnlyPost+timeTextOn+timeFixOn, clean_screen, 1);
  
  
  printf("%d %d %d %s [%d %d %d %d] %d\n", 
    Rotate_rect[trial], Rotate_circle[trial], Disc_pos[trial], TextPool[trial],
    TimeBoxOnly[trial], TimeTextOn[trial],TimeBoxOnlyPost[trial], TimeFixOn[trial], TimeSession[trial]);
  
  trial++;
}
#endif

#if defined EYELINK
// EyeLink Routine
static void end_trial(void)
{
  
  //SEAN
  //clear_full_screen_window(target_background_color);  /* hide display */
  end_realtime_mode();   // NEW: ensure we release realtime lock
  pump_delay(100);       // CHANGED: allow Windows to clean up  
                         // while we record additional 100 msec of data 
  stop_recording();
    
}

void sean_recording_trial(int none){
  int error; // EyeLinklowelltest


  // EyeLink
  error = start_recording(1,1,0,0);   // record samples and events to EDF file only
  // EyeLink
  //if(error != 0)  return error;        // return error code if failed
  // EyeLink
  begin_realtime_mode(100);   // Windows 2000/XP: no interruptions till display start marked
  // record for 100 msec before displaying stimulus 
  

  disc_pos = Disc_pos[trial];
  rotate_rect = Rotate_rect[trial];
  rotate_circle = Rotate_circle[trial];
  
  timeBoxOnly = TimeBoxOnly[trial];
  timeBoxOnlyPost = TimeBoxOnlyPost[trial];
  timeTextOn = TimeTextOn[trial];
  timeFixOn = TimeFixOn[trial];
  strcpy(Text, TextPool[trial]);
  
  make_stimulus(0);
  glutTimerFunc(timeBoxOnly, make_stimulus, 1); 
  glutTimerFunc(timeBoxOnly+timeTextOn, make_stimulus, 2); 
  glutTimerFunc(timeBoxOnly+timeBoxOnlyPost+timeTextOn, make_fixation, 1);
  glutTimerFunc(timeBoxOnly+timeBoxOnlyPost+timeTextOn+timeFixOn, clean_screen, 1);
   // pause code every 45 trials to give subject a break. 
  if (((trial+1) % 45) == 0) {
    //exit(0); // for testing
    glutTimerFunc(timeBoxOnly+timeBoxOnlyPost+timeTextOn+timeFixOn+10, make_pause_screen, 1);
  }
  else {
    glutTimerFunc(timeBoxOnly+timeBoxOnlyPost+timeTextOn+timeFixOn+10, sean_recording_trial, 1);
  }
  /*
       // Trial loop: till timeout or response 
  while(1)   
  {                            // First, check if recording aborted 
    if((error=check_recording())!=0) return error;  
    // Check if trial time limit expired
    if(!bSafety_lock)
    {
      eyemsg_printf("TIMEOUT");    // message to log the timeout 
      end_trial();                 // local function to stop recording
      //button = 0;                  // trial result message is 0 if timeout 
      break;                       // exit trial loop
    }
    if(break_pressed())     // check for program termination or ALT-F4 or CTRL-C keys
    {
      end_trial();         // local function to stop recording
      return ABORT_EXPT;   // return this code to terminate experiment
    }
    
    if(escape_pressed())    // check for local ESC key to abort trial (useful in debugging)   
    {
      end_trial();         // local function to stop recording
      return SKIP_TRIAL;   // return this code if trial terminated
    }
    
  }                       // END OF RECORDING LOOP

  */

  trial++;

  
}
#endif



/*********** assemble DISPLAY ************************************/
//int whichkey = 32;

void keyboard_demo(unsigned char key, int x, int y){

  switch(key){
  case 27 : 
    //printf(" ==> Demo exited \n");
    exit(0); 
    break;
  
  case 32:
    if(!bSafety_lock) sean_recording_trial(1);
    //printf("%d %d\n", winH, winW);
    break;

  case 'a':
      make_stimulus(0);
      glutPostRedisplay();
    break;

  default:
    break;
  }

  //whichkey = key;
}


void displayMono(void){  
  glDrawBuffer(GL_BACK);
  glClearColor(Color_back[0], Color_back[1], Color_back[2], 1.0); 
  glClear(GL_COLOR_BUFFER_BIT);

  glCallList(stimulus);

  glutSwapBuffers();
} 


#if defined TESTING
int main(int argc, char *argv[]){
#endif
#if defined EYELINK
int sean_window(int argc, char *argv[]){
#endif
  srand(time(NULL));
  config_load();

  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);

  glutInit(&argc, argv);
  glutInitWindowSize(winW, winH);
  glutInitWindowPosition(10,10);
  glutCreateWindow(argv[0]);	
  if (fullscreen) glutFullScreen();
  glutReshapeFunc(myReshape); 

  //fix_gl();

  glutKeyboardFunc(keyboard_demo);
  glutDisplayFunc(displayMono);

	//text_display(0);
 
	load_logos();
	glEnable(GL_TEXTURE_2D);


  glutMainLoop(); 
  return 0;
}