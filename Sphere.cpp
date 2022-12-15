#include <Gl/freeglut.h>
#include <windows.h>
#include <cmath>
#include "Sphere.h"

# define M_PI           3.14159265358979323846  /* pi */

//Для проверки
const int MIN_SECTOR_COUNT = 3;
const int MIN_STACK_COUNT = 2;

//Конструктор
Sphere::Sphere(float radius, int sectors, int stacks, bool smooth) : interleavedStride(32)
{
    this->radius = radius;
    this->sectorCount = sectors;
    if (sectors < MIN_SECTOR_COUNT)
        this->sectorCount = MIN_SECTOR_COUNT;
    this->stackCount = stacks;
    if (sectors < MIN_STACK_COUNT)
        this->sectorCount = MIN_STACK_COUNT;
    this->smooth = smooth;

    if (smooth)
        buildVerticesSmooth();
    else
        buildVerticesFlat();
}


// разворот нормалей
void Sphere::reverseNormals()
{
    std::size_t i, j;
    std::size_t count = normals.size();
    for (i = 0, j = 3; i < count; i += 3, j += 8)
    {
        normals[i] *= -1;
        normals[i + 1] *= -1;
        normals[i + 2] *= -1;

        // Обновление массивов
        interleavedVertices[j] = normals[i];
        interleavedVertices[j + 1] = normals[i + 1];
        interleavedVertices[j + 2] = normals[i + 2];
    }

    // Для массива треугольников
    unsigned int tmp;
    count = indices.size();
    for (i = 0; i < count; i += 3)
    {
        tmp = indices[i];
        indices[i] = indices[i + 2];
        indices[i + 2] = tmp;
    }
}


// Рисование по точка, через DrawElements с указанием нормалей и карты текстуры
void Sphere::draw() const
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, interleavedStride, &interleavedVertices[0]);
    glNormalPointer(GL_FLOAT, interleavedStride, &interleavedVertices[3]);
    glTexCoordPointer(2, GL_FLOAT, interleavedStride, &interleavedVertices[6]);

    glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, indices.data());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

// Рисование линий по точкам
void Sphere::drawLines(const float lineColor[4]) const
{
    glColor4fv(lineColor);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, lineColor);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices.data());

    glDrawElements(GL_LINES, (unsigned int)lineIndices.size(), GL_UNSIGNED_INT, lineIndices.data());

    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

///////////////////////////////////////////////////////////////////////////////
// рисование поверхности с линиями
///////////////////////////////////////////////////////////////////////////////
void Sphere::drawWithLines(const float lineColor[4]) const
{
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0f); // задает масштаб и единицы
    this->draw();
    glDisable(GL_POLYGON_OFFSET_FILL);

    // Рисование линий
    drawLines(lineColor);
}

// Чистка массивов (контейнеров)
//void Sphere::clearArrays()
//{
//    std::vector<float>().swap(vertices);
//    std::vector<float>().swap(normals);
//    std::vector<float>().swap(texCoords);
//    std::vector<unsigned int>().swap(indices);
//    std::vector<unsigned int>().swap(lineIndices);
//}

///////////////////////////////////////////////////////////////////////////////
// Просчет точек фигуры по формулам, используя широту и долготу
// x = r * cos(u) * cos(v)
// y = r * cos(u) * sin(v)
// z = r * sin(u)
//  v по (-90 <= u <= 90) 
//  u по (0 <= v <= 360)
///////////////////////////////////////////////////////////////////////////////
void Sphere::buildVerticesSmooth()
{

    float x, y, z, xy;                              // координаты верин фигуры
    float nx, ny, nz, lengthInv = 1.0f / radius;    // для номрали
    float s, t;                                     // для texCoord (карты текстуры)

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;        // по угоу от пи/2 до -пи/2
        xy = radius * cosf(stackAngle);             //Расчет идеальной сферы
        z = radius * sinf(stackAngle);              

        //Деформация согласно варинту
        if (z > 0)
            z += radius;

        // Добавляем координаты в массивы (контейнеры)
        //!! первый послдений треуголькники
        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // от 0 до 2пи

            // Вычисляем x y для вершины, добавляем вершину
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            addVertex(x, y, z);

            // Вычисление и добавление нормали
            //!!выброс 0 исправить
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            addNormal(nx, ny, nz);

            // Добавление точки в карту текстуры
            // 
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            addTexCoord(s, t);
        }
    }

    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    unsigned int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     
        k2 = k1 + sectorCount + 1;      

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                addIndices(k1, k2, k1 + 1);   
            }

            if (i != (stackCount - 1))
            {
                addIndices(k1 + 1, k2, k2 + 1); 
            }

            
            lineIndices.push_back(k1);
            lineIndices.push_back(k2);
            if (i != 0) 
            {
                lineIndices.push_back(k1);
                lineIndices.push_back(k1 + 1);
            }
        }
    }

    //Массив вершин
    buildInterleavedVertices();
}



