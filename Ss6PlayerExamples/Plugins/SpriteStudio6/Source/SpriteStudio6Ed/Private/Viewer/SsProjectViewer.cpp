#include "SsProjectViewer.h"

#include "EditorStyleSet.h"
#include "Editor/PropertyEditor/Public/IDetailsView.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "HAL/PlatformApplicationMisc.h"

#include "SsProjectViewerCommands.h"
#include "Ss6Project.h"
#include "SsAnimePack.h"
#include "SsProjectViewerViewport.h"
#include "SsProjectViewerViewportClient.h"
#include "SsRenderOffScreen.h"


#define LOCTEXT_NAMESPACE "SpriteStudio6Ed"


namespace
{
	static const FName ViewportTabId(TEXT("SspjViewer_Viewport"));
	static const FName DetailsTabId(TEXT("SspjViewer_Details"));


	FString CreateAnimePackDisplayName(FName Name, int32 Index, bool bSequence)
	{
		return FString::Printf(TEXT("%s[%d] %s"), (bSequence ? TEXT("SEQ ") : TEXT("")), Index, *(Name.ToString()));
	}
	FString CreateAnimationDisplayName(FName Name, int32 Index, int32 SequenceId)
	{
		if(0 <= SequenceId)
		{
			return FString::Printf(TEXT("[%d] %s - (%d)"), Index, *(Name.ToString()), SequenceId);
		}
		else
		{
			return FString::Printf(TEXT("[%d] %s"), Index, *(Name.ToString()));
		}
	}
	FName FromDisplayName(const FString& DisplayName)
	{
		FString Left, Right;
		if(DisplayName.Split(TEXT("] "), &Left, &Right))
		{
			FString Temp = Right;
			if(Temp.Split(TEXT(" - ("), &Left, &Right))
			{
				return FName(*Left);
			}
			return FName(*Right);
		}
		return FName();
	}

}


TSharedRef<FSsProjectViewer> FSsProjectViewer::CreateEditor( const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit )
{
	TSharedRef<FSsProjectViewer> NewEditor( new FSsProjectViewer() );

	NewEditor->InitEditor( Mode, InitToolkitHost, ObjectToEdit );

	return NewEditor;
}


void FSsProjectViewer::RegisterTabSpawners( const TSharedRef<class FTabManager>& InTabManager )
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_SsProjectViewer", "SsProjectViewer"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner( ViewportTabId, FOnSpawnTab::CreateSP(this, &FSsProjectViewer::SpawnTab_Viewport) )
		.SetDisplayName( LOCTEXT("ViewportTab", "Viewport") )
		.SetGroup( WorkspaceMenuCategoryRef );

	InTabManager->RegisterTabSpawner( DetailsTabId, FOnSpawnTab::CreateSP(this, &FSsProjectViewer::SpawnTab_Details) )
		.SetDisplayName( LOCTEXT("DetailsTab", "Details") )
		.SetGroup( WorkspaceMenuCategoryRef );
}
void FSsProjectViewer::UnregisterTabSpawners( const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner( ViewportTabId );
	InTabManager->UnregisterTabSpawner( DetailsTabId );
}

bool FSsProjectViewer::OnRequestClose()
{
	if(NULL != RenderOffScreen)
	{
		RenderOffScreen->ReserveTerminate();
		RenderOffScreen = NULL;
	}
	return true;
}

FName FSsProjectViewer::GetToolkitFName() const
{
	return FName("SsProjectViewer");
}
FText FSsProjectViewer::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "SsProject Viewer");
}
FString FSsProjectViewer::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "SsProject").ToString();
}
FLinearColor FSsProjectViewer::GetWorldCentricTabColorScale() const
{
	return FLinearColor(SSPJ_COLOR);
}

