#include <stdio.h>
#include <math.h>

/* helper function for ascii85 handling */
static int coin_encode_ascii85(const unsigned char * in, unsigned char * out);
void coin_output_ascii85(FILE * fp, 
			 const unsigned char val,
			 unsigned char * tuple,
			 unsigned char * linebuf,
			 int * tuplecnt, int * linecnt,
			 const int rowlen,
			 const int flush);
void coin_flush_ascii85(FILE * fp,
                    unsigned char * tuple,
                    unsigned char * linebuf,
                    int * tuplecnt, int * linecnt,
                    const int rowlen);


int writeToPostScript(FILE * fp,
                                       const float *printsize, 
				       const int nc, 
				       const float dpi, 
				       unsigned char * src,
				       const short *size)
{

  //const SbVec2s size = PRIVATE(this)->viewport.getViewportSizePixels();
  const float defaultdpi = 72.0f; // we scale against this value
  const short pixelsize[] = {(short)(printsize[0]*defaultdpi),
			     (short)(printsize[1]*defaultdpi)};

  const int chan = nc <= 2 ? 1 : 3;
  const short scaledsize[] = {(short) ceil(size[0]*defaultdpi/dpi),
				(short) ceil(size[1]*defaultdpi/dpi)};

//  cc_string storedlocale;
//  SbBool changed = coin_locale_set_portable(&storedlocale);

  fprintf(fp, "%%!PS-Adobe-2.0 EPSF-1.2\n");
  fprintf(fp, "%%%%BoundingBox: 0 %d %d %d\n",
          pixelsize[1]-scaledsize[1],
          scaledsize[0],
          pixelsize[1]);
  fprintf(fp, "%%%%Creator: Coin <http://www.coin3d.org>\n");
  fprintf(fp, "%%%%EndComments\n");

  fprintf(fp, "\n");
  fprintf(fp, "/origstate save def\n");
  fprintf(fp, "\n");
  fprintf(fp, "%% workaround for bug in some PS interpreters\n");
  fprintf(fp, "%% which doesn't skip the ASCII85 EOD marker.\n");
  fprintf(fp, "/~ {currentfile read pop pop} def\n\n");
  fprintf(fp, "/image_wd %d def\n", size[0]);
  fprintf(fp, "/image_ht %d def\n", size[1]);
  fprintf(fp, "/pos_wd %d def\n", size[0]);
  fprintf(fp, "/pos_ht %d def\n", size[1]);
  fprintf(fp, "/image_dpi %g def\n", dpi);
  fprintf(fp, "/image_scale %g image_dpi div def\n", defaultdpi);
  fprintf(fp, "/image_chan %d def\n", chan);
  fprintf(fp, "/xpos_offset 0 image_scale mul def\n");
  fprintf(fp, "/ypos_offset 0 image_scale mul def\n");
  fprintf(fp, "/pix_buf_size %d def\n\n", size[0]*chan);
  fprintf(fp, "/page_ht %g %g mul def\n", printsize[1], defaultdpi);
  fprintf(fp, "/page_wd %g %g mul def\n", printsize[0], defaultdpi);
  fprintf(fp, "/image_xpos 0 def\n");
  fprintf(fp, "/image_ypos page_ht pos_ht image_scale mul sub def\n");
  fprintf(fp, "image_xpos xpos_offset add image_ypos ypos_offset add translate\n");
  fprintf(fp, "\n");
  fprintf(fp, "/pix pix_buf_size string def\n");
  fprintf(fp, "image_wd image_scale mul image_ht image_scale mul scale\n");
  fprintf(fp, "\n");
  fprintf(fp, "image_wd image_ht 8\n");
  fprintf(fp, "[image_wd 0 0 image_ht 0 0]\n");
  fprintf(fp, "currentfile\n");
  fprintf(fp, "/ASCII85Decode filter\n");
  // fprintf(fp, "/RunLengthDecode filter\n"); // FIXME: add later. 2003???? pederb.
  if (chan == 3) fprintf(fp, "false 3\ncolorimage\n");
  else fprintf(fp,"image\n");

  const int rowlen = 72;
  int num = size[0] * size[1];
  unsigned char tuple[4];
  unsigned char linebuf[rowlen+5];
  int tuplecnt = 0;
  int linecnt = 0;
  int cnt = 0;
  while (cnt < num) {
    switch (nc) {
    default: // avoid warning
    case 1:
      coin_output_ascii85(fp, src[cnt], tuple, linebuf, &tuplecnt, &linecnt, rowlen, 0);
      break;
    case 2:
      coin_output_ascii85(fp, src[cnt*2], tuple, linebuf, &tuplecnt, &linecnt, rowlen, 0);
      break;
    case 3:
      coin_output_ascii85(fp, src[cnt*3], tuple, linebuf, &tuplecnt, &linecnt, rowlen, 0);
      coin_output_ascii85(fp, src[cnt*3+1], tuple, linebuf, &tuplecnt, &linecnt, rowlen, 0);
      coin_output_ascii85(fp, src[cnt*3+2], tuple, linebuf, &tuplecnt, &linecnt, rowlen, 0);
      break;
    case 4:
      coin_output_ascii85(fp, src[cnt*4], tuple, linebuf, &tuplecnt, &linecnt, rowlen, 0);
      coin_output_ascii85(fp, src[cnt*4+1], tuple, linebuf, &tuplecnt, &linecnt,rowlen, 0);
      coin_output_ascii85(fp, src[cnt*4+2], tuple, linebuf, &tuplecnt, &linecnt, rowlen, 0);
      break;
    }
    cnt++;
  }

  // flush data in ascii85 encoder
  coin_flush_ascii85(fp, tuple, linebuf, &tuplecnt, &linecnt, rowlen);

  fprintf(fp, "~>\n\n"); // ASCII85 EOD marker
  fprintf(fp, "origstate restore\n");
  fprintf(fp, "\n");
  fprintf(fp, "%%%%Trailer\n");
  fprintf(fp, "\n");
  fprintf(fp, "%%%%EOF\n");

//  if (changed) { coin_locale_reset(&storedlocale); }

  return (ferror(fp) == 0);
}


