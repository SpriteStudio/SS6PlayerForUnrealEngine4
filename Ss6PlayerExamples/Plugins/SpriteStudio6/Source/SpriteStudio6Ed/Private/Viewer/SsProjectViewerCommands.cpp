#include "SsProjectViewerCommands.h"

#define LOCTEXT_NAMESPACE "SpriteStudio6Ed"

void FSsProjectViewerCommands::RegisterCommands()
{
	UI_COMMAND(Play, "Play", "Play SsProject", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(PrevFrame, "PrevFrame", "Back One Frame", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NextFrame, "NextFrame", "Forward One Frame", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(Loop, "Loop", "Loop", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(DrawCollision, "Collision", "Draw Collision", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(DrawGrid, "Grid", "Draw Grid", EUserInterfaceActionType::ToggleButton, FInputChord());
}

#undef LOCTEXT_NAMESPACE
