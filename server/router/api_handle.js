//api router

"use strict";
let API_EVENT = {
	GETALLRECORDS: 			'/getallrecords',
	DELALLRECORDS: 			'/delallrecords',
	GETUSERRECORDS:      	'/getuserrecords',
	DELUSERRECORDS:			'/deluserrecords'
}

function _Route(pathname, params){
	console.log('req ... ' + pathname + ' ' + JSON.stringify(params));
	switch (pathname){
		case API_EVENT.GETALLRECORDS:
			return _GetAllRecords();
		case API_EVENT.DELALLRECORDS:
			return _DelAllRecords();
		case API_EVENT.GETUSERRECORDS:
			return _GetUserRecords(params.userid);
		case API_EVENT.DELUSERRECORDS:
			return _DelUserRecords(params.userid);
		default:
			return _Error(pathname);
	}
}

//error
function _Error(pathname){
	return {errno: '404'};
}

//get all records
function _GetAllRecords(){
	return {data: 'getallrecords'};
}

//delete all records
function _DelAllRecords(){
	return {data: 'delallrecords'};
}

//get user's records
function _GetUserRecords(id){
	return {data: 'getuserrecords', id: id};
}

//delete user's records
function _DelUserRecords(id){
	return {data: 'deluserrecords', id: id}
}

module.exports = {
	route: _Route
}