#include "SpriteStudio6PrivatePCH.h"

#include "ssplayer_matrix.h"
#include <stdio.h>
#include <stdlib.h>
//#include <malloc.h>
#include <memory.h>
#include <math.h>

void	IdentityMatrix( float* matrix )
{

	float ident[] = {
		1.0f , 0.0f , 0.0f , 0.0f ,
		0.0f , 1.0f , 0.0f , 0.0f ,
		0.0f , 0.0f , 1.0f , 0.0f ,
		0.0f , 0.0f , 0.0f , 1.0f };


	memcpy( matrix , ident , sizeof(float) * 16 );

}


void    ScaleMatrix( float* _matrix , const float x , const float y , const float z)
{
	memset( _matrix , 0 , sizeof(float) * 16 );

	 _matrix[ 4 * 0 + 0 ] = x;
	 _matrix[ 4 * 1 + 1 ] = y;
	 _matrix[ 4 * 2 + 2 ] = z;
	 _matrix[ 4 * 3 + 3 ] = 1.0f;


}

void    TranslationMatrix( float* _matrix , const float x , const float y , const float z )
{
	memset( _matrix , 0 , sizeof(float) * 16 );

	_matrix[ 0 ] = 1.0f;
	_matrix[ 5 ] = 1.0f;
	_matrix[ 10 ] = 1.0f;


	_matrix[ 12 ] = x;
	_matrix[ 13 ] = y;
	_matrix[ 14 ] = z;
	_matrix[ 15 ] = 1.0f;

}


void MultiplyMatrix(const float *m0, const float *m1, float *matrix)
{

	float _temp[16];

    for (int i = 0; i < 16; ++i) {
        int j = i & ~3, k = i & 3;

        _temp[i] = m0[j + 0] * m1[ 0 + k]
        + m0[j + 1] * m1[ 4 + k]
        + m0[j + 2] * m1[ 8 + k]
        + m0[j + 3] * m1[12 + k];
    }

	memcpy( matrix , _temp , sizeof(float) * 16 );


}



void    Matrix4RotationX( float* _matrix ,const float radians )
{

	_matrix[0] = 1.0f;
	_matrix[1] = 0.0f;
	_matrix[2] = 0.0f;
	_matrix[3] = 0.0f;

	_matrix[4] = 0.0f;
	_matrix[5] = cosf(radians);
	_matrix[6] = sinf(radians);
	_matrix[7] = 0.0f;

	_matrix[8] = 0.0f;
	_matrix[9] = -sinf(radians);
	_matrix[10] = cosf(radians);
	_matrix[11] = 0.0f;

	_matrix[12] = 0.0f;
	_matrix[13] = 0.0f;
	_matrix[14] = 0.0f;
	_matrix[15] = 1.0f;


}


void    Matrix4RotationY( float* _matrix ,const float radians )
{

	_matrix[0] = cosf(radians);
	_matrix[1] = 0.0f;
	_matrix[2] = -sinf(radians);
	_matrix[3] = 0.0f;

	_matrix[4] = 0.0f;
	_matrix[5] = 1.0f;
	_matrix[6] = 0.0f;
	_matrix[7] = 0.0f;

	_matrix[8] = sinf(radians);
	_matrix[9] = 0.0f;
	_matrix[10] = cosf(radians);
	_matrix[11] = 0.0f;

	_matrix[12] = 0.0f;
	_matrix[13] = 0.0f;
	_matrix[14] = 0.0f;
	_matrix[15] = 1.0f;


}



void    Matrix4RotationZ( float* _matrix ,const float radians )
{
	_matrix[0] = cosf(radians);
	_matrix[1] = sinf(radians);
	_matrix[2] = 0.0f;
	_matrix[3] = 0.0f;

	_matrix[4] = -sinf(radians);
	_matrix[5] = cosf(radians);
	_matrix[6] = 0.0f;
	_matrix[7] = 0.0f;

	_matrix[8] = 0.0f;
	_matrix[9] = 0.0f;
	_matrix[10] = 1.0f;
	_matrix[11] = 0.0f;

	_matrix[12] = 0.0f;
	_matrix[13] = 0.0f;
	_matrix[14] = 0.0f;
	_matrix[15] = 1.0f;

}



void  MatrixTransformVector3(float* _matrix  , SsVector3& src, SsVector3& dst)
{
	float vx, vy, vz;
	//SsVector3 vec;
	float *pF1, *pF2, *pF3, *pF4;

	vx = src.x;
	vy = src.y;
	vz = src.z;

	pF1 = &_matrix[0];
	pF2 = &_matrix[4];
	pF3 = &_matrix[8];
	pF4 = &_matrix[12];

	dst.x = vx * (*pF1) + vy * (*pF2) + vz * (*pF3) + (*pF4);
	pF1 += 1; pF2 += 1; pF3 += 1; pF4 += 1;

	dst.y = vx * (*pF1) + vy * (*pF2) + vz * (*pF3) + (*pF4);
	pF1 += 1; pF2 += 1; pF3 += 1; pF4 += 1;

	dst.z = vx * (*pF1) + vy * (*pF2) + vz * (*pF3) + (*pF4);
	pF1 += 1; pF2 += 1; pF3 += 1; pF4 += 1;

	//dst = vec;

}

