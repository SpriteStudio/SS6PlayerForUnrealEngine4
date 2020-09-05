#include "SsTypes.h"

//---------------------------------------------------------------
//相互変換 SsPartType
FString	__EnumToString_( TEnumAsByte<SsPartType::Type> n )
{
	if ( n == SsPartType::Invalid ) return "invalid";
	if ( n == SsPartType::Null ) return "null";
	if ( n == SsPartType::Normal ) return "normal";
	if ( n == SsPartType::Text ) return "text";
	if ( n == SsPartType::Instance ) return "instance";
	if ( n == SsPartType::Effect ) return "effect";
	if ( n == SsPartType::Armature ) return "armature";
	if ( n == SsPartType::Mesh ) return "mesh";
	if ( n == SsPartType::MoveNode ) return "movenode";
	if ( n == SsPartType::Constraint ) return "constraint";
	if ( n == SsPartType::Mask ) return "mask";
	if ( n == SsPartType::Joint ) return "joint";
	if ( n == SsPartType::BonePoint ) return "bonepoint";
	return "invalid";	
}

void 	__StringToEnum_( FString n , TEnumAsByte<SsPartType::Type>& out)
{
	out =  SsPartType::Invalid;
	if ( n == "invalid") out = SsPartType::Invalid;
	if ( n == "null") out = SsPartType::Null;
	if ( n == "normal") out = SsPartType::Normal;
	if ( n == "text") out = SsPartType::Text;
	if ( n == "instance") out = SsPartType::Instance;
	if ( n == "effect") out = SsPartType::Effect;
	if ( n == "armature") out = SsPartType::Armature;
	if ( n == "mesh") out = SsPartType::Mesh;
	if ( n == "movenode") out = SsPartType::MoveNode;
	if ( n == "constraint") out = SsPartType::Constraint;
	if ( n == "mask") out = SsPartType::Mask;
	if ( n == "joint") out = SsPartType::Joint;
	if ( n == "bonepoint") out = SsPartType::BonePoint;
}

//---------------------------------------------------------------
//相互変換 SsPartsSortMode
FString	__EnumToString_(SsPartsSortMode::_enum n)
{
	if ( n == SsPartsSortMode::invalid ) return "invalid";
	if ( n == SsPartsSortMode::prio ) return "prio";
	if ( n == SsPartsSortMode::z ) return "z";
	return "invalid";	
}

void	__StringToEnum_( FString n , SsPartsSortMode::_enum &out )
{
	out = SsPartsSortMode::invalid;
	if ( n == "invalid") out = SsPartsSortMode::invalid;
	if ( n == "prio") out = SsPartsSortMode::prio;
	if ( n == "z") out = SsPartsSortMode::z;
}

//---------------------------------------------------------------
//相互変換 SsBoundsType
FString	__EnumToString_( TEnumAsByte<SsBoundsType::Type> n )
{
	if ( n == SsBoundsType::Invalid ) return "invalid";
	if ( n == SsBoundsType::None ) return "none";
	if ( n == SsBoundsType::Quad ) return "quad";
	if ( n == SsBoundsType::Aabb ) return "aabb";
	if ( n == SsBoundsType::Circle ) return "circle";
	if ( n == SsBoundsType::CircleSmin ) return "circle_smin";
	if ( n == SsBoundsType::CircleSmax ) return "circle_smax";
	return "invalid";	
}

void	__StringToEnum_( FString n , TEnumAsByte<SsBoundsType::Type> &out )
{
	out = SsBoundsType::Invalid;
	if ( n == "invalid") out = SsBoundsType::Invalid;
	if ( n == "none") out = SsBoundsType::None;
	if ( n == "quad") out = SsBoundsType::Quad;
	if ( n == "aabb") out = SsBoundsType::Aabb;
	if ( n == "circle") out = SsBoundsType::Circle;
	if ( n == "circle_smin") out = SsBoundsType::CircleSmin;
	if ( n == "circle_smax") out = SsBoundsType::CircleSmax;
}


