//#define DONUT_DEBUG_MODE
//#define COLOR_DEBUG_MODE
//#define ANDROID_SHADER_EDITOR
//#define QUANT_2_BIT

#define PI 3.141592653589793
#define TWO_PI 6.28318530718

#ifdef ANDROID_SHADER_EDITOR
uniform vec2 resolution;
uniform float time;
uniform int pointerCount;
out vec4 fragColor;
#endif

const float s_iTime_div = 0.75;
const float t_mod_div = PI * 8.0;
const float f_iTime_div = 60.0;
const float half_dither_res = 32.0;
const float noise_res = 128.0;

vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }
float fround(float x) { return floor(x + 0.5); }
vec2 v2round(vec2 x) { return floor(x + 0.5); }
vec3 v3round(vec3 x) { return floor(x + 0.5); }

float appxsin(float _x) {
    _x = fract(_x / 6.283185) * 6.283185;
    return 4.0 * _x * (3.141592 - _x) / (9.869604 - _x * (3.141592 - _x));
}

float appxcos(float _x) {
    _x = fract(_x / 6.283185 + 0.25) * 6.283185;
    return 4.0 * _x * (3.141592 - _x) / (9.869604 - _x * (3.141592 - _x));
}

float random (in vec2 _st) {
    return fract(sin(dot(_st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}


vec3 hsb2rgb( in vec3 c ){

    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),

                             6.0)-3.0)-1.0,

                     0.0,

                     1.0 );

    rgb = rgb*rgb*(3.0-2.0*rgb);

    return c.z * mix( vec3(1.0), rgb, c.y);

}

float f(float x)
{
        if (x >= 0.0031308){
                return (1.055) * pow(x,(1.0/2.4)) - 0.055;
        }else{
                return 12.92 * x;
        }
}


vec3 oklab_to_rgb(vec3 c)
{
        if(c.z>.4){
                return vec3(1.,0,.5);
        }
        if(c.z<-.4){
                return vec3(1.,0,0);
        }
        c.x=clamp(c.x,.0,1.);
        c.y=clamp(c.y,-.4,.4);
        c.z=clamp(c.z,-.4,.4);
float l_ = c.x + 0.3963377774 * c.y + 0.2158037573 * c.z;
float m_ = c.x - 0.1055613458 * c.y - 0.0638541728 * c.z;
float s_ = c.x - 0.0894841775 * c.y - 1.2914855480 * c.z;

float l = l_*l_*l_;
float m = m_*m_*m_;
float s = s_*s_*s_;

return vec3(clamp(f(+4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s),0.,1.),
                        clamp(f(-1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s),0.,1.),
                        clamp(f(-0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s),0.,1.));
}

vec2 random2(vec2 st){

    st = vec2( dot(st,vec2(127.1,311.7)),

              dot(st,vec2(269.5,183.3)) );

    return -1.0 + 2.0*fract(sin(st)*43758.5453123);

}

float snoise(vec2 v) {

    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0

    vec2 i  = floor(v + dot(v, C.yy) );

    vec2 x0 = v -   i + dot(i, C.xx);

    vec2 i1;

    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))

        + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}


float dither(vec2 _uv, float _scale){
    _uv*=_scale;
    return mod(fround(_uv.x) + fround(_uv.y), 2.0);
}


vec2 v2quant(vec2 _pos, float _scale){
    _pos *= _scale;
    return vec2(fround(_pos.x), fround(_pos.y))/ _scale;
}

float fquant(float _f, float _q){
        return fround(_f*_q)/_q;
}


float osin(float _p){
    return 0.5 + 0.5 * sin(_p);
}


