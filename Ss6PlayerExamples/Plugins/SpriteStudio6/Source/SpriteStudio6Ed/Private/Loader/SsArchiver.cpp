#include "SsArchiver.h"
#include "SpriteStudio6EdPrivatePCH.h"

#include "Logging/MessageLog.h"

#include "SsString_uty.h"
#include "SsTypes.h"
#include "SsAttribute.h"
#include "SsValue.h"
#include "SsAnimePack.h"
#include "SsCellMap.h"
#include "SsEffectFile.h"
#include "SsEffectElement.h"
#include "Ss6Project.h"


namespace
{
	// 日本語文字列が含まれるかチェックし、含まれた場合はユニークな文字列に置き換える 
	//    FNameに変換した際に、'?'とかに置き換えられて、別の文字列でも同じとみなされてしまい、参照が切れるのを防ぐため 
	void CheckReplaceJapaneseString(const char* Text, FString& String)
	{
		bool bNG_Char = false;

		for(int32 i = 0; 0 != Text[i]; ++i)
		{
			if((int32)Text[i] != (int32)(*String)[i])
			{
				bNG_Char = true;
				break;
			}
		}

		// 日本語文字列が含まれてしまう場合は、 
		if(bNG_Char)
		{
			String = FString("Replaced");
			for(int32 i = 0; 0 != Text[i]; ++i)
			{
				String = FString::Printf(TEXT("%s/%d"), *String, (int32)Text[i]);
			}
			UE_LOG(LogSpriteStudioEd, Warning, TEXT("Replace Japanese String \"%s\" to \"%s\""), UTF8_TO_TCHAR(Text), *String);
		}
	}
}

bool SsXmlIArchiver::dc_attr(const char* name, FString& member)
{
	AR_SELF_CHECK();
	const char* v = 0;

	v = getxml()->Attribute(name);
	if(v != 0){ member= v; }

	return true;
}
bool SsXmlIArchiver::dc_attr(const char* name, int& member)
{
	AR_SELF_CHECK();

	const char* v = getxml()->Attribute(name);

	member = atoi(v);

	return true;
}

bool SsXmlIArchiver::dc(const char* name, int& member)
{
	AR_SELF_CHECK();
	FString str;
	dc(name , str);
	member = FCString::Atoi(*str);
	if (str.IsEmpty())
	{
		return false;
	}

	return true;
}

bool SsXmlIArchiver::dc(const char* name, float& member)
{
	AR_SELF_CHECK();
	FString str;
	dc(name , str);
	member = FCString::Atof(*str);
	if (str.IsEmpty())
	{
		return false;
	}

	return true;
}

bool SsXmlIArchiver::dc(const char* name, FString& member)
{
	AR_SELF_CHECK();
	XMLElement* e = getxml()->FirstChildElement(name);
	if(e)
	{
		if(e->GetText())
		{
			member = FString(babel::utf8_to_sjis( e->GetText() ).c_str());
			CheckReplaceJapaneseString(e->GetText(), member);
		}
		else
		{
			member ="";
		}
		return true;
	}
	return false;
}

bool SsXmlIArchiver::dc(const char* name, FName& member)
{
	AR_SELF_CHECK();

	FString dummy;
	if(dc(name, dummy))
	{
		member = FName(*dummy);
		return true;
	}
	return false;
}

bool SsXmlIArchiver::dc(const char* name, bool& member)
{
	AR_SELF_CHECK();
	member = false;
	XMLElement* e = getxml()->FirstChildElement(name);
	if(e)
	{
		int ret = GetTextToInt(e , 0);
		if(ret == 1)member = true;
		return true;
	}

	return false;
}

bool SsXmlIArchiver::dc(const char* name, TArray<FString>& list)
{
	AR_SELF_CHECK();
	list.Empty();
	XMLElement* e = getxml()->FirstChildElement(name);
	if(e)
	{
		e = e->FirstChildElement("value");
		while(e)
		{
			const char* txt = e->GetText();
			FString sjis_str( babel::utf8_to_sjis( txt ).c_str() );
			CheckReplaceJapaneseString(txt, sjis_str);

			list.Add( sjis_str );
			e = e->NextSiblingElement();
		}
		return true;
	}
	return false;
}

