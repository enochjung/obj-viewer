#version 330 core

in vec2 UV;
in vec3 vertexPosition_world;
in vec3 vertexNormal_camera;
in vec3 cameraDirection_camera;
in vec3 lightDirection_camera;

uniform mat4 ModelView;
uniform vec3 lightPosition;

void main() {
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
