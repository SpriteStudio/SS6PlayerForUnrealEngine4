#pragma once

#include "tinyxml2.h"
#include "babel.h"
#include "SsTypes.h"

using namespace tinyxml2;



/// シリアライズクラスのInput/Outputの状態
struct EnumSsArchiver
{
	enum Type
	{
		unkown = 0, //不明
		in,			//Input
		out,		//Output
	};
};

class SsXmlIArchiver;

/// アーカイバクラスのインターフェース
class ISsXmlArchiver
{
private:
	XMLElement*		m_xml;

public:

	virtual EnumSsArchiver::Type getType(){ return EnumSsArchiver::unkown;}


	ISsXmlArchiver(){}
	virtual ~ISsXmlArchiver(){}

	XMLElement*	getxml(){ return m_xml;} 

	void	firstChild(ISsXmlArchiver* ar , const char* element_name)
	{
		m_xml = ar->getxml()->FirstChildElement( element_name );
	}

	void	setDocumet( tinyxml2::XMLDocument* doc , const char* element_name)
	{
		m_xml = doc->FirstChildElement(element_name);
	}

	void	setElement(XMLElement* e )
	{
		m_xml = e;
	}

	virtual bool	dc( const char* name , int& member ) = 0;
	virtual bool	dc( const char* name , float& member ) = 0;
	virtual bool	dc( const char* name , bool& member ) = 0;
	virtual bool	dc( const char* name , TArray<FString>& list ) = 0;
	virtual bool	dc( const char* name , TArray<FName>& list ) = 0;
	virtual bool	dc( const char* name , TArray<FVector2D>& list ) = 0;
	virtual bool	dc( const char* name , TArray<FSsTriangle>& list) = 0;

	virtual bool	dc( const char* name , FString& member ) = 0;
	virtual bool	dc( const char* name , FName& member ) = 0;
	virtual bool	dc( const char* name , FVector2D& member ) = 0;
	virtual bool	dc( const char* name , struct FSsCurve& member ) = 0;
	virtual bool	dc( const char* name , FSsTriangle& member) = 0;
	virtual bool	dc( const char* name , TMap<FName, int32>& map) = 0;


	virtual bool	dc_attr( const char* name , FString& member ) = 0;
	virtual bool	dc_attr( const char* name , int& member ) = 0;

};


inline	static int	GetTextToInt(XMLElement* e , int default_value )
{
	int ret =default_value; 
	const char* v = e->GetText();
	if ( v )
	{
		ret = atoi(v);
	}
	return ret;
}


/// アーカイバクラス　インプットクラス
class SsXmlIArchiver : public ISsXmlArchiver
{
public:
	virtual EnumSsArchiver::Type getType(){ return EnumSsArchiver::in;}


	SsXmlIArchiver(ISsXmlArchiver* ar , const char* element_name)
	{
		firstChild( ar , element_name );
	}
	SsXmlIArchiver( tinyxml2::XMLDocument* doc , const char* element_name)
	{
		setDocumet( doc , element_name );
	}

	SsXmlIArchiver(XMLElement* e )
	{
		setElement( e );
	}


	virtual ~SsXmlIArchiver(){}
	

	virtual bool dc(const char* name, int& member);
	virtual bool dc(const char* name, float& member);
	virtual bool dc(const char* name, FString& member);
	virtual bool dc(const char* name, FName& member);
	virtual bool dc(const char* name, bool& member);
	virtual bool dc(const char* name, TArray<FString>& list);
	virtual bool dc(const char* name, TArray<FName>& list);
	virtual bool dc(const char* name, TArray<FVector2D>& list);
	virtual bool dc(const char* name, TArray<FSsTriangle>& list);
	virtual bool dc(const char* name, FVector2D& member);
	virtual bool dc(const char* name, FSsCurve& member);
	virtual bool dc(const char* name, FSsTriangle& member);
	virtual bool dc(const char* name, TMap<FName, int32>& map);

