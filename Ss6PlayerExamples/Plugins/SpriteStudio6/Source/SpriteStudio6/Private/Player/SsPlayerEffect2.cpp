#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayerEffect2.h"

#include "SsCellMap.h"
#include "SsEffectFile.h"
#include "SsPlayerEffectFunction.h"
#include "SsPlayerPartState.h"
#include "SsPlayerMatrix.h"


#define SS_DEBUG_DISP (0)
#define SS_BUILD_ERROR_0418 (0)


namespace
{
	uint8 BlendNumber(uint8 a, uint8 b, float Rate)
	{
		return (a + (b - a) * Rate);
	}
	float BlendFloat(float a, float b, float Rate)
	{
		return (a +(b - a) * Rate);
	}

	double OutQuad(double t, double TotalTime, double InMax, double InMin)
	{
		if(TotalTime == 0.f){ return 0.f; }
		if(t > TotalTime){ t = TotalTime; }
		InMax -= InMin;
		t /= TotalTime;
		return -InMax*t*(t-2)+InMin;
	}

	float GetAngleUnit(const FVector2D& v0, const FVector2D& v1)
	{
		float ip = FVector2D::DotProduct(v0, v1);
		if (ip > 1.0f) { ip = 1.0f; }
		if (ip < -1.0f) { ip = -1.0f; }
		float f = FMath::Acos(ip);
		return f;
	}
	float GetAngle360Unit(const FVector2D& v0, const FVector2D& v1)
	{
		float ang = GetAngleUnit(v0, v1);
		float c = FVector2D::CrossProduct(v0, v1);

		if (c < 0)
		{
			ang = (3.1415926535897932385f)*2.0f - ang;
		}
		return ang;
	}
	float GetAngle360(const FVector2D& v0, const FVector2D& v1)
	{
		FVector2D uv0(v0), uv1(v1);
		uv0.Normalize();
		uv1.Normalize();
		return GetAngle360Unit(uv0, uv1);
	}
}

