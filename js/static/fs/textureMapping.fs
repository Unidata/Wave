precision mediump float;

varying vec2 vTextureCoord;
// varying vec4 vColor;

uniform sampler2D uSampler;

uniform float uAlpha;

void main(void) {
    gl_FragColor = texture2D(uSampler, vTextureCoord);
    const vec3 missing = vec3(0., 0., 0.);
	gl_FragColor.a = (gl_FragColor.rgb == missing) ? 0.1 : uAlpha;
    // gl_FragColor = vColor;
}