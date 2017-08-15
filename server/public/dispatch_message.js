/*
*	add by xiong@2017.04.07
*	message function
*/

"use strict";

//public
function Dispatch(obj, socket){
	//aaaaa
	__LOGGER.log("recv :" + JSON.stringify(obj));
	//check user session
	if (!_CheckSession(obj, socket)){
        //TODO response with 403
        let res = {
            event: __IO_EVENT.RESPONSE,
            from: obj.from,
            to: obj.to,
            data: {
                type: obj.event,
                status_code: '403',
                msg: 'Forbidden'
            }
        };
        socket.Send(res, __PROTO_TITLE.MESSAGE);
		return;
	}

	if (obj && obj.event){
		switch (obj.event){
			case __IO_EVENT.OFFER:
				return _Offer(obj, socket);
			case __IO_EVENT.ANSWER:
				return _Answer(obj, socket);
			case __IO_EVENT.CANDIDATE:
				return _Candidate(obj, socket);
			case __IO_EVENT.CANCEL:
				return _Cancel(obj, socket);
			case __IO_EVENT.REJECT:
				return _Reject(obj, socket);
			case __IO_EVENT.BYE:
				return _Bye(obj, socket);
			case __IO_EVENT.REGISTER:
				return _Register(obj, socket);
			case __IO_EVENT.UNREGISTER:
				return _UnRegister(obj, socket);
			case __IO_EVENT.RESPONSE:
				return _Response(obj, socket);
			default:
				__LOGGER.log('unknow event type ... ' + obj.event);
		}
	}
}

//private
//check socket session
function _CheckSession(obj, socket){
	let [user,] = __UserModel.GetUserBySocket(socket);

	if (user !== null){
		if (obj.event === __IO_EVENT.REGISTER){
			//socket already registered
			return false;
		}
		if (user.id !== obj.from){
			//socket uid doesnt match obj.from
			return false;
		}
	}

	return true;
}

//event call
function _Offer(obj, socket){
	let src = obj.from;
	let dst = obj.to;

	let client = __UserModel.GetClient(dst);
	if (client !== null){
		//relay offer msg
		client.Send(obj, __PROTO_TITLE.MESSAGE);
		__LOGGER.log('relay offer ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	} else {
		//response with 404
		socket.Send(__Regular404Error(obj), __PROTO_TITLE.MESSAGE);
        __LOGGER.log('offer 404 ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	}
}

//event answer
function _Answer(obj, socket){
	let src = obj.from;
	let dst = obj.to;
	let client = __UserModel.GetClient(dst);

	if (client !== null){
		//relay offer msg
		client.Send(obj, __PROTO_TITLE.MESSAGE);
		__LOGGER.log('relay answer ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	} else {
		//response with 404
		socket.Send(__Regular404Error(obj), __PROTO_TITLE.MESSAGE);
        __LOGGER.log('answer 404 ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	}
}

//event candidate
function _Candidate(obj, socket){
	let src = obj.from;
	let dst = obj.to;
	let client = __UserModel.GetClient(dst);

	if (client !== null){
		//relay offer msg
		client.Send(obj, __PROTO_TITLE.MESSAGE);
        __LOGGER.log('relay candidate ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	} else {
		//response with 404
		socket.Send(__Regular404Error(obj), __PROTO_TITLE.MESSAGE);
        __LOGGER.log('candidate 404 ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	}
}

//event cancel
function _Cancel(obj, socket){
	let src = obj.from;
	let dst = obj.to;
	let client = __UserModel.GetClient(dst);

	if (client !== null){
		//relay offer msg
		client.Send(obj, __PROTO_TITLE.MESSAGE);
        __LOGGER.log('relay cancel ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	} else {
		//response with 404
		socket.Send(__Regular404Error(obj), __PROTO_TITLE.MESSAGE);
        __LOGGER.log('cancel 404 ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	}
}

//event reject
function _Reject(obj, socket){
	let src = obj.from;
	let dst = obj.to;
	let client = __UserModel.GetClient(dst);

	if (client !== null){
		//relay offer msg
		client.Send(obj, __PROTO_TITLE.MESSAGE);
        __LOGGER.log('relay reject ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	} else {
		//response with 404
		socket.Send(__Regular404Error(obj), __PROTO_TITLE.MESSAGE);
        __LOGGER.log('reject 404 ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	}
}

//event bye
function _Bye(obj, socket){
	let src = obj.from;
	let dst = obj.to;
	let client = __UserModel.GetClient(dst);

	if (client !== null){
		//relay offer msg
		client.Send(obj, __PROTO_TITLE.MESSAGE);
        __LOGGER.log('relay bye ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	} else {
		//response with 404
		socket.Send(__Regular404Error(obj), __PROTO_TITLE.MESSAGE);
        __LOGGER.log('bye 404 ... [ ' + src + ' ] ---> [ ' + dst + ' ]');
	}
}

//event register
function _Register(obj, socket){
	let id = obj.from;
	let code, msg;

	if (__UserModel.Register(id, socket)){
		//success
		code = 200;
		msg = 'success';
        __LOGGER.log('user register' + ' ... ' + id);
	} else {
		//failed
		code = 403;
		msg = 'forbidden';
        __LOGGER.log('user register forbidden' + ' ... ' + id);
	}

    let res = {
        event: __IO_EVENT.RESPONSE,
        from: obj.from,
        to: obj.to,
        data: {
            type: obj.event,
            status_code: code,
            msg: msg
        }
    };

	socket.Send(res, __PROTO_TITLE.MESSAGE);
}

//event unregister
function _UnRegister(obj, socket){
	let id = obj.from;
	let code, msg;

	if (__UserModel.UnRegister(id)){
		code = 200;
		msg = 'success';
	} else {
		code = 403;
		msg = 'forbidden';
	}

	let res = {
		event: __IO_EVENT.RESPONSE,
		from: obj.from,
		to: obj.to,
		callid: obj.callid,
		data: {
		    type: obj.event,
            status_code: code,
            msg: msg
		}
	};

	//response
	socket.Send(res, __PROTO_TITLE.MESSAGE);
}

//event response
function _Response(obj, socket){

}

//exports
module.exports = {
	Dispatch: Dispatch
}
