//user model
"use strict";

let _users = [];
/* data struct
	User{
		id:
		state:
		socket:
		type:
	}
*/

//public
function Register(id, socket) {
	let [usr, index] = GetUser(id);
	if (usr === null){
		//user with is not registed yet
		socket.setState(__USER_STATE.REGISTERED)
        socket.setUid(id);
		_users.push( {id: id, state: 1, socket: socket} );
        __LOGGER.info('current users ... ' + _users.length);
		return true;
	} else {
		//user with id has been registed
		return false;
	}
}

function UnRegister(id){
	let [obj, index] = GetUser(id);
	if (obj === null){
		return false;
	} else {
		//remove user from online list
        _users.splice(index, 1);
        __LOGGER.info('user unregister:' + obj.id);
        __LOGGER.info('current users ... ' + _users.length);
		return true;
	}
}

function Disconnect(socket){
	let [user, i] = GetUserBySocket(socket);
	if (user === null){
		//TODO user not found
	} else {
		//TODO user disconnect
        _users.splice(i, 1);
        __LOGGER.info('user disconnect:' + user.id);
        __LOGGER.info('current users ... ' + _users.length);
	}

}

function GetClient(id){
	let [user, index] = GetUser(id);
	if (user === null) {
		return null;
	} else {
		return user.socket;
	}
}

function GetUserBySocket(sk){
    for (let i = 0; i < _users.length; ++i){
        let obj = _users[i];
        if (sk.equal(obj.socket)){
            return [obj, i];
        }
    }

    return [null, -1];
}

//private
function GetUser(id){
	for (let i = 0; i < _users.length; ++i){
		let obj = _users[i];
		if (obj.id === id){
			return [obj, i];
		}
	}

	return [null, -1];
}

module.exports = {
	Register: 		Register,		//user register
	UnRegister: 	UnRegister,		//user unregister
    Disconnect:     Disconnect,
	GetClient:		GetClient,
	GetUser:		GetUser,
    GetUserBySocket:GetUserBySocket
};