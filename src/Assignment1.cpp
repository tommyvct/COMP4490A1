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


inline std::vector<point4> generateCheckboard(glm::vec3 origin, float width)
{
    return
    {
        point4(origin.x - width/2, origin.y - width/2, origin.z, 1),
        point4(origin.x + width/2, origin.y - width/2, origin.z, 1),
        point4(origin.x + width/2, origin.y + width/2, origin.z, 1),
        point4(origin.x - width/2, origin.y + width/2, origin.z, 1),
    };
}

inline std::vector<GLuint> generateCheckboardIndices(GLuint size)
{
    return
    {
        0, 1, 2,
        2, 3, 0
    };
}

inline std::vector<glm::vec2> generateCheckBoardTextureCoords(GLuint size)
{
    return
    {
        {0.0, 1.0},
        {1.0, 1.0},
        {1.0, 0.0},
        {0.0, 0.0},
        {1.0, 0.0},
        {0.0, 1.0},
    };
}

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
            4u * i + 2, 4u * i + 3, 4u * i + 0,
            4u * i + 3, 4u * i + 1, 4u * i + 0,
            4u * i + 1, 4u * i + 2, 4u * i + 3  // bottom
        };
        ret.insert(ret.end(), ins.begin(), ins.end());
    }
    return ret;
}


// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, Stop = 3 };
int      Axis = Stop;
GLfloat  Theta[3] = { 30.0, 0.0, 0.0 };
GLuint  Time;

long long millis = 0;

int level = 1;
bool levelIncreasing = true;
bool levelCycle = false;
long long levelCycleCountdown = millis;



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
auto verticesTetrahedron = generateSirTriangle(3, 2);
auto indicesTetrahedron = generateSirTriangleIndices(verticesTetrahedron.size());
bool rerenderTetrahedron = true;

Element checkboardElement;
auto verticesCheckboard = generateCheckboard({0, 0, -5}, 7);
auto indicesCheckboard = generateCheckboardIndices(verticesCheckboard.size());
auto texcoordsCheckboard = generateCheckBoardTextureCoords(verticesCheckboard.size());


const GLuint textureSize = 128;
GLubyte image[textureSize][textureSize][3];


// OpenGL init
void init()
{
    GLuint buffer;

    ///////// Checkboard //////////////////////////////////////////////////////////////////////////////////

    // Create a checkerboard pattern image
    for ( int i = 0; i < textureSize; i++ ) 
    {
        for ( int j = 0; j < textureSize; j++ ) 
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

    // Create a vertex array obj
    glGenVertexArrays(1, &checkboardElement.vao);
    glBindVertexArray(checkboardElement.vao);

    // create and init a buffer object
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 
        sizeof(point4) * verticesCheckboard.size() + sizeof(glm::vec2) * texcoordsCheckboard.size(),
        NULL, GL_STATIC_DRAW);

    GLintptr offset = 0;
    // vertex buffer
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(point4) * verticesCheckboard.size(), verticesCheckboard.data());
    offset += sizeof(point4) * verticesCheckboard.size();

    // Texture Coords buffer
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(glm::vec2) * texcoordsCheckboard.size(), texcoordsCheckboard.data());
    
    // index buffer
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicesCheckboard.size(), indicesCheckboard.data(), GL_STATIC_DRAW);

    // Load shader
    checkboardElement.Program = InitShader("vshaderA1Checkboard.glsl", "fshaderA1Checkboard.glsl");
    glUseProgram(checkboardElement.Program);

    // set up vertex arrays, aka link the correct data to correct places
    offset = 0;

    GLuint vPosition = glGetAttribLocation(checkboardElement.Program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
    offset += sizeof(point4) * verticesCheckboard.size();

    GLuint vTexCoord = glGetAttribLocation(checkboardElement.Program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));

    // retrieve transformation uniform var locations
    checkboardElement.ModelView = glGetUniformLocation(checkboardElement.Program, "ModelView");
    checkboardElement.Projection = glGetUniformLocation(checkboardElement.Program, "Projection");

    glUniform1i( glGetUniformLocation(checkboardElement.Program, "texture"), 0 );

    ///////// Textrahedron /////////////////////////////////////////////////////////////////////////////////


    // Load shader
    tetrahedronElement.Program = InitShader("vshaderA1Tetrahedron.glsl", "fshaderA1Tetrahedron.glsl");
    glUseProgram(tetrahedronElement.Program);

    // retrieve transformation uniform var locations
    tetrahedronElement.ModelView = glGetUniformLocation(tetrahedronElement.Program, "ModelView");
    tetrahedronElement.Projection = glGetUniformLocation(tetrahedronElement.Program, "Projection");

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

