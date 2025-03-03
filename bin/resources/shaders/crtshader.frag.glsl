varying vec2 fragTexCoord;
varying vec4 fragColor;


// Input uniform values
uniform sampler2D texture0;
uniform float iTime;
float Pi = 3.141592653589793238462643383;

const float renderWidth = 1280.0;
const float renderHeight = 960.0;

#define DYNAMIC_CRT_RES 1
// DYNAMIC_CRT_RES changes the number of pixels based on viewport width. otherwise CRT resolution is fixed and pixels stretch
#define RIPPLING_INTERFERENCE 1
#define SCREEN_BULGE 1
#define VIGNETTE 1

#define PIXEL_TYPE 1
// PIXEL_TYPE changes pixel shape and orientation
// 0: square pixels (default)
// 1: vertical line pixels without staggering
// 2: vertical line pixels with staggering
// 3 hexagonal circular pixels


float ripple(float c, float time, float coord){
    return c * (.5 + ( sin((time + coord) * .125 * cos((time + coord) * .15)) * .5)) * 1.25;
    
}

vec3 rippleRgb(float r, float g, float b, float time, vec2 fragXY){
    r *= r * 1.25 - ripple(r, time / 4.2, fragXY.y + .3) * 1.3;
    g *= g * 1.25 - ripple(g, time / 2.4, fragXY.y + .6) * 1.3;
    b *= b * 1.25 - ripple(b, time / 5.6, fragXY.y + .2) * 1.3;
    return vec3(r, g, b);
}



float kindascanline2(float time, vec2 pos, vec2 frag, float screenh, float screenw){
    time *= .125;
    float rhfraction = screenh/Pi;
    float ygrad = pos.y + pos.y*0.01;
    return sin(1.*mod((.25*Pi) + pos.y/rhfraction, time*Pi))  + 0.9 *sin(mod(2.*Pi+time*Pi*8.,pos.y/rhfraction*1.));
}

vec3 circle(vec2 uv, vec2 p, vec3 col, float r, float blur, float ar){
    vec2 scaledUV = uv;
    float d = length(scaledUV - p);
    float c = smoothstep(r, r - blur, (d - sin(d*1.03)));
    return c * col;
}

vec2 screenbulge(vec2 centre, float warpfactor, float ar){
    float r = length(centre);
	float alpha = (r* -warpfactor);
    return ((alpha - sin(alpha)) * centre);
}

vec4 ripplingInterference(float time, vec2 centre, vec4 texcol, float speed, float intensity){
  
  float d = length(centre);
  float modc = sin((time*speed + centre.y*.50) - sin(time*speed + centre.y*.51));
  modc *= .8;
  return  vec4((2.*texcol.x) + mod(texcol.x,modc),(2.*texcol.y) + mod(texcol.y,modc),(2.*texcol.z) + mod(texcol.z,modc),(2.*texcol.w) + mod(texcol.z, modc));
}

float sinepxabs(float coord, float coordScale, float clampCeil, float clampFloor){
    float result = abs(sin((coord*Pi*2.)/coordScale)); //.5+(sinScale*sin(coord/coordScale));
    float clampRange = clampCeil - clampFloor;
    float newresult = ((clamp(result, clampFloor, clampCeil)) - clampFloor);
    newresult *= (result / clampRange);
    return newresult;
}

float sinepxmax(float coord, float coordScale, float clampCeil, float clampFloor){
    float result = max(sin((coord*Pi*2.)/coordScale), 0.); //.5+(sinScale*sin(coord/coordScale));
    float clampRange = clampCeil - clampFloor;
    float newresult = ((clamp(result, clampFloor, clampCeil)) - clampFloor);
    newresult *= (result / clampRange);
    return newresult;
}

float sinepxoffset(float coord, float coordScale, float clampCeil, float clampFloor){
    float result = .5+(sin((coord*Pi*2.)/coordScale)*.5); //.5+(sinScale*sin(coord/coordScale));
    float clampRange = clampCeil - clampFloor;
    float newresult = ((clamp(result, clampFloor, clampCeil)) - clampFloor);
    newresult *= (result / clampRange);
    return newresult;
}

float sinepxhw(float coord, float coordScale, float clampCeil, float clampFloor){
    float result = max(sin((coord*Pi*2.)/coordScale)-mod(coord, coord/4.),0.);
    float clampRange = clampCeil - clampFloor;
    float newresult = ((clamp(result, clampFloor, clampCeil)) - clampFloor);
    newresult *= (result / clampRange);
    return newresult;
}

float sinepxy(vec2 coord, vec2 offset, vec2 scale, vec4 clampRange){
#if (PIXEL_TYPE == 0)
    return sinepxmax(coord.x + offset.x, scale.x, clampRange.x, clampRange.y) * sinepxmax(coord.y + offset.y, scale.y, clampRange.z, clampRange.w);
#elif (PIXEL_TYPE == 999)
    return sinepxhw(coord.x + offset.x, scale.x, clampRange.x, clampRange.y) * sinepxabs(coord.y + offset.y, scale.y, clampRange.z, clampRange.w);
#else
    return sinepxmax(coord.x + offset.x, scale.x, clampRange.x, clampRange.y) * sinepxabs(coord.y + offset.y, scale.y, clampRange.z, clampRange.w);
#endif
}

