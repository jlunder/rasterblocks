#ifndef GLUS_H_INCLUDED
#define GLUS_H_INCLUDED


#if defined STAGE_LIGHTS_LINUX
#include <GL/glfw.h>
#elif defined STAGE_LIGHTS_OSX
#include <GLFW/glfw3.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <math.h>


#define GLUS_UNUSED(x) (void)(x)


extern GLfloat glusVector3Lengthf(const GLfloat vector[3]);
extern bool glusVector3Normalizef(GLfloat vector[3]);
extern void glusVector3Crossf(GLfloat result[3], const GLfloat vector0[3], const GLfloat vector1[3]);
extern void glusMatrix4x4Multiplyf(GLfloat matrix[16], const GLfloat matrix0[16], const GLfloat matrix1[16]);
extern void glusMatrix4x4Identityf(GLfloat matrix[16]);
extern void glusMatrix4x4Scalef(GLfloat matrix[16], const GLfloat x, const GLfloat y, const GLfloat z);
extern void glusMatrix4x4Translatef(GLfloat matrix[16], const GLfloat x, const GLfloat y, const GLfloat z);
extern void glusLookAtf(GLfloat result[16], const GLfloat eyeX, const GLfloat eyeY, const GLfloat eyeZ, const GLfloat centerX, const GLfloat centerY, const GLfloat centerZ, const GLfloat upX, const GLfloat upY, const GLfloat upZ);
extern bool glusFrustumf(GLfloat result[16], const GLfloat left, const GLfloat right, const GLfloat bottom, const GLfloat top, const GLfloat nearVal, const GLfloat farVal);
extern bool glusPerspectivef(GLfloat result[16], const GLfloat fovy, const GLfloat aspect, const GLfloat zNear, const GLfloat zFar);


#endif

