#define GLOBAL_SET                     0
#define CAMERA_BUFFER_BINDING          0
#define SCENE_DATA_BINDING             1
#define SCENE_RENDER_SETTING_BINDING   2
#define GLOBAL_LIGHT_BINDING           3
#define POINT_LIGHT_BINDING            4
#define SPOT_LIGHT_BINDING             5

#define PER_INSTANCE_SET               1
#define MATERIAL_BINDING               0
#define DIFFUSE_TEXTURE_BINDING        1
#define SPECULAR_TEXTURE_BINDING       2
#define NORMAL_TEXTURE_BINDING         3
#define GLOBAL_SHADOW_MAP_BINDING      4
#define SPOT_SHADOW_MAP_BINDING        5
#define POINT_SHADOW_MAP_BINDING       6
#define SPOT_LIGHT_INDINCES_BINDING    7
#define POINT_LIGHT_INDINCES_BINDING   8
#define BONE_TRANSFORMS_BINDING        9

#define MAX_BONES_PER_INSTANCE 100
#define BONE_INFLUENCE 4
// #define USER_SET                       2
// #define POINT_LIGHT_INDINCES_BINDING   0
// #define SPOT_LIGHT_INDINCES_BINDING    1

struct SceneData
{
   uint GlobalLightCount;
   uint PointLightCount;
   uint SpotLightCount;
};

// Percentage closer filtering + poisson 
struct PCFP 
{
	uint Samples;
	float Smooth;
};

struct DirectLightShadowSettings
{
   uint	Kind;
	PCFP	PCFPSettings;
};

struct ShadowS
{
   DirectLightShadowSettings DLShadow;
};

struct RenderSettings
{
   bool DirectionalLightShadows;
   bool OmnidirectionalLightShadows;
   bool SpotLightShadows;

   bool LightCulling;
   bool SSAOEnabled;

   bool _DEBUG_ShowLightComplexity;
   bool _DEBUG_ShowShadowCascades;

   ShadowS ShadowSettings;
};