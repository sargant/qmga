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

/* $Id: trdemo1.c,v 1.5 1999/01/24 17:25:46 brianp Exp $ */


/*
 * $Log: trdemo1.c,v $
 * Revision 1.5  1999/01/24  17:25:46  brianp
 * minor change to prevent unused variable warnings
 *
 * Revision 1.4  1997/07/21  17:35:06  brianp
 * updated for tile borders
 *
 * Revision 1.3  1997/07/21  15:42:36  brianp
 * added return 0 in main()
 *
 * Revision 1.2  1997/04/26  21:23:44  brianp
 * added trRasterPos3f() example code
 *
 * Revision 1.1  1997/04/19  23:26:28  brianp
 * Initial revision
 *
 */


/*
 * Demonstration of using tile rendering to render an image in pieces.
 * A bunch of random colored/sized spheres are drawn.  Also, each sphere
 * has a number printed on it.  The number is printed with glBitmaps
 * and positioned with trRasterPos().
 *
 * Brian Paul
 * April 1997
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GL/glut.h"
#include "tr.h"


#define TILESIZE 100
#define BORDER 0

#define NUMBALLS 30
static GLfloat BallPos[NUMBALLS][3];
static GLfloat BallSize[NUMBALLS];
static GLfloat BallColor[NUMBALLS][4];

static GLboolean Perspective = GL_TRUE;

static int WindowWidth = 500, WindowHeight = 500;


/* Return random float in [0,1] */
static float Random(void)
{
   int i = rand();
   return (float) (i % 1000) / 1000.0;
}


static void DrawBallNumber(int k)
{
   char str[100];
   int i, n;
   sprintf(str, "%d", k);
   n = strlen(str);
   for (i=0;i<n;i++) {
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
   }
}


/* Draw my stuff */
static void DrawScene(TRcontext *tr)
{
   int i;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   for (i=0;i<NUMBALLS;i++) {
      int t;
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, BallColor[i]);
      glPushMatrix();
      glTranslatef(BallPos[i][0], BallPos[i][1], BallPos[i][2]);
      t = 12 + (int) (BallSize[i] * 12);
      glutSolidSphere(BallSize[i], t, t);

      glDisable(GL_LIGHTING);
      glColor3ub(255, 255, 255);
      trRasterPos3f(tr, 0, 0, BallSize[i]);  /* front of ball */
      DrawBallNumber(i);
      glEnable(GL_LIGHTING);

      glPopMatrix();
   }
}


/* Do a demonstration of tiled rendering */
static void Display(void)
{
   GLubyte *image;
   int tileCount = 0;
   TRcontext *tr;
   int i;

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

   /* allocate final image buffer */
   image = malloc(WindowWidth * WindowHeight * 4 * sizeof(GLubyte));
   if (!image) {
      printf("Malloc failed!\n");
      return;
   }

   /* Setup tiled rendering */
   tr = trNew();
   trTileSize(tr, TILESIZE, TILESIZE, BORDER);
   /* We don't call trTileBuffer() since we're not interested in getting
    * each individual tile's data.
    */
   trImageSize(tr, WindowWidth, WindowHeight);
   trImageBuffer(tr, GL_RGB, GL_UNSIGNED_BYTE, image);

   if (Perspective)
      trFrustum(tr, -1.0, 1.0, -1.0, 1.0, 5.0, 25.0);
   else
      trOrtho(tr, -3.0, 3.0, -3.0, 3.0, -3.0, 3.0);

   /* Draw tiles */
   do {
      trBeginTile(tr);
      DrawScene(tr);
      tileCount++;
   } while (trEndTile(tr));
   printf("%d tiles drawn\n", tileCount);

   trDelete(tr);

   /* Show final image, might otherwise write it to a file */
   /* Do this ugliness to set rasterpos to lower-left corner of window */
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0.0, WindowWidth, 0.0, WindowHeight, -1.0, 1.0);
   glRasterPos2i(0,0);

   /* Disable dithering since dithering was done when each tile was generated*/
   glDisable(GL_DITHER);
   glDrawPixels(WindowWidth, WindowHeight, GL_RGB, GL_UNSIGNED_BYTE, image);
   glEnable(GL_DITHER);

   /* restore matrices*/
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glFlush();

   free(image);
}



static void Reshape( int width, int height )
{
   WindowWidth = width;
   WindowHeight = height;
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

   /* Enable this to test wide lines and borders
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glLineWidth(4.0);
   */
}


int main( int argc, char *argv[] )
{
   glutInit( &argc, argv );
   glutInitWindowPosition(0, 0);
   glutInitWindowSize( WindowWidth, WindowHeight );
   glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH );
   glutCreateWindow(argv[0]);
   Init();
   glutReshapeFunc( Reshape );
   glutKeyboardFunc( Key );
   glutDisplayFunc( Display );
   glutMainLoop();
   return 0;
}