bool SsXmlIArchiver::dc(const char* name, TArray<FName>& list)
{
	AR_SELF_CHECK();
	list.Empty();
	XMLElement* e = getxml()->FirstChildElement(name);
	if(e)
	{
		e = e->FirstChildElement("value");
		while(e)
		{
			const char* txt = e->GetText();
			FString sjis_str( babel::utf8_to_sjis( txt ).c_str() );
			CheckReplaceJapaneseString(txt, sjis_str);

			list.Add( FName(*sjis_str) );
			e = e->NextSiblingElement();
		}
		return true;
	}
	return false;
}

bool SsXmlIArchiver::dc(const char* name, TArray<FVector2f>& list)
{
	AR_SELF_CHECK();
	list.Empty();
	XMLElement* e = getxml()->FirstChildElement(name);
	if(e)
	{
		e = e->FirstChildElement("value");
		while(e)
		{
			FString str( e->GetText() );
			FVector2f vec;
			StringToPoint2( str , vec );

			list.Add( vec );
			e = e->NextSiblingElement();
		}
		return true;
	}
	return false;
}

bool SsXmlIArchiver::dc(const char* name, TArray<FSsTriangle>& list)
{
	AR_SELF_CHECK();
	list.Empty();
	XMLElement* e = getxml()->FirstChildElement(name);
	if(e)
	{
		e = e->FirstChildElement("value");
		while (e)
		{
			FString txt( e->GetText() );
			FSsTriangle tri;
			StringToTriangle(txt, tri);

			list.Add(tri);
			e = e->NextSiblingElement();
		}
		return true;
	}
	return false;
}

bool SsXmlIArchiver::dc(const char* name, FVector2f& member)
{
	AR_SELF_CHECK();

	XMLElement* e = getxml()->FirstChildElement(name);
	if(e)
	{
		FString str( e->GetText() );
		return StringToPoint2( str , member );
	}
	return false;
}

bool SsXmlIArchiver::dc(const char* name, FSsCurve& member)
{
	AR_SELF_CHECK();

	XMLElement* e = getxml()->FirstChildElement( name );
	if(e)
	{
		TArray<FString>	str_list;
		split_string( e->GetText() , ' ' , str_list );
		if(str_list.Num() < 4)
		{
			return false;
		}else{
			member.StartTime = FCString::Atof(*(str_list[0]));
			member.StartValue = FCString::Atof(*(str_list[1]));
			member.EndTime = FCString::Atof(*(str_list[2]));
			member.EndValue = FCString::Atof(*(str_list[3]));
			return true;
		}
	}
	return false;
}

bool SsXmlIArchiver::dc(const char* name, FSsTriangle& member)
{
	AR_SELF_CHECK();

	XMLElement* e = getxml()->FirstChildElement(name);
	if(e)
	{
		StringToTriangle(e->GetText(), member);
	}
	return false;
}

bool SsXmlIArchiver::dc(const char* name, TMap<FName, int32>& map)
{
	AR_SELF_CHECK();
	map.Empty();
	XMLElement* e = getxml()->FirstChildElement(name);
	if (e == nullptr){ return false; }
	e = e->FirstChildElement("item");

	while (e)
	{
		FString key(babel::utf8_to_sjis( e->Attribute("key") ).c_str());
		FString val(babel::utf8_to_sjis( e->GetText() ).c_str());

		map.Add(FName(*key), FCString::Atoi(*val));
		e = e->NextSiblingElement();
	}

	return false;
}


void SsArchiverInit()
{
	babel::init_babel();
}

///補間でカーブパラメータが必要なタイプか判定する
bool SsNeedsCurveParams(SsInterpolationType::Type type)
{
	switch (type)
	{
	case SsInterpolationType::Bezier:
	case SsInterpolationType::Hermite:
		return true;
	}
	return false;
}