glm::vec3 viewer_pos(0, 0, 3);

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glm::mat4 rot, model_view;


    ///////// Textrahedron /////////////////////////////////////////////////////////////////////////////////
    
    rot = glm::rotate(rot, glm::radians(Theta[Xaxis]), glm::vec3(1, 0, 0));
    rot = glm::rotate(rot, glm::radians(Theta[Yaxis]), glm::vec3(0, 1, 0));
    rot = glm::rotate(rot, glm::radians(Theta[Zaxis]), glm::vec3(0, 0, 1));
    model_view = glm::translate(glm::mat4(), -viewer_pos) * rot * glm::translate(glm::mat4(), glm::vec3(0, 1, 0));

    glUseProgram(tetrahedronElement.Program);


    if (rerenderTetrahedron)
    {
        GLuint buffer;
        GLuint vPosition = glGetAttribLocation(tetrahedronElement.Program, "vPosition");

        glGenVertexArrays(1, &tetrahedronElement.vao);
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

        rerenderTetrahedron = false;
    }


    // give data to shader
    glUniformMatrix4fv(tetrahedronElement.ModelView, 1, GL_FALSE, glm::value_ptr(model_view));

    glBindVertexArray(tetrahedronElement.vao);
    glDrawElements(GL_TRIANGLES, indicesTetrahedron.size(), GL_UNSIGNED_INT, 0);

    ///////// Checkboard //////////////////////////////////////////////////////////////////////////////////
    
    model_view = glm::translate(glm::mat4(), -viewer_pos) * glm::translate(glm::mat4(), glm::vec3(0, 0, 0));

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
    if (state == GLUT_DOWN) 
    {
        int nextAxis = -1;
        switch (button)
        {
            case GLUT_LEFT_BUTTON:    nextAxis = Xaxis;  break;
            case GLUT_MIDDLE_BUTTON:  nextAxis = Yaxis;  break;
            case GLUT_RIGHT_BUTTON:   nextAxis = Zaxis;  break;
        }

        if (Axis == nextAxis)
        {
            Axis = Stop;
        }
        else
        {
            Axis = nextAxis;
        }
    }
}


void update(void)
{
    if (Axis != Stop)
    {
        Theta[Axis] += 0.5;

        if (Theta[Axis] > 360.0) {
            Theta[Axis] -= 360.0;
        }
    }

    millis = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count();

    if (levelCycle)
    {
        if (millis - levelCycleCountdown > 1000)
        {
            rerenderTetrahedron = true;
            verticesTetrahedron = generateSirTriangle(level, 2);
            indicesTetrahedron = generateSirTriangleIndices(verticesTetrahedron.size());


            levelCycleCountdown = millis;

            if (level > 8)
            {
                levelIncreasing = false;
            }
            else if (level <= 1)
            {
                levelIncreasing = true;
            }

            if (levelIncreasing)
            {
                level++;
            }
            else
            {
                level--;
            }
        }
    }
}


void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        default:
            break;
        case ' ':
            levelCycle = !levelCycle;
            break;
        case 033: // Escape Key
        case 'q': case 'Q':
            exit(EXIT_SUCCESS);
    }

    glutPostRedisplay();
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
