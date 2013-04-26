//
//  main.cpp
//  Snake
//
//  Created by Javier Saldana on 4/9/13.
//  Last Updated by Javier Saldana on 4/23/13.
//  a00618475
//

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <sstream>
#include <time.h>
#include "imageloader.h"

using namespace std;

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
double unitsPerRow = width / unitSize;
double unitsPerCol = height / unitSize;


/*
 Variables 2D
 ******************************/

// Límites del Marco
double limitX = unitsPerRow;
double limitY = unitsPerCol;


/*
 Variables compartidas 2D/3D
 ******************************/

// Coordenadas iniciales de la serpiente
double x = unitsPerRow / 2;
double y = unitsPerCol / 2;

// Marcador
int score = 0;
int scoreMultiplier = 1;

// Timer
double timerTick = 65;
double timerMultiplier = 0.8;

// Dirección de movimiento
double dirX = 1.0; // X=1 Derecha, X=-1 Izq.
double dirY = 0.0; // Y=1 Arriba,  Y=-1 Abajo

// Flag para desplegar el mapa
bool showMap = false;

// Guarda nombre de la textura
static GLuint texName[36];


/*
 Manzana
 ******************************/

// Posición
int appleX, appleY;

// Flag para (re)generar
bool applePresent = false;


/*
 Serpiente
 ******************************/

// Flag para el evento "crece"
int crece = 0;

// Largo inicial
int largo = 2;
int largoMaximo = 100 + largo;

// Serpiente inicial
double snake[100+2][2] = {{0,0},{0,1}};


/** -- Fin de las variables -- **/


/*
 Librería para manejo de texturas
 ********************************/

void loadTexture(Image* image,int k)
{

    glBindTexture(GL_TEXTURE_2D, texName[k]); //Tell OpenGL which texture to edit

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    //Map the image to the texture
    glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
                 0,                            //0 for now
                 GL_RGB,                       //Format OpenGL uses for image
                 image->width, image->height,  //Width and height
                 0,                            //The border of the image
                 GL_RGB, //GL_RGB, because pixels are stored in RGB format
                 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
                 //as unsigned numbers
                 image->pixels);               //The actual pixel data

}


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


/*
 *
 * public: init()
 *
 * Inicialización de:
 *  - Modos de openGL
 *  - Estado del juego
 *
 */