void FSsProjectViewer::InitEditor( const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit )
{
	SsProject = CastChecked<USs6Project>( ObjectToEdit );

	FSsProjectViewerCommands::Register();
	BindCommands();

	const TSharedRef<FTabManager::FLayout> DefaultLayout = FTabManager::NewLayout( "SsProjectViewer_Layout" )
		->AddArea
		(
			FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()
						->SetOrientation(Orient_Vertical)
						->SetSizeCoefficient(0.66f)
						->Split
						(
							FTabManager::NewStack()
								->AddTab(ViewportTabId, ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.9f)
						)
				)
				->Split
				(
					FTabManager::NewStack()
						->AddTab(DetailsTabId, ETabState::OpenedTab)
						->SetSizeCoefficient(0.33f)
				)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor( Mode, InitToolkitHost, FName(TEXT("SsProjectViewerApp")), DefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectToEdit );

	for(int32 i = 0; i < SsProject->AnimeList.Num(); ++i)
	{
		AnimePackNames.Add(MakeShareable(new FString(CreateAnimePackDisplayName(SsProject->AnimeList[i].AnimePackName, i, false))));
	}
	for(int32 i = 0; i < SsProject->SequenceList.Num(); ++i)
	{
		AnimePackNames.Add(MakeShareable(new FString(CreateAnimePackDisplayName(SsProject->SequenceList[i].SequencePackName, i, true))));
	}
	CurrentAnimePack = &(SsProject->AnimeList[0]);
	CurrentSequencePack = nullptr;

	Player.SetSsProject(SsProject);

	uint32 MaxVertexNum = SsProject->MaxVertexNum;
	uint32 MaxIndexNum  = SsProject->MaxIndexNum;

	RenderOffScreen = new FSsRenderOffScreen();
	RenderOffScreen->Initialize(2048, 2048, MaxVertexNum, MaxIndexNum, SsProject->bContainsMaskPart);

	ExtendToolbar();
	RegenerateMenusAndToolbars();

	Viewport->SetPlayer(&Player, RenderOffScreen);


	int32 AnimIndex = SsProject->AnimeList[0].FindMinimumAnimationIndexExcludingSetup();
	AnimationCombo->SetSelectedItem(AnimationNames[AnimIndex]);

	bLoop = true;
	Player.Play(0, AnimIndex);
}

TSharedRef<SDockTab> FSsProjectViewer::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check( Args.GetTabId() == ViewportTabId );

	Viewport = SNew(SSsProjectViewerViewport);

	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTitle", "Viewport"))
		[
			Viewport.ToSharedRef()
		];

}

TSharedRef<SDockTab> FSsProjectViewer::SpawnTab_Details(const FSpawnTabArgs& InArgs)
{
	check( InArgs.GetTabId() == DetailsTabId );

	////
	TSharedPtr<IDetailsView> PropertiesWidget;
	{
		FDetailsViewArgs Args;
		Args.bHideSelectionTip = true;

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertiesWidget = PropertyModule.CreateDetailView(Args);
		PropertiesWidget->SetObject( SsProject );
	}
	////

	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTitle", "Details"))
		[
			PropertiesWidget.ToSharedRef()
		];
}



