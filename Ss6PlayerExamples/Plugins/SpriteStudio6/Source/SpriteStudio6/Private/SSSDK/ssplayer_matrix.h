#ifndef __SSPLAYER_MATRIX__
#define __SSPLAYER_MATRIX__

#include "SsTypes.h"
#include <memory>

void	IdentityMatrix( float* matrix );
//void    ScaleMatrix( float* _matrix , const float x , const float y , const float z);
//void    TranslationMatrix( float* _matrix , const float x , const float y , const float z );
void	MultiplyMatrix(const float *m0, const float *m1, float *matrix);
void    Matrix4RotationX( float* _matrix ,const float radians );
void    Matrix4RotationY( float* _matrix ,const float radians );
void    Matrix4RotationZ( float* _matrix ,const float radians );

void	MatrixTransformVector3(float* _matrix, FVector3f& src, FVector3f& dst);



inline	void	TranslationMatrixM(  float* _matrix , const float x , const float y , const float z )
{
	float	_m[16];
//	IdentityMatrix( _m );
//	TranslationMatrix( _m , x , y , z );

	_m[0] = _m[5] = _m[10] = _m[15] = 1.f;
	_m[1] = _m[2] = _m[3] = _m[4] = _m[6] = _m[7] = _m[8] = _m[9] = _m[11] = 0.f;
	_m[12] = x; _m[13] = y; _m[14] = z;


	MultiplyMatrix( _m , _matrix , _matrix );
}

inline	void	ScaleMatrixM(  float* _matrix , const float x , const float y , const float z )
{

	float	_m[16];
//	IdentityMatrix( _m );
//	ScaleMatrix( _m , x , y , z );

	_m[1] = _m[2] = _m[3] = _m[4] = _m[6] = _m[7] = _m[8] = _m[9] = _m[11] = _m[12] = _m[13] = _m[14] = 0.f;
	_m[0] = x; _m[5] = y; _m[10] = z; _m[15] = 1.f;

	MultiplyMatrix( _m , _matrix , _matrix );
}

inline	void	RotationXYZMatrixM(  float* _matrix , const float x , const float y , const float z )
{

	if ( x != 0.0f )
	{
		float	_m[16];
//		IdentityMatrix( _m );
		Matrix4RotationX( _m , x );

		MultiplyMatrix( _m , _matrix , _matrix );
	}

	if ( y != 0.0f )
	{
		float	_m[16];
//		IdentityMatrix( _m );
		Matrix4RotationY( _m , y );

		MultiplyMatrix( _m , _matrix , _matrix );
	}

	if ( z != 0.0f )
	{
		float	_m[16];
//		IdentityMatrix( _m );
		Matrix4RotationZ( _m , z );

		MultiplyMatrix( _m , _matrix , _matrix );
	}
}



#endif

