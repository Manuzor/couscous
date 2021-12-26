/* quad vertex shader */
#pragma sokol @vs vs
in vec2 position;
out vec2 uv;

void main() {
    gl_Position = vec4(position.x, -position.y, 0.0, 1.0);
    uv = 0.5 * (position + vec2(1.0));
}
#pragma sokol @end

/* quad fragment shader */
#pragma sokol @fs fs
in vec2 uv;
out vec4 frag_color;
uniform sampler2D tex;

void main() {
    const vec4 off_color = vec4(16, 64, 16, 255) / 255;
    const vec4 on_color = vec4(8, 16, 8, 255) / 255;
    const float bit = texture(tex, uv).r * 255.0;
    frag_color = mix(off_color, on_color, bit);
}
#pragma sokol @end

/* quad shader program */
#pragma sokol @program quad vs fs