FSsValue SsValueSeriarizer__MakeValue(const char* v, FName HashKey = NAME_None)
{
	FString temp(v);
	if(HashKey == FName("rgba"))
	{
		SsColor Color;
		ConvertStringToSsColor(temp, Color);
		return FSsValue(Color);
	}
	else if(temp.IsNumeric() && (HashKey != FName("name")))
	{
		return FSsValue((float)atof(v));
	}
	else
	{
		FString ValueStr(babel::utf8_to_sjis(v).c_str());
		CheckReplaceJapaneseString(v, ValueStr);

		// ランタイムでパースする必要のあるアトリビュートは、シリアライズ時にintに変換しておく 
		if(HashKey == FName("target"))
		{
			TEnumAsByte<SsColorBlendTarget::Type> Target;
			__StringToEnum_(ValueStr, Target);
			int32 EnumValue = static_cast<int32>(Target);
			return FSsValue(EnumValue);
		}
		if(HashKey == FName("blendType"))
		{
			TEnumAsByte<SsBlendType::Type> BlendType;
			__StringToEnum_(ValueStr, BlendType);
			int32 EnumValue = static_cast<int32>(BlendType);
			return FSsValue(EnumValue);
		}
		if(    (HashKey == "LT")
			|| (HashKey == "RT")
			|| (HashKey == "LB")
			|| (HashKey == "RB")
			)
		{
			FVector2f Point;
			StringToPoint2(ValueStr, Point);
			return FSsValue(Point);
		}

		return FSsValue(ValueStr);
	}
}
void SsValueSeriarizer(ISsXmlArchiver* ar , FSsValue& v , const FString key = "value")
{
	char tmp[32];

	//インプット
	XMLElement* e = ar->getxml();
	if ( key != "" )
	{
#if PLATFORM_WINDOWS
		sprintf_s(tmp, 32, "%S", *key);
#else
		sprintf(tmp, "%s", TCHAR_TO_ANSI(*key));
#endif
		e = e->FirstChildElement( tmp );
	}
	if ( e )
	{
		XMLElement* child = e->FirstChildElement();

		if ( child == 0 )
		{
			const char* str = e->GetText();

			if (str==0)
			{
#if 0
				const char *err_log1(nullptr), *err_log2(nullptr), *err_log3(nullptr);
				if(e->Parent())
				{
					err_log1 = e->Parent()->Value();
					err_log2 = e->Parent()->ToElement()->GetText();
					if(e->Parent()->Parent() && e->Parent()->Parent()->ToElement() && e->Parent()->Parent()->ToElement()->FirstAttribute())
					{
						e->Parent()->Parent()->ToElement()->FirstAttribute()->Value();
					}
				}
#endif
				return ;
			}
			
			v = SsValueSeriarizer__MakeValue( str );
		}else{
			XMLElement* ce = child;

			SsHash hash;
			while(ce)
			{
				TCHAR ceName[32];
#if PLATFORM_WINDOWS
				FUTF8ToTCHAR_Convert::Convert(ceName, 32, ce->Name(), strnlen_s(ce->Name(), 32) + 1);
#else
				FUTF8ToTCHAR_Convert::Convert(ceName, 32, ce->Name(), strnlen(ce->Name(), 32) + 1);
#endif
				FName fceName(ceName);
				if(!hash.Contains(fceName))
				{
					hash.Add(fceName);
				}

				const char* str = ce->GetText();
				if ( str != 0 )
				{
					hash[fceName] = SsValueSeriarizer__MakeValue(str, fceName);
					ce = (XMLElement*)ce->NextSibling();
				}else{
					//さらに子構造があるようだ
					//さらに子構造があるようだ
					//ce = 0 ;
					SsXmlIArchiver ar2(ce);

					FSsValue tempv;
					SsValueSeriarizer( &ar2 , tempv , "");
					hash[fceName] = tempv;					
					ce = (XMLElement*)ce->NextSibling();
				}
			}

			v = FSsValue(hash);

		}

		return ;
	}
}
void SerializeStruct(FSsKeyframe& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE_ATTRIBUTE("time", Value.Time);
	SSAR_DECLARE_ATTRIBUTE_ENUM("ipType", Value.IpType);

	if(SsNeedsCurveParams(Value.IpType))
	{
		SSAR_DECLARE("curve", Value.Curve);
	}
	SsValueSeriarizer(ar , Value.Value);
}
void SerializeStruct(FSsAttribute& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE_ATTRIBUTE_ENUM("tag", Value.Tag);
	SSAR_DECLARE_LISTEX("key", Value.Key,  "");
}
void SerializeStruct(FSsCell& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("name", Value.CellName);
	SSAR_DECLARE("pos", Value.Pos);
	SSAR_DECLARE("size", Value.Size);
	SSAR_DECLARE("pivot", Value.Pivot);
	SSAR_DECLARE("rotated", Value.Rotated);

	SSAR_DECLARE("ismesh", Value.IsMesh);
	SSAR_DECLARE("innerPoint", Value.InnerPoint);
	SSAR_DECLARE("outerPoint", Value.OuterPoint);
	SSAR_DECLARE("meshPointList", Value.MeshPointList);
	SSAR_DECLARE("meshTriList", Value.MeshTriList);
	SSAR_DECLARE_ENUM("divtype", Value.DivType);
	SSAR_DECLARE("divw", Value.DivW);
	SSAR_DECLARE("divh", Value.DivH);
}
void SerializeStruct(FSsAnimationSettings& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("fps", Value.Fps);
	SSAR_DECLARE("frameCount", Value.FrameCount);
	SSAR_DECLARE_ENUM("sortMode", Value.SortMode);
	SSAR_DECLARE("canvasSize", Value.CanvasSize);
	SSAR_DECLARE("pivot", Value.Pivot);
	SSAR_DECLARE("startFrame", Value.StartFrame);
	SSAR_DECLARE("endFrame", Value.EndFrame);
}
void SerializeStruct(FSsPart& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("name", Value.PartName);
	SSAR_DECLARE("arrayIndex", Value.ArrayIndex);
	SSAR_DECLARE("parentIndex", Value.ParentIndex);

	SSAR_DECLARE_ENUM("type", Value.Type);
	SSAR_DECLARE_ENUM("boundsType", Value.BoundsType);
	SSAR_DECLARE_ENUM("inheritType", Value.InheritType);
	SSAR_DECLARE_ENUM("alphaBlendType", Value.AlphaBlendType);
//	SSAR_DECLARE("show", Value.Show);
//	SSAR_DECLARE("locked", Value.Locked);
	SSAR_DECLARE("colorLabel", Value.ColorLabel);
	SSAR_DECLARE("maskInfluence", Value.MaskInfluence);

	SSAR_DECLARE("refAnimePack", Value.RefAnimePack);
	SSAR_DECLARE("refAnime", Value.RefAnime);

	SSAR_DECLARE("refEffectName", Value.RefEffectName);

	SSAR_DECLARE("boneLength", Value.BoneLength);
	SSAR_DECLARE("bonePosition", Value.BonePosition);
	SSAR_DECLARE("boneRotation", Value.BoneRotation);
	SSAR_DECLARE("weightPosition", Value.WeightPosition);
	SSAR_DECLARE("weightImpact", Value.WeightImpact);
	SSAR_DECLARE("IKDepth", Value.IKDepth);
	SSAR_DECLARE("IKRotationArrow", Value.IKRotationArrow);

	//継承率後に改良を実施
	if (ar->getType() == EnumSsArchiver::in)
	{
		XMLElement* e = ar->getxml()->FirstChildElement("ineheritRates");
		if (e)
		{
			XMLElement* ec = e->FirstChildElement();
			while(ec)
			{
				const char* tag = ec->Value();
				TEnumAsByte<SsAttributeKind::Type> enumattr;

				__StringToEnum_( tag , enumattr );
				Value.InheritRates[(int)enumattr] = (float)atof(ec->GetText());
				ec = ec->NextSiblingElement();
			}
		}
	}
}
void SerializeStruct(FSsMeshBind& Value, SsXmlIArchiver* ar)
{
	const char* Text = ar->getxml()->GetText();
	if(nullptr == Text)
	{
		return;
	}

	FString Str(babel::utf8_to_sjis(Text).c_str());

	TArray<FString> MeshBindInfoStrs;
	Str.ParseIntoArray(MeshBindInfoStrs, TEXT(","));
	
	for(auto It = MeshBindInfoStrs.CreateConstIterator(); It; ++It)
	{
		FSsMeshBindInfo Info;
		{
			TArray<FString> Tok;
			(*It).ParseIntoArray(Tok, TEXT(" "));

			FMemory::Memset(Info.Weight,    0, SSMESHPART_BONEMAX * sizeof(float));
			FMemory::Memset(Info.BoneIndex, 0, SSMESHPART_BONEMAX * sizeof(int32));
			FMemory::Memset(Info.Offset,    0, SSMESHPART_BONEMAX * sizeof(FVector3f));

			Info.BindBoneNum = (0 < Tok.Num()) ? FCString::Atoi(*Tok[0]) : 0;
			for(int32 i = 0; i < Info.BindBoneNum; ++i)
			{
				int32 idx = 1 + (i*4);
				Info.BoneIndex[i] = (idx < Tok.Num()) ? FCString::Atoi(*Tok[idx]) : 0;   ++idx;
				Info.Weight[i]    = (idx < Tok.Num()) ? FCString::Atoi(*Tok[idx]) : 0.f; ++idx;
				Info.Offset[i].X  = (idx < Tok.Num()) ? FCString::Atof(*Tok[idx]) : 0.f; ++idx;
				Info.Offset[i].Y  = (idx < Tok.Num()) ? FCString::Atof(*Tok[idx]) : 0.f;
			}
		}
		Value.MeshVerticesBindArray.Add(Info);
	}
}
void SerializeStruct(FSsModel& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE_LIST("partList", Value.PartList);
	SSAR_DECLARE_LIST("meshList", Value.MeshList);
	SSAR_DECLARE("boneList", Value.BoneList);
	Value.SetupAnimation = nullptr;
}
void SerializeStruct(FSsPartAnime& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("partName", Value.PartName);
	SSAR_DECLARE_LISTEX("attributes", Value.Attributes, "attribute");
}
void SerializeStruct(FSsLabel& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("name", Value.LabelName);
	SSAR_DECLARE("time", Value.Time);
}
void SerializeStruct(FSsAnimation& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("name", Value.AnimationName);
	SSAR_STRUCT_DECLARE("settings", Value.Settings);
	SSAR_DECLARE_LISTEX("partAnimes", Value.PartAnimes, "partAnime");
	SSAR_DECLARE_LISTEX("labels", Value.Labels, "value");
	SSAR_DECLARE("isSetup", Value.IsSetup);
}
void SerializeStruct(FSsVarianceValueFloat& Value, SsXmlIArchiver* ar)
{
	// MEMO: "type"はxml上には記述されていない. 各FSsEffectElementBase派生クラスのコンストラクタ依存. 

	FString ValueStr, SubValueStr;
	SSAR_DECLARE_ATTRIBUTE("value", ValueStr);
	SSAR_DECLARE_ATTRIBUTE("subvalue", SubValueStr);

	if(0 < ValueStr.Len())
	{
		Value.Value = FCString::Atof(*ValueStr);
	}
	if(0 < SubValueStr.Len())
	{
		Value.SubValue = FCString::Atof(*SubValueStr);
	}
}
void SerializeStruct(FSsVarianceValueInt& Value, SsXmlIArchiver* ar)
{
	FString ValueStr, SubValueStr;
	SSAR_DECLARE_ATTRIBUTE("value", ValueStr);
	SSAR_DECLARE_ATTRIBUTE("subvalue", SubValueStr);

	if(0 < ValueStr.Len())
	{
		Value.Value = FCString::Atoi(*ValueStr);
	}
	if(0 < SubValueStr.Len())
	{
		Value.SubValue = FCString::Atoi(*SubValueStr);
	}
}
void SerializeStruct(FSsVarianceValueColor& Value, SsXmlIArchiver* ar)
{
	FString ValueStr, SubValueStr;
	SSAR_DECLARE_ATTRIBUTE("value", ValueStr);
	SSAR_DECLARE_ATTRIBUTE("subvalue", SubValueStr);

	if(0 < ValueStr.Len())
	{
		SsColor ValueCol;
		ConvertStringToSsColor(ValueStr, ValueCol);

		Value.Value.R = (uint8)ValueCol.r;
		Value.Value.G = (uint8)ValueCol.g;
		Value.Value.B = (uint8)ValueCol.b;
		Value.Value.A = (uint8)ValueCol.a;
	}
	if(0 < SubValueStr.Len())
	{
		SsColor SubValueCol;
		ConvertStringToSsColor(SubValueStr, SubValueCol);

		Value.SubValue.R = (uint8)SubValueCol.r;
		Value.SubValue.G = (uint8)SubValueCol.g;
		Value.SubValue.B = (uint8)SubValueCol.b;
		Value.SubValue.A = (uint8)SubValueCol.a;
	}
}
void SerializeStruct(FSsParticleElementBasic& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("maximumParticle", Value.MaximumParticle);
	SSAR_STRUCT_DECLARE("speed", Value.Speed);
	SSAR_STRUCT_DECLARE("lifespan", Value.Lifespan);
	SSAR_DECLARE("angle", Value.Angle);
	SSAR_DECLARE("angleVariance", Value.AngleVariance);
	SSAR_DECLARE("interval", Value.Interval);
	SSAR_DECLARE("lifetime", Value.Lifetime);
	SSAR_DECLARE("attimeCreate",Value. AttimeCreate);
	SSAR_DECLARE("priority", Value.Priority);
}
void SerializeStruct(FSsParticleElementRndSeedChange& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("Seed", Value.Seed);
}
void SerializeStruct(FSsParticleElementDelay& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("DelayTime", Value.DelayTime);
}
void SerializeStruct(FSsParticleElementGravity& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("Gravity", Value.Gravity);
}
void SerializeStruct(FSsParticleElementPosition& Value, SsXmlIArchiver* ar)
{
	SSAR_STRUCT_DECLARE("OffsetX", Value.OffsetX);
	SSAR_STRUCT_DECLARE("OffsetY", Value.OffsetY);
}
void SerializeStruct(FSsParticleElementRotation& Value, SsXmlIArchiver* ar)
{
	SSAR_STRUCT_DECLARE("Rotation", Value.Rotation);
	SSAR_STRUCT_DECLARE("RotationAdd", Value.RotationAdd);
}
void SerializeStruct(FSsParticleElementRotationTrans& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("RotationFactor", Value.RotationFactor);
	SSAR_DECLARE("EndLifeTimePer", Value.EndLifeTimePer);
}
void SerializeStruct(FSsParticleElementTransSpeed& Value, SsXmlIArchiver* ar)
{
	SSAR_STRUCT_DECLARE("Speed", Value.Speed);
}
void SerializeStruct(FSsParticleElementTangentialAcceleration& Value, SsXmlIArchiver* ar)
{
	SSAR_STRUCT_DECLARE("Acceleration", Value.Acceleration);
}
void SerializeStruct(FSsParticleElementInitColor& Value, SsXmlIArchiver* ar)
{
	SSAR_STRUCT_DECLARE("Color", Value.Color);
}
void SerializeStruct(FSsParticleElementTransColor& Value, SsXmlIArchiver* ar)
{
	SSAR_STRUCT_DECLARE("Color", Value.Color);
}
void SerializeStruct(FSsParticleElementAlphaFade& Value, SsXmlIArchiver* ar)
{
	SSAR_STRUCT_DECLARE("disprange", Value.Disprange);
}
void SerializeStruct(FSsParticleElementSize& Value, SsXmlIArchiver* ar)
{
	SSAR_STRUCT_DECLARE("SizeX", Value.SizeX);
	SSAR_STRUCT_DECLARE("SizeY", Value.SizeY);
	SSAR_STRUCT_DECLARE("ScaleFactor", Value.ScaleFactor);
}
void SerializeStruct(FSsParticleElementTransSize& Value, SsXmlIArchiver* ar)
{
	SSAR_STRUCT_DECLARE("SizeX", Value.SizeX);
	SSAR_STRUCT_DECLARE("SizeY", Value.SizeY);
	SSAR_STRUCT_DECLARE("ScaleFactor", Value.ScaleFactor);
}
void SerializeStruct(FSsParticlePointGravity& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("Position", Value.Position);
	SSAR_DECLARE("Power", Value.Power);
}
void SerializeStruct(FSsParticleTurnToDirectionEnabled& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("Rotation", Value.Rotation);
}
void SerializeStruct(FSsParticleInfiniteEmitEnabled& Value, SsXmlIArchiver* ar)
{
}
void SerializeStruct(FSsEffectBehavior& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("CellName", Value.CellName);
	SSAR_DECLARE("CellMapName", Value.CellMapName);
	SSAR_DECLARE_ENUM("BlendType", Value.BlendType);

	if(ar->getxml())
	{
		SsXmlIArchiver list_ar(ar, "list");
		
		XMLElement* e = list_ar.getxml()->FirstChildElement();

		while(e)
		{
			const char* name = e->Attribute("name");

			FSsEffectElementBase* v = NULL;

#define SS_SERIALIZE_PARTICLE_ELEMENT(_key, _type) if(0 == strcmp(name , _key)){ _type* p = new _type(); SerializeStruct(*p, &ar2); v = p; }
			{
				SsXmlIArchiver ar2(e);
				SS_SERIALIZE_PARTICLE_ELEMENT("Basic", FSsParticleElementBasic)
				SS_SERIALIZE_PARTICLE_ELEMENT("OverWriteSeed", FSsParticleElementRndSeedChange)
				SS_SERIALIZE_PARTICLE_ELEMENT("Delay", FSsParticleElementDelay)
				SS_SERIALIZE_PARTICLE_ELEMENT("Gravity", FSsParticleElementGravity)
				SS_SERIALIZE_PARTICLE_ELEMENT("init_position", FSsParticleElementPosition)
				SS_SERIALIZE_PARTICLE_ELEMENT("init_rotation", FSsParticleElementRotation)
				SS_SERIALIZE_PARTICLE_ELEMENT("trans_rotation", FSsParticleElementRotationTrans)
				SS_SERIALIZE_PARTICLE_ELEMENT("trans_speed", FSsParticleElementTransSpeed)
				SS_SERIALIZE_PARTICLE_ELEMENT("add_tangentiala", FSsParticleElementTangentialAcceleration)
				SS_SERIALIZE_PARTICLE_ELEMENT("init_vertexcolor", FSsParticleElementInitColor)
				SS_SERIALIZE_PARTICLE_ELEMENT("trans_vertexcolor", FSsParticleElementTransColor)
				SS_SERIALIZE_PARTICLE_ELEMENT("trans_colorfade", FSsParticleElementAlphaFade)
				SS_SERIALIZE_PARTICLE_ELEMENT("init_size", FSsParticleElementSize)
				SS_SERIALIZE_PARTICLE_ELEMENT("trans_size", FSsParticleElementTransSize)
				SS_SERIALIZE_PARTICLE_ELEMENT("add_pointgravity", FSsParticlePointGravity)
				SS_SERIALIZE_PARTICLE_ELEMENT("TurnToDirection", FSsParticleTurnToDirectionEnabled)
				SS_SERIALIZE_PARTICLE_ELEMENT("InfiniteEmit", FSsParticleInfiniteEmitEnabled)
			}
#undef SS_SERIALIZE_PARTICLE_ELEMENT

			if(v)
			{
				Value.PList.Add(MakeShareable(v));
			}
			e = e->NextSiblingElement();
		}
	}
}
void SerializeStruct(FSsEffectNode& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("arrayIndex", Value.ArrayIndex);
	SSAR_DECLARE("parentIndex", Value.ParentIndex);
	SSAR_DECLARE_ENUM("type", Value.Type);
	SSAR_DECLARE("visible", Value.Visible);
	SSAR_STRUCT_DECLARE("behavior", Value.Behavior);	
}
void SerializeStruct(FSsEffectNode*& Value, SsXmlIArchiver* ar)
{
	Value = new FSsEffectNode();
	SerializeStruct(*Value, ar);
}
void SerializeStruct(FSsEffectModel& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("lockRandSeed", Value.LockRandSeed);
	SSAR_DECLARE("isLockRandSeed", Value.IsLockRandSeed);
	SSAR_DECLARE("fps", Value.FPS);
	SSAR_DECLARE("bgcolor", Value.BgColor);
	SSAR_DECLARE("layoutScaleX", Value.LayoutScaleX); if(0 == Value.LayoutScaleX){ Value.LayoutScaleX = 100; }
	SSAR_DECLARE("layoutScaleY", Value.LayoutScaleY); if(0 == Value.LayoutScaleY){ Value.LayoutScaleY = 100; }
	SSAR_DECLARE_LISTEX("nodeList", Value.NodeList, "node");

	Value.BuildTree();
}
void SerializeStruct(FSsSequenceItem& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("refAnimePack", Value.RefAnimPack);
	SSAR_DECLARE("refAnime", Value.RefAnime);
	SSAR_DECLARE("repeatCount", Value.RepeatCount);
}
void SerializeStruct(FSsSequence& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("name", Value.SequenceName);
	SSAR_DECLARE("index", Value.Id);
	SSAR_DECLARE_ENUM("type", Value.Type);
	SSAR_DECLARE("list", Value.List);
}
void SerializeStruct(FSs6ProjectSetting& Value, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("animeBaseDirectory", Value.AnimeBaseDirectory);
	SSAR_DECLARE("cellMapBaseDirectory", Value.CellMapBaseDirectory);
	SSAR_DECLARE("imageBaseDirectory", Value.ImageBaseDirectory);
	SSAR_DECLARE("effectBaseDirectory", Value.EffectBaseDirectory);
	SSAR_DECLARE("exportBaseDirectory", Value.ExportBaseDirectory);
	SSAR_DECLARE("queryExportBaseDirectory", Value.QueryExportBaseDirectory);
	SSAR_DECLARE_ENUM("wrapMode", Value.WrapMode);
	SSAR_DECLARE_ENUM("filterMode", Value.FilterMode);
	SSAR_DECLARE("vertexAnimeFloat", Value.VertexAnimeFloat);
}


