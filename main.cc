//
//  main.cc
//  Snake
//
//  Created by Javier Saldana on 4/9/13.
//  Last Updated by Javier Saldana on 4/26/13.
//  a00618475
//

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <string>
#include <sstream>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "image.h"
#include "snake.h"

Snake* player;

/*
   Ventana
******************************/

// Límites del viewport
double minX = -1.0, maxX = 1.0;
double minY = -1.0, maxY = 1.0;

// Ancho y alto de la ventana
double width = 640;
double height = 480;


/*
 Unidades de movimiento
 ******************************/

// Tamaño de cada unidad de espacio
double unitSize = 10;

// Unidades por fila y por columna
int unitsPerRow = width / unitSize;
int unitsPerCol = height / unitSize;


/*
 Variables 2D
 ******************************/

// Límites del Marco
int limitX = unitsPerRow;
int limitY = unitsPerCol;


/*
 Variables compartidas 2D/3D
 ******************************/

// Marcador
int score = 0;
int scoreMultiplier = 1;

// Timer
double timerTick = 65;
double timerMultiplier = 0.8;
double speed = 1.0;

// Dirección de movimiento
int dirX = 1; // X=1 Derecha, X=-1 Izq.
int dirY = 0; // Y=1 Arriba,  Y=-1 Abajo

// Flag para desplegar el mapa
bool showMap = false;

// Guarda nombre de la textura
static GLuint texName[36];

// Menu
typedef enum { SPEED1, SPEED2, SPEED3,
  PERSP1, PERSP2, SALIR } opcionesMenu;

bool showSplashScreen = true;

/*
 Manzana
 ******************************/

// Posición
int appleX, appleY;

// Angulo de rotación
int appleAngle = 0;


/*
 Serpiente
 ******************************/

// Flag para el evento "crece"
int crece = 0;


/** -- Fin de las variables -- **/


/********************************************************************************
 ********************************************************************************
 **                                                                            **
 ** Snake 3D                                                                   **
 **                                                                            **
 ** Juego clásico de Snake, pero ahora con vista en 3D.                        **
 **                                                                            **
 ** Controles:                                                                 **
 **   - Flechas: Movimiento                                                    **
 **   - P: cambiar de perspectiva (2D/3D)                                      **
 **   - M: mostrar/esconder minimapa (sólo aplica para perspectiva 3D)         **
 **                                                                            **
 ** El tablero mide 64x48 unidades, la serpiente se mueve una unidad           **
 ** por cada *tick* del timer. Se usa esta posición para mostrar la serpiente  **
 ** tanto en 2D como en 3D.                                                    **
 **                                                                            **
 ** Hay dos vistas del juego:                                                  **
 **   1. 2D (clásica)                                                          **
 **   2. 3D (con minimapa)                                                     **
 **                                                                            **
 ** Las coordenadas (x, y) de la ventana están en el rango                     **
 ** de -1.0 a 1.0, respectivamente.                                            **
 **                                                                            **
 ** 2D: Se divide el total de unidades horizontales entre el rango del ancho,  **
 ** y las verticales entre el rango del alto, para obtener las coordenadas.    **
 **                                                                            **
 ** 3D: Se utilizan las mismas coordenadas de la serpiente,                    **
 ** pero se dibuja una serie de cubos que la componen.                         **
 **                                                                            **
 ********************************************************************************
 ********************************************************************************/

void loadTexture(Image* image,int k) {
  glBindTexture(GL_TEXTURE_2D, texName[k]); // Tell OpenGL which texture to edit

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);

  // Map the image to the texture
  glTexImage2D(GL_TEXTURE_2D,               // Always GL_TEXTURE_2D
               0,                           // 0 for now
               GL_RGB,                      // Format OpenGL uses for image
               image->width, image->height, // Width and height
               0,                           // The border of the image
               GL_RGB,                      // GL_RGB, because pixels are stored in RGB format
               GL_UNSIGNED_BYTE,            // GL_UNSIGNED_BYTE, because pixels are stored
                                            //   as unsigned numbers
               image->pixels);              // The actual pixel data
}

// Reads a bitmap image from a file
Image* loadBMP(const char* filename);

void onMenu(int opcion) {
	switch(opcion) {

    // Casos Velocidad
    case SPEED1:
      speed = 1.0;
      break;
    case SPEED2:
      speed = 2.0;
      break;
    case SPEED3:
      speed = 3.0;
      break;

    // Casos Perspectiva
    case PERSP1:
      showMap = false;
      break;
    case PERSP2:
      showMap = true;
      break;

    // Salir
    case SALIR:
      exit(0);
      break;
	}
	glutPostRedisplay();
}

