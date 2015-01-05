precision mediump float;

varying vec2 vTextureCoord;
// varying vec4 vColor;

uniform sampler2D uSampler;
uniform sampler2D uCmap;

uniform float uAlpha;

uniform bool uUseCmap;

void main(void) {
    gl_FragColor = texture2D(uSampler, vTextureCoord);
    const vec3 missing = vec3(0., 0., 0.);
	if (uUseCmap) {
		// vec4 color = gl_FragColor;
		gl_FragColor = texture2D(uCmap, vec2(gl_FragColor.r * (255. / 220.), 0.));
		// gl_FragColor.a = (color.r == 0.) ? 0. : uAlpha;
	}
	gl_FragColor.a = (gl_FragColor.rgb == missing) ? 0.0 : uAlpha;
    // gl_FragColor = vColor;
}