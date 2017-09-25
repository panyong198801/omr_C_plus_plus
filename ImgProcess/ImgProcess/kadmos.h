/****************************************************************************
*
*  kadmos.h
*
*  $Date: 04-Aug-2015 10:20 $
*
*  Copyright 2002-2015  re Recognition GmbH 
*  Hafenstrasse 50b  8280 Kreuzlingen  Switzerland
*  Phone: +41 (0)71 6780000  Fax: +41 (0)71 6780099
*  www.reRecognition.com  E-mail: info@reRecognition.com
*
****************************************************************************/

#ifndef INC_KADMOS

#define INC_KADMOS "5.0o"
#define KADMOS_MAJOR 0x50
#define KADMOS_MINOR 'o'

#define KADMOS_MKVER(major, minor) (((major)<<8) | (minor))
#define KADMOS_VERSION KADMOS_MKVER(KADMOS_MAJOR, KADMOS_MINOR)
/* sample usage: #if KADMOS_VERSION >= KADMOS_MKVER(0x50,'b') */

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_MSC_VER) || (_MSC_VER<1400)
  #if !defined(INT_PTR)
    #define INT_PTR long
  #endif
#endif

/****************************************************************************
*
*  definition of KADMOS_API
*
****************************************************************************/

#ifndef KADMOS_API
  #if defined(_WIN32) || defined(_WIN64)
    #define KADMOS_API __stdcall
  #else
    #define KADMOS_API
  #endif
#endif


/****************************************************************************
*
*  functions for break and error control
*
****************************************************************************/

#if !defined RE_ERRORHANDLING
#define RE_ERRORHANDLING

/*  *** with multithreading, the following settings and functions are thread-specific */
/*  *** they have an impact only on the related thread */

/*  set/get configuration of the KADMOS error handling */
extern long KADMOS_API re_SetErrorConfig(long config);
extern long KADMOS_API re_GetErrorConfig(void);

/*** macros for error handling configuration ('config' values) */

/*  enable/disable standard program termination by */
/*  Ctrl+Break, ESC, ALT+F4 (consol app. and Windows) or Ctrl+C (consol app. only): */
#define RE_QUITENABLED       0x80000000  /* default */
/*  enable/disable console output of error texts and warnings: */
#define RE_ERRORDISPLAY      0x20000000  /* default */
/*  enable/disable program termination after errors: */
#define RE_ERROREXIT         0x10000000
/*  enable/disable treatment of warnings like errors (except displaying them): */
#define RE_ENABLEWARNINGS    0x08000000

/*  to disable any of the former settings RE_XXXXXXXX, call the following: */
/*    re_SetErrorConfig(re_GetErrorConfig()&~RE_XXXXXXXX); */

/*  disable any error handling, exept return on error: */
#define RE_NOERRORHANDLING   0x00000000 

#define KADMOS_ERROR long

/*  set/get private_data of the KADMOS error text */
extern KADMOS_ERROR KADMOS_API re_SetPrivateData(long private_data);
extern long KADMOS_API re_GetPrivateData(void);

#define RE_ERROR_PROGRAM_LEN  64
#define RE_ERROR_TEXT_LEN    512

typedef struct re_ErrorText_tag {
  KADMOS_ERROR ident;
  char program[RE_ERROR_PROGRAM_LEN];
  char text[RE_ERROR_TEXT_LEN];
  char systext[RE_ERROR_TEXT_LEN];
  long private_data;
} re_ErrorText;

typedef struct re_wErrorText_tag {
  KADMOS_ERROR ident;
  wchar_t program[RE_ERROR_PROGRAM_LEN];
  wchar_t text[RE_ERROR_TEXT_LEN];
  wchar_t systext[RE_ERROR_TEXT_LEN];
  long private_data;
} re_wErrorText;

/* error types (values of ident) */
#define RE_SUCCESS           0x00000000
#define RE_ERROR             0x80000000
#define RE_QUIT              0x40000000
#define RE_WARNING           0x20000000
#define RE_INFO              0x10000000

#define RE_PARAMETERERROR    0x88000000
#define RE_MEMORYERROR       0x84000000
#define RE_FILEERROR         0x82000000
#define RE_PROTECTERROR      0x81000000

#define RE_SEGMENTATIONERROR 0x80080000

/*  retrieve error text, call with NULL for error status (ident) only */
extern KADMOS_ERROR KADMOS_API re_GetErrorText(re_ErrorText*);
extern KADMOS_ERROR KADMOS_API re_wGetErrorText(re_wErrorText*);

/*  set the error text and error status (ident) */
extern void KADMOS_API re_SetErrorText(const re_ErrorText*);
extern void KADMOS_API re_wSetErrorText(const re_wErrorText*);

/*  call with NULL to display KADMOS error text */
extern void KADMOS_API re_DisplayErrorText(const re_ErrorText*);
extern void KADMOS_API re_wDisplayErrorText(const re_wErrorText*);

/*  clear error status (ident) */
extern void KADMOS_API re_ClearError(void);

/*  *** with multithreading, the following settings for error handling */
/*  *** have an impact on all threads */

/*  declaration for private error handling */
typedef void (KADMOS_API *re_ErrorHandler)(const re_ErrorText*);
typedef void (KADMOS_API *re_wErrorHandler)(const re_wErrorText*);

/*  call to install a private error handler, call with NULL to deinstall */
extern re_ErrorHandler KADMOS_API re_SetErrorHandler(re_ErrorHandler);
extern re_wErrorHandler KADMOS_API re_wSetErrorHandler(re_wErrorHandler);

/*  call to retrieve the installed error handler (NULL == KADMOS error handler) */
extern re_ErrorHandler KADMOS_API re_GetErrorHandler(void);
extern re_wErrorHandler KADMOS_API re_wGetErrorHandler(void);

#endif /* RE_ERRORHANDLING */


/****************************************************************************
*
*  structure for initialization
*
****************************************************************************/

typedef struct ReInit_tag {
  char  version[8];        /* to check correct KADMOS version */

  /*** set before calling rel_init() or rep_init() */ 
  int rel_grid_maxlen;     /* maximum number of items in rel_grid */
  int rel_graph_maxlen;    /* maximum number of items in rel_graph */
  int rel_result_maxlen;   /* maximum number of items in rel_result */

  int options;             /* OPTIONS_RESTRICT_LABELS - if set, only labels specified under */
                           /* ReParm.labels and ReParm.rejects are loaded. ReParm.code has to be set accordingly */
                           /* use Chopper.exe to get the labels and rejects of a classifier */ 

  /*** provide enough memory before first call of rep_do() */
  void *rep_memory;        /* memory to receive rep_result(s) */
  long rep_memory_size;    /* size of provided memory */

  /* can be set to gzopen, gzread, and gzclose to read g-zipped files (zlib) */
  /* but can be set to any other equivalent set of decompression functions */
  void * (*zip_open) (const char *file_title, const char *mode);
  void * (*zip_wopen) (const wchar_t *file_title, const char *mode);

  int    (*zip_compressed) (const char *file_title);
  int    (*zip_wcompressed) (const wchar_t *file_title);
  /* if zip_(w)compressed is not specified, the extension ".gz" signals compression */

  int    (*zip_read) (void *file, void *buf, unsigned long len);
  int    (*zip_close)(void *file);

  /*** used internally by re?_init(), re?_info(), re?_do(), and re?_end() */
  INT_PTR interna1[4];     /* do not change */
  INT_PTR interna2[4];     /* do not change */
} ReInit;

#define OPTIONS_RESTRICT_LABELS 1

/****************************************************************************
*
*  structures and macros for image description
*
****************************************************************************/

typedef struct ReRect_tag {
  long left, top;
  long width, height;
} ReRect; 

typedef struct ReStroke_tag {
  long color;
  long thickness;
  long timestamp;
  void *user_data;
  long x0,y0;
  long x1,y1;
} ReStroke;

typedef struct ReImage_tag {
  void           *data;       /* pointer to image/data of type 'imgtype' */
  unsigned long  imgtype;     /* see IMGTYPE_* (pixel, pointer, stroke, ...) */
  unsigned long  width;       /* for IMGTYPE_PIXELARRAY, -_POINTERARRAY, -_FEATURE only */ 
  unsigned long  height;      /* for IMGTYPE_PIXELARRAY, -_POINTERARRAY, -_FEATURE only */
  unsigned long  strokecount;    /* for IMGTYPE_STROKE only */
  unsigned long  max_strokecount;/* for IMGTYPE_STROKE only */
  ReRect         subimage;    /* for all IMGTYPEs except IMGTYPE_FEATURE */
  unsigned short bitsperpixel;/* size of 'data' items (0(=1), 1, 8, 24) */
  unsigned short color;       /* see COLOR_* (binary, gray, rgb) */
  unsigned short photometric; /* see PHOTOMETRIC_* (min is white, ...) */
  unsigned short alignment;   /* alignment of pixel data, 0, 1, 2, 4 */
  unsigned short orientation; /* see ORIENTATION_* (topleft, ...) */
  unsigned short fillorder;   /* see FILLORDER_* (MSB2LSB, LSB2MSB) */
  unsigned short filler;      /* 8 byte alignment */
  unsigned short resunit;     /* see RESUNIT_* (inch, cm, ...) */
  unsigned long  xresolution;
  unsigned long  yresolution;
} ReImage;

