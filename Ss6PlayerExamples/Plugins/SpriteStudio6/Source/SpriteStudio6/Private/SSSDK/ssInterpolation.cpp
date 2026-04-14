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

static float easeIn(float start, float end, float time, float easeingRate)
{
	float t = FMath::Pow(time, easeingRate);
	return linear_(start, end, t);
}
static float easeOut(float start, float end, float time, float easeingRate)
{
	float t = FMath::Pow(time, 1 / easeingRate);
	return linear_(start, end, t);
}
static float easeInOut(float start, float end, float time, float easeingRate)
{
	time *= 2;
	float t = 0;
	if (time < 1)
	{
		t = 0.5f * FMath::Pow(time, easeingRate);
	}
	else
	{
		t = 1.0f - 0.5f * FMath::Pow(2 - time, easeingRate);
	}

	return linear_(start, end, t);
}

static float easeExponentialIn(float start, float end, float time, float easeingRate)
{
	float t = (time == 0 ? 0 : FMath::Pow(2, 10 * (time / 1 - 1)) - 1 * 0.001f);
	return linear_(start, end, t);
}

static float easeExponentialOut(float start, float end, float time, float easeingRate)
{
	float t = (time == 1 ? 1 : (-FMath::Pow(2, -10 * time / 1) + 1));
	return linear_(start, end, t);
}

static float easeExponentialInOut(float start, float end, float time, float easeingRate)
{
	time /= 0.5f;
	float t = 0;
	if (time < 1)
	{
		t = 0.5f * FMath::Pow(2, 10 * (time - 1));
	}
	else
	{
		t = 0.5f * (-FMath::Pow(2, -10 * (time - 1)) + 2);
	}

	return linear_(start, end, t);
}

static float  easeSineIn(float start, float end, float time, float easeingRate)
{
	float t = (-1 * FMath::Cos(time * UE_HALF_PI) + 1);
	return linear_(start, end, t);
}

static float easeSineOut(float start, float end, float time, float easeingRate)
{
	float t = (FMath::Sin(time * UE_HALF_PI));
	return linear_(start, end, t);
}

static float easeSineInOut(float start, float end, float time, float easeingRate)
{
	float t = (-0.5f * (FMath::Cos(UE_PI * time) - 1));
	return linear_(start, end, t);
}

//set period of the wave in radians.
static float easeElasticIn(float start, float end, float time, float fPeriod)
{
	float newT = 0;
	if (time == 0 || time == 1)
	{
		newT = time;
	}
	else
	{
		float s = fPeriod / 4;
		time = time - 1;
		newT = -FMath::Pow(2, 10 * time) * FMath::Sin((time - s) * UE_TWO_PI / fPeriod);
	}

	return linear_(start, end, newT);
}

static float easeElasticOut(float start, float end, float time, float fPeriod)
{
	float newT = 0;
	if (time == 0 || time == 1)
	{
		newT = time;
	}
	else
	{
		float s = fPeriod / 4;
		newT = FMath::Pow(2, -10 * time) * FMath::Sin((time - s) * UE_TWO_PI / fPeriod) + 1;
	}

	return linear_(start, end, newT);
}

static float easeElasticInOut(float start, float end, float time, float fPeriod)
{
	float newT = 0;
	if (time == 0 || time == 1)
	{
		newT = time;
	}
	else
	{
		time = time * 2;
		if (!fPeriod)
		{
			fPeriod = 0.3f * 1.5f;
		}

		float s = fPeriod / 4;

		time = time - 1;
		if (time < 0)
		{
			newT = -0.5f * FMath::Pow(2, 10 * time) * FMath::Sin((time - s) * UE_TWO_PI / fPeriod);
		}
		else
		{
			newT = FMath::Pow(2, -10 * time) * FMath::Sin((time - s) * UE_TWO_PI / fPeriod) * 0.5f + 1;
		}
	}

	return linear_(start, end, newT);

}

