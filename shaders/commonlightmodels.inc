#sginclude "://shaders/commonlightmatdefs.inc"

float phongKd(vec3 lightViewDirection, vec3 viewNormal)
{
    return max(dot(lightViewDirection, viewNormal), 0.0);
}

float phongKs(vec3 viewDirection, vec3 reflectDirection)
{
    return pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
}

float phongKa()
{
    return(1.0);
}

float computeLightAttenuation(float lightViewDistance, vec2 lightAttenuations)
{
    return 1.0 / (1.0 + lightAttenuations.x * lightViewDistance + lightAttenuations.y * lightViewDistance * lightViewDistance);
}

float computeMaterialAlpha()
{
    return material.emission.a + material.ambient.a + material.diffuse.a + material.specular.a;
}

vec3 phongKdKaColor(vec3 lightViewDirection, vec3 viewNormal, vec3 lightAmbient, vec3 lightDiffuse, float lightAttenuation, vec3 textureColor)
{
    vec3 diffuseColor = phongKd(lightViewDirection, viewNormal) * (lightDiffuse * material.diffuse.rgb);
    vec3 ambientColor = phongKa() * lightAmbient * material.ambient.rgb;
    vec3 color = (diffuseColor + ambientColor) * textureColor * lightAttenuation;

    return color;
}

vec3 phongKdKaColorWithAttCalc(vec3 unnormalizedLightViewDirection, vec3 viewNormal, vec3 lightAmbient, vec3 lightDiffuse, vec2 lightAttenuations, vec3 textureColor)
{
    float lightViewDistance = length(unnormalizedLightViewDirection);
    float lightAttenuation = computeLightAttenuation(lightViewDistance, lightAttenuations);
    vec3 lightViewDirection = unnormalizedLightViewDirection / lightViewDistance;
    return phongKdKaColor(lightViewDirection, viewNormal, lightAmbient, lightDiffuse, lightAttenuation, textureColor);
}

vec3 phongKsColor(vec3 lightViewDirection, vec3 viewDirection, vec3 viewNormal, vec3 lightSpecular, float lightAttenuation)
{
    return (phongKs(viewDirection, -reflect(lightViewDirection, viewNormal)) * lightAttenuation) * (lightSpecular * material.specular.rgb);
}

vec3 phong(vec3 unnormalizedLightViewDirection, vec3 viewDirection, vec3 viewNormal, vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular, vec2 lightAttenuations, vec3 textureColor)
{
    float lightViewDistance = length(unnormalizedLightViewDirection);
    float lightAttenuation = computeLightAttenuation(lightViewDistance, lightAttenuations);
    vec3 lightViewDirection = unnormalizedLightViewDirection / lightViewDistance;
    vec3 specularColor = phongKsColor(lightViewDirection, viewDirection, viewNormal, lightSpecular, lightAttenuation);

    return phongKdKaColor(lightViewDirection, viewNormal, lightAmbient, lightDiffuse, lightAttenuation, textureColor) + specularColor;
}

vec3 phong(vec3 viewPosition, vec3 viewDirection, vec3 viewNormal, vec3 textureColor)
{
    vec3 color = vec3(0.0);
    for (int i = 0; i < sunLightCount; ++i)
    {
        color += phong(-sunLights[i].viewDirection, viewDirection, viewNormal, sunLights[i].ambient,
                       sunLights[i].diffuse, sunLights[i].specular, vec2(0.0), textureColor).rgb;
    }

    for (int i = 0; i < pointLightCount; ++i)
    {
        vec3 lightViewDirection = pointLights[i].viewPosition - viewPosition;
        color += phong(lightViewDirection, viewDirection, viewNormal, pointLights[i].ambient,
                       pointLights[i].diffuse, pointLights[i].specular, pointLights[i].attenuations, textureColor).rgb;
    }

    for (int i = 0; i < spotLightCount; ++i)
    {
        vec3 lightViewDirection = spotLights[i].viewPosition - viewPosition;
        float clampedCosine = max(-dot(normalize(lightViewDirection), spotLights[i].viewDirection), 0.0);
        if (clampedCosine >= spotLights[i].falloffAngle)
        {
            // Fragment innerhalb Falloffradius vom Spotlicht => Wende Phong auf Fragment an
            float falloffExponent = pow(clampedCosine, spotLights[i].falloffExponent);
            color += phong(lightViewDirection, viewDirection, viewNormal, spotLights[i].ambient,
                           spotLights[i].diffuse, spotLights[i].specular, spotLights[i].attenuations, textureColor).rgb * falloffExponent;
        }
    }

    return color;
}