/*** for every following group, the first item is default */

#define IMGTYPE_PIXELARRAY    0
#define IMGTYPE_POINTERARRAY  1
#define IMGTYPE_STROKE        2

#define IMGTYPE_FEATURE       4
#define IMGTYPE_FEATURE_INT   5
#define IMGTYPE_FEATURE_UINT  6
#define IMGTYPE_FEATURE_FLOAT 7

/* *** for IMGTYPE_FEATURE_* the following exceptions are valid: 
*    - the feature vector is given under 'data'
*    - height*width is the dimension of the feature vector
*    - bitsperpixel (size of the feature data items) can be 8, 16, 32, 64
*    - subimage and all other items of ReImage are unused
* after loading a feature rec file (rec_init()), imgtype, width, height,
* and bitsperpixel are set to the related classifiers default values */

#define COLOR_BINARY 0
#define COLOR_GRAY   1
#define COLOR_RGB    2

#ifndef TIFFMACROS_INCLUDED      /* define this before if already included */       
  /*** macros according to tiff.h from Sam Leffler (Silicon Graphics Inc.) */
  #define PHOTOMETRIC_MINISWHITE 0
  #define PHOTOMETRIC_MINISBLACK 1

  /* not for IMGTYPE_STROKE */
  #define ORIENTATION_TOPLEFT 1
  #define ORIENTATION_BOTLEFT 4

  /* for COLOR_BINARY only, not for IMGTYPE_STROKE */
  #define FILLORDER_MSB2LSB 1    /* from most to least significant bit */
  #define FILLORDER_LSB2MSB 2    /* from least to most significant bit */

  #define RESUNIT_NONE       1
  #define RESUNIT_INCH       2
  #define RESUNIT_CENTIMETER 3
#endif /* TIFFMACROS_INCLUDED */

#define RESUNIT_METER 4

typedef void *ReImageHandle;

/****************************************************************************
*
*  parameters for REC, REL, and REP
*
****************************************************************************/

/*** parameters to control line and box detection and removal in REL, REP */
typedef struct RelGridParm_tag {
  long grid;                     /* type of the box, see GRID_* macros */
  
  long min_width;                /* minimal width of a box, line, or comb */
  long max_width;                /* maximal width of a box, line, or comb */
  long min_height;               /* minimal height of a box, line, or comb */
  long max_height;               /* maximal height of a box, line, or comb */
  
  long subgrid_count;            /* number of subboxes or comb fields */

  ReRect search_area;            /* rectangle to search for the grid item */
} RelGridParm;

#define REPARM_LABELS_SIZE_MAX 8192 /* maximum value of item labels_size below */

/****************************************************************************
*  As part of RecData, RelData, or RepData, all items of the structure ReParm
*  are cleared by a call to rec_init(), rel_init(), or rep_init() and set to
*  default values. Set new values after rec_init(), rel_init(), or rep_init().
****************************************************************************/

#ifdef _MSC_VER
  #pragma warning(disable:4201)
#endif

typedef struct ReParm_tag {
  long  general;                 /* see GENERAL_* macros */
  /*** preprocessing */
  long  prep;                    /* see PREP_* macros */

  short deskew_min;              /* minimum line skew to do de-skewing */
                                 /* zero means no de-skewing (default) */
  short filler1;                 /* 8 byte alignment */
  short noise_diameter_percent;  /* noise reduction in REC, REL, and REP */
  short noise_diameter_pixel;    /* noise reduction in REC, REL, and REP */

  short noise_width_percent;     /* noise reduction in REC, REL, and REP */
  short noise_length_percent;    /* noise reduction in REC, REL, and REP */
  short noise_width_pixel;       /* noise reduction in REC, REL, and REP */
  short noise_length_pixel;      /* noise reduction in REC, REL, and REP */

  RelGridParm *gridparm;         /* area of RelGridParm structures */
  long  gridparm_maxlen;         /* number of provided structures */

  /*** segmentation */
  long  typograph;               /* see TYPO_* macros */
  long  filler2;                 /* 8 byte alignment */

  short line_space, base_line;   /* line metrics, if known */
  short char_space_min;          /* character metrics (min, max) for width */
  short char_space_max;          /* of character+interspace (if known) */

  /*** classification */
  long  code;                    /* CODE_PAGE_1252, CODE_ISO_8859_1, ... */
  long  options;                 /* exclude, use basic labels, ... */
  long  labels_size;             /* size of allocation of labels and rejects in byte */
  #if defined(__STDC__)
      char  *labels;             /* active labels; memory allocated by re?_init() */
      char  *rejects;            /* active rejects; memory allocated by re?_init() */
                                 /* only used for parameter OPTIONS_RESTRICT_LABELS */
  #else
    union {
      char  *labels;             /* active labels, CP 437 - ISO_8859_15, CODE_ASCII */
      short *labels2;            /* active labels, CODE_UNICODE */
    };
    union {
      char  *rejects;            /* active rejects, CP 437 - ISO_8859_15, CODE_ASCII */
      short *rejects2;           /* active rejects, CODE_UNICODE */
                                 /* only used for parameter OPTIONS_RESTRICT_LABELS */
    };
  #endif
  long  alc;                     /* (additional) active labels, see ALC_* macros */
  long  font;                    /* restriction for alc specification, see FONT_* makros */
  short reject_limit;            /* controls number of alternatives; default 128 */
  short filler3[3];              /* 8 byte alignment */

  /*** context */
  long  pos;                     /* see POS_* macros */

  /*** hook - for private use */
  INT_PTR hookparm;              /* copied from REP to REL, from REL to REC */
} ReParm;

#ifdef _MSC_VER
  #pragma warning(default:4201)
#endif


/****************************************************************************
*
*  macros for process control
*
****************************************************************************/

/*** indicates calling sequence - set and used internally - do not change */
#define GENERAL_RECCALL           0x00010000 /* set by rec_init(), rec_do(),.. */
#define GENERAL_RECACCENTCALL     0x00020000 /* set by rec_accent() */
#define GENERAL_RELCALL           0x00040000 /* set by rel_init(), rel_do(),.. */
#define GENERAL_RELFINDCALL       0x00080000 /* set by rel_find() */
#define GENERAL_REPCALL           0x00100000 /* set by rep_init(), rep_do(),.. */
#define GENERAL_RESPELLCALL       0x00400000 /* set by respell_init(), respell_do(),.. */
#define GENERAL_RECALL            0x00ff0000

#define GENERAL_REPMULTITHREADING 0x00001000 /* default for rep_do() */
                                             /* 2 threads are used generally when set */
                                             /* as much as possible are used in the server version */
#define GENERAL_MULTITHREADING    0x0000ff00 /* mask for multithreading parameters */

#define GENERAL_PREP_ONLY         0x00000001 /* return after preprocessing */
#define GENERAL_LINESHADOW_ONLY   0x00000002 /* return before recognition */

/*** for calls of rep_do() only */
#define GENERAL_LINEPOSITION_ONLY 0x00000004 /* return without recognition */

/*** for calls of rec_do() only */
#define GENERAL_FEATURES_ONLY     0x00000008 /* return after feature extraction */

#define GENERAL_HOOKENGINE_LOADED 0x10000000 /* status (return value) */
#define GENERAL_HOOKENGINE_ON     0x20000000 /* use the hook engine */


/****************************************************************************
*
*  macros to select code pages
*
****************************************************************************/

#define CODE_PAGE_437                   437
#define CODE_PAGE_720                   720
#define CODE_PAGE_737                   737
#define CODE_PAGE_850                   850
#define CODE_PAGE_852                   852
#define CODE_PAGE_855                   855
#define CODE_PAGE_857                   857
#define CODE_PAGE_858                   858
#define CODE_PAGE_864                   864
#define CODE_PAGE_866                   866
#define CODE_PAGE_874                   874

#define CODE_PAGE_1250                 1250
#define CODE_PAGE_1251                 1251
#define CODE_PAGE_1252                 1252
#define CODE_PAGE_1253                 1253
#define CODE_PAGE_1254                 1254
#define CODE_PAGE_1255                 1255
#define CODE_PAGE_1256                 1256
#define CODE_PAGE_1257                 1257
#define CODE_PAGE_1258                 1258