// ツールバーの項目追加
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FSsProjectViewer::ExtendToolbar()
{
	struct Local
	{
		static void FillToolbar(FToolBarBuilder& ToolbarBuilder, const TSharedRef< FUICommandList > ToolkitCommands, TSharedRef<SWidget> LODControl, FSsProjectViewer* Viewer)
		{
			ToolbarBuilder.BeginSection("Play Control");
			{
				ToolbarBuilder.AddToolBarButton(
					FSsProjectViewerCommands::Get().Play,
					NAME_None,
					TAttribute<FText>(),
					TAttribute<FText>(),
					FSlateIcon(ISpriteStudio6Ed::Get().Style->GetStyleSetName(), "PlayIcon"),
					NAME_None
					);
				ToolbarBuilder.AddToolBarButton(
					FSsProjectViewerCommands::Get().PrevFrame,
					NAME_None,
					TAttribute<FText>(),
					TAttribute<FText>(),
					FSlateIcon(ISpriteStudio6Ed::Get().Style->GetStyleSetName(), "PrevIcon"),
					NAME_None
					);
				ToolbarBuilder.AddToolBarButton(
					FSsProjectViewerCommands::Get().NextFrame,
					NAME_None,
					TAttribute<FText>(),
					TAttribute<FText>(),
					FSlateIcon(ISpriteStudio6Ed::Get().Style->GetStyleSetName(), "NextIcon"),
					NAME_None
					);
				ToolbarBuilder.AddToolBarButton(
					FSsProjectViewerCommands::Get().Loop,
					NAME_None,
					TAttribute<FText>(),
					TAttribute<FText>(),
					FSlateIcon(ISpriteStudio6Ed::Get().Style->GetStyleSetName(), "LoopIcon"),
					NAME_None
					);

				// Frame
				ToolbarBuilder.AddWidget(
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SetFrame", "Frame:"))
					]
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(SNumericEntryBox<int32>)
						.AllowSpin(true)
						.MinValue(0)
						.MinSliderValue(0)
						.MaxValue(Viewer, &FSsProjectViewer::GetMaxFrame)
						.MaxSliderValue(Viewer, &FSsProjectViewer::GetMaxFrame)
						.Value(Viewer, &FSsProjectViewer::GetNowFrame)
						.OnValueChanged(Viewer, &FSsProjectViewer::OnSetFrame)
						.OnValueCommitted(Viewer, &FSsProjectViewer::OnSetFrame)
					]
					+SVerticalBox::Slot()
					[
						(Viewer->MaxFrameText = SNew(STextBlock)
						.Text(FText::FromString(TEXT("  / ---")))
						).ToSharedRef()
					]
					);
			} ToolbarBuilder.EndSection();


			ToolbarBuilder.BeginSection("Animation Control");
			{
				// AnimePack
				ToolbarBuilder.AddWidget(
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.3f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("AnimePack", "AnimePack:"))
					]
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.5f)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Fill)
						.FillWidth(0.8f)
						[
							(Viewer->AnimePackCombo = SNew(STextComboBox)
							.OptionsSource(&Viewer->AnimePackNames)
							.OnSelectionChanged(Viewer, &FSsProjectViewer::OnAnimePackChanged)
							.IsEnabled(true)
							).ToSharedRef()
						]
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Fill)
						.FillWidth(0.2f)
						.AutoWidth()
						[
							SNew(SButton)
							.Text(LOCTEXT("Copy", "Copy"))
							.ToolTipText(LOCTEXT("CopyAnimePackToolTip", "Copy AnimePack Name to Clipboard."))
							.OnClicked(Viewer, &FSsProjectViewer::OnClickedCopyAnimPackName)
						]
					]
					);
				// Animation
				ToolbarBuilder.AddWidget(
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.4f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Animation", "Animation:"))
					]
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.6f)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Fill)
						.FillWidth(0.8f)
						[
							(Viewer->AnimationCombo = SNew(STextComboBox)
							.OptionsSource(&Viewer->AnimationNames)
							.OnSelectionChanged(Viewer, &FSsProjectViewer::OnAnimationChanged)
							.IsEnabled(true)
							).ToSharedRef()
						]
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Fill)
						.FillWidth(0.2f)
						.AutoWidth()
						[
							SNew(SButton)
							.Text(LOCTEXT("Copy", "Copy"))
							.ToolTipText(LOCTEXT("CopyAnimationToolTip", "Copy Animation Name to Clipboard."))
							.OnClicked(Viewer, &FSsProjectViewer::OnClickedCopyAnimationName)
						]
					]
					);

				if(0 < Viewer->AnimePackNames.Num())
				{
					Viewer->AnimePackCombo->SetSelectedItem(Viewer->AnimePackNames[0]);
				}
			} ToolbarBuilder.EndSection();


			ToolbarBuilder.BeginSection("View Settings");
			{
				// Collision On/Off
				ToolbarBuilder.AddToolBarButton(
					FSsProjectViewerCommands::Get().DrawCollision,
					NAME_None,
					TAttribute<FText>(),
					TAttribute<FText>(),
					FSlateIcon(ISpriteStudio6Ed::Get().Style->GetStyleSetName(), "CollisionIcon"),
					NAME_None
					);

				// Grid On/Off
				ToolbarBuilder.AddToolBarButton(
					FSsProjectViewerCommands::Get().DrawGrid,
					NAME_None,
					TAttribute<FText>(),
					TAttribute<FText>(),
					FSlateIcon(ISpriteStudio6Ed::Get().Style->GetStyleSetName(), "GridIcon"),
					NAME_None
					);

				// Grid Color/Size
				ToolbarBuilder.AddWidget(
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.4f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("GridSize", "Size:"))
					]
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.6f)
					[
						SNew(SNumericEntryBox<int32>)
						.MinValue(2)
						.Value(Viewer, &FSsProjectViewer::GetGridSize)
						.OnValueCommitted(Viewer, &FSsProjectViewer::OnSetGridSize)
					]
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.4f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("GridColor", "GridColor:"))
					]
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.6f)
					[
						(Viewer->GridColorBlock = SNew(SColorBlock)
						.Color(Viewer, &FSsProjectViewer::GetGridColor)
						.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Ignore)
						.OnMouseButtonDown(Viewer, &FSsProjectViewer::OnGridColorMouseDown)
						).ToSharedRef()
					]
					);

				// Background Color
				ToolbarBuilder.AddWidget(
					SNew(SVerticalBox)
					+SVerticalBox::Slot().FillHeight(0.4f)[SNew(STextBlock).Text(LOCTEXT("", ""))]
					+SVerticalBox::Slot().FillHeight(0.6f)[SNew(STextBlock).Text(LOCTEXT("", ""))]
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.4f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("BackGroundColor", "BackColor:"))
					]
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.6f)
					[
						(Viewer->BackColorBlock = SNew(SColorBlock)
						.Color(Viewer, &FSsProjectViewer::GetBackColor) 
						.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Ignore)
						.OnMouseButtonDown(Viewer, &FSsProjectViewer::OnBackColorMouseDown)
						).ToSharedRef()
					]
					);

			} ToolbarBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	TSharedRef<SWidget> Control = SNew(SBox);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar, GetToolkitCommands(), Control, this)
		);
	AddToolbarExtender(ToolbarExtender);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FSsProjectViewer::BindCommands()
{
	const FSsProjectViewerCommands& Commands = FSsProjectViewerCommands::Get();

	ToolkitCommands->MapAction(
		Commands.Play,
		FExecuteAction::CreateSP(this, &FSsProjectViewer::OnPlay),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FSsProjectViewer::IsPlaying)
		);
	ToolkitCommands->MapAction(
		Commands.PrevFrame,
		FExecuteAction::CreateSP(this, &FSsProjectViewer::OnPrevFrame)
		);
	ToolkitCommands->MapAction(
		Commands.NextFrame,
		FExecuteAction::CreateSP(this, &FSsProjectViewer::OnNextFrame)
		);
	ToolkitCommands->MapAction(
		Commands.Loop,
		FExecuteAction::CreateSP(this, &FSsProjectViewer::OnLoop),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FSsProjectViewer::IsLooping)
		);
	ToolkitCommands->MapAction(
		Commands.DrawCollision,
		FExecuteAction::CreateSP(this, &FSsProjectViewer::OnChangeDrawCollision),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FSsProjectViewer::IsDrawCollision)
		);
	ToolkitCommands->MapAction(
		Commands.DrawGrid,
		FExecuteAction::CreateSP(this, &FSsProjectViewer::OnChangeDrawGrid),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FSsProjectViewer::IsDrawGrid)
		);
}