	virtual bool dc_attr(const char* name, FString& member);
	virtual bool dc_attr(const char* name, int& member);


	template<class myclass> bool dc(const char* name, TArray<myclass>& list, const FString key = FString(TEXT("value")))
	{
		list.Empty();
		XMLElement* e = getxml()->FirstChildElement( name );

		if ( key != "" )
		{
			char tmp[32];
#if PLATFORM_WINDOWS
			sprintf_s(tmp, 32, "%S", *key);
#else
			sprintf(tmp, "%S", *key);
#endif
			e = e->FirstChildElement( tmp );
		}

		while( e )
		{
			myclass _temp;
			SsXmlIArchiver _ar(e);
			SerializeStruct(_temp, &_ar);
			list.Add( _temp );
			e = e->NextSiblingElement();
		}

		return true;
	}

	template<class myclass> bool dc(const char* name , myclass& type)
	{
		FString str;
		dc( name , str );
		__StringToEnum_( str , type);
		
		return false;
	}

	template<class myclass> bool dc_attr( const char* name , myclass& type)
	{
		FString str;
		dc_attr( name , str );
		__StringToEnum_( str , type);
		
		return false;
	}
};


#define	SSAR_DECLARE(n,t)  ar->dc(n,t)
#define	SSAR_DECLARE_ATTRIBUTE(n,t)  ar->dc_attr(n,t)

#define	SSAR_STRUCT_DECLARE(n,t)  {SsXmlIArchiver _ar( ar , n );\
SerializeStruct(t, &_ar);}\

template<class myclass>
inline bool	__SSAR_DECLARE_LIST__( ISsXmlArchiver* ar , TArray<myclass>& list, const char* name ,const FString key = FString(TEXT("value")))
{
	if ( ar->getType() == EnumSsArchiver::in )
	{
		return (static_cast<SsXmlIArchiver*>(ar))->dc( name , list , key );
	}

	if ( ar->getType() == EnumSsArchiver::out )
	{
	}

	return false;
}

#define	SSAR_DECLARE_LIST(n,t) __SSAR_DECLARE_LIST__(ar , t , n)
#define	SSAR_DECLARE_LISTEX(n,t,key) __SSAR_DECLARE_LIST__(ar , t , n , key)

template<class myclass>
inline bool __SSAR_DECLARE_ENUM__(ISsXmlArchiver* ar ,myclass& type, const char* name)
{
	if(ar->getType() == EnumSsArchiver::in)
	{
		return (static_cast<SsXmlIArchiver*>(ar))->dc(name , type);
	}

	if(ar->getType() == EnumSsArchiver::out)
	{
	}
	return false;
}

template<class myclass>
inline bool __SSAR_DECLARE_ATTRIBUTE_ENUM__(ISsXmlArchiver* ar ,myclass& type, const char* name)
{
	if(ar->getType() == EnumSsArchiver::in)
	{
		return (static_cast<SsXmlIArchiver*>(ar))->dc_attr(name , type);
	}

	if(ar->getType() == EnumSsArchiver::out)
	{
	}
	return false;
}


#define SSAR_DECLARE_ENUM(n,t) __SSAR_DECLARE_ENUM__(ar , t , n)
#define SSAR_DECLARE_ATTRIBUTE_ENUM(n,t) __SSAR_DECLARE_ATTRIBUTE_ENUM__(ar , t , n)


///SpriteStudio XMLデータ読み書きの初期化
void SsArchiverInit();


#define AR_SELF_CHECK() if(this->getxml() == 0){ return false; }


void SerializeSsCellMap(struct FSsCellMap& CellMap, SsXmlIArchiver* ar);
void SerializeSsAnimePack(struct FSsAnimePack& AnimePack, SsXmlIArchiver* ar);
void SerializeSsEffectFile(struct FSsEffectFile& EffectFile, SsXmlIArchiver* ar);
void SerializeSsProject(class USs6Project& Proj, SsXmlIArchiver* ar);
