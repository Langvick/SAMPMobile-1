#include "main.h"
#include "game.h"
#include <cmath>

void CQuaternion::SetFromMatrix(MATRIX4X4 mat)
{
	w = sqrt( std::max( (float)0, 1.0f + mat.right.X + mat.up.Y + mat.at.Z ) ) * 0.5f;
	x = sqrt( std::max( (float)0, 1.0f + mat.right.X - mat.up.Y - mat.at.Z ) ) * 0.5f;
	y = sqrt( std::max( (float)0, 1.0f - mat.right.X + mat.up.Y - mat.at.Z ) ) * 0.5f;
	z = sqrt( std::max( (float)0, 1.0f - mat.right.X - mat.up.Y + mat.at.Z ) ) * 0.5f;

	x = static_cast < float > ( copysign( x, mat.at.Y - mat.up.Z ) );
	y = static_cast < float > ( copysign( y, mat.right.Z - mat.at.X ) );
	z = static_cast < float > ( copysign( z, mat.up.X - mat.right.Y ) );
}

void CQuaternion::GetMatrix(PMATRIX4X4 mat)
{
	if(!mat) return;

	float sqw = w * w;
	float sqx = x * x;
	float sqy = y * y;
	float sqz = z * z;

	mat->right.X = ( sqx - sqy - sqz + sqw);
	mat->up.Y = (-sqx + sqy - sqz + sqw);
	mat->at.Z = (-sqx - sqy + sqz + sqw);

	float tmp1 = x * y;
	float tmp2 = z * w;
	mat->up.X = 2.0 * (tmp1 + tmp2);
	mat->right.Y = 2.0 * (tmp1 - tmp2);

	tmp1 = x*z;
	tmp2 = y*w;
	mat->at.X = 2.0 * (tmp1 - tmp2);
	mat->right.Z = 2.0 * (tmp1 + tmp2);
	tmp1 = y*z;
	tmp2 = x*w;
	mat->at.Y = 2.0 * (tmp1 + tmp2);
	mat->up.Z = 2.0 * (tmp1 - tmp2);
}

void CQuaternion::Normalize()
{
	double n = sqrt(x*x + y*y + z*z + w*w);

	w /= n;
	x /= n;
	y /= n;
	z /= n;
}

#define SLERP_DELTA 0.1
void CQuaternion::Slerp(CQuaternion *pQ1, CQuaternion *pQ2, float t)
{
	CQuaternion *pOut = this;

	if(!pQ1 || !pQ2) return;
	if(t > 1) return;

	float p1[4];
	double omega, cosom, sinom, scale0, scale1;
	cosom = pQ1->x*pQ2->x + pQ1->y*pQ2->y + pQ1->z*pQ2->z + pQ1->w*pQ2->w;

	if(cosom < 0.0) {
		cosom = -cosom;
		p1[0] = - pQ2->x;
		p1[1] = - pQ2->y;
		p1[2] = - pQ2->z;
		p1[3] = - pQ2->w;
	} else {
		p1[0] = pQ2->x;
		p1[1] = pQ2->y;
		p1[2] = pQ2->z;
		p1[3] = pQ2->w;
	}

	if((1.0 - cosom) > SLERP_DELTA) {
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = sin((1.0 - t) * omega) / sinom;
		scale1 = sin(t * omega) / sinom;
	} else {
		scale0 = 1.0 - t;
		scale1 = t;
	}

	Set(scale0 * pQ1->x + scale1 * p1[0],
		scale0 * pQ1->y + scale1 * p1[1],
		scale0 * pQ1->z + scale1 * p1[2],
		scale0 * pQ1->w + scale1 * p1[3]);
}