//現在時間から産出される位置を求める
//time変数から求められる式とする
//パーティクル座標計算のコア
void FSsEffectEmitter::UpdateParticle(float InTime, FParticleDrawData* ParticleDrawData, bool bRecalc/*=false*/)
{
	float _t = (float)(InTime - ParticleDrawData->STime);
	float _tm = (float)(_t - 1.0f );
	float _t2 = _t * _t; //(経過時間の二乗)
	float _life = (float)(ParticleDrawData->Lifetime - ParticleDrawData->STime);

	if(_life == 0)
	{
		return;
	}
	float _lifeper = (float)(_t / _life);


	//_t = 0時点の値を作る
	//シード値で固定化されることが前提
	uint32 pseed = SeedList[ParticleDrawData->Id % SeedTableLen];


	//自身のシード値、エミッターのシード値、親パーティクルのＩＤをシード値とする
	Rand.init_genrand((pseed + EmitterSeed + ParticleDrawData->PId + SeedOffset));


	float rad = Particle.Angle + (Rand.genrand_float32() * (Particle.AngleVariance ) - Particle.AngleVariance/2.0f);
	float speed = Particle.Speed + (Particle.Speed2 * Rand.genrand_float32());


	//接線加速度
	float addr = 0.f;
	if(Particle.bUseTanAccel)
	{
		float accel = Particle.TangentialAccel + (Rand.genrand_float32() * Particle.TangentialAccel2);

		float _speed = speed;
		if(_speed <= 0){ _speed = 0.1f; }
		//平均角速度を求める
		float l = _life * _speed * 0.2f; //円の半径
		float c = 3.14 * l;

		//最円周 / 加速度(pixel)
		addr = (accel / c) * _t;
	}

	float x = FMath::Cos(rad + addr) * speed * (float)_t;
	float y = FMath::Sin(rad + addr) * speed * (float)_t;

	if(Particle.bUseTransSpeed)
	{
		float transspeed = Particle.TransSpeed + (Particle.TransSpeed2 * Rand.genrand_float32());
		float speedadd = transspeed / _life;

		float addtx = FMath::Cos(rad + addr) * speed;
		float addtx_trans = FMath::Cos(rad + addr) * speedadd;

		float addx = ((addtx_trans * _t) + addtx) * (_t+1.0f) / 2.0f;


		float addty = FMath::Sin(rad + addr) * speed;
		float addty_trans = FMath::Sin(rad + addr) * speedadd;

		float addy = ((addty_trans * _t) + addty) * (_t+1.0f) / 2.0f;

		x = addx;
		y = addy;
	}


	//重力加速度の計算
	if(Particle.bUseGravity)
	{
		x += (0.5 * Particle.Gravity.X * (_t2));
		y += (0.5 * Particle.Gravity.Y * (_t2));
	}

	//初期位置オフセット
	float ox,oy;
	ox = oy = 0;
	if(Particle.bUseOffset)
	{
		ox = (Particle.Offset.X + (Particle.Offset2.X * Rand.genrand_float32()));
		oy = (Particle.Offset.Y + (Particle.Offset2.Y * Rand.genrand_float32()));
	}

	//角度初期値
	ParticleDrawData->Rot = 0;
	if(Particle.bUseRotation)
	{
		ParticleDrawData->Rot = Particle.Rotation + (Rand.genrand_float32() * Particle.Rotation2);
		float add = Particle.RotationAdd + (Rand.genrand_float32() * Particle.RotationAdd2);

		//角度変化
		if(Particle.bUseRotationTrans)
		{
			//到達までの絶対時間
			float lastt = _life * Particle.EndLifeTimePer;

			float addf = 0;
			if(lastt == 0)
			{
				float addrf = (add * Particle.RotationFactor) * _t;
				ParticleDrawData->Rot += addrf;
			}
			else
			{
				//1フレームで加算される量
				addf = (add * Particle.RotationFactor - add) / lastt;

				//あまり時間
				float mod_t = _t - lastt;
				if(mod_t < 0){ mod_t = 0; }

				//現在時間（最終時間でリミット
				float nowt = _t;
				if(nowt > lastt){ nowt = lastt; }

				//最終項 + 初項 x F / 2
				float final_soul = add + addf * nowt;
				float addrf = (final_soul + add) * (nowt+1.0f) / 2.0f;
				addrf -= add;
				addrf+= (mod_t * final_soul); //あまりと終項の積を加算
				ParticleDrawData->Rot+=addrf;
			}
		}
		else
		{
			ParticleDrawData->Rot+= (add*_t);
		}
	}

	//カラーの初期値、カラーの変化
	ParticleDrawData->Color.A = 0xff;
	ParticleDrawData->Color.R = 0xff;
	ParticleDrawData->Color.G = 0xff;
	ParticleDrawData->Color.B = 0xff;

	if(Particle.bUseColor)
	{
		ParticleDrawData->Color.A = Particle.InitColor.A + (Rand.genrand_float32() * Particle.InitColor2.A);
		ParticleDrawData->Color.R = Particle.InitColor.R + (Rand.genrand_float32() * Particle.InitColor2.R);
		ParticleDrawData->Color.G = Particle.InitColor.G + (Rand.genrand_float32() * Particle.InitColor2.G);
		ParticleDrawData->Color.B = Particle.InitColor.B + (Rand.genrand_float32() * Particle.InitColor2.B);
	}
	if(Particle.bUseTransColor)
	{
		FSsU8Color ecolor;
		ecolor.A = Particle.TransColor.A + (Rand.genrand_float32() * Particle.TransColor2.A);
		ecolor.R = Particle.TransColor.R + (Rand.genrand_float32() * Particle.TransColor2.R);
		ecolor.G = Particle.TransColor.G + (Rand.genrand_float32() * Particle.TransColor2.G);
		ecolor.B = Particle.TransColor.B + (Rand.genrand_float32() * Particle.TransColor2.B);

		ParticleDrawData->Color.A = BlendNumber(ParticleDrawData->Color.A, ecolor.A, _lifeper);
		ParticleDrawData->Color.R = BlendNumber(ParticleDrawData->Color.R, ecolor.R, _lifeper);
		ParticleDrawData->Color.G = BlendNumber(ParticleDrawData->Color.G, ecolor.G, _lifeper);
		ParticleDrawData->Color.B = BlendNumber(ParticleDrawData->Color.B, ecolor.B, _lifeper);
	}
	if(Particle.bUseAlphaFade)
	{
		float start = Particle.AlphaFade;
		float end = Particle.AlphaFade2;
		float per = _lifeper * 100.0f;

		if(per < start)
		{
			float alpha = (start - per) / start;
			ParticleDrawData->Color.A *= 1.0f - alpha;
		}
		else
		{
			if(per > end)
			{
				if(end>=100.0f)
				{
					ParticleDrawData->Color.A = 0;
				}
				else
				{
					float alpha = (per-end) / (100.0f-end);
					if(alpha >=1.0f){ alpha = 1.0f; }
					ParticleDrawData->Color.A *= 1.0f - alpha;
				}
			}
		}
	}

	//スケーリング
	ParticleDrawData->Scale.X = 1.0f;
	ParticleDrawData->Scale.Y = 1.0f;
	float scalefactor = 1.0f;
	if(Particle.bUseInitScale)
	{
		ParticleDrawData->Scale.X = Particle.Scale.X + (Rand.genrand_float32() * Particle.ScaleRange.X);
		ParticleDrawData->Scale.Y = Particle.Scale.Y + (Rand.genrand_float32() * Particle.ScaleRange.Y);

		scalefactor = Particle.ScaleFactor + (Rand.genrand_float32() * Particle.ScaleFactor2);
	}

	if(Particle.bUseTransScale)
	{
		FVector2D s2;
		float sf2;
		s2.X = Particle.Transscale.X + (Rand.genrand_float32() * Particle.TransscaleRange.X);
		s2.Y = Particle.Transscale.Y + (Rand.genrand_float32() * Particle.TransscaleRange.Y);

		sf2 = Particle.TransscaleFactor + (Rand.genrand_float32() * Particle.TransscaleFactor2);

		ParticleDrawData->Scale.X = BlendFloat(ParticleDrawData->Scale.X, s2.X, _lifeper);
		ParticleDrawData->Scale.Y = BlendFloat(ParticleDrawData->Scale.Y, s2.Y, _lifeper);
		scalefactor = BlendFloat(scalefactor, sf2, _lifeper);
	}

	ParticleDrawData->Scale.X *= scalefactor;
	ParticleDrawData->Scale.Y *= scalefactor;

	ParticleDrawData->X = x + ox + Position.X;//エミッタからのオフセットを加算
	ParticleDrawData->Y = y + oy + Position.Y;//エミッタからのオフセットを加算


	//指定の点へよせる
	if(Particle.bUsePGravity)
	{
		//生成地点からの距離 
		FVector2D v(
			Particle.GravityPos.X - (ox + Position.X),
			Particle.GravityPos.Y - (oy + Position.Y)
			);

		FVector2D nv(v);
		nv.Normalize();

		float gp = Particle.GravityPower;
		if(gp > 0)
		{
			FVector2D v2(ParticleDrawData->X, ParticleDrawData->Y);
			float len = FVector2D::Distance(FVector2D::ZeroVector, v);  //生成位置からの距離 
			float et = len / gp * 0.90f;
			float _gt = _t;
			if(_gt >= (int)et)
			{
				_gt = et * 0.90f;
			}

			nv = nv * gp * _gt;
			ParticleDrawData->X += nv.X;
			ParticleDrawData->Y += nv.Y;

			float blend = OutQuad(_gt, et, 0.90f, 0.f);
			blend += (_t / _life * 0.1f);

			ParticleDrawData->X = BlendFloat(ParticleDrawData->X, Particle.GravityPos.X, blend);
			ParticleDrawData->Y = BlendFloat(ParticleDrawData->Y, Particle.GravityPos.Y, blend);
		}
		else
		{
			nv = nv * gp * _t;

			//パワーマイナスの場合は単純に反発させる
			//距離による減衰はない
			ParticleDrawData->X += nv.X;
			ParticleDrawData->Y += nv.Y;
		}
	}

	//前のフレームからの方向を取る
	ParticleDrawData->Direc = 0.0f;
	if(Particle.bUseTurnDirec && !bRecalc)
	{
		FParticleDrawData dp;
		dp = *ParticleDrawData;

		UpdateParticle(InTime + 1.0f , &dp , true);
		ParticleDrawData->Direc = GetAngle360(
				FVector2D(1 , 0),
				FVector2D(ParticleDrawData->X - dp.X, ParticleDrawData->Y - dp.Y)
				) + FMath::DegreesToRadians(90.f) + FMath::DegreesToRadians(Particle.DirecRotAdd);
	}
}


