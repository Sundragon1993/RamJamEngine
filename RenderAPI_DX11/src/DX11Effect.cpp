#include "DX11Effect.h"
#include "../../RamJamEngine/include/System.h"
#include "../../RamJamEngine/include/MaterialFactory.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::string& filename)
{
	mFX = nullptr;
	MaterialFactory::Instance()->RegisterShader(filename);
	RJE_CHECK_FOR_SUCCESS(D3DX11CreateEffectFromFile(filename.c_str(), 0, device, &mFX));
}

Effect::~Effect()
{
	RJE_SAFE_RELEASE(mFX);
}
#pragma endregion

//////////////////////////////////////////////////////////////////////////

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::string& filename)
	: Effect(device, filename)
{
	BasicTech         = mFX->GetTechniqueByName("Basic");
	ViewProj          = mFX->GetVariableByName("gViewProj")->AsMatrix();
	World             = mFX->GetVariableByName("gWorld")->AsMatrix();
	TexTransform      = mFX->GetVariableByName("gDiffuseMapTrf")->AsMatrix();
	EyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor          = mFX->GetVariableByName("gFogColor")->AsVector();
	FogEnabled        = mFX->GetVariableByName("gUseFog")->AsScalar();
	AlphaClipEnabled  = mFX->GetVariableByName("gUseAlphaClip")->AsScalar();
	TextureEnabled    = mFX->GetVariableByName("gUseTexture")->AsScalar();
	FogStart          = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange          = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights         = mFX->GetVariableByName("gDirLights")->AsShaderResource();
	PointLights       = mFX->GetVariableByName("gPointLights")->AsShaderResource();
	SpotLights        = mFX->GetVariableByName("gSpotLights")->AsShaderResource();
	TextureSampler    = (ID3DX11EffectSamplerVariable*) mFX->GetVariableByName("gTextureSampler");
}
//-----------------------
BasicEffect::~BasicEffect(){}
//-----------------------
HRESULT BasicEffect::SetMaterial(Material* mat)
{
	HRESULT res;
	for (u32 i = 0; i < mat->mPropertiesCount; ++i)
	{
		MaterialProperty& property = *(mat->mProperties[i]);
		switch (property.mType)
		{
		case MaterialPropertyType::Type_Int:
			{
				res = mFX->GetVariableBySemantic(property.mName.c_str())->AsScalar()->SetInt(*(reinterpret_cast<int*>(property.mData)));
				if (res != S_OK) return res;
				break;
			}
		case MaterialPropertyType::Type_Bool:
			{
				res = mFX->GetVariableBySemantic(property.mName.c_str())->AsScalar()->SetBool(*(reinterpret_cast<bool*>(property.mData)));
				if (res != S_OK) return res;
				break;
			}
		case MaterialPropertyType::Type_Float:
			{
				res = mFX->GetVariableBySemantic(property.mName.c_str())->AsScalar()->SetFloat(*(reinterpret_cast<float*>(property.mData)));
				if (res != S_OK) return res;
				break;
			}
		case MaterialPropertyType::Type_Vector:
			{
				res = mFX->GetVariableBySemantic(property.mName.c_str())->AsVector()->SetFloatVector(reinterpret_cast<const float*>(property.mData));
				if (res != S_OK) return res;
				break;
			}
		case MaterialPropertyType::Type_Matrix:
			{
				res = mFX->GetVariableBySemantic(property.mName.c_str())->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(property.mData));
				if (res != S_OK) return res;
				break;
			}
		case MaterialPropertyType::Type_Texture:
			{
				res = mFX->GetVariableBySemantic(property.mName.c_str())->AsShaderResource()->SetResource(property.mShaderResource.mTexture);
				if (res != S_OK) return res;
				std::string textureTrfName = property.mName + "_Trf";
				Matrix44 textTrf = Transform::MatrixFromTextureProperties(property.mShaderResource.mTiling, property.mShaderResource.mOffset, property.mShaderResource.mRotationAngleInDegrees);
				res = mFX->GetVariableBySemantic(textureTrfName.c_str())->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&textTrf));
				if (res != S_OK) return res;
				break;
			}
		case MaterialPropertyType::Type_Cubemap:
			{
				// TODO: RJE could use a cubemap :)
				break;
			}
		default:
			break;
		}
	}
	return res;
}
#pragma endregion

//////////////////////////////////////////////////////////////////////////

#pragma region SpriteEffect
SpriteEffect::SpriteEffect(ID3D11Device* device, const std::string& filename)
	: Effect(device, filename)
{
	SpriteTech = mFX->GetTechniqueByName("SpriteTech");
	SpriteMap  = mFX->GetVariableByName("gSpriteTex")->AsShaderResource();
}

SpriteEffect::~SpriteEffect(){}
#pragma endregion

//////////////////////////////////////////////////////////////////////////

#pragma region ColorEffect
ColorEffect::ColorEffect(ID3D11Device* device, const std::string& filename)
	: Effect(device, filename)
{
	ColorTech		= mFX->GetTechniqueByName("ColorTech");
	Color			= mFX->GetVariableByName("gColor")->AsVector();
	World			= mFX->GetVariableByName("gWorld")->AsMatrix();
	ViewProj		= mFX->GetVariableByName("gViewProj")->AsMatrix();
}

ColorEffect::~ColorEffect(){}
#pragma endregion

//////////////////////////////////////////////////////////////////////////

#pragma region Effects

BasicEffect*  DX11Effects::BasicFX  = nullptr;
ColorEffect*  DX11Effects::ColorFX  = nullptr;
SpriteEffect* DX11Effects::SpriteFX = nullptr;

void DX11Effects::InitAll(ID3D11Device* device)
{
	string shaderPath = System::Instance()->mDataPath;

	shaderPath += CIniFile::GetValue("basic",  "shaders", System::Instance()->mResourcesPath);
	BasicFX  = rje_new BasicEffect( device, shaderPath);
	//-------------
	shaderPath = System::Instance()->mDataPath + CIniFile::GetValue("sprite", "shaders", System::Instance()->mResourcesPath);
	SpriteFX = rje_new SpriteEffect(device, shaderPath);
	//-------------
	shaderPath = System::Instance()->mDataPath + CIniFile::GetValue("color", "shaders", System::Instance()->mResourcesPath);
	ColorFX = rje_new ColorEffect(device, shaderPath);
}

void DX11Effects::DestroyAll()
{
	RJE_SAFE_DELETE(BasicFX);
	RJE_SAFE_DELETE(SpriteFX);
	RJE_SAFE_DELETE(ColorFX);
}
#pragma endregion