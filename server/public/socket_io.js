/*
*	add by xiong@2017.04.07
*	socket.io instance
*/

"use strict";
//websocket
let Socket = require('socket.io');
let SocketAdapter = require('./socket_adapter');

function Init(server){
	let io = Socket(server);
	//event
	io.on('connection', function(cli){
		__LOGGER.log('new socket_io user connected');
		let socket = new SocketAdapter('SOCKET_IO', cli);
        cli.on('message', function(data){
        	let obj = _PreProcessData(data);
        	if (obj === null) return;
			require('./dispatch_message').Dispatch(obj, socket);
		});
        cli.on('room', function(data){
        	let obj = _PreProcessData(data);
        	if (obj === null) return;
			require('./dispatch_room').Dispatch(obj, socket);
		});
        cli.on('disconnect', function(){
            __LOGGER.log('socket_io user disconnect');
            __UserModel.Disconnect(socket);
        });
	});
}

function _PreProcessData(data){
	let obj = {};
	try{
		obj = JSON.parse(data);
	} catch (e) {
		__LOGGER.error('socket io data parse error:' + e.name);
		return null;
	}
	return obj;
}
//export model
module.exports = {
	Init: Init
}