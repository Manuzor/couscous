/* quad vertex shader */
#pragma sokol @vs vs
in vec2 position;
out vec2 uv;

void main() {
    gl_Position = vec4(position.x, -position.y, 0.0, 1.0);
    uv = position;
}
#pragma sokol @end

/* quad fragment shader */
#pragma sokol @fs fs
in vec2 uv;
out vec4 frag_color;
uniform sampler2D tex;

void main() {
    frag_color = texture(tex, uv);
}
#pragma sokol @end

/* quad shader program */
#pragma sokol @program quad vs fs