void FSsEffectEmitter::Precalculate2()
{
	Rand.init_genrand(EmitterSeed);

	EmitPattern.Empty();
	OffsetPattern.Empty();

	if(ParticleExistList == nullptr)
	{
		ParticleExistList = new FParticleExistSt[Emitter.Emitmax]; //存在しているパーティクルが入る計算用バッファ
	}

	memset(ParticleExistList, 0, sizeof(FParticleExistSt) * Emitter.Emitmax);


	if(Emitter.Emitnum < 1){ Emitter.Emitnum = 1; }

	int32 cycle = (int32)(((float)(Emitter.Emitmax * Emitter.Interval) / (float)Emitter.Emitnum) + 0.5f);
	int32 group = Emitter.Emitmax / Emitter.Emitnum;

	int32 extendsize = Emitter.Emitmax * SS_EFFECT_LIFE_EXTEND_SCALE;
	if(extendsize < SS_EFFECT_LIFE_EXTEND_MIN)
	{
		extendsize = SS_EFFECT_LIFE_EXTEND_MIN;
	}

	int32 shot = 0;
	int32 offset = Particle.Delay;
	for(int32 i = 0; i < Emitter.Emitmax; i++)
	{
		if(shot >= Emitter.Emitnum)
		{
			shot = 0;
			offset+= Emitter.Interval;
		}
		OffsetPattern.Add(offset);
		shot++;
	}
	for(int32 i = 0; i < extendsize; i++)
	{
		FEmitPattern e;
		e.Uid = i;
		e.Life = Emitter.ParticleLife + Emitter.ParticleLife2 * Rand.genrand_float32();
		e.Cycle = cycle;

		if(e.Life > cycle)
		{
			e.Cycle = e.Life;
		}
		EmitPattern.Add(e);
	}

	if(SeedList != nullptr)
	{
		delete[] SeedList;
	}
	ParticleListBufferSize = Emitter.Emitmax;

	Rand.init_genrand(EmitterSeed);
	
	SeedTableLen = ParticleListBufferSize * 3;
	SeedList = new uint32[SeedTableLen];

	//各パーティクルＩＤから参照するシード値をテーブルとして作成する
	for(int32 i = 0; i < SeedTableLen; ++i)
	{
		SeedList[i] = Rand.genrand_uint32();
	}
}