void initMenu(void) {
	int menuVelocidad, menuPerspectiva, menuPrincipal;

	menuVelocidad = glutCreateMenu(onMenu);
	glutAddMenuEntry("Principiante", SPEED1);
	glutAddMenuEntry("Intermedio", SPEED2);
  glutAddMenuEntry("Avanzado", SPEED3);

	menuPerspectiva = glutCreateMenu(onMenu);
	glutAddMenuEntry("3D", PERSP1);
	glutAddMenuEntry("2D", PERSP2);

	menuPrincipal = glutCreateMenu(onMenu);
	glutAddSubMenu("Nivel", menuVelocidad);
	glutAddSubMenu("Perspectiva (2D/3D)", menuPerspectiva);
  glutAddMenuEntry("Salir", SALIR);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


/*
 *
 * public: init()
 *
 * Inicialización de:
 *  - Modos de openGL
 *  - Estado del juego
 *
 */
static void init() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_FLAT);

  player = new Snake(unitsPerRow / 2, // initial position in X
                     unitsPerCol / 2, // initial position in y
                     100);            // maximum length of tail

  // Random seed
  srand((int) time(NULL));

  // Genera la Manzana por primera vez
  appleX = rand() % unitsPerRow + 1;
  appleY = rand() % unitsPerCol + 1;

  // Crea menu
  initMenu();

  // Texturas
  glGenTextures(2, texName);
  Image* image;

  image = loadBMP("/Users/javier/Documents/Tec/Graficas Computacionales/snake/snake/snake.bmp");
  loadTexture(image, 0);

  image = loadBMP("/Users/javier/Documents/Tec/Graficas Computacionales/snake/snake/apple.bmp");
  loadTexture(image, 1);

  delete image;
}


/*
 *
 * public: xPos2d(double x)
 *
 * x - posición en x del tablero
 * Regresa la posición x para desplegarse en 2D
 *
 *  Ejemplos:
 *
 *    - cuando x = 64, regresa 1.0
 *    - cuando x = 0, regresa -1.0
 *    - cuando x = 32, regresa 0.0
 *
 */
double xPos2d(int x) {
  double wide = maxX - minX;
  double mappedX = x * (wide / unitsPerRow);

  return minX + mappedX;
}

/*
 *
 * public: yPos2d(double x)
 *
 * y - posición en y del tablero
 * Regresa la posición y para desplegarse en 2D
 *
 *  Ejemplos:
 *
 *    - cuando y = 48, regresa 1.0
 *    - cuando y = 0, regresa -1.0
 *    - cuando y = 24, regresa 0.0
 *
 */
double yPos2d(int y) {
  double tall = maxY - minY;
  double mappedY = y * (tall / unitsPerCol);

  return minY + mappedY;
}

// Dibuja un String
void drawString (void *font, const char *s, float x, float y) {
  unsigned int i;
  glRasterPos2f(x, y);

  for (i = 0; i < strlen (s); i++)
    glutBitmapCharacter (font, s[i]);
}

void draw3dString (void *font, const char *s, float x, float y, float z) {
  unsigned int i;

  glPushMatrix();

  glTranslatef(x, y, z);
  glScaled(0.0005, 0.0005, 0.0005);

  for (i = 0; i < strlen (s); i++)
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, s[i]);

  glPopMatrix();
}

void drawSplashScreen() {
  glClear(GL_COLOR_BUFFER_BIT);

  glColor3f(0.0, 1.0, 0.0);
  glLineWidth(1);

  std::stringstream ss; // Helper para desplegar el marcador

  ss << "Snake 3D version 4.22 (64-bit)";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 1.08);
  ss.str("");
  ss.clear();

  ss << "Copyright (C) 2004 Javier Gerardo Trevino Saldana";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 1.0);
  ss.str("");
  ss.clear();

  ss << "Licensed under the GNU General Public License version 2 only";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.92);
  ss.str("");
  ss.clear();

  ss << "Mac OS X 10.8.3 (12C60) running in multiuser mode";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.76);
  ss.str("");
  ss.clear();

  ss << "Memory Page Size: 4096";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.68);
  ss.str("");
  ss.clear();

  ss << "System has 8 Intel core(s) with SSE";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.60);
  ss.str("");
  ss.clear();

  ss << "Running 2 test sequences... (CTRL-C to quit)";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.44);
  ss.str("");
  ss.clear();

  ss << "Running tests on full 12899MB region...";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.28);
  ss.str("");
  ss.clear();

  ss << "  Trabajo Final entregado                      : ok";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.20);
  ss.str("");
  ss.clear();

  ss << "  Materia de Graficas Computacionales aprobada : ok";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.12);
  ss.str("");
  ss.clear();

  ss << "Starting game...";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, -0.04);
  ss.str("");
  ss.clear();

  ss << "  Loading Classes                              : ok";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, -0.12);
  ss.str("");
  ss.clear();

  ss << "  Applying Textures                            : ok";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, -0.20);
  ss.str("");
  ss.clear();

  ss << "  Turning Lights on                            : ok";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, -0.28);
  ss.str("");
  ss.clear();

  ss << "Game Loading Complete...";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, -0.44);
  ss.str("");
  ss.clear();

  ss << "Press [ENTER] to Play";
  drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, -0.60);
  ss.str("");
  ss.clear();

}

