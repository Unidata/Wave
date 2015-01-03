"use strict";

var conn;
var main;
function Backend() {
    conn = new IpythonConnection({
        init_code: 'import wave',
        start: function() {
            main = new mainLoop(this.kernel);
        }
    });
}

function mainLoop(kernel) {
    this.canvas = new ViewCanvas(document.getElementById("canvas"));
    var that = this;
    kernel.execute("manager = wave.DataManager(); manager.bounds()",
        {'output': function(msg_type, content) {
            // callback for updating the texture with new data
            logit(msg_type, content)
            if (msg_type == 'display_data') {
                var payload = content['data'];
                var obj = JSON.parse(payload['application/json']);
                var bounds = obj['bounds'];
                that.canvas.domain = new Rectangle(bounds[0][0], bounds[0][1],
                    bounds[1][0], bounds[1][1]);
                console.log(bounds);
                console.log(that.canvas.domain);
                that.canvas.updatedProjection();
            }
            var marble = new RasterImageLayer(kernel, 'manager.blueMarble()');
            that.canvas.layers.push(marble);

            var satellite = new RasterImageLayer(kernel, 'manager.satellite()');
            that.canvas.layers.push(satellite)
            satellite.alpha = 0.6;

            var satellite = new RasterImageLayer(kernel, 'manager.radar()');
            that.canvas.layers.push(satellite)
            satellite.alpha = 1.0;
        }});

    this.initStats();
    this.tick();
}

mainLoop.prototype.initStats = function() {
    this.stats = new Stats();
    this.stats.setMode(1); // 0: fps, 1: ms
    document.body.appendChild(this.stats.domElement);
}

mainLoop.prototype.tick = function() {
    requestAnimFrame(this.tick.bind(this));
    this.stats.begin();
    this.canvas.drawScene();
    this.stats.end();
}