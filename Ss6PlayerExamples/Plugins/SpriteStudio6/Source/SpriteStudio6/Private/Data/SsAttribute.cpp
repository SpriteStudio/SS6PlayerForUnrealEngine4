#include "SpriteStudio6PrivatePCH.h"
#include "SsAttribute.h"
#include "SsString_uty.h"


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

const FSsKeyframe*	FSsAttribute::FirstKey()
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
	__StringToEnum_( key->Value["target"].get<FString>() , target );
	TEnumAsByte<SsBlendType::Type> blendtype;
	__StringToEnum_( key->Value["blendType"].get<FString>() , blendtype);

	v.blendType = blendtype;
	v.target = target;

	if ( target == SsColorBlendTarget::Vertex )
	{
		SsHash lt = key->Value["LT"].get<SsHash>();
		SsHash rt = key->Value["RT"].get<SsHash>();
		SsHash lb = key->Value["LB"].get<SsHash>();
		SsHash rb = key->Value["RB"].get<SsHash>();

		ConvertStringToSsColor( lt["rgba"].get<FString>() , v.colors[0].rgba);
		v.colors[0].rate = lt["rate"].get<float>();

		ConvertStringToSsColor( rt["rgba"].get<FString>() , v.colors[1].rgba);
		v.colors[1].rate = rt["rate"].get<float>();

		ConvertStringToSsColor( lb["rgba"].get<FString>() , v.colors[2].rgba);
		v.colors[2].rate = lb["rate"].get<float>();

		ConvertStringToSsColor( rb["rgba"].get<FString>() , v.colors[3].rgba);
		v.colors[3].rate = rb["rate"].get<float>();

	}else{
		SsHash color = key->Value["color"].get<SsHash>();

		ConvertStringToSsColor( color["rgba"].get<FString>() , v.color.rgba);
		v.color.rate = color["rate"].get<float>();
	}

}

//頂点カラーアニメデータの取得
void	GetSsColorValue( const FSsKeyframe* key , SsColorAnime& v )
{
	TEnumAsByte<SsColorBlendTarget::Type> target;
	__StringToEnum_( key->Value["target"].get<FString>() , target );
	TEnumAsByte<SsBlendType::Type> blendtype;
	__StringToEnum_( key->Value["blendType"].get<FString>() , blendtype);

	v.blendType = blendtype;
	v.target = target;

	if ( target == SsColorBlendTarget::Vertex )
	{
		SsHash lt = key->Value["LT"].get<SsHash>();
		SsHash rt = key->Value["RT"].get<SsHash>();
		SsHash lb = key->Value["LB"].get<SsHash>();
		SsHash rb = key->Value["RB"].get<SsHash>();

		ConvertStringToSsColor( lt["rgba"].get<FString>() , v.colors[0].rgba);
		v.colors[0].rate = lt["rate"].get<float>();

		ConvertStringToSsColor( rt["rgba"].get<FString>() , v.colors[1].rgba);
		v.colors[1].rate = rt["rate"].get<float>();

		ConvertStringToSsColor( lb["rgba"].get<FString>() , v.colors[2].rgba);
		v.colors[2].rate = lb["rate"].get<float>();

		ConvertStringToSsColor( rb["rgba"].get<FString>() , v.colors[3].rgba);
		v.colors[3].rate = rb["rate"].get<float>();

	}else{
		SsHash color = key->Value["color"].get<SsHash>();
		ConvertStringToSsColor( color["rgba"].get<FString>() , v.color.rgba);
		v.color.rate = color["rate"].get<float>();
	}

}

void	GetSsVertexAnime( const FSsKeyframe* key , FSsVertexAnime& v )
{
	const FString& sLT = key->Value["LT"].get<FString>();
	const FString& sRT = key->Value["RT"].get<FString>();
	const FString& sLB = key->Value["LB"].get<FString>();
	const FString& sRB = key->Value["RB"].get<FString>();
	
	StringToPoint2( sLT , v.Offsets[0] );
	StringToPoint2( sRT , v.Offsets[1] );
	StringToPoint2( sLB , v.Offsets[2] );
	StringToPoint2( sRB , v.Offsets[3] );

}