static void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_FLAT);

    // Posición inicial de la serpiente
    snake[0][0] = x;
    snake[0][1] = y;
    snake[1][0] = x-1;
    snake[1][1] = y;

    // Random seed
    srand((int) time(NULL));

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
double xPos2d(double x) {
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
double yPos2d(double y) {
    double tall = maxY - minY;
    double mappedY = y * (tall / unitsPerCol);

    return minY + mappedY;
}

// Dibuja un String
void drawString (void *font, const char *s, float x, float y)
{
    unsigned int i;
    glRasterPos2f(x, y);

    for (i = 0; i < strlen (s); i++)
        glutBitmapCharacter (font, s[i]);
}

static void drawMap(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

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
    glLineWidth(3);

    glBegin(GL_LINE_LOOP);
    glVertex2f(minX, minY);
    glVertex2f(minX, maxY);
    glVertex2f(maxX, maxY);
    glVertex2f(maxX, minY);
    glEnd();

    glLineWidth(1);

    // (Re)Genera la Manzana
    if (!applePresent) {
        appleX = rand() % (int) unitsPerRow + 1;
        appleY = rand() % (int) unitsPerCol + 1;
        applePresent = true;
    }

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
    for (int i = largo - 1; i >= 0; i--) {
        glVertex2f(xPos2d(snake[i][0]), yPos2d(snake[i][1]));
    }
    glEnd();

    glLineWidth(1);

    // Dibuja el Marcador
    glColor3f(1.0, 1.0, 1.0);

    stringstream ss; // Helper para desplegar el marcador

    ss << "Score: " << std::to_string(score);
    drawString(GLUT_BITMAP_9_BY_15, ss.str().c_str(), -0.85, -0.85);
}

static void drawPerspective(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    glLineWidth(3);

    glBegin(GL_LINE_LOOP);
    glVertex2f(minX, minY);
    glVertex2f(minX, maxY);
    glVertex2f(maxX, maxY);
    glVertex2f(maxX, minY);
    glEnd();

    glLineWidth(1);

    // (Re)Genera la Manzana
    if (!applePresent) {
        appleX = rand() % (int) unitsPerRow + 1;
        appleY = rand() % (int) unitsPerCol + 1;
        applePresent = true;
    }

    glEnable(GL_TEXTURE_2D);

    // Dibuja la Manzana
    glColor3f(1.0, 0.0, 0.0);

    glBindTexture(GL_TEXTURE_2D, texName[1]);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPushMatrix();
    glTranslatef(xPos2d(appleX), yPos2d(appleY), 0.0);
    glutSolidCube(0.05);
    glPopMatrix();

    // Dibuja la Serpiente
    glColor3f(1.0, 1.0, 0.0);

    glBindTexture(GL_TEXTURE_2D, texName[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    for (int i = largo - 1; i >= 0; i--) {
        glPushMatrix();
        glTranslatef(xPos2d(snake[i][0]), yPos2d(snake[i][1]), 0.0);
        glutSolidCube(0.05);
        glPopMatrix();
    }

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    glDisable(GL_TEXTURE_2D);

    // Dibuja el Marcador
    glColor3f(1.0, 1.0, 1.0);

    stringstream ss; // Helper para desplegar el marcador

    ss << "Score: " << std::to_string(score);
    drawString(GLUT_BITMAP_9_BY_15, ss.str().c_str(), -0.85, -0.85);
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 20.0);
}

static void display(void)
{
    double snakeX, snakeY;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (showMap) {
        gluLookAt(0.0, 0.0, 2.0,
                  0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0);

        drawMap();
    } else {
        snakeX = xPos2d(snake[0][0]);
        snakeY = yPos2d(snake[0][1]);

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
bool snakeHits(float x, float y)
{
    double nextX, nextY;

    nextX = snake[0][0];
    nextY = snake[0][1];

    return nextX == x && nextY == y;
}

int counter;

void myTimer(int valor)
{
    double nextX, nextY;

    nextX = snake[0][0];
    nextY = snake[0][1];

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

    // Crece la cola primero para que el jugador tenga mejor control
    if (crece == 1 || snakeHits(appleX, appleY)) {

        // Incrementa el score
        score += (1 * scoreMultiplier);

        if (largo < largoMaximo-1) {

            // Crece la serpiente
            largo += 1;
        } else {

            // o Gana y regresa a su posición, dirección y tamaño inicial
            largo = 2;
            snake[0][0] = x;
            snake[0][1] = y;
            snake[1][0] = x-1;
            snake[1][1] = y;
            dirX = 1;
            dirY = 0;

            // Ahora cada manzana vale más
            scoreMultiplier++;

            // y se aumenta la velocidad de movimiento
            timerTick *= timerMultiplier;
        }

        applePresent = false;
        crece = 0;
    }

    // Corre los elementos
    for (int i = largo; i > 0; i--) {
        snake[i][0] = snake[i-1][0];
        snake[i][1] = snake[i-1][1];
    }

    // Actualizar la cabeza de la serpiente con `dirX`, `dirY` actual
    snake[0][0] = snake[1][0] + dirX;
    snake[0][1] = snake[1][1] + dirY;

    glutPostRedisplay();
    glutTimerFunc(timerTick, myTimer, 1);
}

void myKeyboard(unsigned char theKey, int mouseX, int mouseY)
{
    // Cambia el valor de dirX y dirY dependiendo de la tecla que oprima el usuario.
    // Activa la bandera de crecer para que la funcion `myTimer` crezca la serpiente en una unidad.
    // Debe funcionar para mayuscula y minuscula.
    switch (theKey)
    {
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

        // Salir
        case 27: case 'e': case 'E':
            exit(-1);
    }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(120, 120);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutCreateWindow("Cuarto Avance Snake - ITC 2013");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(myKeyboard);
    glutTimerFunc(100, myTimer, 1);

    glutMainLoop();
    return EXIT_SUCCESS;
}


/*
 Librería para manejo de imágenes
 ********************************/

#include <assert.h>
#include <fstream>

Image::Image(char* ps, int w, int h) : pixels(ps), width(w), height(h)
{

}

Image::~Image()
{
    delete[] pixels;
}

namespace
{
    //Converts a four-character array to an integer, using little-endian form
    int toInt(const char* bytes)
    {
        return (int)(((unsigned char)bytes[3] << 24) |
                     ((unsigned char)bytes[2] << 16) |
                     ((unsigned char)bytes[1] << 8) |
                     (unsigned char)bytes[0]);
    }

    //Converts a two-character array to a short, using little-endian form
    short toShort(const char* bytes)
    {
        return (short)(((unsigned char)bytes[1] << 8) |
                       (unsigned char)bytes[0]);
    }

    //Reads the next four bytes as an integer, using little-endian form
    int readInt(ifstream &input)
    {
        char buffer[4];
        input.read(buffer, 4);
        return toInt(buffer);
    }

    //Reads the next two bytes as a short, using little-endian form
    short readShort(ifstream &input)
    {
        char buffer[2];
        input.read(buffer, 2);
        return toShort(buffer);
    }

    //Just like auto_ptr, but for arrays
    template<class T>
    class auto_array
    {
    private:
        T* array;
        mutable bool isReleased;
    public:
        explicit auto_array(T* array_ = NULL) :
        array(array_), isReleased(false)
        {
        }

        auto_array(const auto_array<T> &aarray)
        {
            array = aarray.array;
            isReleased = aarray.isReleased;
            aarray.isReleased = true;
        }

        ~auto_array()
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
        }

        T* get() const
        {
            return array;
        }

        T &operator*() const
        {
            return *array;
        }

        void operator=(const auto_array<T> &aarray)
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
            array = aarray.array;
            isReleased = aarray.isReleased;
            aarray.isReleased = true;
        }

        T* operator->() const
        {
            return array;
        }

        T* release()
        {
            isReleased = true;
            return array;
        }

        void reset(T* array_ = NULL)
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
            array = array_;
        }

        T* operator+(int i)
        {
            return array + i;
        }

        T &operator[](int i)
        {
            return array[i];
        }
    };
}

Image* loadBMP(const char* filename)
{
    ifstream input;
    input.open(filename, ifstream::binary);
    assert(!input.fail() || !"Could not find file");
    char buffer[2];
    input.read(buffer, 2);
    assert(buffer[0] == 'B' && buffer[1] == 'M' || !"Not a bitmap file");
    input.ignore(8);
    int dataOffset = readInt(input);

    //Read the header
    int headerSize = readInt(input);
    int width;
    int height;
    switch (headerSize)
    {
        case 40:
            //V3
            width = readInt(input);
            height = readInt(input);
            input.ignore(2);
            assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
            assert(readShort(input) == 0 || !"Image is compressed");
            break;
        case 12:
            //OS/2 V1
            width = readShort(input);
            height = readShort(input);
            input.ignore(2);
            assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
            break;
        case 64:
            //OS/2 V2
            assert(!"Can't load OS/2 V2 bitmaps");
            break;
        case 108:
            //Windows V4
            assert(!"Can't load Windows V4 bitmaps");
            break;
        case 124:
            //Windows V5
            assert(!"Can't load Windows V5 bitmaps");
            break;
        default:
            assert(!"Unknown bitmap format");
    }

    //Read the data
    int bytesPerRow = ((width * 3 + 3) / 4) * 4 - (width * 3 % 4);
    int size = bytesPerRow * height;
    auto_array<char> pixels(new char[size]);
    input.seekg(dataOffset, ios_base::beg);
    input.read(pixels.get(), size);

    //Get the data into the right format
    auto_array<char> pixels2(new char[width * height * 3]);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            for (int c = 0; c < 3; c++)
            {
                pixels2[3 * (width * y + x) + c] =
                pixels[bytesPerRow * y + 3 * x + (2 - c)];
            }
        }
    }

    input.close();
    return new Image(pixels2.release(), width, height);
}
