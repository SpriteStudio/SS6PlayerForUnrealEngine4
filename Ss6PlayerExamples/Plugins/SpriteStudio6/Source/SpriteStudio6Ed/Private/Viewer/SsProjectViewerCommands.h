#pragma once

#include "Framework/Commands/Commands.h"
#include "Styling/AppStyle.h"

class FSsProjectViewerCommands
	: public TCommands<FSsProjectViewerCommands>
{
public:
	FSsProjectViewerCommands()
		: TCommands<FSsProjectViewerCommands>("SsProjectViewer", NSLOCTEXT("Contexts", "SsProjectViewer", "SsProject Viewer"), NAME_None, FAppStyle::GetAppStyleSetName())
	{}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> Play;
	TSharedPtr<FUICommandInfo> PrevFrame;
	TSharedPtr<FUICommandInfo> NextFrame;
	TSharedPtr<FUICommandInfo> Loop;
	TSharedPtr<FUICommandInfo> DrawCollision;
	TSharedPtr<FUICommandInfo> DrawGrid;
};