static void drawMap(void) {
  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  glDisable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT);

  // Dibuja la Cuadrícula
  glColor3f(0.2, 0.0, 0.2);
  glLineWidth(1);

  glBegin(GL_LINES);

  for(int i = 0; i <= unitsPerRow; i += 2) {
    glVertex2d(minX, xPos2d(i));
    glVertex2f(maxX, xPos2d(i));
    glVertex2d(xPos2d(i), minY);
    glVertex2f(xPos2d(i), maxY);
  }

  glEnd();

  // Dibuja el Marco
  glColor3f(0.6, 0.0, 0.6);
  glLineWidth(3);

  glBegin(GL_LINE_LOOP);
  glVertex2f(minX, minY);
  glVertex2f(minX, maxY);
  glVertex2f(maxX, maxY);
  glVertex2f(maxX, minY);
  glEnd();

  glLineWidth(1);

  // Dibuja la Manzana
  glColor3f(1.0, 0.0, 0.0);
  glPointSize(6);

  glBegin(GL_POINTS);
  glVertex2f(xPos2d(appleX), yPos2d(appleY));
  glEnd();

  glPointSize(1);

  // Dibuja la Serpiente
  glColor3f(1.0, 1.0, 0.0);
  glLineWidth(unitSize);

  glBegin(GL_LINE_STRIP);
  for (int i = player->length - 1; i >= 0; i--) {
    glVertex2f(xPos2d(player->xAt(i)), yPos2d(player->yAt(i)));
  }
  glEnd();

  glLineWidth(1);

  // Dibuja el Marcador
  glColor3f(1.0, 1.0, 1.0);

  std::stringstream ss; // Helper para desplegar el marcador
  ss << "Score: " << std::to_string(score);
  drawString(GLUT_BITMAP_9_BY_15, ss.str().c_str(), -0.85, -0.85);
}

static void drawPerspective(void) {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  GLfloat ambientLight[] = {0.5f, 0.4f, 0.4f, 1.0f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

  GLfloat directedLight[] = {0.7f, 0.7f, 0.7f, 1.0f};
  GLfloat directedLightPos[] = {-10.0f, 15.0f, 20.0f, 0.0f};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, directedLight);
  glLightfv(GL_LIGHT0, GL_POSITION, directedLightPos);

  // Dibuja la Cuadrícula
  glColor3f(0.2, 0.0, 0.2);
  glLineWidth(1);

  glBegin(GL_LINES);

  for(double i = 0; i <= unitsPerRow; i += 2) {
    glVertex2d(minX, xPos2d(i));
    glVertex2f(maxX, xPos2d(i));
    glVertex2d(xPos2d(i), minY);
    glVertex2f(xPos2d(i), maxY);
  }

  glEnd();

  // Dibuja el Marco
  glColor3f(0.6, 0.0, 0.6);

  glPushMatrix();
  glTranslated(maxX + 0.05, 0.0, 0.0);
  glScaled(1.0, 1.0 * 41, 1.0);
  glutSolidCube(0.05);
  glPopMatrix();

  glPushMatrix();
  glTranslated(minX - 0.05, 0.0, 0.0);
  glScaled(1.0, 1.0 * 41, 1.0);
  glutSolidCube(0.05);
  glPopMatrix();

  glPushMatrix();
  glTranslated(0.0, minY - 0.05, 0.0);
  glScaled(1.0 * 43, 1.0, 1.0);
  glutSolidCube(0.05);
  glPopMatrix();

  glPushMatrix();
  glTranslated(0.0, maxY + 0.05, 0.0);
  glScaled(1.0 * 43, 1.0, 1.0);
  glutSolidCube(0.05);
  glPopMatrix();

  glEnable(GL_TEXTURE_2D);

  // Dibuja la Manzana
  glColor3f(1.5, 0.0, 0.0);

  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);

  glBindTexture(GL_TEXTURE_2D, texName[1]);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

  glPushMatrix();
  glTranslated(xPos2d(appleX), yPos2d(appleY), 0.025);
  glRotated(appleAngle, 0.3, 1.0, 0.0);
  glutSolidCube(0.05);
  glPopMatrix();

  // Dibuja la Serpiente
  glColor3f(0.8, 1.0, 0.0);

  glBindTexture(GL_TEXTURE_2D, texName[0]);

  for (int i = player->length - 1; i >= 0; i--) {
    glPushMatrix();
    glTranslated(xPos2d(player->xAt(i)), yPos2d(player->yAt(i)), 0.025);
    glutSolidCube(0.05);
    glPopMatrix();
  }

  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_2D);

  // Dibuja el Marcador
  glColor3f(1.0, 1.0, 1.0);

  std::stringstream ss; // Helper para desplegar el marcador
  ss << "Score: " << std::to_string(score);
  draw3dString(GLUT_STROKE_MONO_ROMAN, ss.str().c_str(), -0.85, -0.85, 0.0);
}