void FSsProjectViewer::OnPlay()
{
	if(Player.IsPlaying())
	{
		Player.Pause();
	}
	else
	{
		if(Player.GetPlayFrame() == Player.GetAnimeEndFrame())
		{
			Player.SetPlayFrame(0.f);
		}
		Player.LoopCount = bLoop ? 0 : 1;
		Player.Resume();
	}
}
bool FSsProjectViewer::IsPlaying() const
{
	return Player.IsPlaying();
}

void FSsProjectViewer::OnPrevFrame()
{
	float Frame = Player.GetPlayFrame() - 1.f;
	Player.SetPlayFrame((float)Frame);
	if(!Player.IsPlaying())
	{
		float PlayRate = Player.PlayRate;
		Player.PlayRate = -1.f;
		Player.Resume();
		Player.Tick(0.f);
		Player.Pause();
		Player.PlayRate = PlayRate;

		if(RenderOffScreen)
		{
			RenderOffScreen->Render(Player.GetRenderParts());
		}
	}
}
void FSsProjectViewer::OnNextFrame()
{
	float Frame = Player.GetPlayFrame() + 1.f;
	Player.SetPlayFrame((float)Frame);
	if(!Player.IsPlaying())
	{
		Player.Resume();
		Player.Tick(0.f);
		Player.Pause();

		if(RenderOffScreen)
		{
			RenderOffScreen->Render(Player.GetRenderParts());
		}
	}
}

void FSsProjectViewer::OnLoop()
{
	bLoop = !bLoop;
	Player.LoopCount = bLoop ? 0 : 1;
}
bool FSsProjectViewer::IsLooping() const
{
	return bLoop;
}

