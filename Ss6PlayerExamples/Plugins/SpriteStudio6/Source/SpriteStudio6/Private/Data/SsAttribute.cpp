#include "SpriteStudio6PrivatePCH.h"
#include "SsAttribute.h"
#include "SsString_uty.h"


namespace
{
	static const FName ConstName_Target("target");
	static const FName ConstName_BlendType("blendType");
	static const FName ConstName_LT("LT");
	static const FName ConstName_RT("RT");
	static const FName ConstName_LB("LB");
	static const FName ConstName_RB("RB");
	static const FName ConstName_RGBA("rgba");
	static const FName ConstName_Rate("rate");
	static const FName ConstName_Color("color");
	static const FName ConstName_MapId("mapId");
	static const FName ConstName_Name("name");
	static const FName ConstName_Integer("integer");
	static const FName ConstName_Point("point");
	static const FName ConstName_Rect("rect");
	static const FName ConstName_String("string");
	static const FName ConstName_StartTime("startTime");
	static const FName ConstName_Speed("speed");
	static const FName ConstName_Independent("independent");
	static const FName ConstName_StartLabel("startLabel");
	static const FName ConstName_StartOffset("startOffset");
	static const FName ConstName_EndLabel("endLabel");
	static const FName ConstName_EndOffset("endOffset");
	static const FName ConstName_LoopNum("loopNum");
	static const FName ConstName_Infinity("infinity");
	static const FName ConstName_Reverse("reverse");
	static const FName ConstName_Pingpong("pingpong");
}

void FSsKeyframe::Serialize(FArchive& Ar)
{
	Value.Serialize(Ar);
}

void FSsAttribute::Serialize(FArchive& Ar)
{
	for(int32 i = 0; i < Key.Num(); ++i)
	{
		Key[i].Serialize(Ar);
	}
}

const FSsKeyframe*	FSsAttribute::FirstKey() const
{
	if ( 0 == Key.Num() )
		return 0;

	return &Key[0];
}

///時間から左側のキーを取得
const FSsKeyframe*	FSsAttribute::FindLeftKey( int time )
{
	if ( 0 == Key.Num() )
		return 0;

	int32 KeyIndex = GetLowerBoundKeyIndex(time);
	if(KeyIndex < 0)
	{
		return &Key[Key.Num()-1];
	}
	FSsKeyframe* Keyframe = &(Key[KeyIndex]);

	if ( Keyframe->Time == time ) return Keyframe;
	if ( 0 == KeyIndex )
	{
		if ( Keyframe->Time > time ) return 0;
	}else{
		--KeyIndex;
		Keyframe = &(Key[KeyIndex]);
	}

	if ( Keyframe->Time > time ) return 0;

	return Keyframe;
}

//時間から右側のキーを取得する
const FSsKeyframe*	FSsAttribute::FindRightKey( int time )
{
	if ( 0 == Key.Num() )
		return 0;

	int32 KeyIndex = GetUpperBoundKeyIndex(time);
	if(KeyIndex < 0 )
	{
		return 0;
	}
	return &Key[KeyIndex];
}


int32 FSsAttribute::GetLowerBoundKeyIndex(int32 Time)
{
	for(int32 i = 0; i < Key.Num(); ++i)
	{
		if(Time <= Key[i].Time)
		{
			return i;
		}
	}
	return -1;
}
int32 FSsAttribute::GetUpperBoundKeyIndex(int32 Time)
{
	for(int32 i = 0; i < Key.Num(); ++i)
	{
		if(Time < Key[i].Time)
		{
			return i;
		}
	}
	return -1;
}


//頂点カラーアニメデータの取得
void	GetSsPartsColorValue( const FSsKeyframe* key , SsPartsColorAnime& v )
{
	TEnumAsByte<SsColorBlendTarget::Type> target;
	__StringToEnum_( key->Value[ConstName_Target].get<FString>() , target );
	TEnumAsByte<SsBlendType::Type> blendtype;
	__StringToEnum_( key->Value[ConstName_BlendType].get<FString>() , blendtype);

	v.blendType = blendtype;
	v.target = target;

	if ( target == SsColorBlendTarget::Vertex )
	{
		SsHash lt = key->Value[ConstName_LT].get<SsHash>();
		SsHash rt = key->Value[ConstName_RT].get<SsHash>();
		SsHash lb = key->Value[ConstName_LB].get<SsHash>();
		SsHash rb = key->Value[ConstName_RB].get<SsHash>();

		ConvertStringToSsColor( lt[ConstName_RGBA].get<FString>() , v.colors[0].rgba);
		v.colors[0].rate = lt[ConstName_Rate].get<float>();

		ConvertStringToSsColor( rt[ConstName_RGBA].get<FString>() , v.colors[1].rgba);
		v.colors[1].rate = rt[ConstName_Rate].get<float>();

		ConvertStringToSsColor( lb[ConstName_RGBA].get<FString>() , v.colors[2].rgba);
		v.colors[2].rate = lb[ConstName_Rate].get<float>();

		ConvertStringToSsColor( rb[ConstName_RGBA].get<FString>() , v.colors[3].rgba);
		v.colors[3].rate = rb[ConstName_Rate].get<float>();

	}else{
		SsHash color = key->Value[ConstName_Color].get<SsHash>();

		ConvertStringToSsColor( color[ConstName_RGBA].get<FString>() , v.color.rgba);
		v.color.rate = color[ConstName_Rate].get<float>();
	}

}

