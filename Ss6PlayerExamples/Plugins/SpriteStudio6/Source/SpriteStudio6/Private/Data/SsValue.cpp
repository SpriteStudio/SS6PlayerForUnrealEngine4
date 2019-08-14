#include "SpriteStudio6PrivatePCH.h"
#include "SsValue.h"


namespace
{
	static const FName ConstName_SsValue_RGBA("rgba");
	static const FName ConstName_SsValue_Target("target");
	static const FName ConstName_SsValue_BlendType("blendType");
}

// UE4シリアライズ
void FSsValue::Serialize(FArchive& Ar, FName HashKey)
{
	switch(Type)
	{
	case SsValueType::Unkown:
		{} break;
	case SsValueType::StringType:
		{
			if(Ar.IsLoading())
			{
				/////////////////////////////////////////
				// 旧データ互換用 
				if(ConstName_SsValue_RGBA == HashKey)
				{
					FString TempStr;
					Ar << TempStr;
					_Color = new SsColor();
					ConvertStringToSsColor(TempStr, *_Color);
					Type = SsValueType::ColorType;
				}
				else if(ConstName_SsValue_Target == HashKey)
				{
					FString TempStr;
					Ar << TempStr;

					TEnumAsByte<SsColorBlendTarget::Type> Target;
					__StringToEnum_(TempStr, Target);
					_Int = static_cast<int32>(Target);
					Type = SsValueType::IntType;
				}
				else if(ConstName_SsValue_BlendType == HashKey)
				{
					FString TempStr;
					Ar << TempStr;

					TEnumAsByte<SsBlendType::Type> BlendType;
					__StringToEnum_(TempStr, BlendType);
					_Int = static_cast<int32>(BlendType);
					Type = SsValueType::IntType;
				}
				/////////////////////////////////////////
				else
				{
					if(Ar.IsLoading())
					{
						_Str = new FString();
					}
					Ar << *_Str;
				}
			}
			else
			{
				Ar << *_Str;
			}
		} break;
	case SsValueType::IntType:
		{
			Ar << _Int;
		} break;
	case SsValueType::FloatType:
		{
			Ar << _Float;
		} break;
	case SsValueType::BooleanType:
		{
			Ar << _Boolean;
		} break;
	case SsValueType::HashType:
		{
			if(Ar.IsLoading())
			{
				int32 NumHash = 0;
				Ar << NumHash;
				_Hash = NULL;
				if(0 < NumHash)
				{
					_Hash = new SsHash();
					for(int32 i = 0; i < NumHash; ++i)
					{
						FName Key;
						Ar << Key;
						FSsValue& AddValue = _Hash->Add(Key);
						AddValue.StaticStruct()->SerializeTaggedProperties(Ar, (uint8*)&AddValue, NULL, NULL);
						AddValue.Serialize(Ar, Key);
					}
				}
			}
			else
			{
				TArray<FName> Keys;
				int32 NumHash = (NULL == _Hash) ? 0 : _Hash->GetKeys(Keys);
				Ar << NumHash;
				for(int32 i = 0; i < Keys.Num(); ++i)
				{
					Ar << Keys[i];
					FSsValue& SerValue = (*_Hash)[Keys[i]];
					SerValue.StaticStruct()->SerializeTaggedProperties(Ar, (uint8*)&(*_Hash)[Keys[i]], NULL, NULL);
					SerValue.Serialize(Ar, Keys[i]);
				}
			}
		} break;
	case SsValueType::ArrayType:
		{
			if(Ar.IsLoading())
			{
				int32 NumArray = 0;
				Ar << NumArray;
				_Array = NULL;
				if(0 < NumArray)
				{
					_Array = new SsArray();
					_Array->AddUninitialized(NumArray);
					for(int32 i = 0; i < NumArray; ++i)
					{
						FSsValue& SerValue = (*_Array)[i];
						SerValue.StaticStruct()->SerializeTaggedProperties(Ar, (uint8*)&(*_Array)[i], NULL, NULL);
						SerValue.Serialize(Ar);
					}
				}
			}
			else
			{
				int32 NumArray = (NULL == _Array) ? 0 : _Array->Num();
				Ar << NumArray;
				for(int32 i = 0; i < NumArray; ++i)
				{
					FSsValue& SerValue = (*_Array)[i];
					SerValue.StaticStruct()->SerializeTaggedProperties(Ar, (uint8*)&(*_Array)[i], NULL, NULL);
					SerValue.Serialize(Ar);
				}
			}
		} break;
	case SsValueType::ColorType:
		{
			if(Ar.IsLoading())
			{
				_Color = new SsColor();
			}
			Ar << _Color->r;
			Ar << _Color->g;
			Ar << _Color->b;
			Ar << _Color->a;
		} break;
	}
}