void reshape(int w, int h) {
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 20.0);
}

static void display(void) {
  double snakeX, snakeY;

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (showSplashScreen) {

    gluLookAt(0.0, 0.0, 2.0,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);

    drawSplashScreen();

  } else if (showMap) {

    gluLookAt(0.0, 0.0, 2.0,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);

    drawMap();

  } else {

    snakeX = xPos2d(player->x());
    snakeY = yPos2d(player->y());

    snakeY = snakeY <= -0.5 ? -0.5 : snakeY;
    snakeY = snakeY >= 1.5 ? 1.5 : snakeY;

    gluLookAt(0.0, -2.0, 1,
              0.0, snakeY, 0.0,
              0.0, 0.5, 0.0);

    drawPerspective();

  }


  glutSwapBuffers();
}

// Regresa verdadero si la serpiente colisiona con un par de puntos
bool snakeHits(float x, float y) {
  double nextX, nextY;

  nextX = player->x();
  nextY = player->y();

  return nextX == x && nextY == y;
}

int counter;

void myTimer(int valor) {
  int nextX, nextY;

  nextX = player->x();
  nextY = player->y();

  // Revisa si la Serpiente colisiona con el marco
  // y cambia la dirección cuando sea necesario
  if (dirX == 1 && nextX >= unitsPerRow) {
    dirX = 0;
    dirY = 1;
  } else if (dirX == -1 && nextX <= 0) {
    dirX = 0;
    dirY = -1;
  } else if (dirY == 1 && nextY >= unitsPerCol) {
    dirY = 0;
    dirX = -1;
  } else if (dirY == -1 && nextY <= 0) {
    dirY = 0;
    dirX = 1;
  }

  appleAngle = (appleAngle >= 360) ? 0 : appleAngle + 5;

  // Crece la cola primero para que el jugador tenga mejor control
  if (crece == 1 || snakeHits(appleX, appleY)) {

    // Incrementa el score
    score += (1 * scoreMultiplier);

    if (!player->full()) {
      player->eat();
    } else {

      // o Gana y regresa a su posición, dirección y tamaño inicial
      player->reset();
      dirX = 1;
      dirY = 0;

      // Ahora cada manzana vale más
      scoreMultiplier++;

      // y se aumenta la velocidad de movimiento
      timerTick *= timerMultiplier;
    }

    appleX = rand() % unitsPerRow + 1;
    appleY = rand() % unitsPerCol + 1;

    crece = 0;
  }

  player->moveTo(dirX, dirY);

  glutPostRedisplay();
  glutTimerFunc(timerTick / speed, myTimer, 1);
}

void myKeyboard(unsigned char theKey, int mouseX, int mouseY) {
  // Cambia el valor de dirX y dirY dependiendo de la tecla que oprima el usuario.
  // Activa la bandera de crecer para que la funcion `myTimer` crezca la serpiente en una unidad.
  // Debe funcionar para mayuscula y minuscula.
  switch (theKey) {
    // Mueve la serpiente
    case 'w': case 'W':
      if (dirY != -1) { dirX = 0; dirY = 1; }
      break;
    case 's': case 'S':
      if (dirY != 1) { dirX = 0; dirY = -1; }
      break;
    case 'a': case 'A':
      if (dirX != 1) { dirX = -1; dirY = 0; }
      break;
    case 'd': case 'D':
      if (dirX != -1) { dirX = 1; dirY = 0; }
      break;

    // Crece el tamaño de la serpiente
    case 'c': case 'C':
      crece = 1;
      break;

    // Esconde/despliega el mapa
    case 'm': case 'M':
      showMap = !showMap;
      break;

    // Aumenta la velocidad
    case 'v':
      speed *= 1.1;
      break;

    // Disminuye la velocidad
    case 'V':
      speed *= 0.9;
      break;

    case 13:
      showSplashScreen = false;
      break;

    // Salir
    case 27: case 'e': case 'E':
      exit(-1);
  }
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitWindowSize(width, height);
  glutInitWindowPosition(120, 120);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("Snake 3D - ITC 2013");

  init();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(myKeyboard);
  glutTimerFunc(2000, myTimer, 1);

  glutMainLoop();
  return EXIT_SUCCESS;
}