//頂点カラーアニメデータの取得
void	GetSsColorValue( const FSsKeyframe* key , SsColorAnime& v )
{
	TEnumAsByte<SsColorBlendTarget::Type> target;
	__StringToEnum_( key->Value[ConstName_Target].get<FString>() , target );
	TEnumAsByte<SsBlendType::Type> blendtype;
	__StringToEnum_( key->Value[ConstName_BlendType].get<FString>() , blendtype);

	v.blendType = blendtype;
	v.target = target;

	if ( target == SsColorBlendTarget::Vertex )
	{
		SsHash lt = key->Value[ConstName_LT].get<SsHash>();
		SsHash rt = key->Value[ConstName_RT].get<SsHash>();
		SsHash lb = key->Value[ConstName_LB].get<SsHash>();
		SsHash rb = key->Value[ConstName_RB].get<SsHash>();

		ConvertStringToSsColor( lt[ConstName_RGBA].get<FString>() , v.colors[0].rgba);
		v.colors[0].rate = lt[ConstName_Rate].get<float>();

		ConvertStringToSsColor( rt[ConstName_RGBA].get<FString>() , v.colors[1].rgba);
		v.colors[1].rate = rt[ConstName_Rate].get<float>();

		ConvertStringToSsColor( lb[ConstName_RGBA].get<FString>() , v.colors[2].rgba);
		v.colors[2].rate = lb[ConstName_Rate].get<float>();

		ConvertStringToSsColor( rb[ConstName_RGBA].get<FString>() , v.colors[3].rgba);
		v.colors[3].rate = rb[ConstName_Rate].get<float>();

	}else{
		SsHash color = key->Value[ConstName_Color].get<SsHash>();
		ConvertStringToSsColor( color[ConstName_RGBA].get<FString>() , v.color.rgba);
		v.color.rate = color[ConstName_Rate].get<float>();
	}

}

void	GetSsVertexAnime( const FSsKeyframe* key , FSsVertexAnime& v )
{
	const FString& sLT = key->Value[ConstName_LT].get<FString>();
	const FString& sRT = key->Value[ConstName_RT].get<FString>();
	const FString& sLB = key->Value[ConstName_LB].get<FString>();
	const FString& sRB = key->Value[ConstName_RB].get<FString>();
	
	StringToPoint2( sLT , v.Offsets[0] );
	StringToPoint2( sRT , v.Offsets[1] );
	StringToPoint2( sLB , v.Offsets[2] );
	StringToPoint2( sRB , v.Offsets[3] );

}


void GetSsRefCell( const FSsKeyframe* key , SsRefCell& v )
{
	int id = key->Value[ConstName_MapId].get<int>();
	FName name = FName( *(key->Value[ConstName_Name].get<FString>()) );

	v.mapid = id;
	v.name = name;
}


void	GetSsUserDataAnime( const FSsKeyframe* key , SsUserDataAnime& v )
{
	v.integer = 0;
	v.point.X = v.point.Y = 0;
	v.rect.x = v.rect.y = v.rect.w = v.rect.h = 0; 
	v.string = FString(TEXT(""));
	v.useInteger = key->Value.IsExistHashkey(ConstName_Integer);
	v.usePoint = key->Value.IsExistHashkey(ConstName_Point);
	v.useRect = key->Value.IsExistHashkey(ConstName_Rect);
	v.useString = key->Value.IsExistHashkey(ConstName_String);

	if ( v.useInteger )
	{
		v.integer = key->Value[ConstName_Integer].get<int>();
	}

	if ( v.usePoint )
	{
		const FString& str = key->Value[ConstName_Point].get<FString>();
		StringToPoint2( str , v.point );
	}
	
	if ( v.useRect )
	{
		const FString& str = key->Value[ConstName_Rect].get<FString>();
		StringToIRect( str , v.rect );
	}

	if ( v.useString )
	{
		const FString& str = key->Value[ConstName_String].get<FString>();
		v.string = str;
	}

}

void	GetSsEffectParamAnime( const FSsKeyframe* key, SsEffectAttr& v )
{
	v.startTime = key->Value[ConstName_StartTime].get<int>();
	v.speed = key->Value[ConstName_Speed].get<float>();
	v.independent = key->Value[ConstName_Independent].get<bool>();
	v.curKeyframe = key->Time;

	int iflags = 0;
	if (v.independent)
	{
		iflags = iflags | EFFECT_LOOP_FLAG_INFINITY;
	}
	v.loopflag = iflags;
}