void FSsProjectViewer::OnAnimePackChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type)
{
	if(NewSelection.IsValid() && SsProject)
	{
		FName AnimPackName = FromDisplayName(NewSelection.Get()->operator*());
		int32 AnimPackIndex = SsProject->FindAnimePackIndex(AnimPackName);
		if(0 <= AnimPackIndex)
		{
			FSsAnimePack* AnimePack = &(SsProject->AnimeList[AnimPackIndex]);
			if(0 < AnimePack->AnimeList.Num())
			{
				CurrentAnimePack = AnimePack;
				CurrentSequencePack = nullptr;
				AnimationNames.Empty();
				for(int32 i = 0; i < CurrentAnimePack->AnimeList.Num(); ++i)
				{
					AnimationNames.Add(MakeShareable(new FString(CreateAnimationDisplayName(CurrentAnimePack->AnimeList[i].AnimationName, i, -1))));
				}
				int32 AnimeIndex = CurrentAnimePack->FindMinimumAnimationIndexExcludingSetup();
				AnimationCombo->RefreshOptions();
				AnimationCombo->SetSelectedItem((AnimeIndex < AnimationNames.Num()) ? AnimationNames[AnimeIndex] : TSharedPtr<FString>());
			}
		}
		else
		{
			AnimPackIndex = SsProject->FindSequencePackIndex(AnimPackName);
			if(0 <= AnimPackIndex)
			{
				FSsSequencePack* SequencePack = &(SsProject->SequenceList[AnimPackIndex]);
				if(0 < SequencePack->SequenceList.Num())
				{
					CurrentAnimePack = nullptr;
					CurrentSequencePack = SequencePack;
					AnimationNames.Empty();
					for(int32 i = 0; i < CurrentSequencePack->SequenceList.Num(); ++i)
					{
						AnimationNames.Add(MakeShareable(new FString(CreateAnimationDisplayName(CurrentSequencePack->SequenceList[i].SequenceName, i, CurrentSequencePack->SequenceList[i].Id))));
					}
					AnimationCombo->RefreshOptions();
					AnimationCombo->SetSelectedItem(AnimationNames[0]);
				}
			}
		}
	}
}

void FSsProjectViewer::OnAnimationChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type)
{
	if(!NewSelection.IsValid() || (nullptr == SsProject))
	{
		return;
	}

	if(nullptr != CurrentAnimePack)
	{
		if(0 < CurrentAnimePack->AnimeList.Num())
		{
			FName AnimationName = FromDisplayName(NewSelection.Get()->operator*());
			int32 AnimPackIndex, AnimationIndex;
			if(Player.GetAnimationIndex(CurrentAnimePack->AnimePackName, AnimationName, AnimPackIndex, AnimationIndex))
			{
				bool bPlaying =  Player.IsPlaying();
				Player.Play(AnimPackIndex, AnimationIndex, 0, 1.f, bLoop ? 0 : 1);
				Player.Tick(0.f);
				if(!bPlaying)
				{
					Player.Pause();
				}

				FString Text = FString::Printf(TEXT("                    / %3d   "), (int32)(Player.GetAnimeEndFrame()));
				MaxFrameText->SetText(FText::FromString(Text));

				if(RenderOffScreen)
				{
					RenderOffScreen->Render(Player.GetRenderParts());
				}
			}
		}
	}
	else if(nullptr != CurrentSequencePack)
	{
		if(0 < CurrentSequencePack->SequenceList.Num())
		{
			FName SequenceName = FromDisplayName(NewSelection.Get()->operator*());
			int32 SequencePackIndex, SequenceIndex;
			if(Player.GetSequenceIndex(CurrentSequencePack->SequencePackName, SequenceName, SequencePackIndex, SequenceIndex))
			{
				bool bPlaying =  Player.IsPlaying();
				Player.PlaySequence(SequencePackIndex, SequenceIndex);
				Player.Tick(0.f);
				if(!bPlaying)
				{
					Player.Pause();
				}

				FString Text = FString::Printf(TEXT("                    / %3d   "), (int32)(Player.GetAnimeEndFrame()));
				MaxFrameText->SetText(FText::FromString(Text));

				if(RenderOffScreen)
				{
					RenderOffScreen->Render(Player.GetRenderParts());
				}
			}
		}
	}
}

