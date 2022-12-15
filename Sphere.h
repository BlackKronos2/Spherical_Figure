#include <vector> //подключение контейнера Vector

//Класс для удобства
class Sphere
{
    //Инициализация методов для удобства
public:
    // Конструктор
    Sphere(float radius, int sectorCount, int stackCount, bool smooth = true);

    // getters/setters
    void reverseNormals();
    
    // Рисование
    void draw() const;                                  // draw surface
    void drawLines(const float lineColor[4]) const;     // draw lines only
    void drawWithLines(const float lineColor[4]) const; // draw surface and lines

    // Расчет точек
    void buildVerticesSmooth();
    void buildVerticesFlat();
    void buildInterleavedVertices();
    //void clearArrays();

    //Давление вершин по контейнерам
    void addVertex(float x, float y, float z);
    void addNormal(float x, float y, float z);
    void addTexCoord(float s, float t);
    void addIndices(unsigned int i1, unsigned int i2, unsigned int i3);

    std::vector<float> computeFaceNormal(float x1, float y1, float z1,
        float x2, float y2, float z2,
        float x3, float y3, float z3);

    //Поля класса
    float radius;
    int sectorCount;                        
    int stackCount;                         
    bool smooth;

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> lineIndices;

    // interleaved
    std::vector<float> interleavedVertices;
    int interleavedStride;                  // # of bytes to hop to the next vertex (should be 32 bytes)

};