//----------------------------------------------------------------------------------


void FSsEffectEmitter::UpdateEmitter(double Time, int32 Slide)
{
	int onum = OffsetPattern.Num();
	int pnum = EmitPattern.Num();
	Slide = Slide * SS_EFFECT_SEED_MAGIC;

	for(int32 i = 0; i < onum; ++i)
	{
		int32 slide_num = (i + Slide) % pnum;
		FEmitPattern* targetEP = &EmitPattern[slide_num];

		int32 t = (int32)(Time - OffsetPattern[i]);
		ParticleExistList[i].Exist = false;
		ParticleExistList[i].Born = false;


		if(targetEP->Cycle != 0)
		{
			int32 loopnum = t / targetEP->Cycle;
			int32 cycle_top = loopnum * targetEP->Cycle;

			ParticleExistList[i].Cycle = loopnum;

			ParticleExistList[i].STime = cycle_top + OffsetPattern[i];
			ParticleExistList[i].EndTime = ParticleExistList[i].STime + targetEP->Life;

			if((double)ParticleExistList[i].STime <= Time &&  (double)ParticleExistList[i].EndTime > Time)
			{
				ParticleExistList[i].Exist = true;
				ParticleExistList[i].Born = true;
			}

			if(!this->Emitter.bInfinite)
			{
				if(ParticleExistList[i].STime >= this->Emitter.Life) //エミッターが終了している
				{
					ParticleExistList[i].Exist = false;    //作られてない

					//最終的な値に計算し直し <-事前計算しておくといいかも・
					int32 t2 = this->Emitter.Life - OffsetPattern[i];
					int loopnum2 = t2 / targetEP->Cycle;

					int cycle_top2 = loopnum2 * targetEP->Cycle;

					ParticleExistList[i].STime = cycle_top2 + OffsetPattern[i];

					ParticleExistList[i].EndTime = ParticleExistList[i].STime + targetEP->Life;
					ParticleExistList[i].Born = false;
				}
				else
				{
					ParticleExistList[i].Born = true;
				}
			}

			if(t < 0)
			{
				ParticleExistList[i].Exist = false;
				ParticleExistList[i].Born = false;
			}
		}
	}
}


