#include "common.h"

#include <iostream>
#include <chrono>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* WINDOW_TITLE = "Assignment 1";
const double FRAME_RATE_MS = 1000.0 / 60.0;

typedef glm::vec4  color4;
typedef glm::vec4  point4;

const double SIXTH_OF_ROOT_3 = 0.28867513459481288225457439025098;
const double THIRD_OF_ROOT_3 = 0.57735026918962576450914878050196;


// TODO: Checkerboard pattern behind textrahedron (0, 0, -5);

inline std::vector<point4> tetrahedron(glm::vec3 origin = glm::vec3(0, 0, 0), float scale = 1.0)
{
    return
    {
        point4(origin.x,                  origin.y,                              origin.z,                              1),
        point4(origin.x + (scale * -0.5), origin.y + (scale * -THIRD_OF_ROOT_3), origin.z + (scale * -SIXTH_OF_ROOT_3), 1),
        point4(origin.x + (scale * 0.5),  origin.y + (scale * -THIRD_OF_ROOT_3), origin.z + (scale * -SIXTH_OF_ROOT_3), 1),
        point4(origin.x + (scale * 0),    origin.y + (scale * -THIRD_OF_ROOT_3), origin.z + (scale *  THIRD_OF_ROOT_3), 1)
    };
}

inline std::vector<point4> square(glm::vec3 origin, float width)
{
    return
    {
        point4(origin.x,         origin.y,         origin.z, 1),
        point4(origin.x + width, origin.y,         origin.z, 1),
        point4(origin.x + width, origin.y + width, origin.z, 1),
        point4(origin.x,         origin.y + width, origin.z, 1)
    };
}

std::vector<point4> generateCheckboard(float z, float numCell, float width = 3)
{
    float endsAtX    =  width / 2;
    float endsAtY    =  width / 2;
    float startFromX = -width / 2;
    float startFromY = -width / 2;

    float cellWidth = width / numCell;

    auto ret = std::vector<point4>();
    for (size_t i = 0; i < numCell - 1; i++)
    {
        for (int j = 0; j < numCell - 1; ++j)
        {
            auto ins = square({i * width, j * width, z}, width);
            ret.insert(ret.end(), ins.begin(), ins.end());
        }
    }
    return ret;
}

std::vector<GLuint> generateCheckboardIndices(GLuint size)
{
    auto ret = std::vector<GLuint>();
    for (int i = 0; i < size / 4 ; ++i)
    {
        std::vector<GLuint> ins =
        {
            4u * i + 0, 4u * i + 1, 4u * i + 2,
            4u * i + 0, 4u * i + 2, 4u * i + 3
        };
        ret.insert(ret.end(), ins.begin(), ins.end());
    }
    return ret;
}

std::vector<point4> generateSirTriangle(int depth, float scale = 1.0, glm::vec3 origin = glm::vec3(0, 0, 0))
{
    if (depth > 0)
    {
        auto coords = tetrahedron(origin, scale * 0.5);
        auto ret = std::vector<point4>();
        for (int i = 0; i <= 3; ++i)
        {
            auto t = generateSirTriangle(depth - 1, scale * 0.5,coords[i]);
            ret.insert(ret.end(), t.begin(), t.end());
        }
        return ret;
    }
    else
    {
        return tetrahedron(origin, scale);
    }
}

std::vector<GLuint> generateSirTriangleIndices(GLuint size)
{
    auto ret = std::vector<GLuint>();
    for (int i = 0; i < size / 4 ; ++i)
    {
        std::vector<GLuint> ins =
        {
            4u * i + 0, 4u * i + 1, 4u * i + 2,
            4u * i + 0, 4u * i + 2, 4u * i + 3,
            4u * i + 0, 4u * i + 3, 4u * i + 1,
            4u * i + 1, 4u * i + 2, 4u * i + 3
        };
        ret.insert(ret.end(), ins.begin(), ins.end());
    }
    return ret;
}


// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Yaxis;
GLfloat  Theta[NumAxes] = { 30.0, 0.0, 0.0 };
GLuint  Time;

bool rotate = false;


class Element
{
public:
    GLuint vao;
    // Model-view and projection matrices uniform location
    GLuint  ModelView, Projection;
    // shader program variables
    GLuint Program;
    // Texture
    GLuint texture;
};

Element tetrahedronElement;
auto verticesTetrahedron = generateSirTriangle(2, 2);
auto indicesTetrahedron = generateSirTriangleIndices(verticesTetrahedron.size());

Element checkboardElement;
auto verticesCheckboard = generateCheckboard(-5, 10, 3);
auto indicesCheckboard = generateCheckboardIndices(verticesCheckboard.size());


const GLuint textureSize = 64;
GLubyte image[textureSize][textureSize][3];