//---------------------------------------------------------------
//相互変換 SsBoundsType
FString	__EnumToString_( TEnumAsByte<SsInheritType::Type> n )
{
	if ( n == SsInheritType::Invalid ) return "invalid";
	if ( n == SsInheritType::Parent ) return "parent";
	if ( n == SsInheritType::Self ) return "self";
	return "invalid";	
}

void	__StringToEnum_( FString n , TEnumAsByte<SsInheritType::Type> &out )
{
	out = SsInheritType::Invalid;
	if ( n == "invalid") out = SsInheritType::Invalid;
	if ( n == "parent") out = SsInheritType::Parent;
	if ( n == "self") out = SsInheritType::Self;
}


//---------------------------------------------------------------
//相互変換 SsBlendType
FString	__EnumToString_( TEnumAsByte<SsBlendType::Type> n )
{
	if ( n == SsBlendType::Invalid ) return "invalid";
	if ( n == SsBlendType::Mix ) return "mix";
	if ( n == SsBlendType::Mul ) return "mul";
	if ( n == SsBlendType::Add ) return "add";
	if ( n == SsBlendType::Sub ) return "sub";
	if ( n == SsBlendType::MulAlpha ) return "mulalpha";
	if ( n == SsBlendType::Screen ) return "screen";
	if ( n == SsBlendType::Exclusion ) return "exclusion";
	if ( n == SsBlendType::Invert ) return "invert";

	return "invalid";	
}

void	__StringToEnum_( FString n , TEnumAsByte<SsBlendType::Type> &out )
{
	out = SsBlendType::Invalid;
	if ( n == "invalid") out = SsBlendType::Invalid;
	if ( n == "mix") out = SsBlendType::Mix;
	if ( n == "mul") out = SsBlendType::Mul;
	if ( n == "add") out = SsBlendType::Add;
	if ( n == "sub") out = SsBlendType::Sub;
	if ( n == "mulalpha") out = SsBlendType::MulAlpha;
	if ( n == "screen") out = SsBlendType::Screen;
	if ( n == "exclusion") out = SsBlendType::Exclusion;
	if ( n == "invert") out = SsBlendType::Invert;
}




//---------------------------------------------------------------
//相互変換 SsInterpolationType
FString	__EnumToString_( TEnumAsByte<SsInterpolationType::Type> n )
{
	if ( n == SsInterpolationType::Invalid )		return "invalid";
	if ( n == SsInterpolationType::None )		return "none";
	if ( n == SsInterpolationType::Linear )		return "linear";
	if ( n == SsInterpolationType::Hermite )		return "hermite";
	if ( n == SsInterpolationType::Bezier )		return "bezier";
	if ( n == SsInterpolationType::Acceleration ) return "acceleration";
	if ( n == SsInterpolationType::Deceleration ) return "deceleration";

	return "none";	
}

void	__StringToEnum_( FString n , TEnumAsByte<SsInterpolationType::Type> &out )
{
	out = SsInterpolationType::None;
	if ( n == "invalid") out = SsInterpolationType::Invalid;
	if ( n == "none") out = SsInterpolationType::None;
	if ( n == "linear") out = SsInterpolationType::Linear;
	if ( n == "hermite") out = SsInterpolationType::Hermite;
	if ( n == "bezier") out = SsInterpolationType::Bezier;
	if ( n == "acceleration") out = SsInterpolationType::Acceleration;
	if ( n == "deceleration") out = SsInterpolationType::Deceleration;
}

//---------------------------------------------------------------
//相互変換 SsTexWrapMode
FString	__EnumToString_( TEnumAsByte<SsTexWrapMode::Type> n )
{
	if ( n == SsTexWrapMode::Invalid )		return "invalid";
	if ( n == SsTexWrapMode::Clamp )		return "clamp";
	if ( n == SsTexWrapMode::Repeat )		return "repeat";
	if ( n == SsTexWrapMode::Mirror )		return "mirror";

	return "invalid";	
}

void	__StringToEnum_( FString n , TEnumAsByte<SsTexWrapMode::Type> &out )
{
	out = SsTexWrapMode::Invalid;
	if ( n == "invalid") out = SsTexWrapMode::Invalid;
	if ( n == "clamp") out = SsTexWrapMode::Clamp;
	if ( n == "repeat") out = SsTexWrapMode::Repeat;
	if ( n == "mirror") out = SsTexWrapMode::Mirror;

}