#define CODE_ISO_8859_1              885901
#define CODE_ISO_8859_2              885902
#define CODE_ISO_8859_4              885904
#define CODE_ISO_8859_5              885905
#define CODE_ISO_8859_6              885906
#define CODE_ISO_8859_7              885907
#define CODE_ISO_8859_8              885908
#define CODE_ISO_8859_9              885909
#define CODE_ISO_8859_10             885910
#define CODE_ISO_8859_11             885911
#define CODE_ISO_8859_13             885913
#define CODE_ISO_8859_15             885915

#define CODE_ASCII                  1000000  /* Ersatz representation */
#define CODE_UNICODE                2000000
#define CODE_UTF_8                  2000001

/*** special classifier codes */
#define CODE_ASCII_7BIT             1000001
#define CODE_UTF_16_7BIT            2000017


/****************************************************************************
*
*  macros for preprocessing control and grid element detection
*
****************************************************************************/

/*** influences REC */
#define PREP_NOSPACEREMOVAL       0x00000001 /* don't remove space around characters */

/*** influences REL */
#define PREP_NOSPOTREMOVAL        0x00000002 /* don't remove spots */

/*** influences REC + REL + REP */
#define PREP_AUTO_NOISEREDUCTION  0x00000004 /* controlled noise reduction */
                                             /* (default) */ 
#define PREP_SCALING              0x00000008 /* normalize character size */

#define PREP_RGBTOGRAY_COLORGRAY  0x00000000 /* rgb to gray normal (default) */
#define PREP_RGBTOGRAY_COLORMIN   0x00000010 /* rgb to gray remove color */
#define PREP_RGBTOGRAY_COLORMAX   0x00000020 /* rgb to gray enhance color */

#define PREP_GRAYTOBIN_VERY_THIN  0x00000100 /* gray to bin very low level */
#define PREP_GRAYTOBIN_THIN       0x00000200 /* gray to bin low level */
#define PREP_GRAYTOBIN_MEDIUM     0x00000300 /* gray to bin medium level (default) */
#define PREP_GRAYTOBIN_THICK      0x00000400 /* gray to bin high level */
#define PREP_GRAYTOBIN_VERY_THICK 0x00000500 /* gray to bin very high level */

#define PREP_GRAYTOBIN_UNIFORM    0x00000800 /* gray to bin with one threshold only */

#define PREP_BINFILTER_VERY_THIN  0x00001000 /* smoothing very thin pixels */
#define PREP_BINFILTER_THIN       0x00002000 /* smoothing thin pixels */
#define PREP_BINFILTER_MEDIUM     0x00003000 /* smoothing normal pixels */
#define PREP_BINFILTER_THICK      0x00004000 /* smoothing thick pixels */
#define PREP_BINFILTER_VERY_THICK 0x00005000 /* smoothing very thick pixels */
#define PREP_BINFILTER            0x0000f000

#define PREP_INPLACE              0x80000000 /* overwrite image in preprocessing */

#define GRID_LINE_H               0x00000001 /* horizontal line */
#define GRID_LINE_V               0x00000002 /* vertical line */
#define GRID_BOX                  0x00000004 /* box around character(s) */
#define GRID_COMB                 0x00000008 /* comb below character(s) */

#define GRID_ALL                  0x000000FF /* all grid items */
#define GRID_END                  0x00000000 /* for last item of reparm.gridparm */


/****************************************************************************
*
*  macros for segmentation control
*
****************************************************************************/

/*** influences REL */
#define TYPO_PROPORTIONAL          0x00000100 /* assume proportional spacing */
#define TYPO_EQUIDISTANT           0x00000200 /* assume equidistant spacing */
#define TYPO_NOLIGATURES           0x00000400 /* assume no ligatures */
#define TYPO_NOTOUCHINGCHARS       0x00000800 /* assume no touching characters */
#define TYPO_NOSEGALTERNATIV       0x00010000 /* no segmentation alternatives */
#define TYPO_4_SEGALTERNATIV       0x00040000 /* up to 4 segmentation alternatives */ 
#define TYPO_8_SEGALTERNATIV       0x00080000 /* up to 8 segmentation alternatives (default) */
#define TYPO_KEEPIMG               0x00100000 /* keep REC images after rel_do() */
#define TYPO_EXTENDED_SEGMENTATION 0x01000000 /* less errors, more CPU time */


/****************************************************************************
*
*  macros to control classification
*
****************************************************************************/

/*** use or get basic labels instead of group labels */ 
#define OPTIONS_BASICLABELS_PARM   0x00000001 /* use/get them in ReParm */
#define OPTIONS_BASICLABELS_RESULT 0x00000002 /* get them in Re?Result */
#define OPTIONS_BASICLABELS        0x00000003 /* use/get them always */
/*** exclude alternatives of classification if possible */
#define OPTIONS_EXCLUDE            0x00000010
/*** use only classifiers for the selected labels (default) */
#define OPTIONS_STRICTSELECT       0x00000020 
#define OPTIONS_FAST               0x00000080
/*** repeat rec_do() with character specific binarisation; for colour and gray images only */
#define OPTIONS_REL_REC_REPEAT     0x00000100
/*** detect (classify) and delete nonsense objects in rel_do() */
/*   if boxes are detected, everything outside the boxes is deleted */
#define OPTIONS_REL_CLEAN          0x00001000

#ifndef _INC_PENWIN
  #define _INC_PENWIN
  #define ALC_DEFAULT        0x00000000
  #define ALC_LCALPHA        0x00000001
  #define ALC_UCALPHA        0x00000002
  #define ALC_ALPHA (ALC_LCALPHA|ALC_UCALPHA)

  #define ALC_NUMERIC        0x00000004
  #define ALC_ALPHANUMERIC (ALC_ALPHA|ALC_NUMERIC)

  /* #define ALC_PUNC        0x00000008 - replaced by ALC_SPECIAL */
  /* #define ALC_MATH        0x00000010 - replaced by ALC_SPECIAL */
  /* #define ALC_MONETARY    0x00000020 - replaced by ALC_SPECIAL */
  /* #define ALC_OTHER       0x00000040 - replaced by ALC_SPECIAL */
  #define ALC_GESTURE        0x00004000 
#endif
/* #define ALC_SPECIAL (ALC_PUNC|ALC_MATH|ALC_MONETARY|ALC_OTHER) */
#define ALC_SPECIAL          0x00000078
#define ALC_ACCENT           0x00000080
#define ALC_ALL (ALC_ALPHANUMERIC|ALC_SPECIAL|ALC_ACCENT|ALC_GESTURE)

/*** font specification */
#define FONT_HAND            0x00000001
#define FONT_MACHINE         0x00000002
#define FONT_HM (FONT_HAND|FONT_MACHINE)

#define FONT_LATIN           0x00000010
#define FONT_FRAKTUR         0x00000020
#define FONT_GREEK           0x00000040
#define FONT_CYRILLIC        0x00000080
#define FONT_ARABIC          0x00000100
#define FONT_FARSI           0x00000200
#define FONT_HEBREW          0x00000400
#define FONT_THAI            0x00000800
#define FONT_LANGUAGE (FONT_LATIN|FONT_FRAKTUR|FONT_GREEK|FONT_CYRILLIC|FONT_ARABIC|FONT_FARSI|FONT_HEBREW|FONT_THAI)

#define FONT_OCRA            0x00020000
#define FONT_OCRB            0x00040000
#define FONT_CMC7            0x00080000
#define FONT_E13B            0x00100000
#define FONT_F7B             0x00200000
#define FONT_SEMI            0x00400000
#define FONT_LCD             0x00800000
#define FONT_BRAILLE         0x01000000
#define FONT_NORM (FONT_OCRA|FONT_OCRB|FONT_CMC7|FONT_E13B|FONT_F7B|FONT_SEMI|FONT_LCD|FONT_BRAILLE)

#define FONT_LN (FONT_LANGUAGE|FONT_NORM)
#define FONT_ALL (FONT_HM|FONT_LN)

/****************************************************************************
*
*  macros for line position context evaluation
*
****************************************************************************/

#define POS_NOLINECONTEXT    0x00000001 /* no line context for recognition */
#define POS_SOFTLINECONTEXT  0x00000002 /* soft line context for recognition */
#define POS_HARDLINECONTEXT  0x00000004 /* heavy line context for recognition */
#define POS_NOWORDCONTEXT    0x00000100 /* no word context (uppercase-, lowercase word, number) */


/****************************************************************************
*
*  wParam for WM_USER messages from RE?_* (Windows only)
*
****************************************************************************/

