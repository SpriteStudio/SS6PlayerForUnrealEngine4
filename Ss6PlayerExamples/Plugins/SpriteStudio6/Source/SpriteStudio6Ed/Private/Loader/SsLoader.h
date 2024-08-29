#pragma once

#include "SsLoader.h"


class FSsLoader
{
public:
	static class USs6Project* LoadSsProject(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size, TArray<FString>& OutWarnings);
	static bool LoadSsAnimePack(struct FSsAnimePack* AnimePack, const uint8*& Buffer, size_t Size, int32& OutSortOrder, TArray<FString>& OutWarnings);
	static bool LoadSsCellMap(struct FSsCellMap* CellMap, const uint8*& Buffer, size_t Size, TArray<FString>& OutWarnings);
	static bool LoadSsEffectFile(struct FSsEffectFile* EffectFile, const uint8*& Buffer, size_t Size, TArray<FString>& OutWarnings);
	static bool LoadSsSequence(struct FSsSequencePack* SequencePack, const uint8* Buffer, size_t Size, TArray<FString>& OutWarnings);
};