void FSsProjectViewer::OnSetFrame(int32 Frame, ETextCommit::Type)
{
	OnSetFrame(Frame);
}
void FSsProjectViewer::OnSetFrame(int32 Frame)
{
	if(Frame < Player.GetAnimeEndFrame())
	{
		Player.SetPlayFrame((float)Frame);
		if(!Player.IsPlaying())
		{
			Player.Resume();
			Player.Tick(0.f);
			Player.Pause();

			if(RenderOffScreen)
			{
				RenderOffScreen->Render(Player.GetRenderParts());
			}
		}
	}
}
TOptional<int32> FSsProjectViewer::GetNowFrame() const
{
	return FMath::FloorToInt(Player.GetPlayFrame());
}
TOptional<int32> FSsProjectViewer::GetMaxFrame() const
{
	return (int32)Player.GetAnimeEndFrame() - 1;
}

void FSsProjectViewer::OnChangeDrawCollision()
{
	Viewport->ViewportClient->bDrawCollision = !Viewport->ViewportClient->bDrawCollision;
}
bool FSsProjectViewer::IsDrawCollision() const
{
	return Viewport->ViewportClient->bDrawCollision;
}

void FSsProjectViewer::OnChangeDrawGrid()
{
	Viewport->ViewportClient->bDrawGrid = !Viewport->ViewportClient->bDrawGrid;
}
bool FSsProjectViewer::IsDrawGrid() const
{
	return Viewport->ViewportClient->bDrawGrid;
}

void FSsProjectViewer::OnSetGridSize(int32 Size, ETextCommit::Type)
{
	if(2 <= Size)
	{
		Viewport->ViewportClient->GridSize = Size;
	}
}
TOptional<int32> FSsProjectViewer::GetGridSize() const
{
	return Viewport->ViewportClient->GridSize;
}

FReply FSsProjectViewer::OnGridColorMouseDown(const FGeometry&, const FPointerEvent& MouseEvent)
{
	if(MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}
	FColorPickerArgs PickerArgs;
	PickerArgs.ParentWidget = GridColorBlock;
	PickerArgs.bUseAlpha = true;
	PickerArgs.DisplayGamma = TAttribute<float>::Create( TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma) );
	PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP( this, &FSsProjectViewer::OnSetGridColor);
	PickerArgs.InitialColor = Viewport->ViewportClient->GridColor;
	PickerArgs.bOnlyRefreshOnOk = false;
	OpenColorPicker(PickerArgs);
	return FReply::Handled();
}
void FSsProjectViewer::OnSetGridColor(FLinearColor Color)
{
	Viewport->ViewportClient->GridColor = Color;
}
FLinearColor FSsProjectViewer::GetGridColor() const
{
	return Viewport->ViewportClient->GridColor;
}

FReply FSsProjectViewer::OnBackColorMouseDown(const FGeometry&, const FPointerEvent& MouseEvent)
{
	if(MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}
	FColorPickerArgs PickerArgs;
	PickerArgs.ParentWidget = BackColorBlock;
	PickerArgs.bUseAlpha = true;
	PickerArgs.DisplayGamma = TAttribute<float>::Create( TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma) );
	PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP( this, &FSsProjectViewer::OnSetBackColor);
	PickerArgs.InitialColor = Viewport->ViewportClient->GetBackgroundColor();
	PickerArgs.bOnlyRefreshOnOk = false;
	OpenColorPicker(PickerArgs);
	return FReply::Handled();
}
void FSsProjectViewer::OnSetBackColor(FLinearColor Color)
{
	Viewport->ViewportClient->SetBackgroundColor(Color);
}
FLinearColor FSsProjectViewer::GetBackColor() const
{
	return Viewport->ViewportClient->GetBackgroundColor();
}
FReply FSsProjectViewer::OnClickedCopyAnimPackName()
{
	FName AnimePackName = FromDisplayName(*AnimePackCombo->GetSelectedItem().Get());
	FPlatformApplicationMisc::ClipboardCopy(*AnimePackName.ToString());
	return FReply::Handled();
}
FReply FSsProjectViewer::OnClickedCopyAnimationName()
{
	FName AnimationName = FromDisplayName(*AnimationCombo->GetSelectedItem().Get());
	FPlatformApplicationMisc::ClipboardCopy(*AnimationName.ToString());
	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE
