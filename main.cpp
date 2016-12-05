#include <iostream>
#include "stdio.h"
#include "math.h"
#define _USE_MATH_DEFINES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <vector>
#include <cmath>

GLenum    polyMode = GL_FILL;
GLint     iDataSetSize = 16;
GLfloat   fStepSize = 1.0/iDataSetSize;
GLfloat   isoValue = 48.0;
GLboolean bSpin = true;
GLboolean bMove = true;
GLboolean bLight = true;
GLsizei screenWidth = 640.0;
GLsizei screenHeight = 480.0;
static float zoomFactor = 1;
static GLfloat fPitch = 0.0;
static GLfloat fYaw   = 0.0;
static const GLfloat verticesOffset[8][3] = { {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0, 1.0}, {1.0, 1.0, 1.0}, {0.0, 1.0, 1.0} };
static const GLint edgeConnectionTable[12][2] = { {0,1}, {1,2}, {2,3}, {3,0}, {4,5}, {5,6}, {6,7}, {7,4}, {0,4}, {1,5}, {2,6}, {3,7}};
static const GLfloat edgeDirectionTable[12][3] = { {1.0, 0.0, 0.0},{0.0, 1.0, 0.0},{-1.0, 0.0, 0.0},{0.0, -1.0, 0.0}, {1.0, 0.0, 0.0},{0.0, 1.0, 0.0},{-1.0, 0.0, 0.0},{0.0, -1.0, 0.0}, {0.0, 0.0, 1.0},{0.0, 0.0, 1.0},{ 0.0, 0.0, 1.0},{0.0,  0.0, 1.0} };

struct GLvector
{
    GLfloat xVec;
    GLfloat yVec;
    GLfloat zVec;
};
GLvector  sSourcePoint[2];

void vIdle();
void vDrawScene();
void vResize(GLsizei, GLsizei);
void vKeyboard(unsigned char cKey, int iX, int iY);
void vSpecial(int iKey, int iX, int iY);
// Functions for menu rendering
void main_menu(int value);
void density_control(int value);
void color_control(int value);
void size_control(int value);
void camera_control(int value);
void shape_control(int value);

GLvoid vPrintHelp();
GLvoid init();
GLfloat fSampleSphere(GLfloat xVec, GLfloat yVec, GLfloat zVec);
GLfloat fSampleIntersection(GLfloat xVec, GLfloat yVec, GLfloat zVec);
GLfloat (*fSample)(GLfloat xVec, GLfloat yVec, GLfloat zVec) = fSampleSphere;
GLvoid marchingCubesAux();
GLvoid marchingCubes(GLfloat xVec, GLfloat yVec, GLfloat zVec, GLfloat scalingFactor);
GLvoid (*vMarchCube)(GLfloat xVec, GLfloat yVec, GLfloat zVec, GLfloat scalingFactor) = marchingCubes;

