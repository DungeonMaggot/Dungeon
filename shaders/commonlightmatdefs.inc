#sgdefine COMMONLIGHTMATDEFS_INC //Define setzen um Mehrfache inkludierung auszuschliessen

#define MAX_LIGHTS 8 //maximum number of lights in a scene

struct SunLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 viewDirection;
};

struct PointLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 viewPosition;
    vec2 attenuations; // x = linearAttenuation, y = quadraticAttenuation
};

struct SpotLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 viewPosition;
    vec3 viewDirection;
    vec2 attenuations; // x = linearAttenuation, y = quadraticAttenuation
    float falloffAngle;
    float falloffExponent;
};

struct Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 emission;
    float shininess;
};

layout (shared) uniform LightUniforms
{
    SunLight sunLights[MAX_LIGHTS];
    PointLight pointLights[MAX_LIGHTS];
    SpotLight spotLights[MAX_LIGHTS];
};

uniform int sunLightCount;
uniform int pointLightCount;
uniform int spotLightCount;

layout (shared) uniform MaterialUniforms
{
    Material material;
};
