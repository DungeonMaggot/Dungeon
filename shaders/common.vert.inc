uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;

in vec4 vertexPosition;
in vec3 vertexNormal;
in vec4 vertexColor;

vec4 vPos()
{
    return projectionMatrix * modelViewMatrix * vertexPosition;
}