#define REC_MESSAGE       0x0100  /* lParam = &recdata */
#define RECACCENT_MESSAGE 0x0200  /* lParam = &recdata, wParam = &accents */
#define REL_MESSAGE       0x0400  /* lParam = &reldata */
#define RELFIND_MESSAGE   0x0800  /* lParam = &reldata */
#define REP_MESSAGE       0x1000  /* lParam = &repdata */

#define REX_IMGREADY      0x0020  /* preprocessing of image performed */
#define REX_ERROR         0x0040  /* error return */
#define REX_FINISHED      0x0080  /* module has terminated */

#define REP_LINE_CNT      0x1001  /* lParam = rep_result_len */
#define REP_RESULT        0x1002  /* lParam = &rep_result[.] */


/****************************************************************************
*
*  result structures for rec_accent(), rel_do() and rep_do()
*
****************************************************************************/


#define REC_CHAR_SIZE 16   /* size of rec_char in byte */
#define REC_ALT        8   /* maximum number of recognition alternatives */
#define SEG_ALT        8   /* maximum number of segmentation alternatives */

typedef struct RecGraph_tag {
  short result_number[SEG_ALT]; /* index of result alternative in rec_result */
  short next[SEG_ALT];     /* index of next element in RecGraph; -1: end */
} RecGraph;

typedef struct RelGrid_tag { 
  long grid;               /* detected grid item (see GRID_* macros) */
  long subgrid_count;      /* number of subboxes of the given box or comb */
  long left, top;          /* position of the grid item */
  long width, height;
} RelGrid;

typedef struct RelGraph_tag {
  short leading_blanks;    /* count of blanks preceeding the character(s) */
  short filler[3];         /* 8 byte alignment */
  short result_number[SEG_ALT]; /* index of result alternative in rel_result */
  short next[SEG_ALT];     /* index of next element in RelGraph; -1: end */
  short seg_value[SEG_ALT][SEG_ALT]; /* certainty of alternative segmentations */
} RelGraph;

#ifdef _MSC_VER
  #pragma warning(disable:4201)
#endif

typedef struct RelResult_tag {
  long  left, top;         /* position of character in line image */
  long  width, height;
  
  long  result_flags;      /* RESULT_FLAG_RESPELL_CONFIRMED, ... */ 

  void  *result_image;     /* segmented character image (if not NULL) */
  long  result_width;      /* width of result_image (may be compressed) */
  long  result_height;     /* height of result_image (may be compressed) */
  long  projection[2];     /* slant projection on base line */

  short italic, bold;      /* stylus (slant, thickness) */
  short strokelen, black;  /* stroke length, black pixels (%) */

  #if defined(__STDC__)
    char  rec_char[REC_ALT][REC_CHAR_SIZE]; /* recognition result */
  #else
    union {
      char  rec_char[REC_ALT][REC_CHAR_SIZE]; /* recognition result, CP437-ISO_8859_15, ASCII,... */
      short rec_char2[REC_ALT][REC_CHAR_SIZE/2]; /* recognition result, CODE_UNICODE only */
    };
  #endif
  unsigned char rec_value[REC_ALT];  /* uncertainty of recognition 0...255 */
  long rec_top[REC_ALT];             /* should-be-positions in line image */
  long rec_bottom[REC_ALT];          /* should-be-positions in line image */
} RelResult;

#define RecResult RelResult  /* we use same data structure in rec_accent() */

#define RESULT_FLAG_RESPELL_CONFIRMED 0x10000000 /* set if confirmed by RESPELL */
#define RESULT_FLAG_RESPELL_GENERATED 0x20000000 /* set if generated by RESPELL */
#define RESULT_FLAG_RESPELL           0x30000000 

#define RESULT_FLAG_ACCENT_TOP        0x01000000 /* set if related character is top accent */
#define RESULT_FLAG_ACCENT_BOTTOM     0x02000000 /* set if related character is bottom accent */
#define RESULT_FLAG_ACCENT            0x03000000 /* RESULT_FLAG_ACCENT_TOP | RESULT_FLAG_ACCENT_BOTTOM */
#define RESULT_FLAG_ACCENT_START      0x00100000 /* first component of an accented character */
#define RESULT_FLAG_ACCENT_MEMBER     0x00200000 /* component of an accented character */
#define RESULT_FLAG_ACCENT_END        0x00400000 /* last component of an accented character */

#ifdef _MSC_VER
  #pragma warning(default:4201)
#endif

typedef struct RepResult_tag {
  long left, top;          /* position of line image in provided image */
  long width, height;

  short rel_deskew;        /* corrected skew of the line image */
  short rel_char_space;    /* mean width of character+interspace */
  short rel_blank_width;   /* width of blank (signals proportional font) */
  short rel_blank_min;     /* minimum width of blank (proportional font) */

  short rel_grid_len;      /* number of items in rel_grid */
  short rel_graph_len;     /* number of items in rel_graph */
  short rel_result_len;    /* number of items in rel_result */
  short filler1;           /* 8 byte alignment */

  RelGrid *rel_grid;       /* detected grid elements */
  RelGraph *rel_graph;     /* knots in segmentation */
  RelResult *rel_result;   /* result of single character recognition */
  void *filler2;           /* 8 byte alignment */
} RepResult;


/****************************************************************************
*
*  module REC
*
****************************************************************************/

#ifdef _MSC_VER
  #pragma warning(disable:4201)
#endif

typedef struct RecData_tag {
  ReInit init;               /* set before calling rec_init() */

  /*** parameters for rec_do() - set after calling rec_init() */
  ReParm parm;               /* defaults will be set by rec_init() */

  /*** representation of image data */
  ReImage image;             /* image to be recognized */
  long  image_top;           /* image position in line (if known) */
  long  image_bottom;

  /*** recognition control */
  long  busy;                /* !=0: recognition under progress */
  INT_PTR hWND_rec_finished; /* window to send WM_USER message with */
                             /*   wParam == REC_MESSAGE */
                             /*   wParam |= REX_ERROR in case of error */
                             /*   wParam |= REX_IMGREADY after preprocessing */
                             /*   wParam |= REX_FINISHED after recognition */
                             /*   lParam == &recdata */
                             /* after execution (Windows only) */

  /*** results of rec_do() */
  long  left, top;           /* position of character after noise reduction */
  long  width, height;

  long  result_flags;        /* RESULT_FLAG_RESPELL_CONFIRMED, ... */ 
  
  void  *result_image;       /* segmented character image (if not NULL) */
  long  result_width;        /* width of result_image (may be compressed) */
  long  result_height;       /* height of result_image (may be compressed) */
  long  projection[2];       /* slant projection on base line */
  
  short italic, bold;        /* stylus (slant, thickness) */
  short strokelen, black;    /* stroke length, black pixels (%) */

  #if defined(__STDC__)
    char  rec_char[REC_ALT][REC_CHAR_SIZE]; /* recognition result */
  #else
    union {
      char  rec_char[REC_ALT][REC_CHAR_SIZE]; /* recognition result, CP437-ISO_8859_15 */
      short rec_char2[REC_ALT][REC_CHAR_SIZE/2]; /* recognition result, CODE_UNICODE */
    };
  #endif
  unsigned char rec_value[REC_ALT];  /* uncertainty of recognition 0...255 */
  long rec_top[REC_ALT];             /* should-be-positions */
  long rec_bottom[REC_ALT];          /* should-be-positions */
} RecData;

#ifdef _MSC_VER
  #pragma warning(default:4201)
#endif

extern KADMOS_ERROR KADMOS_API rec_init         (RecData *, const char *file_title);
extern KADMOS_ERROR KADMOS_API rec_winit        (RecData *, const wchar_t *file_title);
extern KADMOS_ERROR KADMOS_API rec_info         (RecData *);
extern KADMOS_ERROR KADMOS_API rec_filetitle    (const RecData *, char *file_title); /* char, wchar_t */
extern KADMOS_ERROR KADMOS_API rec_do           (RecData *);
extern KADMOS_ERROR KADMOS_API rec_accent       (RecData *, KADMOS_ERROR (KADMOS_API *rec_hook)(RecData *),
                                                 RecGraph *recg, int rec_graph_maxlen, int *rec_graph_len, 
                                                 RecResult *accents, int rec_result_maxlen, int *rec_result_len);
                                                /* allocate recg, accents accordingly before rec_accent()-call */
extern KADMOS_ERROR KADMOS_API rec_group_labels (RecData *);
extern KADMOS_ERROR KADMOS_API rec_end          (RecData *);
extern KADMOS_ERROR KADMOS_API rec_get_features (const RecData *, 
                                                 void **features,  
                                                 char *type, /* in8, un16, ... */
                                                 int *dimension);


/****************************************************************************
*
*  module REL
*
****************************************************************************/