//---------------------------------------------------------------
//相互変換 SsTexFilterMode
FString	__EnumToString_( TEnumAsByte<SsTexFilterMode::Type> n )
{
	if ( n == SsTexFilterMode::Invalid )		return "invalid";
	if ( n == SsTexFilterMode::Nearest )		return "nearlest";	// sspj側の誤植？
	if ( n == SsTexFilterMode::Linear )		return "linear";

	return "invalid";	
}

void	__StringToEnum_( FString n , TEnumAsByte<SsTexFilterMode::Type> &out )
{
	out = SsTexFilterMode::Invalid;
	if ( n == "invalid") out = SsTexFilterMode::Invalid;
	if ( n == "nearlest") out = SsTexFilterMode::Nearest;	// sspj側の誤植？
	if ( n == "linear") out = SsTexFilterMode::Linear;

}

//---------------------------------------------------------------
//相互変換 SsTexFilterMode
FString	__EnumToString_( TEnumAsByte<SsColorBlendTarget::Type> n )
{
	if ( n == SsColorBlendTarget::Invalid )		return "invalid";
	if ( n == SsColorBlendTarget::Whole )		return "whole";
	if ( n == SsColorBlendTarget::Vertex )		return "vertex";

	return "invalid";	
}

void	__StringToEnum_( FString n , TEnumAsByte<SsColorBlendTarget::Type> &out )
{
	out = SsColorBlendTarget::Invalid;
	if ( n == "invalid") out = SsColorBlendTarget::Invalid;
	if ( n == "whole") out = SsColorBlendTarget::Whole;
	if ( n == "vertex") out = SsColorBlendTarget::Vertex;

}

//---------------------------------------------------------------
//相互変換 SsAttributeKind
FString	__EnumToString_( TEnumAsByte<SsAttributeKind::Type> n )
{
	if ( n == SsAttributeKind::Invalid )		return "invalid";
	if ( n == SsAttributeKind::Cell )		return "CELL";
	if ( n == SsAttributeKind::Posx )		return "POSX";
	if ( n == SsAttributeKind::Posy )		return "POSY";
	if ( n == SsAttributeKind::Posz )		return "POSZ";
	if ( n == SsAttributeKind::Rotx )		return "ROTX";
	if ( n == SsAttributeKind::Roty )		return "ROTY";
	if ( n == SsAttributeKind::Rotz )		return "ROTZ";
	if ( n == SsAttributeKind::Sclx )		return "SCLX";
	if ( n == SsAttributeKind::Scly )		return "SCLY";
	if ( n == SsAttributeKind::Losclx )		return "LSCX";
	if ( n == SsAttributeKind::Loscly )		return "LSCY";
	if ( n == SsAttributeKind::Alpha )		return "ALPH";
	if ( n == SsAttributeKind::Loalpha )		return "LALP";
	if ( n == SsAttributeKind::Prio )		return "PRIO";
	if ( n == SsAttributeKind::Fliph )		return "FLPH";
	if ( n == SsAttributeKind::Flipv )		return "FLPV";
	if ( n == SsAttributeKind::Hide )		return "HIDE";
	if ( n == SsAttributeKind::PartsColor )	return "PCOL";
	if ( n == SsAttributeKind::Color )		return "VCOL";
	if ( n == SsAttributeKind::Vertex )		return "VERT";
	if ( n == SsAttributeKind::Pivotx )		return "PVTX";
	if ( n == SsAttributeKind::Pivoty )		return "PVTY";
	if ( n == SsAttributeKind::Anchorx )		return "ANCX";
	if ( n == SsAttributeKind::Anchory )		return "ANCY";
	if ( n == SsAttributeKind::Sizex )		return "SIZX";
	if ( n == SsAttributeKind::Sizey )		return "SIZY";
	if ( n == SsAttributeKind::Imgfliph )	return "IFLH";
	if ( n == SsAttributeKind::Imgflipv )	return "IFLV";
	if ( n == SsAttributeKind::Uvtx )		return "UVTX";
	if ( n == SsAttributeKind::Uvty )		return "UVTY";
	if ( n == SsAttributeKind::Uvrz )		return "UVRZ";
	if ( n == SsAttributeKind::Uvsx )		return "UVSX";
	if ( n == SsAttributeKind::Uvsy )		return "UVSY";
	if ( n == SsAttributeKind::Boundr )		return "BNDR";
	if ( n == SsAttributeKind::User )		return "USER";
	if ( n == SsAttributeKind::Instance )	return "IPRM";
	if ( n == SsAttributeKind::Effect)		return "EFCT";
	if ( n == SsAttributeKind::Mask )		return "MASK";
	if ( n == SsAttributeKind::Deform )		return "DEFM";

	return "invalid";	
}