vec3 squarePixel(vec2 fragXY, vec2 uv, vec2 ires, vec4 texcol, vec2 scaleXY, vec2 pxOffset, vec4 clampRange, float time){
    float asp = ires.x/ires.y;
    float r = texcol.x * sinepxy(fragXY, vec2(0.), scaleXY, vec4(clampRange));
    float g = texcol.y * sinepxy(fragXY, vec2(pxOffset.x/2., 0.), scaleXY, vec4(clampRange));
    float b = texcol.z * sinepxy(fragXY, vec2(pxOffset.x/2., pxOffset.y/2.), scaleXY, vec4(clampRange));
    return vec3(r,g,b);
}

vec3 verticalPixel(vec2 fragXY, vec2 uv, vec2 ires, vec4 texcol, vec2 scaleXY, vec2 pxOffset, vec4 clampRange, float time){
    float asp = ires.x/ires.y;
    float pixelThird = pxOffset.x/3.;
    float r = texcol.x * sinepxy(fragXY, vec2(0.), vec2(scaleXY.x, scaleXY.y*2.), clampRange);
    float g = texcol.y * sinepxy(fragXY, vec2(pxOffset.x + pixelThird, 0), vec2(scaleXY.x, scaleXY.y*2.), clampRange);
    float b = texcol.z * sinepxy(fragXY, vec2(pxOffset.x + (pixelThird * 2.), 0.), vec2(scaleXY.x, scaleXY.y*2.), clampRange);
    return vec3(r,g,b);
}

vec3 testPixel(vec2 fragXY, vec2 uv, vec2 ires, vec4 texcol, vec2 scaleXY, vec2 pxOffset, vec4 clampRange, float time){
    float asp = ires.x/ires.y;
    float pixelThird = pxOffset.x/3.;
    float r = texcol.x * sinepxy(fragXY, vec2(0.), vec2(scaleXY.x, scaleXY.y*2.), clampRange);
    float g = texcol.y * sinepxy(fragXY, vec2(pxOffset.x + pixelThird, 0), vec2(scaleXY.x, scaleXY.y*2.), clampRange);
    float b = texcol.z * sinepxy(fragXY, vec2(pxOffset.x + (pixelThird * 2.), 0.), vec2(scaleXY.x, scaleXY.y*2.), clampRange);
    return vec3(r,g,b);
}

void main()
{
    vec2 iResolution = vec2(renderWidth, renderHeight);
    vec2 uv = fragTexCoord;
    float ar = iResolution.x / iResolution.y; //aspect ratio    
    vec2 fragTexCopy = fragTexCoord.xy * iResolution.xy;
#if (DYNAMIC_CRT_RES == 1)
    float crtw = iResolution.x/8.;
    float crth = iResolution.y/8.;
#else
    float crtw = 5.;
    float crth = 5.;
#endif
    float pxPerCrtX = iResolution.x/crtw;
    float pxPerCrtY = iResolution.y/crth;
    
#if (SCREEN_BULGE == 1)
    vec2 screenCentre = iResolution.xy / 2.;
    fragTexCopy += screenbulge(screenCentre - fragTexCopy, 0.00135, ar);
#endif
    //vec2 crtthing = vec2(fragTexCopy.x - mod(fragTexCopy.x, pxPerCrtX * Pi * 2.),fragTexCopy.y - mod(fragTexCopy.y, pxPerCrtY * Pi * 2.)); //
    vec2 uvcrt = vec2(fragTexCopy.x - mod(fragTexCopy.x, pxPerCrtX),fragTexCopy.y - mod(fragTexCopy.y, pxPerCrtY)) / iResolution.xy;
    vec4 texcol = texture2D(texture0, uvcrt) *2.25;
    //texcol *= vec4(rippleRgb(texcol.x, texcol.y, texcol.z, iTime/.4, fragTexCopy),0.);
#if (RIPPLING_INTERFERENCE == 1)
    texcol =  ripplingInterference(iTime, screenCentre - fragTexCopy, texcol, 10., .25);
#endif
#if (VIGNETTE == 1)
    texcol *=  vec4(circle(uv, vec2(0.5,0.5), vec3(1,1,1), .024, .01, ar),1);
#endif
    
    vec3 crtMask;
#if (PIXEL_TYPE == 0)
    crtMask = .7 * squarePixel(fragTexCopy, uvcrt, iResolution.xy, texcol, vec2(pxPerCrtX, pxPerCrtY), vec2(pxPerCrtX, pxPerCrtY), vec4(.8,.2,.8,.2), iTime);
#elif (PIXEL_TYPE == 1)
    crtMask = 1.25 * verticalPixel(fragTexCopy, uvcrt, iResolution.xy, texcol, vec2(pxPerCrtX, pxPerCrtY), vec2(pxPerCrtX, pxPerCrtY), vec4(.8,.2,.8,.2), iTime);
#elif (PIXEL_TYPE == 999)
    crtMask = 1.25 * testPixel(fragTexCopy, uvcrt, iResolution.xy, texcol, vec2(pxPerCrtX, pxPerCrtY), vec2(pxPerCrtX, pxPerCrtY), vec4(.8,.2,.8,.2), iTime);
#endif
    gl_FragColor = vec4(crtMask, 1);
}













