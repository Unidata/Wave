"use strict";

function Backend() {
    var that = this;
    $([IPython.events]).on('status_started.Kernel', function(evt, data) {
        setTimeout(function() {
            that.init_namespace(data.kernel);
            that.main = new mainLoop(kernel);
            // do_request_update(data.kernel);
        }, 50);
    });
    
    var kernel = new IPython.Kernel('/kernels');
    
    var ws_url = 'ws' + document.location.origin.substring(4);
    setTimeout(function() {
        kernel._kernel_started({kernel_id: '1', ws_url: ws_url});
    }, 50);
}

Backend.prototype.init_namespace = function(kernel) {
    var code = "import json\n" +
    "from IPython.core.display import JSON, Image, display\n" +
    "import wave\n" +
    "wave.foo()\n" +
    "def update_plot(n, xmax=10, npoints=200):\n" +
    "    print n, xmax, npoints\n" +
    "    display(Image('static/world.topo.bathy.200406.3x5400x2700.png'))";
    kernel.execute(code, {'output': this.logit});
}

Backend.prototype.logit = function(msg_type, content) {
    if (msg_type == 'stream') {
        console.log(content['data']);
    }
}

function mainLoop(kernel) {
    this.canvas = new ViewCanvas(document.getElementById("canvas"));
    this.canvas.layers.push(new RasterImageLayer(kernel));

    this.initStats();
    this.tick();
}

mainLoop.prototype.initStats = function() {
    this.stats = new Stats();
    this.stats.setMode(1); // 0: fps, 1: ms

    // Align top-left
    this.stats.domElement.style.position = 'absolute';
    this.stats.domElement.style.left = '0px';
    this.stats.domElement.style.top = '0px';
    document.body.appendChild( this.stats.domElement );
}

mainLoop.prototype.tick = function() {
    requestAnimFrame(this.tick.bind(this));
    this.stats.begin();
    this.canvas.drawScene();
    this.stats.end();
}