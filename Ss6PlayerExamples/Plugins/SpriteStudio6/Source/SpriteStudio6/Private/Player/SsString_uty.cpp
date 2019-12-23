#include "SsString_uty.h"


//=========================================================================================
//! 文字列の切り分け
/*!
	検索キーを元に文字列を切り分け、文字列の配列を返します。
	@param[in] in_str 入力文字列
	@param[in] key 検索キー
	@param[out] out_array 出力文字列配列
	@retval なし

*/
//=========================================================================================
void	split_string( const FString &in_str , 
						const TCHAR key, 
						TArray<FString>& out_array )
{
	out_array.Empty();

	FString InS(in_str);
	FString Key = FString::Chr(key);
	FString LeftS, RightS;
	bool DoIt = false;
	while(InS.Split(Key, &LeftS, &RightS, ESearchCase::IgnoreCase, ESearchDir::FromStart))
	{
		out_array.Add(LeftS);
		DoIt = true;
		InS = RightS;
	}
	if(DoIt)
	{
		out_array.Add(RightS);
	}
}


FString path2dir(const FString &path)
{
	FString LeftS, RightS;
	if(path.Split(TEXT("/"), &LeftS, &RightS, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		return LeftS + TEXT("/");
	}
	if(path.Split(TEXT("\\"), &LeftS, &RightS, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
	{
		return LeftS + TEXT("\\");
	}
	return path;
}

FString path2file(const FString &path)
{
	FString LeftS, RightS;
	if(path.Split(TEXT("/"), &LeftS, &RightS, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
	{
		return RightS;
	}
	if(path.Split(TEXT("\\"), &LeftS, &RightS, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
	{
		return RightS;
	}
	return FString(TEXT(""));
}

FString getFullPath( const FString& basePath , const FString &relPath )
{
	return basePath + relPath;
}