int main(int argc, char **argv)
{
    int density_menu, color_menu, size_menu, camera_menu, shape_menu;
    GLfloat afPropertiesAmbient [] = {0.50, 0.50, 0.50, 1.00};
    GLfloat afPropertiesDiffuse [] = {0.75, 0.75, 0.75, 1.00};
    GLfloat afPropertiesSpecular[] = {1.00, 1.00, 1.00, 1.00};

    glutInit(&argc, argv);
    glutInitWindowPosition( 0, 0);
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE );
    glutCreateWindow( "Cubos marchantes" );
    glutDisplayFunc( vDrawScene );
    glutIdleFunc( vIdle );
    glutReshapeFunc( vResize );
    glutKeyboardFunc( vKeyboard );
    glutSpecialFunc( vSpecial );

    glClearColor( 0.32, 0.32, 0.32, 1.0 );
    glClearDepth( 1.0 );

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, polyMode);

    glLightfv( GL_LIGHT0, GL_AMBIENT,  afPropertiesAmbient);
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  afPropertiesDiffuse);
    glLightfv( GL_LIGHT0, GL_SPECULAR, afPropertiesSpecular);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0);

    glEnable( GL_LIGHT0 );

    static const GLfloat afAmbientGreen [] = {1.00, 0.25, 0.00, 1.00};
    static const GLfloat afAmbientBlue  [] = {1.00, 0.00, 0.25, 1.00};
    static const GLfloat afDiffuseGreen [] = {1.00, 0.75, 0.00, 1.00};
    static const GLfloat afDiffuseBlue  [] = {1.00, 0.00, 0.75, 1.00};
    static const GLfloat afSpecularWhite[] = {1.00, 1.00, 1.00, 1.00};

    glMaterialfv(GL_BACK,  GL_AMBIENT,   afAmbientGreen);
    glMaterialfv(GL_BACK,  GL_DIFFUSE,   afDiffuseGreen);
    glMaterialfv(GL_FRONT, GL_AMBIENT,   afAmbientBlue);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   afDiffuseBlue);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  afSpecularWhite);
    glMaterialf( GL_FRONT, GL_SHININESS, 25.0);

    vResize(screenWidth, screenHeight);

    density_menu = glutCreateMenu(density_control);
    glutAddMenuEntry("More", 1);
    glutAddMenuEntry("Less", 2);

    color_menu = glutCreateMenu(color_control);
    glutAddMenuEntry("Psychedelic", 1);
    glutAddMenuEntry("Default", 2);

    size_menu = glutCreateMenu(size_control);
    glutAddMenuEntry("Smaller", 1);
    glutAddMenuEntry("Bigger", 2);

    camera_menu = glutCreateMenu(camera_control);
    glutAddMenuEntry("Zoom in", 1);
    glutAddMenuEntry("Zoom out", 2);
    glutAddMenuEntry("Rotate right", 4);
    glutAddMenuEntry("Rotate left", 3);
    glutAddMenuEntry("Rotate up", 5);
    glutAddMenuEntry("Rotate down", 6);
    
    shape_menu = glutCreateMenu(shape_control);
    glutAddMenuEntry("Sphere", 1);
    glutAddMenuEntry("Pipes", 2);

    glutCreateMenu(main_menu);
    glutAddMenuEntry("Quit", -1);
    glutAddSubMenu("Density"  , density_menu);
    glutAddSubMenu("Color"    , color_menu);
    glutAddSubMenu("Size"     , size_menu);
    glutAddSubMenu("Camera"   , camera_menu);
    glutAddSubMenu("Shape"   , shape_menu);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
}

void main_menu(int value)
{
    if (value == -1)
        exit(0);
}

void density_control(int value)
{
    switch (value) {
        case 1:
            ++iDataSetSize;
            fStepSize = 1.0/iDataSetSize;
            break;
        case 2:
            if(iDataSetSize > 1)
            {
                --iDataSetSize;
                fStepSize = 1.0/iDataSetSize;
            }
            break;
    }
    glutPostRedisplay();
}

void color_control(int value)
{
    switch (value) {
        case 1:
            glDisable(GL_LIGHTING);
            bLight = !bLight;
            break;
        case 2:
            glEnable(GL_LIGHTING);
            bLight = !bLight;
            break;
    }
    glutPostRedisplay();
}

void size_control(int value)
{
    switch (value) {
        case 1:
            if(isoValue < 1000.0)
            {
                isoValue *= 1.2;
            }
            break;
        case 2:
            if(isoValue > 1.0)
            {
                isoValue /= 1.2;
            }
            break;
    }
    glutPostRedisplay();
}

void camera_control(int value)
{
    switch (value) {
        case 1:
            zoomFactor -=  0.1; break;
        case 2:
            if(zoomFactor > 0.1) zoomFactor +=  0.1; break;
        case 3:
            fYaw += 4.0;  break;
        case 4:
            fYaw -= 4.0;  break;
        case 5:
            fPitch += 4.0;  break;
        case 6:
            fPitch -= 4.0;  break;
            
    }
    glutPostRedisplay();
}

void shape_control(int value)
{
    switch (value) {
        case 1:
            fSample =  fSampleSphere; break;
        case 2:
            fSample = fSampleIntersection; break;
    }
    glutPostRedisplay();
}

void vResize( GLsizei screenWidth, GLsizei screenHeight )
{
    GLfloat aspectValue, midPointView = (sqrt(2)/2);

    glViewport( 0, 0, screenWidth, screenHeight );
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    if(screenWidth <= screenHeight)
    {
        aspectValue = (GLfloat)screenHeight / (GLfloat)screenWidth;
        glOrtho(-midPointView*zoomFactor, midPointView*zoomFactor, -midPointView*aspectValue*zoomFactor,
                midPointView*aspectValue*zoomFactor, -10*midPointView, 10*midPointView);
    }
    else
    {
        aspectValue = (GLfloat)screenWidth / (GLfloat)screenHeight;
        glOrtho(-midPointView*aspectValue*zoomFactor, midPointView*aspectValue*zoomFactor, -midPointView*zoomFactor,
                midPointView*zoomFactor, -10*midPointView, 10*midPointView);
    }
    glMatrixMode( GL_MODELVIEW );
}

