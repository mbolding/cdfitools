// based on v0.305cylopean_MAE_ACR_exp.c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GL/glut.h"
#include "sean_display.h"
#define TRUE 1 
#define FALSE 0


//SEAN
LPSTR sean_lpCmdLine;
int sean_nCmdShow;


/* default values */
static int winW = 1024;
static int winH = 800;			
int fullscreen = TRUE;
int memoryAllocated = FALSE;
int oppositeContrast = FALSE;

int anaglyph_shift = 50; //20;

  //contrast
float Color_back[3] = {0.5, 0.5, 0}; 
float Color_G1[3] = {0.5, 1, 0}; //{0.67, 0.05, 0};
float Color_G2[3] = {0.5, 0, 0}; //{0.67, 0.05, 0};
float Color_R1[3] = {0.9, 0.5, 0}; //{0.67, 0.05, 0};
float Color_R2[3] = {0, 0.5, 0}; //{0.67, 0.05, 0};


/* RUN-TIME variables */
int disparity = 3;
int direction = 1;

int fix_x = 0, fix_y = 0;
char File_config[64] = {"config.txt"};

GLuint monogram_anaglyph = 1;
GLuint mono_cross = 11;

/** -------------- COMMON ROUTINE -------------- **/
void config_load(void){
  FILE *fRead;
  char String_input[256];
  char *locater;

  if((fRead = fopen(File_config, "r")) == NULL ){
    printf("!!ERROR!! The file '%s' is not opened(load)\n", File_config);
  }
  else{
    while(!feof(fRead)) {
      fgets(String_input, 256, fRead);
      locater = strchr(String_input, '#');
      if(locater != NULL) strcpy(locater, "\x0");
      printf("%s", String_input); 
      
      if(strstr(String_input, "red")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%f %f %f   %f %f %f", 
          &Color_R1[0], &Color_R1[1], &Color_R1[2], &Color_R2[0], &Color_R2[1], &Color_R2[2]);
      }
      else if(strstr(String_input, "green")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%f %f %f   %f %f %f", 
          &Color_G1[0], &Color_G1[1], &Color_G1[2], &Color_G2[0], &Color_G2[1], &Color_G2[2]);
      }
      else if(strstr(String_input, "back")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%f %f %f", 
          &Color_back[0], &Color_back[1], &Color_back[2]);
      }
      else if(strstr(String_input, "anaglyph")){
        locater = (strchr(String_input, ':'));
        sscanf(locater+1, "%d", &anaglyph_shift);
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
/*** ----------------------------------------- **/
// usage :  sprintf(TextString, "~~");
// print2screen(x_location(1, 0), y_location('b'/'t', 2), TextString);
void *font = GLUT_BITMAP_8_BY_13; 

int x_location(int division, int n_th){
  int x_location;  
  x_location = (int) (-0.5*winW +n_th *winW/division);
  return(x_location);
}

int y_location(char t_b, int n_th){
  int y_location;
  if(t_b == 'b') y_location = (int) (-0.5*winH+4 +n_th*15);
  else if(t_b == 't') y_location = (int) (0.5*winH-10 -n_th*15);
  return(y_location);
}

void print2screen(int x, int y, char* TextString){
  int i, string_length;
  glRasterPos2f(x, y);
  string_length = (int)strlen(TextString);
  for(i=0;i<string_length;i++){
    glutBitmapCharacter(font, TextString[i]);
  }
}

/************************ FIXation ****************************/
void fix_gl(void){

  glNewList(mono_cross, GL_COMPILE); 
    glLineWidth(2); 
    glBegin(GL_LINES); 
      glVertex2i(-5, 0); 
      glVertex2i(5,0);
      glVertex2i(0, 6); 
      glVertex2i(0,-6); 
    glEnd(); 
  glEndList(); 
}


/*********** assemble DISPLAY ************************************/
int whichkey = 32;

void keyboard_demo(unsigned char key, int x, int y){
  switch(key){
  case 27 : 
    printf(" ==> Demo exited \n");
    exit(0); 
    break;
  
  case 32:
    break;

  case 'c':
    oppositeContrast = !oppositeContrast;
    break;

  default:
    break;
  }

  whichkey = key;
  glutPostRedisplay();
}

void specialKeys_demo(int key, int x, int y){
  double incr;

  switch(key){
    case GLUT_KEY_UP: 
      incr=1;
      break;
    case GLUT_KEY_DOWN:
      incr=-0.5;
      break;
    case GLUT_KEY_RIGHT:
      ++disparity;
      break;
    case GLUT_KEY_LEFT:
      --disparity;
      break;
  }

  switch(whichkey){
  case 'r':
    Color_back[0] = Color_back[0] +0.1*incr; 
    if(Color_back[0] > 1.0) Color_back[0] = 1.0;
    else if(Color_back[0] < 0.0) Color_back[0] = 0;
    Color_G1[0] = Color_back[0];
    Color_G2[0] = Color_back[0];
    break;
  case 'g':
    Color_back[1] = Color_back[1] +0.1*incr;
    if(Color_back[1] > 1.0) Color_back[1] = 1.0;
    else if(Color_back[1] < 0.0) Color_back[1] = 0;
    Color_R1[1] = Color_back[1];
    Color_R2[1] = Color_back[1];
    break;

  case 'a':
    anaglyph_shift =  anaglyph_shift + (int)(2*incr); 
    break;
  }
  glutPostRedisplay();
}

void Parameters_onScreen_demo(void){
  char TextString[64];

  glColor3f(1.0, 1.0, 1.0);
  sprintf(TextString, " [a]naglyph_shift: %d", anaglyph_shift);
  print2screen(x_location(8, 0), y_location('t', 0), TextString); 

  
  sprintf(TextString, " R(%3.2f %3.2f %3.2f)/(%3.2f %3.2f %3.2f)", 
    Color_R1[0], Color_R1[1],Color_R1[2],Color_R2[0],Color_R2[1],Color_R2[2]);
  print2screen(x_location(8, 0), y_location('b', 0), TextString); 
  sprintf(TextString, "G(%3.2f %3.2f %3.2f)/(%3.2f %3.2f %3.2f)",
    Color_G1[0], Color_G1[1],Color_G1[2],Color_G2[0],Color_G2[1],Color_G2[2]);
  print2screen(x_location(8, 3), y_location('b', 0), TextString); 

  sprintf(TextString, "bg: [r]%3.2f  [g]%3.2f (b)%3.2f", 
    Color_back[0], Color_back[1],Color_back[2]);
  print2screen(x_location(8, 6), y_location('b', 0), TextString); 
} 

int height = 50;
int width = 5;
int y_offset = 12;

void mono_bar(void){
  glRecti(-width, -height, width, height);
}

void displayMono(void){  
  glDrawBuffer(GL_BACK);
  glClearColor(Color_back[0], Color_back[1], Color_back[2], 1.0); 
  glClear(GL_COLOR_BUFFER_BIT);

    glColor3fv(Color_G1);
    glPushMatrix();
    glTranslatef(-anaglyph_shift, 0, 0);
    glPushMatrix();
    glTranslatef(disparity, height+y_offset, 0);
    mono_bar();
    glPopMatrix(); 
    glCallList(mono_cross);
    glPushMatrix();
    glTranslatef(-disparity, -(height+y_offset), 0);
    if(oppositeContrast) glColor3fv(Color_G2); 
    else glColor3fv(Color_G1);
    mono_bar();
    glPopMatrix();
    glPopMatrix();

    glColor3fv(Color_R1);
    glPushMatrix();
    glTranslatef(anaglyph_shift, 0, 0);
    glPushMatrix();
    glTranslatef(-disparity, height+y_offset, 0);
    mono_bar();
    glPopMatrix(); 
    glCallList(mono_cross);
    glPushMatrix();
    glTranslatef(disparity, -(height+y_offset), 0);
    if(oppositeContrast) glColor3fv(Color_R2); 
    else glColor3fv(Color_R1);
    mono_bar();
    glPopMatrix(); 
    glPopMatrix();


    Parameters_onScreen_demo();
  glutSwapBuffers();
} 


int sean_window(int argc, char *argv[]){
  //config_load();

  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);

  glutInit(&argc, argv);
  glutInitWindowSize(winW, winH);
  glutInitWindowPosition(10,10);
  glutCreateWindow("test");//argv[0]);	
  if (fullscreen) glutFullScreen();
  glutReshapeFunc(myReshape); 

  //fix_gl();

  //glutKeyboardFunc(keyboard_demo);
  //glutSpecialFunc(specialKeys_demo);
  glutDisplayFunc(displayMono);

  glutMainLoop(); 
  return 0;
}