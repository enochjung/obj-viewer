#version 150

in vec3 vPosition;
uniform mat4 mProjection;
uniform mat4 mTranslation;

void main() { 
	gl_Position = mProjection * vec4(vPosition, 1.0);
}