// То же просчет точек фигуры по формулам, используя широту и долготу, но с обычной заливкой
void Sphere::buildVerticesFlat()
{
    //Свой тип данных для удобства
    struct Vertex
    {
        float x, y, z, s, t;
    };
    std::vector<Vertex> tmpVertices;

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        float xy = radius * cosf(stackAngle);       // r * cos(u)
        float z = radius * sinf(stackAngle);        // r * sin(u)

        //Деформация согласно варинту
        if (z > 0)
            z += radius;

        // Коориднаты вершин
        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           //  от 0 до 2пи

            Vertex vertex;
            vertex.x = xy * cosf(sectorAngle);      // x = r * cos(u) * cos(v)
            vertex.y = xy * sinf(sectorAngle);      // y = r * cos(u) * sin(v)
            vertex.z = z;                           // z = r * sin(u)
            vertex.s = (float)j / sectorCount;        // s
            vertex.t = (float)i / stackCount;         // t
            tmpVertices.push_back(vertex);
        }
    }

    Vertex v1, v2, v3, v4;                          // для вершины и текстуры 
    std::vector<float> n;                           // для нормали

    int i, j, k, vi1, vi2;
    int index = 0;                                  // индекс вершины
    for (i = 0; i < stackCount; ++i)
    {
        vi1 = i * (sectorCount + 1);                
        vi2 = (i + 1) * (sectorCount + 1);

        for (j = 0; j < sectorCount; ++j, ++vi1, ++vi2)
        {
            
            //  v1--v3
            //  |    |
            //  v2--v4
            v1 = tmpVertices[vi1];
            v2 = tmpVertices[vi2];
            v3 = tmpVertices[vi1 + 1];
            v4 = tmpVertices[vi2 + 1];

            // Для 1 треугольника хванятся все 3
            // для остальных
            if (i == 0) // 1 тр-к
            {
                // Новый треугольник
                addVertex(v1.x, v1.y, v1.z);
                addVertex(v2.x, v2.y, v2.z);
                addVertex(v4.x, v4.y, v4.z);

                // текст. координаты
                addTexCoord(v1.s, v1.t);
                addTexCoord(v2.s, v2.t);
                addTexCoord(v4.s, v4.t);

                // Ставим нормаль
                n = computeFaceNormal(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v4.x, v4.y, v4.z);
                for (k = 0; k < 3; ++k) 
                {
                    addNormal(n[0], n[1], n[2]);
                }

                
                addIndices(index, index + 1, index + 2);

                
                lineIndices.push_back(index);
                lineIndices.push_back(index + 1);

                index += 3;    
            }
            else if (i == (stackCount - 1)) // для треугольников с 1 по последний
            {
                // добавление треугольника
                addVertex(v1.x, v1.y, v1.z);
                addVertex(v2.x, v2.y, v2.z);
                addVertex(v3.x, v3.y, v3.z);

                //добавление карты текстуры
                addTexCoord(v1.s, v1.t);
                addTexCoord(v2.s, v2.t);
                addTexCoord(v3.s, v3.t);

                // вычисление нормали
                n = computeFaceNormal(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z);
                for (k = 0; k < 3; ++k) 
                {
                    addNormal(n[0], n[1], n[2]);
                }

                
                addIndices(index, index + 1, index + 2);

                
                lineIndices.push_back(index);
                lineIndices.push_back(index + 1);
                lineIndices.push_back(index);
                lineIndices.push_back(index + 2);

                index += 3;   
            }
            else
            {
                // вершины v1-v2-v3-v4
                addVertex(v1.x, v1.y, v1.z);
                addVertex(v2.x, v2.y, v2.z);
                addVertex(v3.x, v3.y, v3.z);
                addVertex(v4.x, v4.y, v4.z);

                // для 
                addTexCoord(v1.s, v1.t);
                addTexCoord(v2.s, v2.t);
                addTexCoord(v3.s, v3.t);
                addTexCoord(v4.s, v4.t);

                // вычисление нормали
                n = computeFaceNormal(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z);
                for (k = 0; k < 4; ++k) 
                {
                    addNormal(n[0], n[1], n[2]);
                }

                
                addIndices(index, index + 1, index + 2);
                addIndices(index + 2, index + 1, index + 3);

                
                lineIndices.push_back(index);
                lineIndices.push_back(index + 1);
                lineIndices.push_back(index);
                lineIndices.push_back(index + 2);

                index += 4;     
            }
        }
    }

    //Массив вершин
    buildInterleavedVertices();
}


