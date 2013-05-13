uniform sampler2D		texture_uniform;
uniform float cx,cy;
varying vec2 v_frag_uv;

uniform mediump vec3 text_color;

const mediump float SmoothCenter = 0.5;
const mediump float SmoothWidth = 0.04;

void main(void)
{
    float x = cx + v_frag_uv.x;
    float y = cy + v_frag_uv.y;

    mediump vec4 color = texture2D(texture_uniform, vec2(x, y));
    mediump float distance = color.a;
    mediump float alpha = smoothstep(SmoothCenter - SmoothWidth,
                                     SmoothCenter + SmoothWidth,
                                     distance);
    gl_FragColor = vec4(text_color, alpha);
}