void	GetSsInstparamAnime( const FSsKeyframe* key , SsInstanceAttr& v )
{
	const FString& sstartLabel = key->Value[ConstName_StartLabel].get<FString>();
	const int& sstartOffset = key->Value[ConstName_StartOffset].get<int>();
	const FString& sendLabel = key->Value[ConstName_EndLabel].get<FString>();
	const int& sendOffset = key->Value[ConstName_EndOffset].get<int>();

	const float& sspeed = key->Value[ConstName_Speed].get<float>();

	const int& sloopNum = key->Value[ConstName_LoopNum].get<int>();
	const bool& sinfinity = key->Value[ConstName_Infinity].get<bool>();
	const bool& sreverse = key->Value[ConstName_Reverse].get<bool>();
	const bool& spingpong = key->Value[ConstName_Pingpong].get<bool>();
	const bool& sindependent = key->Value[ConstName_Independent].get<bool>();


	v.startLabel = FName(*sstartLabel);
	v.startOffset = sstartOffset;
	v.endLabel = FName(*sendLabel);
	v.endOffset = sendOffset;
	v.speed = sspeed;

	v.loopNum = sloopNum;
	v.infinity = sinfinity;
	v.reverse = sreverse;
	v.pingpong = spingpong;
	v.independent = sindependent;
	v.curKeyframe = key->Time;


	int iflags = 0;
	if (sinfinity)
	{
		iflags = iflags | INSTANCE_LOOP_FLAG_INFINITY;
	}
	if (sreverse)
	{
		iflags = iflags | INSTANCE_LOOP_FLAG_REVERSE;
	}
	if (spingpong)
	{
		iflags = iflags | INSTANCE_LOOP_FLAG_PINGPONG;
	}
	if (sindependent)
	{
		iflags = iflags | INSTANCE_LOOP_FLAG_INDEPENDENT;
	}
	v.loopflag = iflags;
}

//デフォームアニメデータの取得
void	GetSsDeformAnime(const FSsKeyframe* key, SsDeformAttr& v)
{
	const int& svsize = key->Value["vsize"].get<int>();
	const FString& sVchg = key->Value["vchg"].get<FString>();

	v.verticeChgList.Empty(svsize);

	TArray<FString>	str_list;
	split_string(sVchg, ' ', str_list);
	if (str_list.Num() < 1)
	{
	}
	else
	{
		//移動していいない点は出力されていない
		int datasize = FCString::Atoi(*str_list[0]);		//データ数
		int cnt = 0;
		for (int i = 0; i < svsize; i++)
		{
			FVector2D param(0, 0);
			if (cnt < datasize)
			{
				int idx = FCString::Atoi(*str_list[1 + (cnt * 3)]);		//index
				float x = FCString::Atof(*str_list[2 + (cnt * 3)]);	//x
				float y = FCString::Atof(*str_list[3 + (cnt * 3)]);	//y


				if (i == idx)
				{
					param.X = x;
					param.Y = y;
					cnt++;
				}
			}
			v.verticeChgList.Add(param);
		}
		if (cnt != datasize)
		{
			//データがおかしいのでは？
			UE_LOG(LogSpriteStudio, Error, TEXT("Deform Attr Data Error"));
		}
	}
}

bool StringToPoint2(const FString& str, FVector2D& point)
{
	FString LeftS, RightS;
	if(str.Split(TEXT(" "), &LeftS, &RightS))
	{
		point.X = FCString::Atof(*LeftS);
		point.Y = FCString::Atof(*RightS);
		return true;
	}
	else
	{
		point.X = point.Y = 0.f;
		return false;
	}
}

bool StringToIRect( const FString& str, SsIRect& rect )
{
	TArray<FString>	str_list;
	split_string( str , ' ' , str_list );
	if ( str_list.Num() < 4 )
	{
		rect.x = 0;
		rect.y = 0;
		rect.w = 0;
		rect.h = 0;
		return false;
	}else{
		rect.x = FCString::Atof(*(str_list[0]));
		rect.y = FCString::Atof(*(str_list[1]));
		rect.w = FCString::Atof(*(str_list[2]));
		rect.h = FCString::Atof(*(str_list[3]));
	}

	return true;
}

bool StringToTriangle(const FString& str, FSsTriangle& tri)
{
	TArray<FString>	str_list;
	split_string(str, ' ', str_list);
	if (str_list.Num() < 3)
	{
		return false;
	}
	else {
		tri.IdxPo1 = FCString::Atoi(*str_list[0]);
		tri.IdxPo2 = FCString::Atoi(*str_list[1]);
		tri.IdxPo3 = FCString::Atoi(*str_list[2]);

		return true;
	}
	return true;
}
