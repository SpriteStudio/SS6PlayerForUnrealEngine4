//#include "ssloader.h"
#include "ssInterpolation.h"



//---------------------------------------------------------------------------
/**
	等速運動
*/
//---------------------------------------------------------------------------
static float	// nowにおける補間された値
linear_(
	float	start,	// 始点
	float	end,	// 終点
	float	now)	// 現在の時間 0.f~1.f
{
	return start + (now * (end - start));
}

//---------------------------------------------------------------------------
/**
	加速運動
*/
//---------------------------------------------------------------------------
static float	// nowにおける補間された値
accelerating_(
	float	start,	// 始点
	float	end,	// 終点
	float	now)	// 現在の時間 0.f~1.f
{
	float rate = now * now;
	return start + (rate * (end - start));
}

//---------------------------------------------------------------------------
/**
	減速運動
*/
//---------------------------------------------------------------------------
static float	// nowにおける補間された値
decelerating_(
	float	start,	// 始点
	float	end,	// 終点
	float	now)	// 現在の時間 0.f~1.f
{
	float time = 1 - now;
	float rate = 1 - time * time;
	return start + (rate * (end - start));
}


static float bezier_(float start, float end, float time, const FSsCurve * c)
{
	//値が変化しない場合は左キーを補間値とする
	if ((start == end) && (c->StartValue == 0.0f) && (c->EndValue == 0.0f))
	{
		return start;
	}


	float fCurrentPos = (c->EndKeyTime - c->StartKeyTime) * time + c->StartKeyTime;

	float fRet = end;
	float fCurrentCalc = 0.5f;
	float fCalcRange = 0.5f;

	float fTemp1;
	float fTemp2;
	float fTemp3;

	float fCurrentX;

	for(int iLoop = 0; iLoop < 8; iLoop++ )
	{// more count of loop, better precision increase
		fTemp1 = 1.0f - fCurrentCalc;
		fTemp2 = fTemp1 * fTemp1;
		fTemp3 = fTemp2 * fTemp1;
		fCurrentX = ( fTemp3 * c->StartKeyTime ) +
					( 3 * fTemp2 * fCurrentCalc * (c->StartTime + c->StartKeyTime) ) +
					( 3 * fTemp1 * fCurrentCalc * fCurrentCalc * (c->EndTime + c->EndKeyTime) ) +
					( fCurrentCalc * fCurrentCalc * fCurrentCalc * c->EndKeyTime);

		fCalcRange /= 2.0f;
		if( fCurrentX > fCurrentPos )
		{
			fCurrentCalc -= fCalcRange;
		}
		else
		{
			fCurrentCalc += fCalcRange;
		}
	}

	// finally calculate with current value
	fTemp1 = 1.0f - fCurrentCalc;
	fTemp2 = fTemp1 * fTemp1;
	fTemp3 = fTemp2 * fTemp1;
	fRet = ( fTemp3 * start ) +
				( 3 * fTemp2 * fCurrentCalc * (c->StartValue + start) ) +
				( 3 * fTemp1 * fCurrentCalc * fCurrentCalc * (c->EndValue + end) ) +
				( fCurrentCalc * fCurrentCalc * fCurrentCalc * end );

	return fRet;
}

/**
	エルミートでは c->startTime, c->endTime は必要ない

	スロープ値を事前計算しておけばカーブ計算用パラメータは１つになる
	が、ベジェと共用するためこのままの形にしておく。
*/
static float hermite_(float start, float end, float time, const FSsCurve * c)
{
	float t2 = time * time;
	float t3 = t2 * time;
	float result =
		(2 * t3 - 3 * t2 + 1) * start +
		(-2 * t3 + 3 * t2) * end +
		(t3 - 2 * t2 + time) * (c->StartValue - start) +
		(t3 - t2) * (c->EndValue - end);
	return result;
}

/// SsVector2 のメンバ全体の補間
FVector2f	SsInterpolate(SsInterpolationType::Type ipType, float time, FVector2f start, FVector2f end, const FSsCurve * curve)
{
	FVector2f out;
	out.X = SsInterpolate(ipType, time, start.X, end.X, curve);
	out.Y = SsInterpolate(ipType, time, start.Y, end.Y, curve);

	return(out);
}

//----------------------------------------------------------------------------
/**
	タイプを指定して補間する
*/
//----------------------------------------------------------------------------
float	SsInterpolate(SsInterpolationType::Type type, float time, float start, float end, const FSsCurve * curve)
{
	float r(0.f);
	switch (type)
	{
	case SsInterpolationType::None:
		r = start;
		break;
	case SsInterpolationType::Linear:
		r = linear_(start, end, time);
		break;
	case SsInterpolationType::Acceleration:
		r = accelerating_(start, end, time);
		break;
	case SsInterpolationType::Deceleration:
		r = decelerating_(start, end, time);
		break;
	case SsInterpolationType::Bezier:
		r = bezier_(start, end, time, curve);
		break;
	case SsInterpolationType::Hermite:
		r = hermite_(start, end, time, curve);
		break;
	default:
		//SS_ASSERT_ID(type);
		break;
	}
	return r;
}
