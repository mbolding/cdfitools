#define COLORS struct colors
#define NFIGS 8
#define NDISPLAY 3
#define TWOEYES 2
#define MAXAPOINTS 32
#define DEGRAD 57.29578
#define MAXIMAGE 100

COLORS {
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	};

#define RANDOMDOTS struct randomdots

RANDOMDOTS {
	GLint gridsz;
	GLint gridx;
	GLint gridy;
	GLfloat cellsz;
	GLint dotsz;
	GLint numdots;
	GLfloat deltax;
	GLfloat deltay;
	GLint grid;
	GLint hemi;
	GLfloat dotvel;
	GLfloat expvel;
	GLfloat cirvel;
	GLfloat anglex;
	GLfloat cohere;
	GLint special;
	GLint initialize;
	GLint clkcount;
        GLfloat disp;  /* JKG - this is not in 'real' gl.h */
	};


#define GXDRAW struct glxdraw

GXDRAW  {
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat xsize;
	GLfloat nochgsizecm;
	GLfloat viewdist;
	GLfloat widthincm;
	GLfloat heightincm;
	GLfloat track;  /* for when color changes */
	GLint option;
	GLint skipframes;
	GLint noshow;
	GLint nosize;
	GLint wsize;
	GLint target;
	GLint left;
	GLint changeit;
	COLORS fore;
	COLORS back;
	COLORS extra;
	GLfloat angle;
	GLfloat r;
	GLfloat alpha;
	GLfloat beta;
	};

//#define INFINITY 6000.
#define UPCLOSE  viewdist 
