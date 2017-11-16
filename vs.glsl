#version 130

in vec3 vertex;
in vec3 normal;
in vec2 uv;

uniform mat4 mvp;
uniform mat4 modelview_matrix;

out vec4 color;

void main() {
    gl_Position = mvp * vec4(vertex, 1);
    vec4 n = normalize(modelview_matrix * vec4(normal, 0));
    color = vec4(n.z, n.z, n.z, 1.0);
}
