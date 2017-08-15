/*
 * add by xiong@2017.05.10
 */

"use strict";
//interface
module.exports = function() {
    global.__FORMAT = FORMAT;
    global.__RFORMAT = RFORMAT;
    global.__LOGGER = LOGGER;
    global.__IO_EVENT = IO_EVENT;
    global.__PROTO_TITLE = PROTO_TITLE;
    global.__USER_STATE = USER_STATE;
    global.__UserModel = UserModel;
    global.__ConfModel = ConfModel;
    global.__Regular404Error = Regular404Error;
};

//
let UserModel = require('./usermodel');
let ConfModel = require('./confmodel');

function FORMAT(data){
	return JSON.stringify(data);
}

function RFORMAT(data){
	try{
		return JSON.parse(data);
	} catch(e) {
		console.log('json parse exception ... ' + e.name);
		console.log(data);
		return {};
	}
}

function _TimeStr() {
	return new Date().toLocaleString();
}

function log(msg) {
	console.log('[' + _TimeStr() + ']:' + msg);
}

function warn(msg) {
	console.warn('[' + _TimeStr() + ']:' + msg);
}

function error(msg) {
	console.error('[' + _TimeStr() + ']:' + msg);
}

function info(msg) {
	console.info('[' + _TimeStr() + ']:' + msg);
}

let LOGGER = {
	log: log,
	warn : warn,
	error : error,
	info : info
};

//io event type
let IO_EVENT = {
	OFFER		: '_offer',
	ANSWER 		: '_answer',
	CANDIDATE 	: '_ice_candidate',
	CANCEL 		: '_cancel',
	REJECT 		: '_reject',
	BYE 		: '_bye',
	REGISTER 	: '_register',
	UNREGISTER	: '_unregister',
	RESPONSE 	: '_response',
    JOINROOM    : '_join',
    LEAVEROOM   : '_leave',
	ERRORTYPE   : '_errortype'
};

//proto title
let PROTO_TITLE = {
	MESSAGE 	: 'message',
	ROOM 		: 'room'
}

//user state
let  USER_STATE = {
	ONLINE 		: 'online',
	OFFLINE  	: 'offline',
	REGISTERED  : 'registered',
	UNREGISTERED: 'unregistered'
}

//regular 404 error
function Regular404Error(obj, socket, title){
	let res = {
		event: 		IO_EVENT.RESPONSE,
		from: 		obj.from,
		to: 		obj.to,
		callid: 	obj.callid
	};

	res.data = {
		type: 			obj.event,
		status_code: 	'404',
		msg: 			'not found'
	};

	return res;
}