// OpenGL init
void init()
{
    // Create a checkerboard pattern
    for ( int i = 0; i < 64; i++ ) 
    {
        for ( int j = 0; j < 64; j++ ) 
        {
            GLubyte c = (((i & 0x8) == 0) ^ ((j & 0x8)  == 0)) * 255;
            image[i][j][0]  = c;
            image[i][j][1]  = c;
            image[i][j][2]  = c;
        }
    }

    // init texture obj
    glGenTextures(1, &checkboardElement.texture);
    glBindTexture(GL_TEXTURE_2D, checkboardElement.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSize, textureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, checkboardElement.texture );
    // TODO: figure out how texture works.

    GLuint buffer;
    GLuint vPosition;

    ///////// Checkboard //////////////////////////////////////////////////////////////////////////////////

    glGenVertexArrays(1, &checkboardElement.vao);

    // Load shader
    checkboardElement.Program = InitShader("vshaderA1Checkboard.glsl", "fshaderA1Checkboard.glsl");
    glUseProgram(checkboardElement.Program);
    vPosition = glGetAttribLocation(checkboardElement.Program, "vPosition");

    glBindVertexArray(checkboardElement.vao);

    // create and init a buffer object
    // vertex buffer
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * verticesCheckboard.size(), verticesCheckboard.data(), GL_STATIC_DRAW);

    // index buffer
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicesCheckboard.size(), indicesCheckboard.data(), GL_STATIC_DRAW);

    // set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    // retrieve transformation uniform var locations
    checkboardElement.ModelView = glGetUniformLocation(checkboardElement.Program, "ModelView");
    checkboardElement.Projection = glGetUniformLocation(checkboardElement.Program, "Projection");



    ///////// Textrahedron /////////////////////////////////////////////////////////////////////////////////

    glGenVertexArrays(1, &tetrahedronElement.vao);

    // Load shader
    tetrahedronElement.Program = InitShader("vshaderA1Tetrahedron.glsl", "fshaderA1Tetrahedron.glsl");
    glUseProgram(tetrahedronElement.Program);
    vPosition = glGetAttribLocation(tetrahedronElement.Program, "vPosition");

    glBindVertexArray(tetrahedronElement.vao);

    // create and init a buffer object
    // vertex buffer
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * verticesTetrahedron.size(), verticesTetrahedron.data(), GL_STATIC_DRAW);

    // index buffer
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicesTetrahedron.size(), indicesTetrahedron.data(), GL_STATIC_DRAW);

    // set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    // retrieve transformation uniform var locations
    tetrahedronElement.ModelView = glGetUniformLocation(tetrahedronElement.Program, "ModelView");
    tetrahedronElement.Projection = glGetUniformLocation(tetrahedronElement.Program, "Projection");

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 1.0);
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // generate the model-view matrix
    glm::vec3 viewer_pos(0, 0, 3);
    glm::vec3 model_trans(0, 1, 0);

    glm::mat4 rot, model_view;


    ///////// Textrahedron /////////////////////////////////////////////////////////////////////////////////
    
    rot = glm::rotate(rot, glm::radians(Theta[Xaxis]), glm::vec3(1, 0, 0));
    rot = glm::rotate(rot, glm::radians(Theta[Yaxis]), glm::vec3(0, 1, 0));
    rot = glm::rotate(rot, glm::radians(Theta[Zaxis]), glm::vec3(0, 0, 1));
    model_view = glm::translate(glm::mat4(), -viewer_pos) * rot * glm::translate(glm::mat4(), model_trans);

    glUseProgram(tetrahedronElement.Program);

    // give data to shader
    glUniformMatrix4fv(tetrahedronElement.ModelView, 1, GL_FALSE, glm::value_ptr(model_view));

    glBindVertexArray(tetrahedronElement.vao);
    glDrawElements(GL_TRIANGLES, indicesTetrahedron.size(), GL_UNSIGNED_INT, 0);

    ///////// Checkboard //////////////////////////////////////////////////////////////////////////////////
    
    rot = glm::rotate(rot, glm::radians(0.0f), glm::vec3(1, 0, 0));
    rot = glm::rotate(rot, glm::radians(0.0f), glm::vec3(0, 1, 0));
    rot = glm::rotate(rot, glm::radians(0.0f), glm::vec3(0, 0, 1));
    model_view = glm::translate(glm::mat4(), -viewer_pos) * rot * glm::translate(glm::mat4(), model_trans);

    glUseProgram(checkboardElement.Program);

    // give data to shader
    glUniformMatrix4fv(checkboardElement.ModelView, 1, GL_FALSE, glm::value_ptr(model_view));

    glBindVertexArray(checkboardElement.vao);
    glDrawElements(GL_TRIANGLES, indicesTetrahedron.size(), GL_UNSIGNED_INT, 0);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    glutSwapBuffers();
}


void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        switch (button) {
        case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
        case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
        case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
        }
    }
}


void update(void)
{
    if (rotate)
    {
        Theta[Axis] += 0.5;

        if (Theta[Axis] > 360.0) {
            Theta[Axis] -= 360.0;
        }
    }
}


void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 033: // Escape Key
    case 'q': case 'Q':
        exit(EXIT_SUCCESS);
        break;
    case ' ':
        rotate = !rotate;
    }
}


void reshape(int width, int height)
{
    glViewport(0, 0, width, height);

    GLfloat aspect = GLfloat(width) / height;
    glm::mat4  projection = glm::perspective(glm::radians(45.0f), aspect, 0.5f, 10.0f);

    glUseProgram(tetrahedronElement.Program);
    glUniformMatrix4fv(tetrahedronElement.Projection, 1, GL_FALSE, glm::value_ptr(projection));
    glUseProgram(checkboardElement.Program);
    glUniformMatrix4fv(checkboardElement.Projection, 1, GL_FALSE, glm::value_ptr(projection));
}
