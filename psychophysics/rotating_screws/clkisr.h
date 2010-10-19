
#define CLKISR_IO_MAGIC 'c'

#define CLKISR_IORESET _IO(CLKISR_IOC_MAGIC, 0)

#define CLKISR_FUNCTION_GLOBAL_VARS _IOR(CLKISR_IO_MAGIC, 1, int) 
#define CLKISR_FUNCTION_DMA_BUF _IOR(CLKISR_IO_MAGIC, 2, int) 
#define CLKISR_FUNCTION_GLOBAL_VARS3 _IOR(CLKISR_IO_MAGIC, 3, int) 
#define CLKISR_FUNCTION_PREWRITE _IOR(CLKISR_IO_MAGIC, 4, int) 
#define CLKISR_FUNCTION_TEST _IOR(CLKISR_IO_MAGIC, 5, int) 
#define CLKISR_FUNCTION_FILEOPEN _IOR(CLKISR_IO_MAGIC, 6, int) 
#define CLKISR_FUNCTION_FILECLOSE _IOR(CLKISR_IO_MAGIC, 7, int) 
#define CLKISR_FUNCTION_EOD _IOR(CLKISR_IO_MAGIC, 8, int) 
#define CLKISR_FUNCTION_INIT _IOR(CLKISR_IO_MAGIC, 9, int) 
#define CLKISR_FUNCTION_REWARD _IOR(CLKISR_IO_MAGIC, 10, int) 
#define CLKISR_FUNCTION_BEEP _IOW(CLKISR_IO_MAGIC, 11, int) 
#define CLKISR_FUNCTION_INIT_CLOCK _IOW(CLKISR_IO_MAGIC, 12, int) 
#define CLKISR_FUNCTION_INIT_ATOD _IOW(CLKISR_IO_MAGIC, 13, int) 
#define CLKISR_FUNCTION_CLOSEDATA _IOW(CLKISR_IO_MAGIC, 14, int) 
#define CLKISR_FUNCTION_XUNLOADABLE _IOW(CLKISR_IO_MAGIC, 15, int) 
#define CLKISR_FUNCTION_GLOBAL_VARS2 _IOR(CLKISR_IO_MAGIC, 16, int) 
#define CLKISR_FUNCTION_ITRACK _IOR(CLKISR_IO_MAGIC, 17, int) 
#define CLKISR_FUNCTION_CFG _IOW(CLKISR_IO_MAGIC, 18, int) 
#define CLKISR_FUNCTION_19 _IOW(CLKISR_IO_MAGIC, 19, int) 
#define CLKISR_FUNCTION_20 _IOW(CLKISR_IO_MAGIC, 20, int) 

struct clkisr_dev {
	void **data;
	int quantam;
	int qset;
	unsigned long size;
	unsigned int access_key;
	unsigned int usage;
};

struct DATAX {
		int count;
		char *buffer;
};

struct global_vars {
		int one;
		int two;
		int three;
		char string[128];
};

struct vmmapping {
		unsigned long address;
		int file;
		int page;
};