const FParticleExistSt* FSsEffectEmitter::GetParticleDataFromID(int32 Id)
{
	return &ParticleExistList[Id];
}


//パラメータをコピーする
void FSsEffectRenderV2::InitEmitter(FSsEffectEmitter* Emitter, FSsEffectNode* Node)
{
	Emitter->RefData = Node->GetMyBehavior();
	Emitter->RefCell = Emitter->RefData->RefCell;

	//セルの初期化
	FSsCelMapLinker* link = this->CurCellMapManager->GetCellMapLink(Emitter->RefData->CellMapName);

	if(link)
	{
		FSsCell* cell = link->FindCell(Emitter->RefData->CellName);
		GetCellValue(
			this->CurCellMapManager,
			Emitter->RefData->CellMapName,
			Emitter->RefData->CellName,
			Emitter->DispCell
			);
	}
	else
	{
		UE_LOG(LogSpriteStudio, Warning,
			TEXT("cell not found : %s , %s"),
			*(Emitter->RefData->CellMapName.ToString()),
			*(Emitter->RefData->CellName.ToString())
			);
	}

	FSsEffectFunctionExecuter::InitializeEffect(Emitter->RefData, Emitter);

	Emitter->EmitterSeed = this->MySeed;

	if(Emitter->Particle.bUserOverrideRSeed)
	{
		Emitter->EmitterSeed = Emitter->Particle.OverrideRSeed;
	}
	else
	{
		if(this->EffectData->IsLockRandSeed)
		{
			Emitter->EmitterSeed = (this->EffectData->LockRandSeed+1) * SS_EFFECT_SEED_MAGIC;
		}
	}

	Emitter->Emitter.Life += Emitter->Particle.Delay;//ディレイ分加算
}


void FSsEffectRenderV2::ClearEmitterList()
{
	for(size_t i = 0; i < this->EmitterList.Num(); i++)
	{
		delete EmitterList[i];
	}

	EmitterList.Empty();
	UpdateList.Empty();
}


void FSsEffectRenderV2::SetEffectData(FSsEffectModel* Data)
{
	EffectData = Data;
	Reload();
}