typedef struct RelData_tag {
  ReInit init;               /* set before calling rel_init() */

  /*** parameters for rel_do() - set after calling rel_init() */
  ReParm parm;               /* defaults will be set by rel_init() */

  /*** representation of image data */
  ReImage image;             /* image to be recognized */
  long  image_top;           /* horizontal image position in line (if known) */
  long  image_bottom;

  /*** recognition control */
  long  busy;                /* !=0: recognition under progress */
  INT_PTR hWND_rel_finished; /* window to send WM_USER message with */
                             /*   wParam == REL_MESSAGE */
                             /*   wParam |= REX_ERROR in case of error */
                             /*   wParam |= REX_IMGREADY after preprocessing */
                             /*   wParam |= REX_FINISHED after recognition */
                             /*   lParam == &reldata */
                             /* after execution (Windows only) */

  /*** used to get results from every internal call of rec_do() */
  KADMOS_ERROR (KADMOS_API *rec_hook)(RecData *);  /* executed after rec_do() */
  void *filler1;             /* 8 byte alignment */

  /***  results of rel_do() */
  long  left, top;           /* position of line image in full image */
  long  width, height;

  short rel_deskew;          /* corrected skew of the line image */
  short rel_char_space;      /* mean width of character+interspace */
  short rel_blank_width;     /* width of blank (signals proportional font) */
  short rel_blank_min;       /* minimum width of blank (proportional font) */

  /*** number of results of rel_do() */
  short rel_grid_len;        /* number of items in rel_grid */
  short rel_graph_len;       /* number of items in rel_graph */
  short rel_result_len;      /* number of items in rel_result */
  short filler2;             /* 8 byte alignment */

  /*** results of rel_do(), but allocate before first call of rel_do() */
  RelGrid *rel_grid;         /* detected grid elements */
  RelGraph *rel_graph;       /* knots of segmentation */
  RelResult *rel_result;     /* results of single character recognition */
  void *filler3;             /* 8 byte alignment */
} RelData;

extern KADMOS_ERROR KADMOS_API rel_init         (RelData *, const char *file_title);
extern KADMOS_ERROR KADMOS_API rel_winit        (RelData *, const wchar_t *file_title);
extern KADMOS_ERROR KADMOS_API rel_info         (RelData *);
extern KADMOS_ERROR KADMOS_API rel_filetitle    (const RelData *, char *file_title); /* char, wchar_t */
extern KADMOS_ERROR KADMOS_API rel_do           (RelData *);
extern KADMOS_ERROR KADMOS_API rel_group_labels (RelData *);
extern KADMOS_ERROR KADMOS_API rel_find         (RelData *, int char_count);
extern KADMOS_ERROR KADMOS_API rel_findg        (RelData *, int char_count, int area_width, int char_height, int char_width);
                                                /* negative value of char_count: up to ... */
extern KADMOS_ERROR KADMOS_API rel_end          (RelData *);

#define SHADOW_FONT_HAND    0x01
#define SHADOW_FONT_MACHINE 0x02
extern KADMOS_ERROR KADMOS_API rel_lineshadow(RelData *, unsigned char **vertical_shadow);

extern KADMOS_ERROR KADMOS_API rel_freeimages(RelData *); /* see TYPO_KEEPIMG */

/*** APIs to support pen-computing */
extern KADMOS_ERROR KADMOS_API rel_clear(RelData *);
extern KADMOS_ERROR KADMOS_API rel_recset(RelData *, const RecData *, const ReRect *);
extern KADMOS_ERROR KADMOS_API rel_corr(RelData *);

/*** APIs to support dictionaries and spellcheckers */
typedef struct RelBranch_tag {
  short seg_alt_number;
  short rec_alt_number;
} RelBranch;
extern unsigned char rel_word_value(const RelGraph *relg, const RelResult *relr,
  int rel_graph_len, int rel_result_len, int rel_graph_start, 
  const RelBranch *rel_branch, int rel_branch_len);


/****************************************************************************
*
*  module REP
*
****************************************************************************/

typedef struct RepData_tag {
  ReInit init;             /* set before calling rep_init() */

  /*** parameters for rep_do() - set after calling rep_init() */
  ReParm parm;             /* defaults will be set by rep_init() */

  /*** representation of image data */
  ReImage image;           /* image to be recognized */

  /*** recognition control */
  long  busy;              /* !=0: recognition under progress */
  INT_PTR hWND_rep_finished; /* window to send WM_USER message with */
                             /*   wParam == REP_MESSAGE */
                             /*   wParam |= REX_ERROR in case of error */
                             /*   wParam |= REX_IMGREADY after preprocessing */
                             /*   wParam |= REX_FINISHED after recognition */
                             /*   lParam == &repdata */
                             /* after execution (Windows only) */
  INT_PTR hWND_line_message; /* window to send WM_USER messages with results */
                             /* after recognition of every line */
                             /*   wParam == REP_LINE_CNT or */
                             /*   wParam == REP_RESULT */
                             /* (Windows only) */
  long  filler1;             /* 8 byte alignment */

  /*** used to get results from every internal call of rec_do() or rel_do()*/
  KADMOS_ERROR (KADMOS_API *rec_hook)(RecData *);  /* executed after rec_do() */
  KADMOS_ERROR (KADMOS_API *rel_hook)(RelData *);  /* executed after rel_do() */

  /*** results of rep_do() */
  short rep_deskew;          /* corrected skew of the image */
  short rep_result_len;      /* number of items in rep_result */
  RepResult *rep_result;     /* results of line recognition */
} RepData;

extern KADMOS_ERROR KADMOS_API rep_init         (RepData *, const char *file_title);
extern KADMOS_ERROR KADMOS_API rep_winit        (RepData *, const wchar_t *file_title);
extern KADMOS_ERROR KADMOS_API rep_info         (RepData *);
extern KADMOS_ERROR KADMOS_API rep_filetitle    (const RepData *, char *file_title); /* char, wchar_t */
extern KADMOS_ERROR KADMOS_API rep_do           (RepData *);
extern KADMOS_ERROR KADMOS_API rep_group_labels (RepData *);
extern KADMOS_ERROR KADMOS_API repr_group_labels(RepData *, RepResult *);
extern KADMOS_ERROR KADMOS_API rep_end          (RepData *);


/****************************************************************************
*
*  module RESPELL
*
*  Created with support from Milind Joshi, CK Pradeep
*  IDEA TECHNOSOFT INC. info@ideatechnosoft.com, http://www.ideatechnosoft.com
*
*  For dictionary support, this software (KADMOS.DLL, RESPELL.LIB) is
*  based in part on the open source ispell software.
*  http://www.gnu.org/software/ispell/ispell.html
*
*  For Hunspell dictionary support, the file HUNSPELL.DLL is from the open 
*  source Hunspell software. You may obtain a copy of the License at 
*  http://www.mozilla.org/MPL/.
*
****************************************************************************/

typedef struct ReSpellInit_tag {  

  /*** set before calling respell_init() */

  /* initialization of ispell dictionary search */
  long  ispell_config;             /* ispell configuration - not yet used */

  /* initialization of respell */
  short respell_maxlen;            /* maximum length of legal words */

  short rel_graph_out_maxlen;      /* maximum number of items in rel_graph_out */
  short rel_result_out_maxlen;     /* maximum number of items in rel_result_out */
  short result_text_maxlen;        /* maximum number of characters in result_text */
  short filler[2];                 /* 8 byte alignment */

  /*** used internally by respell_init(), respell_do(), respell_end(), and respell_filetitle() */
  INT_PTR interna1[4];             /* do not change */
  INT_PTR interna2[4];             /* do not change */
  INT_PTR interna3[4];             /* do not change */

} ReSpellInit;

#define RESPELL_ALLOW_COMPOUND_WORDS  0x00000001

typedef struct ReSpellParm_tag {

  long  respell_config;            /* RESPELL_ALLOW_COMPOUND_WORDS ispell only, default */
  long  respell_font;              /* restriction for dictionary use, see FONT_* makros */
  short rel_alternative_maximum;   /* maximum of word alternatives (1...SEG_ALT); default 1 */ 
  short filler1;                   /* 8 byte alignment */

  /* parameters as in re*_textline() to provide the result text */
  unsigned char reject_level;
  unsigned char filler2;           /* 8 byte alignment */
  short reject_char;
  long text_format;

} ReSpellParm;