void SerializeSsCellMap(FSsCellMap& CellMap, SsXmlIArchiver* ar)
{
	SSAR_DECLARE_ATTRIBUTE("version", CellMap.Version);
	SSAR_DECLARE("name", CellMap.CellMapName);
	CellMap.CellMapNameEx = FName(*(CellMap.CellMapName.ToString() + TEXT(".ssce")));
	SSAR_DECLARE("imagePath", CellMap.ImagePath);
	SSAR_DECLARE("pixelSize", CellMap.PixelSize);
	SSAR_DECLARE("overrideTexSettings", CellMap.OverrideTexSettings);
	SSAR_DECLARE_ENUM("wrapMode", CellMap.WrapMode);
	SSAR_DECLARE_ENUM("filterMode", CellMap.FilterMode);

	SSAR_DECLARE_LISTEX("cells", CellMap.Cells, "cell");
}
void SerializeSsAnimePack(FSsAnimePack& AnimePack, SsXmlIArchiver* ar, int32& OutSortOrder)
{
	SSAR_DECLARE_ATTRIBUTE("version", AnimePack.Version);
	SSAR_DECLARE("name", AnimePack.AnimePackName);
	SSAR_STRUCT_DECLARE("Model", AnimePack.Model);
	SSAR_DECLARE("cellmapNames", AnimePack.CellmapNames);
	SSAR_DECLARE_LISTEX("animeList", AnimePack.AnimeList, "anime");
	if(!SSAR_DECLARE("order", OutSortOrder))
	{
		OutSortOrder = INT32_MAX;
	}

	for(auto It = AnimePack.AnimeList.CreateIterator(); It; ++It)
	{
		if(It->IsSetup)
		{
			AnimePack.Model.SetupAnimation = &(*It);
			break;
		}
	}

	// 重複パーツ名チェック 
	// FNameは大文字小文字を区別しないが、SSでは区別されるため重複パーツ名があり得てしまう 
	for(int32 i = 0; i < AnimePack.Model.PartList.Num(); ++i)
	{
		for(int32 j = i+1; j < AnimePack.Model.PartList.Num(); ++j)
		{
			if(AnimePack.Model.PartList[i].PartName == AnimePack.Model.PartList[j].PartName)
			{
				FString ErrStr = FString::Printf(TEXT("アニメーション [%s] 内のパーツ名 [%s] が重複しています\nUEプラグインでは大文字小文字が区別されません"),
					*AnimePack.AnimePackName.ToString(),
					*AnimePack.Model.PartList[i].PartName.ToString()
				);
				UE_LOG(LogSpriteStudioEd, Error, TEXT("%s"), *ErrStr);
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ErrStr));
			}
		}
	}
}
void SerializeSsEffectFile(FSsEffectFile& EffectFile, SsXmlIArchiver* ar)
{
	SSAR_DECLARE("name", EffectFile.Name);
	SSAR_STRUCT_DECLARE("effectData", EffectFile.EffectData);
	EffectFile.EffectData.EffectName = EffectFile.Name;
}
void SerializeSsSequencePack(FSsSequencePack& SequencePack, SsXmlIArchiver* ar)
{
	SSAR_DECLARE_ATTRIBUTE("version", SequencePack.Version);
	SSAR_DECLARE("name", SequencePack.SequencePackName);
	SSAR_DECLARE_LISTEX("sequenceList", SequencePack.SequenceList, "sequence");
}
void SerializeSsProject(USs6Project& Proj, SsXmlIArchiver* ar)
{
	SSAR_DECLARE_ATTRIBUTE("version", Proj.Version);
	SSAR_STRUCT_DECLARE("settings", Proj.Settings);
	SSAR_DECLARE("cellmapNames", Proj.CellmapNames);
	SSAR_DECLARE("animepackNames", Proj.AnimepackNames);
	SSAR_DECLARE("effectFileNames", Proj.EffectFileNames);
	SSAR_DECLARE("sequencepackNames", Proj.SequencePackNames);
}

