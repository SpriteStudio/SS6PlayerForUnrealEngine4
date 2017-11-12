#pragma once


/*
 * @brief     文字列を指定のkeyで分割して返します。
 *
 * @param[in] in_str		分割する文字列
 * @param[in] key			分割のキーとなる文字列
 * @param[out] out_array	分割した文字列を格納する文字列リスト
 * @retval なし
*/
void SPRITESTUDIO6_API split_string( const FString &in_str , 
						const TCHAR key, 
						TArray<FString>& out_array );

/*
 * @brief ファイルのフルパスからフォルダパスのみを取得します。
 * @param[in] ファイルパス
 * @retval ファイルパスからファイル名を取り除いた文字列
*/
FString SPRITESTUDIO6_API path2dir(const FString &path);


/*
 * @brief ファイルのフルパスからファイル名のみを取得します。
 * @param[in] ファイルパス
 * @retval ファイルパスからフォルダ名を取り除いた文字列
*/
FString SPRITESTUDIO6_API path2file(const FString &path);


/*
 * @brief 相対パスを絶対パスへ変換する
 * param[in] basePath 基準ディレクトリ
 * param[int] relPath 相対パス
 * retval relpathを絶対パスへ変換した値
*/
FString SPRITESTUDIO6_API getFullPath( const FString& basePath , const FString &relPath);