typedef struct ReSpellData_tag {
  ReSpellInit init;           /* set before calling respell_init() */

  /*** parameters for respell_do() - set after calling respell_init() */
  ReSpellParm parm;           /* defaults are set by respell_init() */

  /* input data */
  short rel_graph_in_len;     /* number of items in rel_graph_in */
  short rel_result_in_len;    /* number of items in rel_result_in */

  const RelGraph *rel_graph_in;
  const RelResult *rel_result_in;
  long  rel_codepage;
  
  /*** use a private/oem spellchecker for respell_do() */
  KADMOS_ERROR (KADMOS_API *oem_spell_lookup)(const char *word, char *results, int results_size);
  long  oem_codepage;         /* code page of the private/oem spellchecker */ 
  char  oem_wordchars[512];   /* all characters of all words of the private/oem spellchecker */
  short oem_reject_char;      /* reject character of the private/oem spellchecker */
  short filler1[3];           /* 8 byte alignment */

  /*** call rel_do() again internally for every single word which hasn't been found in the dictionary */
  const RelData *rel_repeat;  /* only one of them (rel_repeat or rep_repeat) has to be set if required */
                              /* rel_repeat has to contain the recognition results for rel_graph_in, rel_result_in */
  const RepData *rep_repeat;  /* with rel_repeat or rep_repeat, PREP_INPLACE must NOT be set in the related .parm.prep */
  RepResult     *repr;        /* recognition results for recognized text line in case of rep_repeat */

  /*** number of results of respell_do() */
  short rel_graph_out_len;
  short rel_result_out_len;
  short result_text_len;
  short filler2;              /* 8 byte alignment */

  /*** results of respell_do(), but allocate before first call of respell_do() */
  RelGraph *rel_graph_out; 
  RelResult *rel_result_out;
  void *result_text;      
  void *filler3;              /* 8 byte alignment */

} ReSpellData;

extern KADMOS_ERROR KADMOS_API respell_init       (ReSpellData *, const char *file_title);
extern KADMOS_ERROR KADMOS_API respell_winit      (ReSpellData *, const wchar_t *file_title);

extern KADMOS_ERROR KADMOS_API respell_do         (ReSpellData *);
  /* if rel_graph_in, rel_result_in are generated with OPTIONS_BASICLABELS_RESULT, */
  /* then call re*_group_labels() before calling respell_do() */

extern KADMOS_ERROR KADMOS_API respell_lookup     (ReSpellData *, long code, const char *word, 
                                                   char *results, int results_size, 
                                                   long text_format);
  /* text_format for word and results can be 0 or TEXT_FORMAT_KADMOS_MULTIBYTE (see below) */

extern KADMOS_ERROR KADMOS_API respell_end        (ReSpellData *);

extern KADMOS_ERROR KADMOS_API respell_filetitle  (ReSpellData *, char *file_title); /* char, wchar_t */
extern long         KADMOS_API respell_codepage   (ReSpellData *);
extern KADMOS_ERROR KADMOS_API respell_wordchars  (ReSpellData *, char *wordchars, int wordchars_size);
extern short        KADMOS_API respell_reject_char(ReSpellData *);

extern KADMOS_ERROR KADMOS_API respell_freeimages (ReSpellData *); /* see TYPO_KEEPIMG */


/****************************************************************************
*
*  module RE_CDO
*
*  Color-drop-out
*  generates gray images and layered binary images from color images
*  as well as a list of rectangles of the lines of the images
*
****************************************************************************/

typedef struct ReCdoLine_tag {  
  ReRect rect;
  short aoi_number;
  short order_number;
  unsigned char mask;
  unsigned char gray;
  unsigned char filler[2];  /* 8 byte alignment */
} ReCdoLine;

typedef struct ReCdoInit_tag {  

  short result_maxlen;  /* maximum number of items in result_line */
  short filler[3];      /* 8 byte alignment */

  /*** used internally by re_cdo_init(), re_cdo_do(), and re_cdo_end() */
  INT_PTR interna[4];   /* do not change */

} ReCdoInit;

typedef struct ReCdoData_tag {
  ReCdoInit init; 

  /*** parameters for re_cdo_do() - set after calling re_cdo_init() */
  ReParm parm;                 /* defaults are set by re_cdo_init() */

  /*** areas of interest - set before calling re_cdo_do() */
  ReRect *aoi_rect; /* NULL means full image */
  short aoi_len; /* number of items in aoi_rect */

  /* input data */
  ReImage color_image_in;
 
  /* output data */
  ReImage gray_image_out;
  ReImageHandle h_gray_image_out; /* allocation handle of gray_image_out */

  ReImage result_image_out;
  ReImageHandle h_result_image_out; /* allocation handle of result_image_out */

  short result_len;         /*** number of results of re_cdo_do() */
  ReCdoLine *result_line;   /*** results of re_cdo_do(), but allocate before first call of re_cdo_do() */

} ReCdoData;

extern KADMOS_ERROR KADMOS_API re_cdo_init  (ReCdoData *, const char *rec_file_title);
extern KADMOS_ERROR KADMOS_API re_cdo_winit (ReCdoData *, const wchar_t *rec_file_title);

extern KADMOS_ERROR KADMOS_API re_cdo_do    (ReCdoData *);

extern KADMOS_ERROR KADMOS_API re_cdo_end   (ReCdoData *);


/****************************************************************************
*
*  module RE_LAYOUT
*
*  Layout analysis
*  Finds the text area in a given image after deskewing 
*  returns a list of subimages containing text paragraphs, images, or grids
*
****************************************************************************/

#define RESULT_FLAG_NOTEXT 0
#define RESULT_FLAG_GRID   1
#define RESULT_FLAG_TEXT   2

typedef struct ReLayoutResult_tag {
  long left, top;                    /* position of subimage in the deskewed input image */
  long width, height;
  
  long result_flag;                  /* RESULT_FLAG_NOTEXT, RESULT_FLAG_GRID, RESULT_FLAG_TEXT, ... */ 
  long filler;                       /* 8 byte alignment */
  ReImageHandle h_result_image;      /* image handle of segmented subimage as no text or as text image for rep_do() */
                                     /* call re_imagehandle2image() to get the result_image */
                                     /* call re_freeimage(*.h_result_image) to free memory */
} ReLayoutResult;

typedef struct ReLayoutData_tag {  
  ReImage image_in;                  /* image to be analyzed */

  /*** results of relayout() */
  ReImage image_deskewed;            /* deskewed image */
  ReImageHandle h_image_deskewed;    /* image handle of deskewed image if re_layout_deskew!=0; call re_freeimage(*.h_image_deskewed) to free memory */
  ReRect content;                    /* found area of interest */ 

  short re_layout_deskew;            /* corrected skew of image_in */

  short re_layout_result_max_len;    /* number of allocated items in re_layout_result */
  short re_layout_result_len;        /* number of used items in re_layout_result */
  short filler;                      /* 8 byte alignment */

  ReLayoutResult *re_layout_result;  /* results of layout analysis - call free(*.re_layout_result) to free memory */
} ReLayoutData;

extern KADMOS_ERROR KADMOS_API re_layout(ReLayoutData *);


/****************************************************************************
*
*  APIs to support multithreading:
*    re_ClearThreadData()
*  APIs to copy, save, or restore recognition parameters:
*    re_copyparm(), re_readparm(), re_readparm2(), re_readparm3(), re_writeparm(), re_writeparm2(), re_writeparm3() 
*    re_wreadparm(), re_wreadparm2(), re_wreadparm3(), re_wwriteparm(), re_wwriteparm2(), re_wwriteparm3() 
*  APIs to config recognizer (Windows only, reconfig.dll is required):
*    rec_config(), rel_config(), rel_config2(), rel_config3()
*    rep_config(), rep_config2(), rep_config3(), re_config_close()
*  APIs to read and write image files (kadmos.dll or reimageio.lib is required):         
*    re_openimagefile(), re_wopenimagefile() - open an image file
*    re_readimagefile() - read from an open image file into ReImage data
*    re_writeimagefile() - write a ReImage into an open image file
*    re_endofimagefile() - check end of file
*    re_closeimagefile() - close the image file
*    re_readimage(), re_wreadimage() - read single/first image into ReImage data
*    re_writeimage(), re_wwriteimage() - write one single ReImage into an image file
*  APIs to convert images from and to bitmap data:
*    re_image2bmp() - to convert ReImage data into bitmaps
*    re_bmp2image() - to convert bitmaps into ReImage data
*  APIs to read/write images from/to the clipboard:
*    re_clipboard2image() - get the clipboard content into ReImage data
*    re_image2clipboard() - place a ReImage on the clipboard
*  APIs to convert images from and to HBITMAP data:
*    re_image2hbitmap() - to convert ReImage data into HBITMAP data
*    re_hbitmap2image() - to convert HBITMAP data into ReImage data
*  API to get the image from an image handle:
*    re_imagehandle2image() - extract the content of an image handle  
*  API to free allocated memory from all the image functions above:
*    re_freeimage() - free allocated memory of images
*  APIs to manipulate a ReImage:
*    re_createimage()
*    re_cloneimage()
*    re_subimage()
*    re_rotateimage()
*    re_getpixel()
*    re_setpixel()
*    re_fillimage()
*  API to handle/expand ligatures:
*    code_expand_lig()
*  APIs to get recognized text string(s):
*    rel_(w)textline(), repr_(w)textline(), rep_(w)textline()
*
****************************************************************************/

