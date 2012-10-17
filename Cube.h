#ifndef CUBE_H
#define CUBE_H

// Vertices
//
// 4 floats per vertex
// 4 vertices per face
// 6 faces
static const int vertexDataCount = 6 * 4 * 4;
static const float vertexData[vertexDataCount] = {
    // Left face
    -0.5f, -0.5f, -0.5f, 1.0f,
    -0.5f, -0.5f,  0.5f, 1.0f,
    -0.5f,  0.5f,  0.5f, 1.0f,
    -0.5f,  0.5f, -0.5f, 1.0f,

    // Top face
    -0.5f, 0.5f, -0.5f, 1.0f,
    -0.5f, 0.5f,  0.5f, 1.0f,
     0.5f, 0.5f,  0.5f, 1.0f,
     0.5f, 0.5f, -0.5f, 1.0f,

    // Right face
    0.5f,  0.5f, -0.5f, 1.0f,
    0.5f,  0.5f,  0.5f, 1.0f,
    0.5f, -0.5f,  0.5f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f,

    // Bottom face
     0.5f, -0.5f, -0.5f, 1.0f,
     0.5f, -0.5f,  0.5f, 1.0f,
    -0.5f, -0.5f,  0.5f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f,

    // Front face
     0.5f, -0.5f, 0.5f, 1.0f,
     0.5f,  0.5f, 0.5f, 1.0f,
    -0.5f,  0.5f, 0.5f, 1.0f,
    -0.5f, -0.5f, 0.5f, 1.0f,

    // Back face
     0.5f,  0.5f, -0.5f, 1.0f,
     0.5f, -0.5f, -0.5f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f,
    -0.5f,  0.5f, -0.5f, 1.0f
};


// Normal vectors
static const int normalDataCount = 6 * 4 * 3;
static const float normalData[normalDataCount] = {
    // Left face
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,

    // Top face
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,

    // Right face
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,

    // Bottom face
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,

    // Front face
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,

    // Back face
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f
};


// Texure coords
static const int textureCoordDataCount = 6 * 4 * 2;
static const float textureCoordData[textureCoordDataCount] = {
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f
};

static const int indexDataCount = 6 * 3 * 2;
static const unsigned int indexData[indexDataCount] = {
    0,  1,  2,  0,  2,  3,  // Left face
    4,  5,  6,  4,  6,  7,  // Top face
    8,  9,  10, 8,  10, 11, // Right face
    12, 14, 15, 12, 13, 14, // Bottom face
    16, 17, 18, 16, 18, 19, // Front face
    20, 22, 23, 20, 21, 22  // Back face
};


class Cube
{
public:
    Cube()
    {
    };

    static const float* vertices()
    {
        return vertexData;
    };
    static int vertexDataSize()
    {
        return vertexDataCount * sizeof(float);
    };

    static const float* normals()
    {
         return normalData;
    };
    static int normalDataSize()
    {
        return normalDataCount * sizeof( float );
    };

    static const float* textureCoordinates()
    {
        return textureCoordData;
    };
    static int textureCoordinateDataSize()
    {
        return textureCoordDataCount * sizeof( float );
    };

    static const unsigned int* indices()
    {
        return indexData;
    };
    static int indexDataSize()
    {
        return indexDataCount * sizeof( unsigned int );
    };
    static int indexCount()
    {
        return indexDataCount;
    };
};

#endif // CUBE_H