void vKeyboard(unsigned char cKey, int iX, int iY)
{
    switch(cKey)
    {
        case 's':
            if(isoValue < 1000.0)
            {
                isoValue *= 1.2;
            }
        break;
        case 'b':
            if(isoValue > 1.0)
            {
                isoValue /= 1.2;
            }
        break;
        case '+' :
        case '=' :
        {
            ++iDataSetSize;
            fStepSize = 1.0/iDataSetSize;
        } break;
        case '-' :
        {
            if(iDataSetSize > 1)
            {
                --iDataSetSize;
                fStepSize = 1.0/iDataSetSize;
            }
        } break;
        case 'l' :
        {
            if(bLight)
            {
                glDisable(GL_LIGHTING);//use vertex colors
            }
            else
            {
                glEnable(GL_LIGHTING);//use lit material color
            }

            bLight = !bLight;
        }
    }
}


void vSpecial(int iKey, int iX, int iY)
{
    switch(iKey)
    {
        case GLUT_KEY_PAGE_UP : {fPitch += 4.0; } break;
        case GLUT_KEY_PAGE_DOWN : {fPitch -= 4.0; } break;
        case GLUT_KEY_HOME : {fYaw += 4.0; } break;
        case GLUT_KEY_END : {fYaw -= 4.0 ; } break;
        case GLUT_KEY_UP : {
            zoomFactor +=  0.1;
        } break;
        case GLUT_KEY_DOWN : {
            if(zoomFactor > 0.1) zoomFactor -=  0.1;
        } break;

    }
}

void vIdle()
{
    GLfloat aspectValue, midPointView = (sqrt(2)/2);
    
    glViewport( 0, 0, screenWidth, screenHeight );
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    
    if(screenWidth <= screenHeight)
    {
        aspectValue = (GLfloat)screenHeight / (GLfloat)screenWidth;
        glOrtho(-midPointView*zoomFactor, midPointView*zoomFactor, -midPointView*aspectValue*zoomFactor,
                midPointView*aspectValue*zoomFactor, -10*midPointView, 10*midPointView);
    }
    else
    {
        aspectValue = (GLfloat)screenWidth / (GLfloat)screenHeight;
        glOrtho(-midPointView*aspectValue*zoomFactor, midPointView*aspectValue*zoomFactor, -midPointView*zoomFactor,
                midPointView*zoomFactor, -10*midPointView, 10*midPointView);
    }
    glMatrixMode( GL_MODELVIEW );
    glutPostRedisplay();
}

void vDrawScene()
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushMatrix();
    init();
    glTranslatef(0.0, 0.0, -1.0);
    glRotatef( -fPitch, 1.0, 0.0, 0.0);
    glRotatef(     0.0, 0.0, 1.0, 0.0);
    glRotatef(    fYaw, 0.0, 0.0, 1.0);

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0);
    glPopAttrib();


    glPushMatrix();
    glTranslatef(-0.5, -0.5, -0.5);
    glBegin(GL_TRIANGLES);
    marchingCubesAux();
    glEnd();
    glPopMatrix();


    glPopMatrix();

    glutSwapBuffers();
}

GLfloat fGetOffset(GLfloat fValue1, GLfloat fValue2, GLfloat fValueDesired)
{
    GLdouble fDelta = fValue2 - fValue1;

    if(fDelta == 0.0)
    {
        return 0.5;
    }
    return (fValueDesired - fValue1)/fDelta;
}

GLvoid vGetColor(GLvector &rfColor, GLvector &rfPosition, GLvector &rfNormal)
{
    GLfloat xVec = rfNormal.xVec;
    GLfloat yVec = rfNormal.yVec;
    GLfloat zVec = rfNormal.zVec;
    rfColor.xVec = (xVec > 0.0 ? xVec : 0.0) + (yVec < 0.0 ? -0.5*yVec : 0.0) + (zVec < 0.0 ? -0.5*zVec : 0.0);
    rfColor.yVec = (yVec > 0.0 ? yVec : 0.0) + (zVec < 0.0 ? -0.5*zVec : 0.0) + (xVec < 0.0 ? -0.5*xVec : 0.0);
    rfColor.zVec = (zVec > 0.0 ? zVec : 0.0) + (xVec < 0.0 ? -0.5*xVec : 0.0) + (yVec < 0.0 ? -0.5*yVec : 0.0);
}