/****************************************************************************
*
*  For PNG support, this software (kadmos.dll, reimageio.lib) uses
*  portions of libpng. Copyright 2000-2002, Glenn-Randers
*
*  For JPEG support, this software (kadmos.dll, reimageio.lib) is based in
*  part on the work of the Independent JPEG Group.
*
*  For TIFF support, this software (kadmos.dll, reimageio.lib) is based in part 
*  on work of Sam Leffler, Silicon Graphics Inc., and the University of California.
*    Copyright (c) 1988-1997 Sam Leffler 
*    Copyright (c) 1991-1997 Silicon Graphics, Inc.
*    Copyright (c) 1985, 1986 The Regents of the University of California
*
****************************************************************************/

/* support for multithreading - call directly before terminating a thread */
extern KADMOS_ERROR KADMOS_API re_ClearThreadData(void);

/* copy recognition parameters */
extern KADMOS_ERROR KADMOS_API re_copyparm(const ReParm *source, ReParm *destination);

/* save, restore recognition parameters */
extern KADMOS_ERROR KADMOS_API re_readparm(ReParm *, 
       unsigned char *reject_level, const char *file_title, const char *section);
extern KADMOS_ERROR KADMOS_API re_wreadparm(ReParm *, 
       unsigned char *reject_level, const wchar_t *file_title, const char *section);

extern KADMOS_ERROR KADMOS_API re_readparm2(ReParm *, ReSpellParm *, 
       unsigned char *reject_level, const char *file_title, const char *section);
extern KADMOS_ERROR KADMOS_API re_wreadparm2(ReParm *, ReSpellParm *, 
       unsigned char *reject_level, const wchar_t *file_title, const char *section);

extern KADMOS_ERROR KADMOS_API re_readparm3(ReParm *, ReSpellParm *, ReSpellParm *, 
       unsigned char *reject_level, const char *file_title, const char *section);
extern KADMOS_ERROR KADMOS_API re_wreadparm3(ReParm *, ReSpellParm *, ReSpellParm *,
       unsigned char *reject_level, const wchar_t *file_title, const char *section);

extern KADMOS_ERROR KADMOS_API re_writeparm(const ReParm *, 
       unsigned char reject_level, const char *file_title, const char *section);
extern KADMOS_ERROR KADMOS_API re_wwriteparm(const ReParm *, 
       unsigned char reject_level, const wchar_t *file_title, const char *section);

extern KADMOS_ERROR KADMOS_API re_writeparm2(const ReParm *, const ReSpellParm *, 
       unsigned char reject_level, const char *file_title, const char *section);
extern KADMOS_ERROR KADMOS_API re_wwriteparm2(const ReParm *, const ReSpellParm *, 
       unsigned char reject_level, const wchar_t *file_title, const char *section);

extern KADMOS_ERROR KADMOS_API re_writeparm3(const ReParm *, const ReSpellParm *, const ReSpellParm *, 
       unsigned char reject_level, const char *file_title, const char *section);
extern KADMOS_ERROR KADMOS_API re_wwriteparm3(const ReParm *, const ReSpellParm *, const ReSpellParm *,
       unsigned char reject_level, const wchar_t *file_title, const char *section);

#if (defined(_WIN32) || defined(_WIN64)) && !defined(_WIN32_WCE)
  /* dialog to set recognition parameters (config classifier) */
  extern KADMOS_ERROR KADMOS_API rec_config(RecData *, unsigned char *reject_level);
  extern KADMOS_ERROR KADMOS_API rel_config(RelData *, unsigned char *reject_level);
  extern KADMOS_ERROR KADMOS_API rep_config(RepData *, unsigned char *reject_level);

  extern KADMOS_ERROR KADMOS_API rel_config2(RelData *, ReSpellData *, unsigned char *reject_level);
  extern KADMOS_ERROR KADMOS_API rep_config2(RepData *, ReSpellData *, unsigned char *reject_level);

  extern KADMOS_ERROR KADMOS_API rel_config3(RelData *, ReSpellData *, ReSpellData *, unsigned char *reject_level);
  extern KADMOS_ERROR KADMOS_API rep_config3(RepData *, ReSpellData *, ReSpellData *, unsigned char *reject_level);

  extern void KADMOS_API re_config_close(void);
  extern KADMOS_ERROR KADMOS_API about_kadmos(void);
  /* the above functions are implemented in a separate DLL, */
  /* no error-handling is supported (except the return value) */
#endif

/* read/write images from/to files */
typedef void *ReFileHandle;

#if ((defined(_WIN32) || defined(_WIN64)) && !defined(_WIN32_WCE)) || defined(_CONSOLE)
  extern KADMOS_ERROR KADMOS_API GetPrivateFileName(char *file_title, 
                                   const char *filter, unsigned int mode, const char *boxtext, 
                                   const char *inifile, const char *section, const char *entry);
  /* filter, mode, boxtext=Title  see Windows function GetOpenFileName() */
  /* inifile, section, entry      see Windows function GetPrivateProfileString() */
  #if !defined(_WIN32) && !defined(_WIN64) && !defined(OF_READ)
    /* values of mode */
    #define OF_READ             0x00000000
    #define OF_CREATE           0x00001000 /* has to be set if the file doesn't exist! */
    #define OF_PROMPT           0x00002000
    #define OF_EXIST            0x00004000
  #endif
#endif

extern ReFileHandle KADMOS_API re_openimagefile(const char *file_title, const char *mode);  
extern ReFileHandle KADMOS_API re_wopenimagefile(const wchar_t *file_title, const char *mode);  
       /* mode: "r" for read, "w" for write, "a" for append */

extern ReImageHandle KADMOS_API re_readimagefile(ReFileHandle, long image_number, ReImage *image);
       /* image_number=0 for first image in file */

extern KADMOS_ERROR KADMOS_API re_writeimagefile(ReFileHandle, const ReImage *image);
       /* images are always written at the end of the file */

extern int KADMOS_API re_endofimagefile(ReFileHandle);
       /* returns 1 in case of eof, 0 else */

extern KADMOS_ERROR KADMOS_API re_closeimagefile(ReFileHandle);

/* read one or first image from a file */
extern ReImageHandle KADMOS_API re_readimage(const char *file_title, ReImage *image);
extern ReImageHandle KADMOS_API re_wreadimage(const wchar_t *file_title, ReImage *image);

/* write images from ReImage data into image files */
extern KADMOS_ERROR KADMOS_API re_writeimage(const ReImage *image, const char *file_title);
extern KADMOS_ERROR KADMOS_API re_wwriteimage(const ReImage *image, const wchar_t *file_title);

/* convert images from ReImage data into bitmap data */
  /* on return, bmp_buffer points to a BITMAPINFOHEADER structure */
  /* the bitmap bits are returned at position: */
  /* (char *)bmp_buffer + (BITMAPINFOHEADER*)bmp_buffer->biSize + (BITMAPINFOHEADER*)bmp_buffer->biClrUsed * sizeof(RGBQUAD) */
extern ReImageHandle KADMOS_API re_image2bmp(const ReImage *image, void **bmp_buffer);

/* convert images from bitmap data into ReImage data */
  /* bmp_buffer is expected to point to a BITMAPINFOHEADER structure */
  /* for bmp_bits==NULL the bits are expected at position: */
  /* (char *)bmp_buffer + ((BITMAPINFOHEADER*)bmp_buffer)->biSize + ((BITMAPINFOHEADER*)bmp_buffer)->biClrUsed * sizeof(RGBQUAD) */
extern ReImageHandle KADMOS_API re_bmp2image(const void *bmp_buffer, const void *bmp_bits, ReImage *image);

#if (defined(_WIN32) || defined(_WIN64)) && !defined(_WIN32_WCE)
  /* get clipboard content */
  extern ReImageHandle KADMOS_API re_clipboard2image(ReImage *image);
  /* place an image on the clipboard */
  extern KADMOS_ERROR KADMOS_API re_image2clipboard(const ReImage *image);

  /* convert images from HBITMAP handles to ReImage data */
  extern ReImageHandle KADMOS_API re_hbitmap2image(HBITMAP hbmp, ReImage *image);
  /* convert images from ReImage data to HBITMAP handles */
  extern KADMOS_ERROR KADMOS_API re_image2hbitmap(const ReImage *image, HBITMAP *hbmp);
  /* to free the HBITMAP handle hbmp after usage, call DeleteObject(hbmp) */
