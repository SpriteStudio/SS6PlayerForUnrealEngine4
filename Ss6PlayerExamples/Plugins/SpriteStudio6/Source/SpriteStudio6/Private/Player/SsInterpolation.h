#pragma once


struct FSsCurve;

///カーブパラメータ、補完方法により保管された値を生成する
float	SsInterpolate(SsInterpolationType::Type type, float time, float start, float end, const FSsCurve * curve);