static float bounceTime(float time)
{
	if (time < 1 / 2.75)
	{
		return 7.5625f * time * time;
	}
	else
		if (time < 2 / 2.75)
		{
			time -= 1.5f / 2.75f;
			return 7.5625f * time * time + 0.75f;
		}
		else
			if (time < 2.5 / 2.75)
			{
				time -= 2.25f / 2.75f;
				return 7.5625f * time * time + 0.9375f;
			}

	time -= 2.625f / 2.75f;
	return 7.5625f * time * time + 0.984375f;
}

static float easeBounceIn(float start, float end, float time, float easeingRate)
{
	float newT = 1 - bounceTime(1 - time);
	return linear_(start, end, newT);
}

static float easeBounceOut(float start, float end, float time, float easeingRate)
{
	float newT = bounceTime(time);
	return linear_(start, end, newT);
}

static float easeBounceInOut(float start, float end, float time, float easeingRate)
{
	float newT = 0;
	if (time < 0.5f)
	{
		time = time * 2;
		newT = (1 - bounceTime(1 - time)) * 0.5f;
	}
	else
	{
		newT = bounceTime(time * 2 - 1) * 0.5f + 0.5f;
	}

	return linear_(start, end, newT);
}

static float easeBackIn(float start, float end, float time, float easeingRate)
{
	float overshoot = 1.70158f;
	float newT = (time * time * ((overshoot + 1) * time - overshoot));
	return linear_(start, end, newT);

}

static float easeBackOut(float start, float end, float time, float easeingRate)
{
	float overshoot = 1.70158f;

	time = time - 1;
	float newT = (time * time * ((overshoot + 1) * time + overshoot) + 1);
	return linear_(start, end, newT);

}

static float easeBackInOut(float start, float end, float time, float easeingRate)
{
	float overshoot = 1.70158f * 1.525f;

	time = time * 2;
	float newT = 0;
	if (time < 1)
	{
		newT = ((time * time * ((overshoot + 1) * time - overshoot)) / 2);
	}
	else
	{
		time = time - 2;
		newT = ((time * time * ((overshoot + 1) * time + overshoot)) / 2 + 1);
	}
	return linear_(start, end, newT);
}

/// SsVector2 のメンバ全体の補間
FVector2f	SsInterpolate(SsInterpolationType::Type ipType, float easingRate, float time, FVector2f start, FVector2f end, const FSsCurve * curve)
{
	FVector2f out;
	out.X = SsInterpolate(ipType, easingRate , time, start.X, end.X, curve);
	out.Y = SsInterpolate(ipType, easingRate , time, start.Y, end.Y, curve);

	return(out);
}

//----------------------------------------------------------------------------
/**
	タイプを指定して補間する
*/
//----------------------------------------------------------------------------
float	SsInterpolate(SsInterpolationType::Type type,  float easingRate, float time, float start, float end, const FSsCurve * curve)
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

	case SsInterpolationType::EaseIn:
		r = easeIn(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseOut:
		r = easeOut(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseInOut:
		r = easeInOut(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseExponentialIn:
		r = easeExponentialIn(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseExponentialOut:
		r = easeExponentialOut(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseExponentialInOut:
		r = easeExponentialInOut(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseSineIn:
		r = easeSineIn(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseSineOut:
		r = easeSineOut(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseSineInOut:
		r = easeSineInOut(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseElasticIn:
		r = easeElasticIn(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseElasticOut:
		r = easeElasticOut(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseElasticInOut:
		r = easeElasticInOut(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseBounceIn:
		r = easeBounceIn(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseBounceOut:
		r = easeBounceOut(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseBounceInOut:
		r = easeBounceInOut(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseBackIn:
		r = easeBackIn(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseBackOut:
		r = easeBackOut(start, end, time, easingRate);
		break;
	case SsInterpolationType::EaseBackInOut:
		r = easeBackInOut(start, end, time, easingRate);
		break;

	default:
		//SS_ASSERT_ID(type);
		break;
	}
	return r;
}
