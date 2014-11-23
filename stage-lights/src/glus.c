#if defined SL_USE_GLFW3_GLES2_HARNESS || defined SL_USE_SDL2_GLES2_HARNESS

#include "glus.h"


#define PI 3.1415926535897932384626433832f


GLfloat glusVector3Lengthf(const GLfloat vector[3])
{
    return sqrtf(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
}


bool glusVector3Normalizef(GLfloat vector[3])
{
    GLint i;

    GLfloat length = glusVector3Lengthf(vector);

    if (length == 0.0f)
    {
        return false;
    }

    for (i = 0; i < 3; i++)
    {
        vector[i] /= length;
    }

    return true;
}


void glusVector3Crossf(GLfloat result[3], const GLfloat vector0[3], const GLfloat vector1[3])
{
    GLint i;

    GLfloat temp[3];

    temp[0] = vector0[1] * vector1[2] - vector0[2] * vector1[1];
    temp[1] = vector0[2] * vector1[0] - vector0[0] * vector1[2];
    temp[2] = vector0[0] * vector1[1] - vector0[1] * vector1[0];

    for (i = 0; i < 3; i++)
    {
        result[i] = temp[i];
    }
}


void glusMatrix4x4Multiplyf(GLfloat matrix[16], const GLfloat matrix0[16], const GLfloat matrix1[16])
{
    GLint i;
    GLint k;

    GLfloat temp[16];

    GLint row;
    GLint column;
    for (i = 0; i < 16; i++)
    {
        temp[i] = 0.0f;

        row = i % 4;
        column = (i / 4) * 4;

        for (k = 0; k < 4; k++)
        {
            //			  	   row   column   		  row column
            temp[i] += matrix0[row + k * 4] * matrix1[k + column];
        }
    }

    for (i = 0; i < 16; i++)
    {
        matrix[i] = temp[i];
    }
}


void glusMatrix4x4Identityf(GLfloat matrix[16])
{
    matrix[0] = 1.0f;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 1.0f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = 1.0f;
    matrix[11] = 0.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}


void glusMatrix4x4Scalef(GLfloat matrix[16], const GLfloat x, const GLfloat y, const GLfloat z)
{
    GLfloat temp[16];

    glusMatrix4x4Identityf(temp);

    temp[0] = x;
    temp[5] = y;
    temp[10] = z;

    glusMatrix4x4Multiplyf(matrix, matrix, temp);
}


void glusMatrix4x4Translatef(GLfloat matrix[16], const GLfloat x, const GLfloat y, const GLfloat z)
{
    GLfloat temp[16];

    glusMatrix4x4Identityf(temp);

    temp[12] = x;
    temp[13] = y;
    temp[14] = z;

    glusMatrix4x4Multiplyf(matrix, matrix, temp);
}


void glusLookAtf(GLfloat result[16], const GLfloat eyeX, const GLfloat eyeY, const GLfloat eyeZ, const GLfloat centerX, const GLfloat centerY, const GLfloat centerZ, const GLfloat upX, const GLfloat upY, const GLfloat upZ)
{
    GLfloat forward[3], side[3], up[3];

    forward[0] = centerX - eyeX;
    forward[1] = centerY - eyeY;
    forward[2] = centerZ - eyeZ;

    glusVector3Normalizef(forward);

    up[0] = upX;
    up[1] = upY;
    up[2] = upZ;

    glusVector3Crossf(side, forward, up);
    glusVector3Normalizef(side);

    glusVector3Crossf(up, side, forward);

    result[0] = side[0];
    result[1] = up[0];
    result[2] = -forward[0];
    result[3] = 0.0f;
    result[4] = side[1];
    result[5] = up[1];
    result[6] = -forward[1];
    result[7] = 0.0f;
    result[8] = side[2];
    result[9] = up[2];
    result[10] = -forward[2];
    result[11] = 0.0f;
    result[12] = 0.0f;
    result[13] = 0.0f;
    result[14] = 0.0f;
    result[15] = 1.0f;

    glusMatrix4x4Translatef(result, -eyeX, -eyeY, -eyeZ);
}


bool glusFrustumf(GLfloat result[16], const GLfloat left, const GLfloat right, const GLfloat bottom, const GLfloat top, const GLfloat nearVal, const GLfloat farVal)
{
	if ((right - left) == 0.0f || (top - bottom) == 0.0f || (farVal - nearVal) == 0.0f)
	{
		return false;
	}

	result[0] = 2.0f * nearVal / (right - left);
    result[1] = 0.0f;
    result[2] = 0.0f;
    result[3] = 0.0f;
    result[4] = 0.0f;
    result[5] = 2.0f * nearVal / (top - bottom);
    result[6] = 0.0f;
    result[7] = 0.0f;
    result[8] = (right + left) / (right - left);
    result[9] = (top + bottom) / (top - bottom);
    result[10] = -(farVal + nearVal) / (farVal - nearVal);
    result[11] = -1.0f;
    result[12] = 0.0f;
    result[13] = 0.0f;
    result[14] = -(2.0f * farVal * nearVal) / (farVal - nearVal);
    result[15] = 0.0f;

    return true;
}


bool glusPerspectivef(GLfloat result[16], const GLfloat fovy, const GLfloat aspect, const GLfloat zNear, const GLfloat zFar)
{
    GLfloat xmin, xmax, ymin, ymax;

    ymax = zNear * tanf(fovy * (PI / 360.0f));
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;

    return glusFrustumf(result, xmin, xmax, ymin, ymax, zNear, zFar);
}


#endif // SL_USE_GLES2_HARNESS