float ocos(float _p){
    return 0.5 + 0.5 * cos(_p);
}
#ifdef ANDROID_SHADER_EDITOR
void main(void) {
#else
void mainImage( out vec4 fragColor, in vec2 fragCoord ){
#endif
#ifdef ANDROID_SHADER_EDITOR
    float s_time = time / s_iTime_div;
    vec2 st = gl_FragCoord.xy / resolution.xy;
#else
    float s_time = iTime / s_iTime_div;
    vec2 st = gl_FragCoord.xy / iResolution.xy;
#endif
    float t = mod(s_time, t_mod_div);
    float t_fr = floor(mod(t,f_iTime_div)*f_iTime_div);
#ifdef ANDROID_SHADER_EDITOR
    st.y*=2.;
        st.y-=.5;
#endif
    vec2 stq = v2quant(st, noise_res);
    st = stq;
    st.y*=2.;
        st.y-=.5;
    float z = length(vec2(.5)-st);

    vec2 uv = (st-.5)*2.;

    //float z = length(vec2(.5)-uv);

    float ua = osin(uv.x+(t/3.53))*.5;
    float a = sin(uv.x+t+ua)/4.;
    float ub = osin(uv.x+(t/4.13))*.5;
    float b = cos(uv.y + t + cos(t + 5.)+ub) / 2.;
    float pna = snoise(vec2(a,z));
    float pnb = snoise(vec2(z,b));
    float pnabz = (pna*pnb)*(1.-z);

    float mr0 = min(random(stq+t_fr), random(stq-t_fr/1.5));
    float mr1 = min(random(stq-t_fr/3.), random(stq+t_fr));
    float mr00 = 0.5-0.5*(min(random(stq), mr0)/1.)-1.;
    float mr11 = 0.5+0.75*(min(random(stq), mr1)/1.);
    float br = random(stq+t_fr);
    float brs = 0.5+br*0.5;


    float sn = smoothstep(mr00, mr11, pnabz);
    float dr = dither(uv, half_dither_res);
    float drs =0.3+(0.5*(dr));
    float basecol = -.25;
    float coldiff = 0.01;
    float t_off = 1.7;
    float t_mult=1.0;
    float bsat = 0.2;
    float ss_l_mi =0.5;
    float ss_l_ma= 1.0;
    float l_ma= 1.5;


    float amx = osin(s_time/13. + TWO_PI);
    float mmx = osin(s_time/5. + TWO_PI + amx);
    float shmx = (pna * mmx) + (pnb * (1.-mmx));
    vec3 okl1 = vec3(0.);
    vec3 okl2 = vec3(0.);

    vec3 colours[4] = vec3[4](
            vec3(0.0,-0.2, 0.3),
            vec3(0.95,0.35, 0.4),
            vec3(0.1,-0.1, -0.4),
            vec3(0.9,0.2, -0.4)
        );
    float lo = osin(uv.x*shmx + TWO_PI*(t/5.)); //is this ever lower than 0.2???
    float bright = clamp((pnabz * log(.000985+lo))*max(brs,br),0.,1.);


    float sscol = smoothstep(0.2,0.8,bright);
    vec3 fc1 = vec3(
        mix(colours[0].x, colours[1].x, sscol),
        mix(colours[0].y, colours[1].y, sscol),
        mix(colours[0].z, colours[1].z, sscol));
    fc1 = oklab_to_rgb(fc1);
    lo = osin(uv.x*shmx + TWO_PI*((t+(0.5*uv.x))/5.));
    bright = clamp((pnabz * log(.00785+lo))*max(brs,br),0.,1.);
    vec3 fc2 = vec3(
        mix(colours[2].x, colours[3].x, sscol),
        mix(colours[2].y, colours[3].y, sscol),
        mix(colours[2].z, colours[3].z, sscol));
    fc2 = oklab_to_rgb(fc2);
    //okl1.z = basecol-coldiff+sin( +.03 * sin(PI * pnabz + (s_time+t_off)*t_mult))*.4;
    //okl1.y = basecol/sin(mod(sin(sn+t),PI))*.4;
    //okl1.x = clamp((pnabz * log(.000985+lo))*max(brs,br),0.,1.);


    //lo = osin(uv.x*shmx + TWO_PI*((t+(0.5*uv.x))/5.));
    //okl2.z = basecol+coldiff+sin( +.03 * sin(PI * pnabz + (s_time+t_off)*t_mult))*.4;
    //okl2.y = basecol-coldiff+sin(mod(sin(sn+t),PI))*.4;
    //okl2.x = clamp((pnabz * log(.00785+lo))*max(brs,br),0.,1.);


    vec3 rgb1 = oklab_to_rgb(okl1);
    vec3 rgb2 = oklab_to_rgb(okl2);
    vec3 rgb3 = hsb2rgb(vec3(0.05+pnabz*shmx*8.5,0.7, 1.0));
    vec3 rgb4 = hsb2rgb(vec3(0.03+pnabz*shmx*100.2,0.9, 0.65));

    float l1 = length(rgb1);
    float l2 = length(rgb2);
    float brg = exp(l1+l2);
#ifdef DONUT_DEBUG_MODE
    bool ase_debug = false;
#ifdef ANDROID_SHADER_EDITOR
    ase_debug = true;
    if(pointerCount == 1){
        dr=0.0;
    } else if(pointerCount == 2){
        dr=1.0;
    }else if(pointerCount == 3){
        //normal dither
    }
    #endif
    if (!ase_debug) {
        if(uv.x > 0.0){
            dr=1.;
        } else {
            dr=0.;
        }
    }

#endif
    vec3 vbr = vec3(clamp(brg-2.85,0.,0.));
    vec3 c1 = rgb1+(rgb1*rgb3*vbr*max((vbr.x)-0.2, 0.));
    vec3 c2 = rgb2+(rgb2*rgb4*vbr*max((vbr.x*0.93)-0.25, 0.));
    vec3 c = mix(fc1, fc2, dr);
#ifdef QUANT_2_BIT

        c.r = fquant(c.r,2.);
        c.g = fquant(c.g,2.);
        c.b = fquant(c.b,2.);

#endif
#ifdef COLOR_DEBUG_MODE
    //color tests
    vec2 start = vec2(-0.4);
    float size = 0.1;

    if(uv.x > start.x && uv.x < start.x+size*float(colours.length())){
            if(uv.y > start.y && uv.y < start.y+size){
                for(int i = 0; i < colours.length(); i++){
                    vec2 current = start;
                    current.x += float(i) * size;
                    if(uv.x > current.x && uv.x < current.x+size){
                        if(uv.y > current.y && uv.y < current.y+size){
                c = colours[i];
                        }
                    }
                }
            }
        }
#endif


#ifdef ANDROID_SHADER_EDITOR
    fragColor = vec4(c, 1.);
#else
        fragColor = vec4(c, 0.0);
#endif
}
