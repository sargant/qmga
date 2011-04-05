/******************************************************************************
** This file is part of QMGA a tool to display convex bodies.
** Copyright (C) 2005 Adrian Gabriel 
** Phillips-University of Marburg (Germany)
** qmga@users.sourceforge.net
**
** QMGA is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** QMGA is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with QMGA; if not, write to the Free Software
** Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

/* $Id: trdemo2.c,v 1.5 1999/01/24 17:25:46 brianp Exp $ */


/*
 * $Log: trdemo2.c,v $
 * Revision 1.5  1999/01/24  17:25:46  brianp
 * minor change to prevent unused variable warnings
 *
 * Revision 1.4  1997/07/21  17:35:24  brianp
 * updated for tile borders
 *
 * Revision 1.3  1997/07/21  15:42:36  brianp
 * added return 0 in main()
 *
 * Revision 1.2  1997/04/24  13:46:44  brianp
 * added free(buffer)
 *
 * Revision 1.1  1997/04/19  23:26:38  brianp
 * Initial revision
 *
 */


/*
 * Demonstration of generating a large image in tiles and writing to a
 * file incrementall.y
 * With this technique one can generate very large image files without
 * using much memory.
 *
 * Brian Paul
 * April 1997
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GL/glut.h"
#include "tr.h"


#define TILE_WIDTH 256
#define TILE_HEIGHT 256
#define TILE_BORDER 10

#define IMAGE_WIDTH 1500
#define IMAGE_HEIGHT 1100

#define FILENAME "tileimg.ppm"


#define NUMBALLS 30
static GLfloat BallPos[NUMBALLS][3];
static GLfloat BallSize[NUMBALLS];
static GLfloat BallColor[NUMBALLS][4];

static GLboolean Perspective = GL_TRUE;



/* Return random float in [0,1] */
static float Random(void)
{
   int i = rand();
   return (float) (i % 1000) / 1000.0;
}


static void PrintString(const char *str)
{
   int i;
   int n = strlen(str);
   for (i=0;i<n;i++) {
      glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
   }
}


/* Draw my stuff */
static void DrawScene(void)
{
   int i;

   glClearColor(0.4, 0.4, 0.4, 0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glEnable(GL_LIGHTING);
   glEnable(GL_DEPTH_TEST);
   for (i=0;i<NUMBALLS;i++) {
      int t;
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, BallColor[i]);
      glPushMatrix();
      glTranslatef(BallPos[i][0], BallPos[i][1], BallPos[i][2]);
      t = 12 + (int) (BallSize[i] * 12);
      glutSolidSphere(BallSize[i], t, t);
      glPopMatrix();
   }

   glDisable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);
   glColor3f(1.0, 1.0, 1.0);

   /* Print a string to be sure image isn't upside down */
   glPushMatrix();
   glTranslatef(-1.3, 0.0, 0.0);
   glScalef(.003, .003, .003);
   glLineWidth(2.0);
   PrintString("Tile Rendering");
   glPopMatrix();
}