GLvoid vNormalizeVector(GLvector &rfVectorResult, GLvector &rfVectorSource)
{
    GLfloat fOldLength;
    GLfloat scalingFactor;

    fOldLength = sqrtf( (rfVectorSource.xVec * rfVectorSource.xVec) +
                       (rfVectorSource.yVec * rfVectorSource.yVec) +
                       (rfVectorSource.zVec * rfVectorSource.zVec) );

    if(fOldLength == 0.0)
    {
        rfVectorResult.xVec = rfVectorSource.xVec;
        rfVectorResult.yVec = rfVectorSource.yVec;
        rfVectorResult.zVec = rfVectorSource.zVec;
    }
    else
    {
        scalingFactor = 1.0/fOldLength;
        rfVectorResult.xVec = rfVectorSource.xVec*scalingFactor;
        rfVectorResult.yVec = rfVectorSource.yVec*scalingFactor;
        rfVectorResult.zVec = rfVectorSource.zVec*scalingFactor;
    }
}

GLvoid init()
{
    for(GLint i = 0; i < 2; i++) {
        sSourcePoint[i].xVec = 0.5;
        sSourcePoint[i].yVec = 0.5;
        sSourcePoint[i].zVec = 0.5;
    }

}

GLfloat fSampleSphere(GLfloat xVec, GLfloat yVec, GLfloat zVec)
{
    GLdouble fResult = 0.0;
    GLdouble fDx, fDy, fDz;
    fDx = xVec - sSourcePoint[1].xVec;
    fDy = yVec - sSourcePoint[1].yVec;
    fDz = zVec - sSourcePoint[1].zVec;
    fResult += 1.5/(fDx*fDx + fDy*fDy + fDz*fDz);

    return fResult;
}

GLfloat fSampleIntersection(GLfloat xVec, GLfloat yVec, GLfloat zVec)
{
    GLdouble fResult = 0.0;
    GLdouble fDx, fDy, fDz;
    fDx = xVec - sSourcePoint[0].xVec;
    fDy = yVec - sSourcePoint[0].yVec;
    fResult += 0.5/(fDx*fDx + fDy*fDy);
    
    fDy = yVec - sSourcePoint[1].yVec;
    fDz = zVec - sSourcePoint[1].zVec;
    fResult += 1.0/(fDy*fDy + fDz*fDz);
    
    return fResult;
}

GLvoid vGetNormal(GLvector &rfNormal, GLfloat xVec, GLfloat yVec, GLfloat zVec)
{
    rfNormal.xVec = fSample(xVec-0.01, yVec, zVec) - fSample(xVec+0.01, yVec, zVec);
    rfNormal.yVec = fSample(xVec, yVec-0.01, zVec) - fSample(xVec, yVec+0.01, zVec);
    rfNormal.zVec = fSample(xVec, yVec, zVec-0.01) - fSample(xVec, yVec, zVec+0.01);
    vNormalizeVector(rfNormal, rfNormal);
}

GLvoid marchingCubes(GLfloat xVec, GLfloat yVec, GLfloat zVec, GLfloat scalingFactor)
{
    extern GLint cubeEdgeConfigurations[256];
    extern GLint a2iTriangleConnectionTable[256][16];

    GLint iCorner, currentVertex, iVertexTest, currentEdge, currentTriangle, cubeConfig, currentEdgeFlags;
    GLfloat fOffset;
    GLvector sColor;
    GLfloat afCubeValue[8];
    GLvector asEdgeVertex[12];
    GLvector asEdgeNorm[12];

    for(currentVertex = 0; currentVertex < 8; currentVertex++){
        afCubeValue[currentVertex] = fSample(xVec + verticesOffset[currentVertex][0]*scalingFactor, 
                                             yVec + verticesOffset[currentVertex][1]*scalingFactor, 
                                             zVec + verticesOffset[currentVertex][2]*scalingFactor);
    }

    cubeConfig = 0;
    for(iVertexTest = 0; iVertexTest < 8; iVertexTest++)
    {
        if(afCubeValue[iVertexTest] <= isoValue)
            cubeConfig |= 1<<iVertexTest;
    }

    currentEdgeFlags = cubeEdgeConfigurations[cubeConfig];

    if(currentEdgeFlags == 0)
    {
        return;
    }

    for(currentEdge = 0; currentEdge < 12; currentEdge++)
    {
        if(currentEdgeFlags & (1<<currentEdge)) //currentEdge * 2, (currentEdgeFlags == (currentEdge*2))
        {
            fOffset = fGetOffset(afCubeValue[ edgeConnectionTable[currentEdge][0] ], afCubeValue[ edgeConnectionTable[currentEdge][1] ], isoValue);
            asEdgeVertex[currentEdge].xVec = xVec + (verticesOffset[ edgeConnectionTable[currentEdge][0]][0]  +  fOffset * edgeDirectionTable[currentEdge][0]) * scalingFactor;
            asEdgeVertex[currentEdge].yVec = yVec + (verticesOffset[ edgeConnectionTable[currentEdge][0]][1]  +  fOffset * edgeDirectionTable[currentEdge][1]) * scalingFactor;
            asEdgeVertex[currentEdge].zVec = zVec + (verticesOffset[ edgeConnectionTable[currentEdge][0]][2]  +  fOffset * edgeDirectionTable[currentEdge][2]) * scalingFactor;

            vGetNormal(asEdgeNorm[currentEdge], asEdgeVertex[currentEdge].xVec, asEdgeVertex[currentEdge].yVec, asEdgeVertex[currentEdge].zVec);
        }
    }

    for(currentTriangle = 0; currentTriangle < 5; currentTriangle++)
    {
        if(a2iTriangleConnectionTable[cubeConfig][3*currentTriangle] < 0)
            break;

        for(iCorner = 0; iCorner < 3; iCorner++)
        {
            currentVertex = a2iTriangleConnectionTable[cubeConfig][3*currentTriangle+iCorner];
            vGetColor(sColor, asEdgeVertex[currentVertex], asEdgeNorm[currentVertex]);
            glColor3f(sColor.xVec, sColor.yVec, sColor.zVec);
            glNormal3f(asEdgeNorm[currentVertex].xVec,   asEdgeNorm[currentVertex].yVec,   asEdgeNorm[currentVertex].zVec);
            glVertex3f(asEdgeVertex[currentVertex].xVec, asEdgeVertex[currentVertex].yVec, asEdgeVertex[currentVertex].zVec);
        }
    }
}