void GetSsRefCell( const FSsKeyframe* key , FSsRefCell& v )
{
	int id = key->Value["mapId"].get<int>();
	FName name = FName( *(key->Value["name"].get<FString>()) );

	v.Mapid = id;
	v.Name = name;
}


void	GetSsUserDataAnime( const FSsKeyframe* key , FSsUserDataAnime& v )
{
	v.Integer = 0;
	v.Point.X = v.Point.Y = 0;
	v.Rect.x = v.Rect.y = v.Rect.w = v.Rect.h = 0; 
	v.String = FString(TEXT(""));
	v.UseInteger = key->Value.IsExistHashkey("integer");
	v.UsePoint = key->Value.IsExistHashkey("point");
	v.UseRect = key->Value.IsExistHashkey("rect");
	v.UseString = key->Value.IsExistHashkey("string");

	if ( v.UseInteger )
	{
		v.Integer = key->Value["integer"].get<int>();
	}

	if ( v.UsePoint )
	{
		const FString& str = key->Value["point"].get<FString>();
		StringToPoint2( str , v.Point );
	}
	
	if ( v.UseRect )
	{
		const FString& str = key->Value["rect"].get<FString>();
		StringToIRect( str , v.Rect );
	}

	if ( v.UseString )
	{
		const FString& str = key->Value["string"].get<FString>();
		v.String = str;
	}

}

void	GetSsEffectParamAnime( const FSsKeyframe* key, FSsEffectAttr& v )
{
	v.StartTime = key->Value["startTime"].get<int>();
	v.Speed = key->Value["speed"].get<float>();
	v.Independent = key->Value["independent"].get<bool>();
	v.CurKeyframe = key->Time;

	int iflags = 0;
	if (v.Independent)
	{
		iflags = iflags | SsEffectLoopFlag::EFFECT_LOOP_FLAG_INFINITY;
	}
	v.LoopFlag = iflags;
}

void	GetSsInstparamAnime( const FSsKeyframe* key , FSsInstanceAttr& v )
{
	const FString& sstartLabel = key->Value["startLabel"].get<FString>();
	const int& sstartOffset = key->Value["startOffset"].get<int>();
	const FString& sendLabel = key->Value["endLabel"].get<FString>();
	const int& sendOffset = key->Value["endOffset"].get<int>();

	const float& sspeed = key->Value["speed"].get<float>();

	const int& sloopNum = key->Value["loopNum"].get<int>();
	const bool& sinfinity = key->Value["infinity"].get<bool>();
	const bool& sreverse = key->Value["reverse"].get<bool>();
	const bool& spingpong = key->Value["pingpong"].get<bool>();
	const bool& sindependent = key->Value["independent"].get<bool>();


	v.StartLabel = FName(*sstartLabel);
	v.StartOffset = sstartOffset;
	v.EndLabel = FName(*sendLabel);
	v.EndOffset = sendOffset;
	v.Speed = sspeed;

	v.LoopNum = sloopNum;
	v.Infinity = sinfinity;
	v.Reverse = sreverse;
	v.Pingpong = spingpong;
	v.Independent = sindependent;
	v.CurKeyframe = key->Time;


	int iflags = 0;
	if (sinfinity)
	{
		iflags = iflags | SsInstanceLoopFlag::INSTANCE_LOOP_FLAG_INFINITY;
	}
	if (sreverse)
	{
		iflags = iflags | SsInstanceLoopFlag::INSTANCE_LOOP_FLAG_REVERSE;
	}
	if (spingpong)
	{
		iflags = iflags | SsInstanceLoopFlag::INSTANCE_LOOP_FLAG_PINGPONG;
	}
	if (sindependent)
	{
		iflags = iflags | SsInstanceLoopFlag::INSTANCE_LOOP_FLAG_INDEPENDENT;
	}
	v.LoopFlag = iflags;
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
