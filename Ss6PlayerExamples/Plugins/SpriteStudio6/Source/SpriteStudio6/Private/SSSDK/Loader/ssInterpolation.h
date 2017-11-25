#ifndef __SSINTERPOLATION__
#define __SSINTERPOLATION__


struct FSsCurve;

///補間でカーブパラメータが必要なタイプか判定する
inline bool SsNeedsCurveParams(SsInterpolationType::Type type)
{
	switch (type)
	{
	case SsInterpolationType::Bezier:
	case SsInterpolationType::Hermite:
		return true;
	}
	return false;
}

///カーブパラメータ、補完方法により保管された値を生成する
float	SsInterpolate(SsInterpolationType::Type type, float time, float start, float end, const FSsCurve * curve);

#endif