GLvoid marchingCubesAux()
{
    GLint iX, iY, iZ;
    for(iX = 0; iX < iDataSetSize; iX++)
        for(iY = 0; iY < iDataSetSize; iY++)
            for(iZ = 0; iZ < iDataSetSize; iZ++)
            {
                vMarchCube(iX*fStepSize, iY*fStepSize, iZ*fStepSize, fStepSize);
            }
}


// Table for the cube configurations in the edges
// For each of the 8 vertices there are two possible states, inside or ouside of surface, in total 256
// there are 12 edges
GLint cubeEdgeConfigurations[256]=
{
    0x000, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
    0x190, 0x099, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c, 0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
    0x230, 0x339, 0x033, 0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
    0x3a0, 0x2a9, 0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5, 0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
    0x460, 0x569, 0x663, 0x76a, 0x066, 0x16f, 0x265, 0x36c, 0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
    0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0x0ff, 0x3f5, 0x2fc, 0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
    0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x055, 0x15c, 0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
    0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc, 0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
    0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc, 0x0cc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
    0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c, 0x15c, 0x055, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
    0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
    0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c, 0x36c, 0x265, 0x16f, 0x066, 0x76a, 0x663, 0x569, 0x460,
    0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa, 0x1a3, 0x2a9, 0x3a0,
    0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x033, 0x339, 0x230,
    0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c, 0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x099, 0x190,
    0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x000
};

//  For each of the possible vertex states listed in cubeEdgeConfigurations there is a specific triangulation
//  We found this table 
GLint a2iTriangleConnectionTable[256][16] =
{
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
    {3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
    {4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
    {9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
    {10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
    {5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
    {8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
    {2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
    {11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
    {5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
    {11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
    {11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
    {9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
    {6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
    {6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
    {8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
    {7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
    {3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
    {9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
    {8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
    {0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
    {6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
    {10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
    {10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
    {0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
    {3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
    {9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
    {8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
    {3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
    {10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
    {10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
    {7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
    {1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
    {11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
    {8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
    {0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
    {7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
    {7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
    {10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
    {0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
    {7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
    {9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
    {6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
    {4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
    {10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
    {8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
    {1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
    {10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
    {10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
    {9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
    {7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
    {3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
    {7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
    {3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
    {6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
    {9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
    {1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
    {4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
    {7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
    {6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
    {0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
    {6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
    {0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
    {11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
    {6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
    {5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
    {9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
    {1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
    {10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
    {0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
    {5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
    {11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
    {9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
    {7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
    {2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
    {9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
    {1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
    {10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
    {2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
    {0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
    {0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
    {9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
    {5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
    {5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
    {9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
    {1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
    {3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
    {4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
    {9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
    {11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
    {2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
    {9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
    {3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
    {1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
    {4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
    {0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
    {1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};