void	__StringToEnum_( FString n , TEnumAsByte<SsAttributeKind::Type> &out )
{

	out = SsAttributeKind::Invalid;
	if ( n == "invalid") out = SsAttributeKind::Invalid;
	if ( n == "CELL") out = SsAttributeKind::Cell;
	if ( n == "POSX") out = SsAttributeKind::Posx;
	if ( n == "POSY") out = SsAttributeKind::Posy;
	if ( n == "POSZ") out = SsAttributeKind::Posz;
	if ( n == "ROTX") out = SsAttributeKind::Rotx;
	if ( n == "ROTY") out = SsAttributeKind::Roty;
	if ( n == "ROTZ") out = SsAttributeKind::Rotz;
	if ( n == "SCLX") out = SsAttributeKind::Sclx;
	if ( n == "SCLY") out = SsAttributeKind::Scly;
	if ( n == "LSCX") out = SsAttributeKind::Losclx;
	if ( n == "LSCY") out = SsAttributeKind::Loscly;
	if ( n == "ALPH") out = SsAttributeKind::Alpha;
	if ( n == "LALP") out = SsAttributeKind::Loalpha;
	if ( n == "PRIO") out = SsAttributeKind::Prio;
	if ( n == "FLPH") out = SsAttributeKind::Fliph;
	if ( n == "FLPV") out = SsAttributeKind::Flipv;
	if ( n == "HIDE") out = SsAttributeKind::Hide;
	if ( n == "PCOL") out = SsAttributeKind::PartsColor;
	if ( n == "VCOL") out = SsAttributeKind::Color;
	if ( n == "VERT") out = SsAttributeKind::Vertex;
	if ( n == "PVTX") out = SsAttributeKind::Pivotx;
	if ( n == "PVTY") out = SsAttributeKind::Pivoty;
	if ( n == "ANCX") out = SsAttributeKind::Anchorx;
	if ( n == "ANCY") out = SsAttributeKind::Anchory;
	if ( n == "SIZX") out = SsAttributeKind::Sizex;
	if ( n == "SIZY") out = SsAttributeKind::Sizey;
	if ( n == "IFLH") out = SsAttributeKind::Imgfliph;
	if ( n == "IFLV") out = SsAttributeKind::Imgflipv;
	if ( n == "UVTX") out = SsAttributeKind::Uvtx;
	if ( n == "UVTY") out = SsAttributeKind::Uvty;
	if ( n == "UVRZ") out = SsAttributeKind::Uvrz;
	if ( n == "UVSX") out = SsAttributeKind::Uvsx;
	if ( n == "UVSY") out = SsAttributeKind::Uvsy;
	if ( n == "BNDR") out = SsAttributeKind::Boundr;
	if ( n == "USER") out = SsAttributeKind::User;
	if ( n == "IPRM") out = SsAttributeKind::Instance;
	if ( n == "EFCT") out = SsAttributeKind::Effect;
	if ( n == "MASK") out = SsAttributeKind::Mask;
	if ( n == "DEFM") out = SsAttributeKind::Deform;
}


//---------------------------------------------------------------
//相互変換 SsPartType
FString	__EnumToString_( TEnumAsByte<SsEffectNodeType::Type> n )
{
	if ( n == SsEffectNodeType::Invalid )	return "invalid";
	if ( n == SsEffectNodeType::Root )		return "root";
	if ( n == SsEffectNodeType::Emmiter )	return "emmiter";
	if ( n == SsEffectNodeType::Particle )	return "particle";

	return "invalid";	
}

