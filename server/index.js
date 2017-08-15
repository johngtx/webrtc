/*
*	add by xiong@2017.04.07
*	start http service
*/

"use strict";
let url = require('url');
let socket = require('./public/socket_io');
let config = require('./config.js');
let api_handle = require('./router/api_handle.js');

//init global
require('./public/global')();
require('./public/socket_tcp').Init(config);

let port = config.port || '3000';

let app;
if (config.https){
	//use https
} else {
	//use http
	app = require('http').createServer(handler);
}

//init socket.io server;
socket.Init(app);
app.listen(port);
app.on('error', function(err){
	if (err.syscall !== 'listen'){
		throw err;
	}

	//handle specific listen errors with friendly message
	switch (err.code){
		case 'EACCES':
			__LOGGER.error(port + ' requires elevated privileges');
			process.exit(1);
			break;
		case 'EADDRINUSE':
            __LOGGER.error(port + ' is already in use');
			process.exit(1);
			break;
		default:
			throw error;
	}
});
app.on('listening', function(){
    __LOGGER.log('socket.io listening @port:' + port);
});

function handler(req, res){
	//res.writeHead(403);
	//res.end();
	let url_obj = url.parse(req.url, true);
	let content = api_handle.route(url_obj.pathname, url_obj.query);
	res.end(JSON.stringify(content));
}