void FSsEffectRenderV2::Update()
{
	if(!bIsPlay){ return; }

	TargetFrame = NowFrame;

	if(!this->bInfinite)
	{
		if(this->IsLoop()) //自動ループの場合
		{
			if(NowFrame > EffectTimeLength)
			{
				TargetFrame = (int32)((int32)NowFrame % EffectTimeLength);
				int l = (NowFrame / EffectTimeLength);
				SetSeedOffset(l);
			}
		}
	}
}

void FSsEffectRenderV2::Reload()
{
	NowFrame = 0;

	//updateが必要か
	Stop();
	ClearEmitterList();

	FSsEffectNode* root = this->EffectData->Root;

	TArray<FSsEffectNode>& list = this->EffectData->NodeList;

	LayoutScale.X = (float)(this->EffectData->LayoutScaleX) / 100.0f;
	LayoutScale.Y = (float)(this->EffectData->LayoutScaleY) / 100.0f;

	TArray<int> cnum;
	cnum.AddZeroed(list.Num());

	bool _Infinite = false;
	//パラメータを取得 
	//以前のデータ形式から変換 
	for(int32 i = 0; i < list.Num(); ++i)
	{
		FSsEffectNode& node = list[i];

		if(node.GetType() == SsEffectNodeType::Emmiter)
		{
			FSsEffectEmitter* e = new FSsEffectEmitter();
			//パラメータをコピー 

			e->ParentIndex = node.ParentIndex;
			//繋ぎ先は恐らくパーティクルなのでエミッタに変換 
			if(e->ParentIndex != 0)
			{
				e->ParentIndex = list[e->ParentIndex].ParentIndex;
			}

			cnum[e->ParentIndex]++;
			if(cnum[e->ParentIndex] > 10)
			{
				bIsWarningData = true;
				continue; //子１０ノード表示制限 
			}

			//孫抑制対策 
			if(e->ParentIndex != 0)
			{
				int a = list[e->ParentIndex].ParentIndex;
				if(a != 0)
				{
					if(list[a].ParentIndex > 0)
					{
						bIsWarningData = true;
						continue;
					}
				}
			}


			InitEmitter(e, &node);

			this->EmitterList.Add(e);

			if(e->Emitter.bInfinite){ _Infinite = true; }
		}
		else
		{
			//エミッター同士を繋ぎたいので
			this->EmitterList.Add(nullptr);
		}
	}

	cnum.Empty();
	bInfinite = _Infinite;

	//親子関係整理

	EffectTimeLength = 0;
	//事前計算計算  updateListにルートの子を配置し親子関係を結ぶ
	for(int32 i = 0; i < this->EmitterList.Num(); ++i)
	{
		if(EmitterList[i] != nullptr)
		{
			EmitterList[i]->Uid = i;
			EmitterList[i]->Precalculate2(); //ループ対応形式

			int32 parent_index = EmitterList[i]->ParentIndex;

			if(EmitterList[i]->ParentIndex == 0)  //ルート直下 
			{
				EmitterList[i]->Parent = 0;
				EmitterList[i]->GlobalTime = EmitterList[i]->GetTimeLength();
				UpdateList.Add(EmitterList[i]);
			}
			else
			{
				void* t = this->EmitterList[parent_index];
				EmitterList[i]->Parent = EmitterList[parent_index];

				EmitterList[i]->GlobalTime = EmitterList[i]->GetTimeLength() + this->EmitterList[parent_index]->GetTimeLength();

				UpdateList.Add(EmitterList[i]);
			}

			if(EmitterList[i]->GlobalTime > EffectTimeLength)
			{
				EffectTimeLength = EmitterList[i]->GlobalTime;
			}
		}
	}

	//プライオリティソート
	UpdateList.Sort();
}