// Запись 
void Sphere::buildInterleavedVertices()
{
    std::vector<float>().swap(interleavedVertices);

    std::size_t i, j;
    std::size_t count = vertices.size();
    for (i = 0, j = 0; i < count; i += 3, j += 2)
    {
        interleavedVertices.push_back(vertices[i]);
        interleavedVertices.push_back(vertices[i + 1]);
        interleavedVertices.push_back(vertices[i + 2]);

        interleavedVertices.push_back(normals[i]);
        interleavedVertices.push_back(normals[i + 1]);
        interleavedVertices.push_back(normals[i + 2]);

        interleavedVertices.push_back(texCoords[j]);
        interleavedVertices.push_back(texCoords[j + 1]);
    }
}



// Добавление новой вершины
void Sphere::addVertex(float x, float y, float z)
{
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
}



// Добавление новой нормали в контейнер
void Sphere::addNormal(float nx, float ny, float nz)
{
    normals.push_back(nx);
    normals.push_back(ny);
    normals.push_back(nz);
}

// Добавление новой точки в карту текстуры
void Sphere::addTexCoord(float s, float t)
{
    texCoords.push_back(s);
    texCoords.push_back(t);
}

// Индексы для расчетов
void Sphere::addIndices(unsigned int i1, unsigned int i2, unsigned int i3)
{
    indices.push_back(i1);
    indices.push_back(i2);
    indices.push_back(i3);
}



// Вычисление нормаль. для грани треугольника
// Если вернет 0, значи нет грани length = 0
// !!!!Выброс искл. 0 исправить
std::vector<float> Sphere::computeFaceNormal(float x1, float y1, float z1,  // для в1 и  далее 2, 3
    float x2, float y2, float z2, 
    float x3, float y3, float z3)  
{
    //Для проверки на 0
    const float EPSILON = 0.000001f;

    std::vector<float> normal(3, 0.0f);     
    float nx, ny, nz;

    // ребра
    float ex1 = x2 - x1;
    float ey1 = y2 - y1;
    float ez1 = z2 - z1;
    float ex2 = x3 - x1;
    float ey2 = y3 - y1;
    float ez2 = z3 - z1;

    // Перекрестное произведение (нужно для вычисления нормали)
    nx = ey1 * ez2 - ez1 * ey2;
    ny = ez1 * ex2 - ex1 * ez2;
    nz = ex1 * ey2 - ey1 * ex2;

    // Нормаль
    float length = sqrtf(nx * nx + ny * ny + nz * nz);
    if (length > EPSILON)
    {
        // Вычисление
        float lengthInv = 1.0f / length;
        normal[0] = nx * lengthInv;
        normal[1] = ny * lengthInv;
        normal[2] = nz * lengthInv;
    }

    return normal;
}
