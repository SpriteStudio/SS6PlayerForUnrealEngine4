#pragma once

class FSsProjectViewerCommands
	: public TCommands<FSsProjectViewerCommands>
{
public:
	FSsProjectViewerCommands()
		: TCommands<FSsProjectViewerCommands>("SsProjectViewer", NSLOCTEXT("Contexts", "SsProjectViewer", "SsProject Viewer"), NAME_None, FEditorStyle::GetStyleSetName())
	{}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> Play;
	TSharedPtr<FUICommandInfo> PrevFrame;
	TSharedPtr<FUICommandInfo> NextFrame;
	TSharedPtr<FUICommandInfo> Loop;
	TSharedPtr<FUICommandInfo> DrawGrid;
};
