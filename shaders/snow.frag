//-----------------------------------------------------
// SnowIsFalling.glsl
//  v1.0  2017-08-24  initial version
//  v1.1  2018-04-17  test with mysteryMountain
// combining background of https://www.shadertoy.com/view/4dl3R4
// and falling snow of     https://www.shadertoy.com/view/ldsGDn
//-----------------------------------------------------

#version 310 es
precision highp float;

#define PI 3.14159265358979323846

#define LAYERS 36

#define DEPTH1 0.3
#define WIDTH1 0.4
#define SPEED1 0.2

#define DEPTH2 0.1
#define WIDTH2 0.3
#define SPEED2 0.1


uniform vec2  u_viewport;
uniform float u_time;
uniform vec2  u_mouse;

out vec4 fragColor;

#define mod289(x) mod(x, 289.0)

vec3 permute(vec3 x)
{
    return mod289(((x * 34.0) + 1.0) * x);
}

float snoise(vec2 v)
{
    const vec4 C = vec4(
        0.211324865405187,
        0.366025403784439,
       -0.577350269189626,
        0.024390243902439
    );

    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);

    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0)
                            : vec2(0.0, 1.0);

    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    i = mod289(i);
    vec3 p = permute(
                 permute(i.y + vec3(0.0, i1.y, 1.0))
               + i.x + vec3(0.0, i1.x, 1.0)
             );

    vec3 m = max(
        0.5 - vec3(
            dot(x0, x0),
            dot(x12.xy, x12.xy),
            dot(x12.zw, x12.zw)
        ),
        0.0
    );

    m *= m;
    m *= m;

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    m *= 1.79284291400159
       - 0.85373472095314 * (a0 * a0 + h * h);

    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;

    return 130.0 * dot(m, g);
}

float fbm(vec2 p)
{
    float f = 0.0;
    float w = 0.5;

    for (int i = 0; i < 5; i++)
    {
        f += w * snoise(p);
        p *= 2.0;
        w *= 0.5;
    }
    return f;
}

float background(vec2 uv)
{
    uv.x += u_mouse.x / u_viewport.x - 1.0;

    vec2 sunCenter = vec2(0.3, 0.9);
    float suns  = clamp(1.2 - distance(uv, sunCenter), 0.0, 1.0);
    float sunsh = smoothstep(0.85, 0.95, suns);

    float slope = 1.0 - smoothstep(
        0.55, 0.0,
        0.8 + uv.x - 2.3 * uv.y
    );

    float noise = abs(fbm(uv * 1.5));
    slope = (noise * 0.2)
          + (slope - ((1.0 - noise) * slope * 0.1)) * 0.6;

    slope = clamp(slope, 0.0, 1.0);

    return 0.35 + slope * (suns + 0.3) + sunsh * 0.6;
}

float snowing(vec2 uv)
{
    const mat3 p = mat3(
        13.323122, 23.5112,  21.71123,
        21.1212,   28.7312,  11.9312,
        21.8112,   14.7212,  61.3934
    );

    vec2 mp = u_mouse / u_viewport;
    uv.x += mp.x * 4.0;
    mp.y *= 0.25;

    float depth = smoothstep(DEPTH1, DEPTH2, mp.y);
    float width = smoothstep(WIDTH1, WIDTH2, mp.y);
    float speed = smoothstep(SPEED1, SPEED2, mp.y);

    float acc = 0.0;
    float dof = 5.0 * sin(u_time * 0.1);

    for (int i = 0; i < LAYERS; i++)
    {
        float fi = float(i);
        vec2 q = uv * (1.0 + fi * depth);

        float w = width * mod(fi * 7.238917, 1.0)
                - width * 0.1 * sin(u_time * 2.0 + fi);

        q += vec2(
            q.y * w,
            speed * u_time / (1.0 + fi * depth * 0.03)
        );

        vec3 n = vec3(floor(q), 31.189 + fi);
        vec3 m = floor(n) * 0.00001 + fract(n);
        vec3 r = fract((31415.9 + m) / fract(p * m));

        vec2 s = abs(mod(q, 1.0) - 0.5 + 0.9 * r.xy - 0.45);
        s += 0.01 * abs(2.0 * fract(10.0 * q.yx) - 1.0);

        float d = 0.6 * max(s.x - s.y, s.x + s.y)
                + max(s.x, s.y) - 0.01;

        float edge = 0.05 + 0.05 * min(0.5 * abs(fi - 5.0 - dof), 1.0);

        acc += smoothstep(edge, -edge, d)
             * (r.x / (1.0 + 0.02 * fi * depth));
    }

    return acc;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / u_viewport.y;

    vec3 color = vec3(0.0);
    // float bg = background(uv);
    // color = vec3(bg * 0.9, bg, bg * 1.1);

    float snow = snowing(uv);
    color += vec3(snow);

    fragColor = vec4(color, 0.1);
}