void FSsEffectRenderV2::CreateRenderPart(

	TArray<FSsRenderPart>& OutRenderParts,
	FSsPartState* State,
	const FVector2D& CurAnimeCanvasSize,
	const FVector2D& CurAnimePivot,

	FSsEffectEmitter* Emitter,
	float Time,
	FSsEffectEmitter* Parent/*=nullptr*/,
	FParticleDrawData* DrawData/*=nullptr*/
	)
{
	//MEMO:
	//SSSDK SsEffectRenderV2::particleDraw 相当 

	if(Emitter == nullptr)
	{
		return;
	}

	int pnum = Emitter->GetParticleIDMax();

	int slide = (Parent == nullptr) ? 0 : DrawData->Id;

	Emitter->UpdateEmitter(Time, slide);

	for(int32 id = 0; id < pnum; ++id)
	{
		const FParticleExistSt* drawe = Emitter->GetParticleDataFromID(id);

		if(!drawe->Born)
		{
			continue;
		}

		float targettime = (Time + 0.f);
		FParticleDrawData lp;
		FParticleDrawData pp;
		pp.X = 0; pp.Y = 0;

		lp.Id = id + drawe->Cycle;
		lp.STime = drawe->STime;
		lp.Lifetime = drawe->EndTime;
		lp.PId = 0;

		if(Parent)
		{
			lp.PId = DrawData->Id;
		}

		if(drawe->Exist)
		{
			if(Parent)
			{
				//親から描画するパーティクルの初期位置を調べる
				pp.Id = DrawData->Id;
				pp.STime = DrawData->STime;
				pp.Lifetime = DrawData->Lifetime;
				pp.PId = DrawData->PId;
				//パーティクルが発生した時間の親の位置を取る

				int ptime = lp.STime + pp.STime;
				if (ptime > lp.Lifetime) { ptime = lp.Lifetime; }

				//逆算はデバッグしずらいかもしれない
				Parent->UpdateParticle(lp.STime + pp.STime, &pp);
				Emitter->Position.X = pp.X;
				Emitter->Position.Y = pp.Y;
			}

			Emitter->UpdateParticle(targettime, &lp);

			SsFColor fcolor;
			fcolor.FromARGB(lp.Color.ToARGB());

			//MEMO:
			//drawSpriteココから
			FSsRenderPart RenderPart;
			RenderPart.PartIndex = State->Index;
			RenderPart.Texture = Emitter->DispCell.Texture;
			RenderPart.AlphaBlendType = SsRenderBlendTypeToBlendType(Emitter->RefData->BlendType);
			RenderPart.ColorBlendType = SsBlendType::Effect;
			
			//MEMO:
			//FSsPlayerEffect::CreateRenderVertices 相当 
			{
				float matrix[4 * 4];
				IdentityMatrix(matrix);

				float parentAlpha = 1.f;
				if(ParentState)
				{
					memcpy(matrix, ParentState->Matrix, sizeof(float)*16);
					parentAlpha = ParentState->Alpha;
				}

				TranslationMatrixM(matrix, lp.X * LayoutScale.X, lp.Y * LayoutScale.Y, 0.f);
				RotationXYZMatrixM(matrix, 0.f, 0.f, FMath::DegreesToRadians(lp.Rot) + lp.Direc);
				ScaleMatrixM(matrix, lp.Scale.X, lp.Scale.Y, 1.f);

				fcolor.A *= parentAlpha;

				if(Emitter->DispCell.Cell && (0.f < fcolor.A))
				{
					FVector2D pivot = Emitter->DispCell.Cell->Pivot;
					pivot.X *= Emitter->DispCell.Cell->Size.X;
					pivot.Y *= Emitter->DispCell.Cell->Size.Y;

					FVector2D dispscale = Emitter->DispCell.Cell->Size;


					// RenderTargetに対する描画基準位置
					float OffX = (float)(CurAnimeCanvasSize.X / 2) + pivot.X + CurAnimePivot.X * CurAnimeCanvasSize.X;
					float OffY = (float)(CurAnimeCanvasSize.Y / 2) + pivot.Y - CurAnimePivot.Y * CurAnimeCanvasSize.Y;

					// 頂点座標
					FMatrix ViewMatrix(
						FVector(matrix[0], matrix[1], matrix[2]),
						FVector(matrix[4], matrix[5], matrix[6]),
						FVector(matrix[8], matrix[9], matrix[10]),
						FVector(matrix[12], matrix[13], matrix[14])
						);
					FVector Vertices[4] =
					{
						FVector(-(dispscale.X / 2.f),  (dispscale.Y / 2.f), 0.f),
						FVector((dispscale.X / 2.f),  (dispscale.Y / 2.f), 0.f),
						FVector(-(dispscale.X / 2.f), -(dispscale.Y / 2.f), 0.f),
						FVector((dispscale.X / 2.f), -(dispscale.Y / 2.f), 0.f),
					};

					for (int32 i = 0; i < 4; ++i)
					{
						FVector4 V = ViewMatrix.TransformPosition(Vertices[i]);
						RenderPart.Vertices[i].Position.X = (V.X + OffX) / CurAnimeCanvasSize.X;
						RenderPart.Vertices[i].Position.Y = (-V.Y + OffY) / CurAnimeCanvasSize.Y;

						RenderPart.Vertices[i].TexCoord = Emitter->DispCell.Uvs[i];
						RenderPart.Vertices[i].Color = FColor(lp.Color.R, lp.Color.G, lp.Color.B, (uint8)(lp.Color.A * parentAlpha));
						RenderPart.Vertices[i].ColorBlendRate = (Emitter->Particle.bUseColor || Emitter->Particle.bUseTransColor) ? 1.f : 0.f;
					}

					OutRenderParts.Add(RenderPart);
				}
			}

		}
	}
}