/* helper function for ascii85 handling */
static int
coin_encode_ascii85(const unsigned char * in, unsigned char * out)
{
  unsigned int data =
    ((unsigned int)(in[0])<<24) |
    ((unsigned int)(in[1])<<16) |
    ((unsigned int)(in[2])<< 8) |
    ((unsigned int)(in[3]));

  if (data == 0) {
    out[0] = 'z';
    return 1;
  }
  out[4] = (unsigned char) (data%85 + '!');
  data /= 85;
  out[3] = (unsigned char) (data%85 + '!');
  data /= 85;
  out[2] = (unsigned char) (data%85 + '!');
  data /= 85;
  out[1] = (unsigned char) (data%85 + '!');
  data /= 85;
  out[0] = (unsigned char) (data%85 + '!');
  return 5;
}

void
coin_output_ascii85(FILE * fp,
                    const unsigned char val,
                    unsigned char * tuple,
                    unsigned char * linebuf,
                    int * tuplecnt, int * linecnt,
                    const int rowlen,
                    const int flush)
{
  int i;
  if (flush) {
    /* fill up tuple */
    for (i = *tuplecnt; i < 4; i++) tuple[i] = 0;
  }
  else {
    tuple[(*tuplecnt)++] = val;
  }
  if (flush || *tuplecnt == 4) {
    if (*tuplecnt) {
      int add = coin_encode_ascii85(tuple, linebuf + *linecnt);
      if (flush) {
        if (add == 1) {
          for (i = 0; i < 5; i++) linebuf[*linecnt + i] = '!';
        }
        *linecnt += *tuplecnt + 1;
      }
      else *linecnt += add;
      *tuplecnt = 0;
    }
    if (*linecnt >= rowlen) {
      unsigned char store = linebuf[rowlen];
      linebuf[rowlen] = 0;
      fprintf(fp, "%s\n", linebuf);
      linebuf[rowlen] = store;
      for (i = rowlen; i < *linecnt; i++) {
        linebuf[i-rowlen] = linebuf[i];
      }
      *linecnt -= rowlen;
    }
    if (flush && *linecnt) {
      linebuf[*linecnt] = 0;
      fprintf(fp, "%s\n", linebuf);
    }
  }
}

void
coin_flush_ascii85(FILE * fp,
                   unsigned char * tuple,
                   unsigned char * linebuf,
                   int * tuplecnt, int * linecnt,
                   const int rowlen)
{
  coin_output_ascii85(fp, 0, tuple, linebuf, tuplecnt, linecnt, rowlen, 1);
}
