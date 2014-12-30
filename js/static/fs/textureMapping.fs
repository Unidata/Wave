precision mediump float;

varying vec2 vTextureCoord;
// varying vec4 vColor;

uniform sampler2D uSampler;

void main(void) {
    gl_FragColor = texture2D(uSampler, vTextureCoord);
    const vec3 missing = vec3(0., 0., 0.);
    if (gl_FragColor.rgb == missing)
    {
    	gl_FragColor.rgba = vec4(1., 0., 0., 0.);
    }
    // gl_FragColor = vColor;
}