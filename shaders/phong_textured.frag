uniform float hasTexture=0.0f;
uniform sampler2D textureMap;

in vec2 texCoords;
in vec4 color; // phong
out vec4 fragColor;

vec4 colorFromTex;

void main()
{
    if(hasTexture > 0.5)
    {
        vec4 textureFrag = texture(textureMap, texCoords);
        colorFromTex = vec4(textureFrag.rgb, textureFrag.a);
    }

       //fragColor = (color + colorFromTex)*0.5;
       fragColor.r = colorFromTex.r * color.r;
       fragColor.g = colorFromTex.g * color.g;
       fragColor.b = colorFromTex.b * color.b;
       fragColor.a = colorFromTex.a * color.a;

//    fragColor = vec4(texCoords.x,texCoords.y,0.,1.);
}