void FSsEffectRenderV2::CreateRenderParts(TArray<FSsRenderPart>& OutRenderParts, FSsPartState* State, const FVector2D& CurAnimeCanvasSize, const FVector2D& CurAnimePivot)
{
	if(NowFrame < 0)
	{
		return;
	}

	for(int32 i = 0; i < UpdateList.Num(); ++i)
	{
		FSsEffectEmitter* e = UpdateList[i];
		if(e)
		{
			e->SetSeedOffset(SeedOffset);
		}
	}
	for (int32 i = 0; i < UpdateList.Num(); ++i)
	{
		FSsEffectEmitter* e = UpdateList[i];
		if(e && e->Parent)
		{
			//グローバルの時間で現在親がどれだけ生成されているのかをチェックする
			e->Parent->UpdateEmitter(TargetFrame, 0);

			int32 loopnum = e->Parent->GetParticleIDMax();
			for(int32 n = 0; n < loopnum; ++n)
			{
				const FParticleExistSt* drawe = e->Parent->GetParticleDataFromID(n);
				if(drawe->Born)
				{
					FParticleDrawData lp;
					lp.STime = drawe->STime;
					lp.Lifetime = drawe->EndTime;
					lp.Id = n;
					lp.PId = 0;

					float targettime = (TargetFrame + 0.f);
					float ptime = (targettime - lp.STime);

					CreateRenderPart(
						OutRenderParts,
						State,
						CurAnimeCanvasSize,
						CurAnimePivot,
						e,
						ptime,
						e->Parent,
						&lp
						);
				}
			}
		}
		else
		{
			CreateRenderPart(
				OutRenderParts,
				State,
				CurAnimeCanvasSize,
				CurAnimePivot,
				e,
				TargetFrame
				);
		}
	}
}

int32 FSsEffectRenderV2::GetCurrentFPS()
{
	if(EffectData)
	{
		if(EffectData->FPS == 0){ return 30; }

		return EffectData->FPS;
	}
	return 30;
}

void FSsEffectRenderV2::SetSeedOffset(int32 Offset)
{
	if(EffectData->IsLockRandSeed)
	{
		SeedOffset = 0;
	}
	else
	{
		SeedOffset = Offset;
	}
}