#endif

/* get the image from an image handle */
extern KADMOS_ERROR KADMOS_API re_imagehandle2image(ReImageHandle, ReImage *image);

/* free the memory allocated from above functions */
extern KADMOS_ERROR KADMOS_API re_freeimage(ReImageHandle);

/* create/allocate an empty image based on the specification in the parameter 'image' */
extern ReImageHandle KADMOS_API re_createimage(ReImage *image);

/* duplicate an existing image */
extern ReImageHandle KADMOS_API re_cloneimage(const ReImage *image, ReImage *cloneimage);

/* generate a subimage of an existing image */
extern ReImageHandle KADMOS_API re_subimage(const ReImage *image, const ReRect *rect, ReImage *subimage);

/* rotate an image */
extern ReImageHandle KADMOS_API re_rotateimage(const ReImage *image, ReImage *rotateimage, int angle);

/* set or get the pixel color in pixel images */
extern KADMOS_ERROR KADMOS_API re_setpixel(ReImage *image, long x, long y, long color);
extern KADMOS_ERROR KADMOS_API re_getpixel(const ReImage *image, long x, long y, long *color);
/* fill a pixel image (or image->subimage, if specified) with color */
extern KADMOS_ERROR KADMOS_API re_fillimage(ReImage *image, long color);

/* expands ligatures if given, else copies label from src to dst */
/* returns number of (w)characters written to dst */
extern int KADMOS_API code_expand_lig(const char *src, char *dst, long code);

#define TEXT_FORMAT_ANSI              0x00000001   /* default, applies also to CODE_UNICODE or CODE_UTF_8 */
#define TEXT_FORMAT_POSTSCRIPT        0x00000002   /* not yet implemented */
#define TEXT_FORMAT_KADMOS_MULTIBYTE  0x10000000   /* return multi byte labels */
#define TEXT_FORMAT_RELRESULT_INDICES 0x80000000   /* return indices as short array, terminated by -1 */

extern KADMOS_ERROR KADMOS_API rel_textline(const RelData *, void *buffer, 
                                    int buffsize, unsigned char reject_level, 
                                    int reject_char, long text_format);
extern KADMOS_ERROR KADMOS_API repr_textline(const RepResult *, long code,
                                    void *buffer, int buffsize,
                                    unsigned char reject_level, 
                                    int reject_char, long text_format);
extern KADMOS_ERROR KADMOS_API rep_textline(const RepData *, int line_number,
                                    void *buffer, int buffsize, 
                                    unsigned char reject_level, 
                                    int reject_char, long text_format);


/****************************************************************************
*
*  APIs to collect images
*
****************************************************************************/

/* call before first call of rec_do(), rel_do(), or rep_do() */
extern KADMOS_ERROR KADMOS_API re_collect_init(const char *image_file_title,
                                               unsigned char save_value_min, 
                                               unsigned char save_value_max);
extern KADMOS_ERROR KADMOS_API re_collect_winit(const wchar_t *image_file_title,
                                                unsigned char save_value_min, 
                                                unsigned char save_value_max);

/* call after rec_do() or set as rec_hook before calling rel_do(), rep_do() */
extern KADMOS_ERROR KADMOS_API rec_collect_kernel(const RecData  *rec);
/* call after rel_do() or set as rel_hook before calling rep_do(), */
/* but specify TYPO_KEEPIMG to get correct images */
extern KADMOS_ERROR KADMOS_API rel_collect_kernel(const RelData  *rel);

/* call to stop collection and to close the image file correctly */
extern KADMOS_ERROR KADMOS_API re_collect_end(void);


#if (defined(_WIN32) || defined(_WIN64)) && !defined(_WIN32_WCE)
/****************************************************************************
*
*  hook engine interface
*
*    If KadmosHookEngine is empty, then under Windows at every call of
*    rec_init(), rel_init(), or rep_init() a hook engine is searched for,
*    loaded if found, and the flag GENERAL_HOOKENGINE_LOADED is set.
*
*    A hook engine has to be activated by setting GENERAL_HOOKENGINE_ON
*    before any call to rec_do(), rel_do(), or rep_do().
*
****************************************************************************/

typedef struct HookEngineData_tag {

  KADMOS_ERROR (KADMOS_API *HookRecDoStart)(RecData *);
  KADMOS_ERROR (KADMOS_API *HookRecDoEnd)(RecData *);

  KADMOS_ERROR (KADMOS_API *HookRelDoStart)(RelData *);
  KADMOS_ERROR (KADMOS_API *HookRelDoEnd)(RelData *);

  KADMOS_ERROR (KADMOS_API *HookRepDoStart)(RepData *);
  KADMOS_ERROR (KADMOS_API *HookRepDoEnd)(RepData *);
} HookEngineData;

extern HookEngineData KadmosHookEngine;
#endif /* (defined(_WIN32) || defined(_WIN64)) && !defined(_WIN32_WCE) */


/****************************************************************************
*
*  APIs to support Visual Basic. To use, include <wtypes.h>
*    re_ArrayToString(): convert arrays into Visual Basic 'String'
*    re_StringToArray(): convert Visual Basic 'String' into arrays
*    re_ErrorText_bas: Visual Basic version of structure re_ErrorText
*    re_GetErrorText_bas(): Visual Basic version of re_GetErrorText()
*    re_SetErrorText_bas(): Visual Basic version of re_SetErrorText()
*    re_DisplayErrorText_bas(): VB version of re_DisplayErrorText()
*    pass pointers to KADMOS functions:
*    GetPointer(): return a pointer as 'INT_PTR'
*    FunctionAddress(): return a function address as 'INT_PTR'
*    GetRe*(): copy data from an array element into a structure
*    SetRe*(): copy data from a structure into an array element
*
****************************************************************************/
#if defined(_WIN32)
#if defined(__wtypes_h__)

/* convert arrays into Visual Basic strings and vice versa */
KADMOS_ERROR KADMOS_API re_ArrayToString(void *array, BSTR *bstr, long maxlen, long array_type);
KADMOS_ERROR KADMOS_API re_StringToArray(BSTR *bstr, void *array, long maxlen, long array_type);

#define ARRAY_TYPE_BYTE  0
#define ARRAY_TYPE_WCHAR 1

#pragma pack(push)
#pragma pack(4)
typedef struct re_ErrorText_bas_tag {
 int  ident;
 BSTR program;
 BSTR text;
 BSTR systext;
 int  private_data;
} re_ErrorText_bas;
#pragma pack(pop)

KADMOS_ERROR KADMOS_API re_GetErrorText_bas(re_ErrorText_bas *errtext);
void KADMOS_API re_SetErrorText_bas(re_ErrorText_bas *errtext);
void KADMOS_API re_DisplayErrorText_bas(re_ErrorText_bas *errtext);

#endif /* defined(__wtypes_h__) */

/* pass pointers to KADMOS functions */
INT_PTR KADMOS_API GetPointer(void *p);
INT_PTR KADMOS_API FunctionAddress(INT_PTR p);

/* Get*(): copy data from an array element into a structure */
/* Set*(): copy data from a structure into an array element */

extern KADMOS_ERROR KADMOS_API GetRelGridParm(const RelGridParm*, int n, RelGridParm*);
extern KADMOS_ERROR KADMOS_API SetRelGridParm(const RelGridParm*, RelGridParm*, int n);

extern KADMOS_ERROR KADMOS_API GetRelGrid(const RelGrid*, int n, RelGrid*);
extern KADMOS_ERROR KADMOS_API SetRelGrid(const RelGrid*, RelGrid*, int n);

extern KADMOS_ERROR KADMOS_API GetRelGraph(const RelGraph*, int n, RelGraph*);
extern KADMOS_ERROR KADMOS_API SetRelGraph(const RelGraph*, RelGraph*, int n);

extern KADMOS_ERROR KADMOS_API GetRelResult(const RelResult*, int n, RelResult*);
extern KADMOS_ERROR KADMOS_API SetRelResult(const RelResult*, RelResult*, int n);

extern KADMOS_ERROR KADMOS_API GetRepResult(const RepResult*, int n, RepResult*);
extern KADMOS_ERROR KADMOS_API SetRepResult(const RepResult*, RepResult*, int n);

#endif /* defined(_WIN32) */

/****************************************************************************
*
*  APIs to support Delphi
*
*    re_CharArrayToPointer(): copy the Char Array content to memory
*    re_PointerToCharArray(): copy the memory content to Char Array
*
****************************************************************************/

extern void KADMOS_API re_CharArrayToPointer(const char *array, void *pointer, long len);
extern void KADMOS_API re_PointerToCharArray(void *pointer, char *array, long len);

#ifdef __cplusplus
}
#endif

#endif /* INC_KADMOS */

