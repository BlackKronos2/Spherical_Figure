#include <glut.h>
#include "Bmp.h"
#include "Sphere.h"

//Константы
const int   SCREEN_WIDTH = 1500;
const int   SCREEN_HEIGHT = 800;
const float CAMERA_DISTANCE = 4.0f;


//Текущее положение экраны
int screenWidth;
int screenHeight;

//Для отслеживания действ-й поль-я
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
int drawMode;


//Номер текстуры
unsigned int texId;

//Создаем экземпляры класса Sphere
Sphere sphere1(1.0f, 36, 18, false);    // радиус, широта и долгота, сглаживание
Sphere sphere2(1.0f, 36, 18);           



//Инициализация света
void initLights()
{
    //Характеристики света
    GLfloat lightKa[] = { .3f, .3f, .3f, 1.0f };  // Окружающий свет
    GLfloat lightKd[] = { .7f, .7f, .7f, 1.0f };  // Рассеивание
    GLfloat lightKs[] = { 1, 1, 1, 1 };           // Зерк.
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = { 0, 0, 1, 0 }; // directional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    //Вкл. источника света
    glEnable(GL_LIGHT0);                        
}


// Включения OpenGl
void initGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

    //Параметры цвета материала
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    //Значение по умолчанию — GL_AMBIENT_AND_DIFFUSE
    glEnable(GL_COLOR_MATERIAL);

    //Фон
    glClearColor(0, 0, 0, 0);                   

    //Инициализация света
    initLights();
}


///////////////////////////////////////////////////////////////////////////////
// Инициализация камеры
///////////////////////////////////////////////////////////////////////////////
bool initCamera()
{
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = mouseMiddleDown = false;
    mouseX = mouseY = 0;

    cameraAngleX = cameraAngleY = 0.0f;
    cameraDistance = CAMERA_DISTANCE;

    drawMode = 0; //Режим полной прорисовки


    return true;
}



// Инициализация / Загрузка текстуры используя пакет BMP
unsigned int loadTexture(const char* fileName, bool wrap)
{
    Image::Bmp bmp;
    if (!bmp.read(fileName))
        return 0;     // если файл не будет найден

    // Для методов библиотеки нужны хар-ки изображения
    int width = bmp.getWidth();
    int height = bmp.getHeight();
    const unsigned char* data = bmp.getDataRGB();
    GLenum type = GL_UNSIGNED_BYTE;    // only allow BMP with 8-bit per channel

    // Проверка для работы с разными цветовми моделями
    GLenum format;
    int bpp = bmp.getBitCount();
    if (bpp == 8)
        format = GL_LUMINANCE;
    else if (bpp == 24)
        format = GL_RGB;
    else if (bpp == 32)
        format = GL_RGBA;
    else
        return 0;               // если ничего не поддерживает

    // ID текстуры
    GLuint texture;
    glGenTextures(1, &texture);

    // Делаем текстуру активной
    glBindTexture(GL_TEXTURE_2D, texture);

    // Модуляция
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    //Параметры текстуры
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    // лог. переменная wrap дял переключения режимов GL_REPEA и GL_CLAMP
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP);

    // Данные текстуры
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, data);

    //Построение текстуры
    switch (bpp)
    {
    case 8:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 1, width, height, GL_LUMINANCE, type, data);
        break;
    case 24:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, type, data);
        break;
    case 32:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, type, data);
        break;
    }

    return texture;
}


// Ортогональная камера
void toOrtho()
{
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-3.5f, 3.5f, -2.5f, 2.5f, -1, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Перспективная камера
void toPerspective()
{
    
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0f, (float)(screenWidth) / screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void displayCB()
{
    //Буфер
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //Сохраняем перед изменением
    glPushMatrix();

    //Перемещаем камеру
    glTranslatef(0, 0, -cameraDistance);

    //Характеристики материала
    float ambient[] = { 0.5f, 0.5f, 0.5f, 1 };
    float diffuse[] = { 0.7f, 0.7f, 0.7f, 1 };
    float specular[] = { 1.0f, 1.0f, 1.0f, 1 };
    float shininess = 128;
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    // Цвет линии
    float lineColor[] = { 0.2f, 0.2f, 0.2f, 1 };

    // Рисуем фигуру без текстуры
    glPushMatrix();
    glTranslatef(-2.5f, 0, 0);
    glRotatef(cameraAngleX, 1, 0, 0);   // Сдвигаем
    glRotatef(cameraAngleY, 0, 1, 0);    
    glRotatef(-90, 1, 0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    sphere1.drawWithLines(lineColor);
    //sphere1.drawLines(lineColor);
    glPopMatrix();

    // Рисуем фигуру с текстурой
    glPushMatrix();
    glRotatef(cameraAngleX, 1, 0, 0);
    glRotatef(cameraAngleY, 0, 1, 0);
    glRotatef(-90, 1, 0, 0);
    glBindTexture(GL_TEXTURE_2D, texId);
    sphere2.draw();
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();

    glutSwapBuffers();
}


//Переключение камер
void reshapeCB(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    toPerspective();
    //toOrtho();
}


void keyboardCB(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // ESCAPE
        exit(0);
        break;

    case 'f': // Смена режима прорисовки
    case 'F':
        ++drawMode;
        drawMode %= 3;
        if (drawMode == 0)        // Полное изображение
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
        else if (drawMode == 1)  // Каркасное изображение
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        else                    // Режим точек
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        break;

    case 'd':
    case 'D': glTranslated(-0.1f, 0, 0); break;

    case 'a':
    case 'A': glTranslated(0.1f, 0, 0);break;

    case 's':
    case 'S': glTranslated(0.0, 0.1f, 0);break;

    case 'W':
    case 'w': glTranslated(0.0, -0.1f, 0);break;
    default:
        ;
    }
}

//Переключение режимов управления мышью
void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if (state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if (state == GLUT_UP)
            mouseRightDown = false;
    }

    else if (button == GLUT_MIDDLE_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if (state == GLUT_UP)
            mouseMiddleDown = false;
    }
}

//Передвижение и отдаление камеры,
void mouseMotionCB(int x, int y)
{
    if (mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if (mouseRightDown)
    {
        cameraDistance -= (y - mouseY) * 0.2f;
        mouseY = y;
    }
}
void Update() {
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    // Инициализация камеры
    initCamera();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL); 

    glutInitWindowSize(screenWidth, screenHeight);  
    glutInitWindowPosition(100, 100);               
    glutCreateWindow("РГР2 Арефьев К.Б.");

    initGL();

    // Загрузка текстуры из bmp файла
    texId = loadTexture("earth2048.bmp", true);

    glutDisplayFunc(displayCB);
    glutIdleFunc(Update); 
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);

    glutMainLoop(); 

    return 0;
}