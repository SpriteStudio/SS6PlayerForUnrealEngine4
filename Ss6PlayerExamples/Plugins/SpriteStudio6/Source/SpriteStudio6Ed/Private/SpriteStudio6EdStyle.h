#pragma once

#include "SlateStyle.h"

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

class FSpriteStudio6EdStyle : public FSlateStyleSet
{
public:
	FSpriteStudio6EdStyle()
		: FSlateStyleSet("SpriteStudio6EdStyle")

	{
		SetContentRoot(FPaths::ProjectPluginsDir() / TEXT("SpriteStudio6/Resources"));

		Set("PlayIcon", new IMAGE_BRUSH("play_icon", FVector2D(40.f, 40.f)));
		Set("PrevIcon", new IMAGE_BRUSH("prev_icon", FVector2D(40.f, 40.f)));
		Set("NextIcon", new IMAGE_BRUSH("next_icon", FVector2D(40.f, 40.f)));
		Set("LoopIcon", new IMAGE_BRUSH("loop_icon", FVector2D(40.f, 40.f)));
		Set("GridIcon", new IMAGE_BRUSH("grid_icon", FVector2D(40.f, 40.f)));

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

	~FSpriteStudio6EdStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}

};

#undef IMAGE_BRUSH
