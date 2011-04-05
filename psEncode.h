#ifndef PSENCODE_H
#define PSENCODE_H

#ifdef __cplusplus
extern "C" {
#endif

extern int writeToPostScript(FILE * fp,
                                       const float *printsize, 
				       const int nc, 
				       const float dpi, 
				       unsigned char * src,
				       const short *size);

#ifdef __cplusplus
}
#endif


#endif
