#sgdefine COMMONLIGHT_VERT_INC //Define setzen um Mehrfache inkludierung auszuschliessen

#sginclude "://shaders/common.vert.inc"
#sginclude "://shaders/commonlightmatdefs.inc"
#sginclude "://shaders/commonlightmodels.inc"

vec3 eyePosition()
{
    return (modelViewMatrix * vertexPosition).xyz;
}

vec3 eyeNormalVec()
{
    return normalize(normalMatrix * vertexNormal);
}

vec3 eyeLightVec(vec3 lightViewPosition)
{
    return lightViewPosition - eyePosition().xyz;
}

vec3 eyeViewVec()
{
    return -normalize(eyePosition().xyz);
}

vec3 eyeReflectVec(vec3 lightViewDirection)
{
    return -reflect(lightViewDirection, eyeNormalVec());
}

vec3 phong(vec3 lightViewDirection, vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular, vec2 lightAttenuations)
{
    return phong(lightViewDirection, eyeViewVec(), eyeNormalVec(), lightAmbient, lightDiffuse, lightSpecular, lightAttenuations, vec3(1.0));
}

vec3 lambert(vec3 viewPosition, vec3 viewDirection, vec3 viewNormal, vec3 textureColor)
{
    vec3 color = vec3(0.0);
    for (int i = 0; i < sunLightCount; ++i)
    {
        color += phongKdKaColorWithAttCalc(-sunLights[i].viewDirection, viewNormal, sunLights[i].ambient,
                       sunLights[i].diffuse, vec2(0.0), textureColor);
    }

    for (int i = 0; i < pointLightCount; ++i)
    {
        vec3 lightViewDirection = pointLights[i].viewPosition - viewPosition;
        color += phongKdKaColorWithAttCalc(lightViewDirection, viewNormal, pointLights[i].ambient,
                       pointLights[i].diffuse, pointLights[i].attenuations, textureColor);
    }

    for (int i = 0; i < spotLightCount; ++i)
    {
        vec3 lightViewDirection = spotLights[i].viewPosition - viewPosition;
        float clampedCosine = max(-dot(normalize(lightViewDirection), spotLights[i].viewDirection), 0.0);
        if (clampedCosine >= spotLights[i].falloffAngle)
        {
            // Fragment innerhalb Falloffradius vom Spotlicht => Wende Phong auf Fragment an
            float falloffExponent = pow(clampedCosine, spotLights[i].falloffExponent);
            color += phongKdKaColorWithAttCalc(lightViewDirection, viewNormal, spotLights[i].ambient,
                           spotLights[i].diffuse, spotLights[i].attenuations, textureColor) * falloffExponent;
        }
    }

    return color;
}
