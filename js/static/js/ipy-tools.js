function IpythonConnection(json) {
    this.init_code = json['init_code'];
    this.start_func = json['start'].bind(this);
    var that = this;
    $([IPython.events]).on('status_started.Kernel', function(evt, data) {
        setTimeout(that.init_kernel.bind(that), 50);
    });
    // $([IPython.events]).on('status_started.Kernel', this.init_kernel.bind(this));
    
    this.kernel = new IPython.Kernel('/kernels');
    setTimeout(this.start_kernel.bind(this), 50);
    // this.start_kernel();
}

IpythonConnection.prototype.start_kernel = function() {
    var ws_url = 'ws' + document.location.origin.substring(4);
    this.kernel._kernel_started({kernel_id: '1', ws_url: ws_url});
}

IpythonConnection.prototype.init_kernel = function() {
    this.init_namespace(this.kernel);
    this.start_func();
}

IpythonConnection.prototype.init_namespace = function(kernel) {
    kernel.execute(this.init_code, {'output': logit});
}

function logit(msg_type, content) {
    if (msg_type == 'stream') {
        console.log(content['data']);
    } else if (msg_type == 'pyerr') {
        console.error(content['ename'] + ': ' + content['evalue']);
    }
}

function parseImage(data) {
    for (var attr in data) {
        if (attr.slice(0, 5) == 'image') {
            return 'data:' + attr + ';base64,' + data[attr];
        }
    }
    return null;
}