/* Do a demonstration of tiled rendering */
static void Display(void)
{
   TRcontext *tr;
   GLubyte *buffer;
   GLubyte *tile;
   FILE *f;
   int more;
   int i;

   printf("Generating %d by %d image file...\n", IMAGE_WIDTH, IMAGE_HEIGHT);

   /* Generate random balls */
   for (i=0;i<NUMBALLS;i++) {
      BallPos[i][0] = -2.0 + 4.0 * Random();
      BallPos[i][1] = -2.0 + 4.0 * Random();
      BallPos[i][2] = -2.0 + 4.0 * Random();
      BallSize[i] = Random();
      BallColor[i][0] = Random();
      BallColor[i][1] = Random();
      BallColor[i][2] = Random();
      BallColor[i][3] = 1.0;
   }

   /* allocate buffer large enough to store one tile */
   tile = malloc(TILE_WIDTH * TILE_HEIGHT * 3 * sizeof(GLubyte));
   if (!tile) {
      printf("Malloc of tile buffer failed!\n");
      return;
   }

   /* allocate buffer to hold a row of tiles */
   buffer = malloc(IMAGE_WIDTH * TILE_HEIGHT * 3 * sizeof(GLubyte));
   if (!buffer) {
      free(tile);
      printf("Malloc of tile row buffer failed!\n");
      return;
   }

   /* Setup.  Each tile is TILE_WIDTH x TILE_HEIGHT pixels. */
   tr = trNew();
   trTileSize(tr, TILE_WIDTH, TILE_HEIGHT, TILE_BORDER);
   trTileBuffer(tr, GL_RGB, GL_UNSIGNED_BYTE, tile);
   trImageSize(tr, IMAGE_WIDTH, IMAGE_HEIGHT);
   trRowOrder(tr, TR_TOP_TO_BOTTOM);

   if (Perspective)
      trFrustum(tr, -1.0, 1.0, -1.0, 1.0, 6.0, 25.0);
   else
      trOrtho(tr, -3.0, 3.0, -3.0, 3.0, -3.0, 3.0);

   /* Prepare ppm output file */
   f = fopen(FILENAME, "w");
   if (!f) {
      printf("Couldn't open image file: %s\n", FILENAME);
      return;
   }
   fprintf(f,"P6\n");
   fprintf(f,"# ppm-file created by %s\n", "trdemo2");
   fprintf(f,"%i %i\n", IMAGE_WIDTH, IMAGE_HEIGHT);
   fprintf(f,"255\n");
   fclose(f);
   f = fopen(FILENAME, "ab");  /* now append binary data */
   if (!f) {
      printf("Couldn't append to image file: %s\n", FILENAME);
      return;
   }

   /*
    * Should set GL_PACK_ALIGNMENT to 1 if the image width is not
    * a multiple of 4, but that seems to cause a bug with some NVIDIA
    * cards/drivers.
    */
   glPixelStorei(GL_PACK_ALIGNMENT, 4);

   /* Draw tiles */
   more = 1;
   while (more) {
      int curColumn;
      trBeginTile(tr);
      curColumn = trGet(tr, TR_CURRENT_COLUMN);
      DrawScene();      /* draw our stuff here */
      more = trEndTile(tr);

      /* save tile into tile row buffer*/
      {
	 int curTileWidth = trGet(tr, TR_CURRENT_TILE_WIDTH);
	 int bytesPerImageRow = IMAGE_WIDTH*3*sizeof(GLubyte);
	 int bytesPerTileRow = (TILE_WIDTH-2*TILE_BORDER) * 3*sizeof(GLubyte);
	 int xOffset = curColumn * bytesPerTileRow;
	 int bytesPerCurrentTileRow = (curTileWidth-2*TILE_BORDER)*3*sizeof(GLubyte);
	 int i;
	 int curTileHeight = trGet(tr, TR_CURRENT_TILE_HEIGHT);
	 for (i=0;i<curTileHeight;i++) {
	    memcpy(buffer + i*bytesPerImageRow + xOffset, /* Dest */
		   tile + i*bytesPerTileRow,              /* Src */
		   bytesPerCurrentTileRow);               /* Byte count*/
	 }
      }
      
      if (curColumn == trGet(tr, TR_COLUMNS)-1) {
	 /* write this buffered row of tiles to the file */
	 int curTileHeight = trGet(tr, TR_CURRENT_TILE_HEIGHT);
	 int bytesPerImageRow = IMAGE_WIDTH*3*sizeof(GLubyte);
	 int i;
	 GLubyte *rowPtr;
         /* The arithmetic is a bit tricky here because of borders and
          * the up/down flip.  Thanks to Marcel Lancelle for fixing it.
          */
	 for (i=2*TILE_BORDER;i<curTileHeight;i++) {
	    /* Remember, OpenGL images are bottom to top.  Have to reverse. */
	    rowPtr = buffer + (curTileHeight-1-i) * bytesPerImageRow;
	    fwrite(rowPtr, 1, IMAGE_WIDTH*3, f);
	 }
      }

   }
   trDelete(tr);

   fclose(f);
   printf("%s complete.\n", FILENAME);

   free(tile);
   free(buffer);

   exit(0);
}



static void Reshape( int width, int height )
{
   glViewport( 0, 0, width, height );
   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
   if (Perspective)
      glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 25.0 );
   else
      glOrtho( -3.0, 3.0, -3.0, 3.0, -3.0, 3.0);
   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();
   if (Perspective)
      glTranslatef( 0.0, 0.0, -15.0 );
}


static void Key( unsigned char key, int x, int y )
{
   (void) x;
   (void) y;
   switch (key) {
      case 27:
         exit(0);
         break;
   }
   glutPostRedisplay();
}


static void Init( void )
{
   static GLfloat pos[4] = {0.0, 0.0, 10.0, 0.0};
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, pos);
   glEnable(GL_NORMALIZE);
   glEnable(GL_DEPTH_TEST);
}


int main( int argc, char *argv[] )
{
   glutInit( &argc, argv );
   glutInitWindowPosition(0, 0);
   glutInitWindowSize( TILE_WIDTH, TILE_HEIGHT );
   glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH );
   glutCreateWindow(argv[0]);
   Init();
   glutReshapeFunc( Reshape );
   glutKeyboardFunc( Key );
   glutDisplayFunc( Display );
   glutMainLoop();
   return 0;
}
