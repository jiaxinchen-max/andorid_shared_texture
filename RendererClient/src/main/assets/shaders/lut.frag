#version 300 es
precision mediump float;
in vec4 v_color;
in vec2 v_texcoord;

out vec4 FragColor;

void main() {
    FragColor = vec4(1, 0, 0, 1.0);
}