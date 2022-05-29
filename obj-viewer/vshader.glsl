#version 150

in vec3 vPosition;
uniform mat4 mProjection;
uniform mat4 mModel;

void main() { 
	gl_Position = vec4(vPosition, 1.0) * mModel * mProjection;
}
