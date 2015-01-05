#!/usr/bin/env python
# coding: utf-8
"""A simple webapp with a single IPython Notebook Cell

Authors:

* Min RK
"""
#-----------------------------------------------------------------------------
#  Copyright (C) 2013  Min RK
#
#  Distributed under the terms of the BSD License.  The full license is in
#  the file COPYING, distributed as part of this software.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Imports
#-----------------------------------------------------------------------------

# stdlib
import logging
import os

import IPython

# Install the pyzmq ioloop. This has to be done before anything else from
# tornado is imported.
from zmq.eventloop import ioloop
ioloop.install()

from tornado import httpserver
from tornado import web

try:
    from tornado.log import app_log
except ImportError:
    logging.basicConfig()
    app_log = logging.getLogger()


# IPython
from IPython.kernel.multikernelmanager import MultiKernelManager
from IPython.html.services.kernels.handlers import (
    KernelHandler, KernelActionHandler,
    IOPubHandler, ShellHandler, StdinHandler,
)
from IPython.html.services.kernels.handlers import _kernel_action_regex

#-----------------------------------------------------------------------------
# The Tornado web application
#-----------------------------------------------------------------------------

_kernel_id_regex = r"(?P<kernel_id>\w+)"

class IndexHandler(web.RequestHandler):
    def get(self):
        return self.render('index.html')

class NoCacheStaticFileHandler(web.StaticFileHandler):
    def set_extra_headers(self, path):
        # Disable cache
        self.set_header('Cache-Control', 'no-store, no-cache, must-revalidate, max-age=0')

class WebApp(web.Application):

    def __init__(self, kernel_manager):
        handlers = [
            (r"/", IndexHandler),
            (r"/kernels/%s" % _kernel_id_regex, KernelHandler),
            (r"/kernels/%s/%s" % (_kernel_id_regex, _kernel_action_regex), KernelActionHandler),
            (r"/kernels/%s/iopub" % _kernel_id_regex, IOPubHandler),
            (r"/kernels/%s/shell" % _kernel_id_regex, ShellHandler),
            (r"/kernels/%s/stdin" % _kernel_id_regex, StdinHandler)
        ]

        settings = dict(
            template_path=os.path.join(os.path.dirname(__file__), "templates"),
            static_path='static',
            cookie_secret='secret',
            cookie_name='ignored',
            kernel_manager=kernel_manager,
            static_handler_class=NoCacheStaticFileHandler,
            autoreload=True,
            debug=True
        )

        super(WebApp, self).__init__(handlers, **settings)

#-----------------------------------------------------------------------------
# start the app
#-----------------------------------------------------------------------------

def main():
    kernel_manager = MultiKernelManager()

    # we are only using one kernel:
    kernel_id = '1'
    kernel_manager.start_kernel(kernel_id=kernel_id)

    logging.basicConfig(level=logging.INFO)
    app = WebApp(kernel_manager)
    server = httpserver.HTTPServer(app)
    server.listen(8000, '127.0.0.1')
    app_log.info("Serving at http://127.0.0.1:8000")
    try:
        ioloop.IOLoop.instance().start()
    except KeyboardInterrupt:
        app_log.info("Interrupted...")
    finally:
        kernel_manager.shutdown_all()


if __name__ == '__main__':
    main()