#pragma once

#include "SsValue.generated.h"

struct FSsValue;

typedef	wchar_t					SsChar;
typedef TArray<FSsValue>		SsArray;
typedef TMap<FName,FSsValue>	SsHash;

UENUM()
namespace SsValueType
{
	enum Type
	{
		Unkown,
		StringType,
		IntType,
		FloatType,
		BooleanType,
		HashType,
		ArrayType,
	};
}

USTRUCT()
struct FSsValue
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar);

public:
	UPROPERTY(VisibleAnywhere, Category=SsValue)
	TEnumAsByte<SsValueType::Type> Type;

	UPROPERTY(VisibleAnywhere, Category=SsValue)
	FString		ValueName;

	union{
		FString*	_Str;
		int32		_Int;
		float		_Float;
		bool		_Boolean;
		SsArray*	_Array;
		SsHash*		_Hash;
	};

	UPROPERTY()
	int32 Int_temp;

	UPROPERTY()
	float Float_temp;

	UPROPERTY()
	bool Bool_temp;


	FSsValue()
		: Type(SsValueType::Unkown)
		, _Str(0)
		, Int_temp(0)
		, Float_temp(0.f)
		, Bool_temp(false)
	{}

	explicit FSsValue(bool b) : Type(SsValueType::BooleanType){ _Boolean = b; }
	explicit FSsValue(int n) : Type(SsValueType::IntType){ _Int = n; }
	explicit FSsValue(float n) : Type(SsValueType::FloatType){ _Float = n; }
	explicit FSsValue(FString& str){ Type = SsValueType::StringType; _Str = new FString(str); }
	explicit FSsValue(const char* str){ Type = SsValueType::StringType; _Str = new FString(str); }
	explicit FSsValue(SsArray& n)
	{
		Type = SsValueType::ArrayType; 
		_Array = new SsArray(n);
	}
	explicit FSsValue(SsHash& n){ Type = SsValueType::HashType; _Hash = new SsHash(n); }


	FSsValue(const FSsValue& x)
	{
		switch( x.Type )
		{
			case SsValueType::StringType:
				_Str = new FString( *x._Str );
				Float_temp = FCString::Atof( **_Str );
				Int_temp = FCString::Atoi( **_Str );
				break;
			case SsValueType::IntType:
				_Int = x._Int;
				Float_temp = (float)_Int;
				Bool_temp = _Int == 1 ? true : false;
				break;
			case SsValueType::FloatType:
				_Float = x._Float;
				Int_temp = (int)_Float;
				Bool_temp = _Int > 0 ? true : false;
				break;
			case SsValueType::BooleanType:
				_Boolean = x._Boolean;
				break;
			case SsValueType::ArrayType:
				_Array = new SsArray( *x._Array);
				break;
			case SsValueType::HashType:
				_Hash = new SsHash( *x._Hash);
				break;
		}
		Type = x.Type;
	}
	FSsValue& operator=(const FSsValue& x)
	{
		if(this != &x)
		{
			this->release();
			ValueName.Empty();
			new (this) FSsValue(x);
		}
		return *this;
	}

	//リリースの判断が甘い
	void release()
	{
		if(Type == SsValueType::StringType && _Str)
		{
			delete _Str;
			return;
		}

		if(Type == SsValueType::ArrayType && _Array)
		{
			delete _Array;
			return;
		}
		if(Type == SsValueType::HashType && _Hash)
		{
			delete _Hash;
			return;
		}
	}

	~FSsValue()
	{
		release();
	}

	template <typename T> bool is() const;
	template <typename T> const T& get() const;
	template <typename T> T& get();

	const FSsValue& operator[](const FName& key) const
	{
		static FSsValue r_value;

		if ( Type == SsValueType::HashType )
		{
			FSsValue* Value = _Hash->Find(key);
			return (nullptr != Value) ? *Value : r_value;
		}

		return *this;
	}

	bool IsExistHashkey( const FName& key ) const
	{
		if(Type == SsValueType::HashType)
		{
			return _Hash->Contains(key);
		}
		return false;
	}
};

template <> inline const FString& FSsValue::get<FString>() const
{
	static FString ret(TEXT(""));
	if(this->Type != SsValueType::StringType)
	{
		return ret;
	}
	return *_Str;
}
template <> inline FString& FSsValue::get<FString>()
{
	static FString ret(TEXT(""));
	if(this->Type != SsValueType::StringType)
	{
		return ret;
	}
	return *_Str;
}
template <> inline const int& FSsValue::get<int>() const
{
	if(this->Type == SsValueType::FloatType)
	{
		return Int_temp;
	}
	else
	{
		return _Int;
	}
}
template <> inline int& FSsValue::get<int>()
{
	if(this->Type == SsValueType::FloatType)
	{
		return Int_temp;
	}
	else
	{
		return _Int;
	}
}
template <> inline const float& FSsValue::get<float>() const
{
	if(this->Type == SsValueType::FloatType)
	{
		return _Float;
	}
	else
	{
		return Float_temp;
	}
}
template <> inline float& FSsValue::get<float>()
{
	if(this->Type == SsValueType::FloatType)
	{
		return _Float;
	}
	else
	{
		return Float_temp;
	}
}
template <> inline const bool& FSsValue::get<bool>() const
{
	if(this->Type == SsValueType::BooleanType)
	{
		return _Boolean;
	}
	else
	{
		return Bool_temp;
	}
}
template <> inline bool& FSsValue::get<bool>()
{
	if(this->Type == SsValueType::BooleanType)
	{
		return _Boolean;
	}
	else
	{
		return Bool_temp;
	}
}
template <> inline const SsArray& FSsValue::get<SsArray>() const
{
	return *_Array;
}
template <> inline SsArray& FSsValue::get<SsArray>()
{
	return *_Array;
}
template <> inline const SsHash& FSsValue::get<SsHash>() const
{
	return *_Hash;
}
template <> inline SsHash& FSsValue::get<SsHash>()
{
	return *_Hash;
}
template <> inline bool FSsValue::is<bool>() const
{
	return Type == SsValueType::BooleanType;
}
template <> inline bool FSsValue::is<int>() const
{
	return Type == SsValueType::IntType;
}
template <> inline bool FSsValue::is<float>() const
{
	return Type == SsValueType::FloatType;
}
template <> inline bool FSsValue::is<FString>() const
{
	return Type == SsValueType::StringType;
}
template <> inline bool FSsValue::is<SsArray>() const
{
	return Type == SsValueType::ArrayType;
}
template <> inline bool FSsValue::is<SsHash>() const
{
	return Type == SsValueType::HashType;
}