void 	__StringToEnum_( FString n , TEnumAsByte<SsEffectNodeType::Type>& out)
{
	out =  SsEffectNodeType::Invalid;
	if ( n == "invalid")	out = SsEffectNodeType::Invalid;
	if ( n == "root")		out = SsEffectNodeType::Root;
	if ( n == "emmiter")	out = SsEffectNodeType::Emmiter;
	if ( n == "particle")	out = SsEffectNodeType::Particle;
}

//---------------------------------------------------------------
//相互変換 SsPartType
FString	__EnumToString_( TEnumAsByte<SsRenderBlendType::Type> n )
{
	if ( n == SsRenderBlendType::Invalid )	return "invalid";
	if ( n == SsRenderBlendType::Mix )		return "Mix";
	if ( n == SsRenderBlendType::Add )		return "Add";

	return "invalid";	
}

void 	__StringToEnum_( FString n , TEnumAsByte<SsRenderBlendType::Type>& out)
{
	out =  SsRenderBlendType::Invalid;
	if ( n == "invalid")	out = SsRenderBlendType::Invalid;
	if ( n == "Mix")		out = SsRenderBlendType::Mix;
	if ( n == "Add")		out = SsRenderBlendType::Add;
}

TEnumAsByte<SsBlendType::Type> SsRenderBlendTypeToBlendType(TEnumAsByte<SsRenderBlendType::Type> n)
{
	switch(n)
	{
	case SsRenderBlendType::Mix: return SsBlendType::Mix;
	case SsRenderBlendType::Add: return SsBlendType::Add;
	}
	return SsBlendType::Mix;
}

//---------------------------------------------------------------
//相互変換 SsIkRotationArrow
FString	__EnumToString_( TEnumAsByte<SsIkRotationArrow::Type> n )
{
	if ( n == SsIkRotationArrow::Arrowfree)		return "arrowfree";
	if ( n == SsIkRotationArrow::Clockwise)		return "clockwise";
	if ( n == SsIkRotationArrow::Anticlockwise)	return "anticlockwise";

	return "unknown";
}

void 	__StringToEnum_( FString n , TEnumAsByte<SsIkRotationArrow::Type>& out)
{
	out =  SsIkRotationArrow::Unknown;
	if ( n == "arrowfree")		out = SsIkRotationArrow::Arrowfree;
	if ( n == "clockwise")		out = SsIkRotationArrow::Clockwise;
	if ( n == "anticlockwise")	out = SsIkRotationArrow::Anticlockwise;
}

//---------------------------------------------------------------
//相互変換 SsSequenceType
FString	__EnumToString_( TEnumAsByte<SsSequenceType::Type> n )
{
	if ( n == SsSequenceType::Last)		return "LAST";
	if ( n == SsSequenceType::Keep)		return "KEEP";
	if ( n == SsSequenceType::Top)		return "TOP";

	return "invalid";
}

void 	__StringToEnum_( FString n , TEnumAsByte<SsSequenceType::Type>& out)
{
	out =  SsSequenceType::Invalid;
	if ( n == "LAST")	out = SsSequenceType::Last;
	if ( n == "KEEP")	out = SsSequenceType::Keep;
	if ( n == "TOP")	out = SsSequenceType::Top;
}

//---------------------------------------------------------------
//相互変換 SsMeshDivType
FString	__EnumToString_( TEnumAsByte<SsMeshDivType::Type> n )
{
	if ( n == SsMeshDivType::PolylineBase)	return "polyline_base";
	if ( n == SsMeshDivType::Boxdiv)		return "boxdiv";

	return "unknown";
}

void 	__StringToEnum_( FString n , TEnumAsByte<SsMeshDivType::Type>& out)
{
	out =  SsMeshDivType::Unknown;
	if ( n == "polyline_base")	out = SsMeshDivType::PolylineBase;
	if ( n == "boxdiv")			out = SsMeshDivType::Boxdiv;
}
