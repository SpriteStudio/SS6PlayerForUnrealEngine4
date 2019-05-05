#pragma once

#include "SsTypes.h"
#include "SsEffectBehavior.h"

#include "SsEffectFile.generated.h"


USTRUCT()
struct SPRITESTUDIO6_API FSsSimpleTree
{
	GENERATED_USTRUCT_BODY()

public:
	FSsSimpleTree *Parent;
	FSsSimpleTree *Ctop;
	FSsSimpleTree *Prev;
	FSsSimpleTree *Next;

public:
	FSsSimpleTree()
		: Parent(nullptr),Ctop(nullptr),Prev(nullptr),Next(nullptr)
	{}

	void AddChildEnd(FSsSimpleTree* c)
	{
		if(!Ctop)
		{
			Ctop = c; 
		}
		else
		{
			Ctop->AddSiblingEnd(c);
		}
		c->Parent = this;
	}
	void AddSiblingEnd(FSsSimpleTree* c)
	{
		if(!Next)
		{
			c->Prev = this;
			Next = c;
		}
		else
		{
			Next->AddSiblingEnd(c);
		}

		c->Parent = this->Parent;
	}
};


USTRUCT()
struct SPRITESTUDIO6_API FSsEffectNode : public FSsSimpleTree
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar);

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectNode)
	int32 ArrayIndex;

	UPROPERTY(VisibleAnywhere, Category=SsEffectNode)
	int32 ParentIndex;

	UPROPERTY(VisibleAnywhere, Category=SsEffectNode)
	TEnumAsByte<SsEffectNodeType::Type> Type;

	UPROPERTY(VisibleAnywhere, Category=SsEffectNode)
	bool Visible;

	UPROPERTY(VisibleAnywhere, Category=SsEffectNode)
	FSsEffectBehavior Behavior;

public:
	FSsEffectNode()
		: ArrayIndex(0)
		, ParentIndex(0)
		, Type(SsEffectNodeType::Invalid)
		, Visible(false)
	{}

	SsEffectNodeType::Type GetType(){ return Type; }
	FSsEffectBehavior* GetMyBehavior(){ return &Behavior;}
};


USTRUCT()
struct SPRITESTUDIO6_API FSsEffectModel
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar);

public:
	FSsEffectNode* Root;

	UPROPERTY(VisibleAnywhere, Category=SsEffectModel)
	TArray<FSsEffectNode> NodeList;

	//	ランダムシード固定値 
	UPROPERTY(VisibleAnywhere, Category=SsEffectModel)
	int32		LockRandSeed;

	//	ランダムシードを固定するか否か 
	UPROPERTY(VisibleAnywhere, Category=SsEffectModel)
	bool		IsLockRandSeed;

	UPROPERTY(VisibleAnywhere, Category=SsEffectModel)
	int32		FPS;

	UPROPERTY(VisibleAnywhere, Category=SsEffectModel)
	FString		BgColor;

	UPROPERTY(VisibleAnywhere, Category=SsEffectModel)
	FName		EffectName;

	UPROPERTY(VisibleAnywhere, Category = SsEffectModel)
	int32		LayoutScaleX;

	UPROPERTY(VisibleAnywhere, Category = SsEffectModel)
	int32		LayoutScaleY;

public:
	FSsEffectModel()
		: Root(nullptr)
		, LockRandSeed(0)
		, IsLockRandSeed(false)
		, FPS(0)
		, LayoutScaleX(100)
		, LayoutScaleY(100)
	{}

	void BuildTree();
};


USTRUCT()
struct SPRITESTUDIO6_API FSsEffectFile 
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar);

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectFile)
	FSsEffectModel EffectData;

	UPROPERTY(VisibleAnywhere, Category=SsEffectFile)
	FName Name;
};

