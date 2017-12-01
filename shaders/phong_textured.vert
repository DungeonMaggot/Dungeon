#sginclude "://shaders/commonlight.vert.inc"

in vec2 textureCoords;

out vec2 texCoords;

out vec4 color;

void main()
{
    vec3 viewPosition = eyePosition().xyz;
    vec3 viewDirection = -normalize(viewPosition);
    vec3 vertexColor = phong(viewPosition, viewDirection, eyeNormalVec().xyz, vec3(1.0));
    color = vec4(vertexColor + material.emission.rgb, computeMaterialAlpha());

    gl_Position = projectionMatrix * modelViewMatrix * vertexPosition;
    texCoords = textureCoords;

    // gl_Position = vPos();
}
