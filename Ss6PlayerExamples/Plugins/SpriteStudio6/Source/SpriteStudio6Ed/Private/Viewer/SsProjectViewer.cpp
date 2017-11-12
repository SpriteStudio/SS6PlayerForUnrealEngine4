#include "SpriteStudio6EdPrivatePCH.h"
#include "SsProjectViewer.h"

#include "EditorStyleSet.h"
#include "Editor/PropertyEditor/Public/IDetailsView.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"

#include "SsProjectViewerCommands.h"
#include "SsProject.h"
#include "SsAnimePack.h"
#include "SsProjectViewerViewport.h"
#include "SsProjectViewerViewportClient.h"
#include "SsRenderOffScreen.h"


#define LOCTEXT_NAMESPACE "SsProjectViewer"


namespace
{
	static const FName ViewportTabId(TEXT("SspjViewer_Viewport"));
	static const FName DetailsTabId(TEXT("SspjViewer_Details"));


	FString CreateDisplayName(FName Name, int32 Index)
	{
		return FString::Printf(TEXT("[%d] %s"), Index, *(Name.ToString()));
	}
	FName FromDisplayName(FString DisplayName)
	{
		FString Left, Right;
		if(DisplayName.Split(TEXT("] "), &Left, &Right))
		{
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


void FSsProjectViewer::Tick(float /*DeltaTime*/)
{
	// SsProjectがReimportされたら、Windowを閉じる
	if(Player.GetSsProject().IsStale())
	{
		Viewport->SetPlayer(NULL, NULL);
		CloseWindow();
	}
}


void FSsProjectViewer::InitEditor( const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit )
{
	SsProject = CastChecked<USsProject>( ObjectToEdit );

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
								->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.1f)
						)
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
		AnimePackNames.Add(MakeShareable(new FString(CreateDisplayName(SsProject->AnimeList[i].AnimePackName, i))) );
	}
	CurrentAnimePack = &(SsProject->AnimeList[0]);

	Player.SetSsProject(SsProject);

	RenderOffScreen = new FSsRenderOffScreen();
	RenderOffScreen->Initialize(2048, 2048, SsProject->CalcMaxRenderPartsNum());
	RenderOffScreen->bSupportAlphaBlendMode = true;

	ExtendToolbar();
	RegenerateMenusAndToolbars();

	Viewport->SetPlayer(&Player, RenderOffScreen);
	Player.Play(0, 0);
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
				TSharedPtr<STextComboBox> AnimePackCombo;
				ToolbarBuilder.AddWidget(
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.4f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("AnimePack", "AnimePack:"))
					]
					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.FillHeight(0.6f)
					[
						(AnimePackCombo = SNew(STextComboBox)
						.OptionsSource(&Viewer->AnimePackNames)
						.OnSelectionChanged(Viewer, &FSsProjectViewer::OnAnimePackChanged)
						.IsEnabled(true)
						).ToSharedRef()
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
						(Viewer->AnimationCombo = SNew(STextComboBox)
						.OptionsSource(&Viewer->AnimationNames)
						.OnSelectionChanged(Viewer, &FSsProjectViewer::OnAnimationChanged)
						.IsEnabled(true)
						).ToSharedRef()
					]
					);

				if(0 < Viewer->AnimePackNames.Num())
				{
					AnimePackCombo->SetSelectedItem(Viewer->AnimePackNames[0]);
				}
			} ToolbarBuilder.EndSection();


			ToolbarBuilder.BeginSection("View Settings");
			{
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
						.IgnoreAlpha(true)
						.OnMouseButtonDown(Viewer, &FSsProjectViewer::OnGridColorMouseDown)
						).ToSharedRef()
					]
					);

				// Background Color
				ToolbarBuilder.AddWidget(
					SNew(SVerticalBox)
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
						(Viewer->GridColorBlock = SNew(SColorBlock)
						.Color(Viewer, &FSsProjectViewer::GetBackColor) 
						.IgnoreAlpha(true)
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

void FSsProjectViewer::OnAnimePackChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type)
{
	if(NewSelection.IsValid() && SsProject)
	{
		FName AnimationName = FromDisplayName(NewSelection.Get()->operator*());
		int32 AnimPackIndex = SsProject->FindAnimePackIndex( AnimationName );
		if(AnimPackIndex < 0){ return; }

		FSsAnimePack* AnimePack = &(SsProject->AnimeList[AnimPackIndex]);
		if(0 < AnimePack->AnimeList.Num())
		{
			CurrentAnimePack = AnimePack;
			AnimationNames.Empty(0);
			for(int32 i = 0; i < CurrentAnimePack->AnimeList.Num(); ++i)
			{
				AnimationNames.Add(MakeShareable(new FString(CreateDisplayName(CurrentAnimePack->AnimeList[i].AnimationName, i))));
			}
			AnimationCombo->RefreshOptions();
			AnimationCombo->SetSelectedItem((0 < AnimationNames.Num()) ? AnimationNames[0] : TSharedPtr<FString>());
		}
	}
}

void FSsProjectViewer::OnAnimationChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type)
{
	if(NewSelection.IsValid() && SsProject)
	{
		if(CurrentAnimePack && (0 < CurrentAnimePack->AnimeList.Num()))
		{
			FName AnimationName = FromDisplayName(NewSelection.Get()->operator*());
			int32 AnimPackIndex, AnimationIndex;
			if(Player.GetAnimationIndex(CurrentAnimePack->AnimePackName, AnimationName, AnimPackIndex, AnimationIndex))
			{
				bool bPlaying =  Player.IsPlaying();
				Player.Play(AnimPackIndex, AnimationIndex);
				Player.Tick(0.f);
				if(!bPlaying)
				{
					Player.Pause();
				}

				FString Text = FString::Printf(TEXT("                    / %3d   "), (int)Player.GetAnimeEndFrame());
				MaxFrameText->SetText(Text);

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
	PickerArgs.InitialColorOverride = Viewport->ViewportClient->GridColor;
	PickerArgs.bOnlyRefreshOnOk = true;
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
	PickerArgs.ParentWidget = GridColorBlock;
	PickerArgs.bUseAlpha = true;
	PickerArgs.DisplayGamma = TAttribute<float>::Create( TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma) );
	PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP( this, &FSsProjectViewer::OnSetBackColor);
	PickerArgs.InitialColorOverride = Viewport->ViewportClient->GetBackgroundColor();
	PickerArgs.bOnlyRefreshOnOk = true;
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

#undef LOCTEXT_NAMESPACE
