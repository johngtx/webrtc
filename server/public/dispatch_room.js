/*
 *	add by xiong@2017.04.07
 *	room function
 */

"use strict";

//public
function Dispatch(data, cli){
    //parse msg string
    let obj = {};
    try{
        obj = JSON.parse(data)
    } catch(e){
        //json parse execption
        __LOGGER.log('json parse exception ... ' + e.name);
        __LOGGER.log(data);
    }

    //check user session
    let socket = __UserModel.GetClient(obj.from);
    if (socket === null){
        //TODO response with 401
        let res = {
            event: __IO_EVENT.RESPONSE,
            from: obj.from,
            to: obj.to,
            data: {
                type: obj.event,
                status_code: '401',
                msg: 'Unauthorized'
            }
        };
        cli.send(__FORMAT(res));
        return;
    }


    if (obj && obj.event){
        switch (obj.event){
            case __IO_EVENT.JOINROOM:
                return _EnterConf(obj, socket);
            case __IO_EVENT.LEAVEROOM:
                return _ExitConf(obj, socket);
            default:
                __LOGGER.log('unknow event type ... ' + obj.event);
        }
    }
}

//private
function _EnterConf(obj, socket){
    //check if conf with confid is exists
    __LOGGER.log('enter conf ... [ ' + obj.from + ' ] ---> [ ' + obj.roomid + ' ]');
    let confid = obj.roomid;
    let conf = CONFS.GetConf(confid);
    if (conf === null){
        //not exists
        conf = CONFS.CreateConf(confid);
        conf.creator = obj.from;
    }
    //already exists
    let data = {};
    data.type = obj.event;
    if (conf.curcount <= conf.maxcount){
        //room is full
        data.status_code = '403';
        data.msg = 'room is full';
        data.member = [];
    } else {
        data.status_code = '200';
        data.msg = 'success';
        data.member = conf.members;
    }

    //res
    socket.emit('room', __FORMAT({
        event: __IO_EVENT.RESPONSE,
        from: obj.from,
        roomid: conf.confid,
        data: data
    }));
    //update conf
    conf.members.push(obj.from);
    conf.curcount += 1;
}

function _ExitConf(obj){
    __LOGGER.log('exit conf ... [ ' + obj.from + ' ] ---> [ ' + obj.roomid + ' ]');
    let data = { type: obj.event };
    let conf = CONFS.GetConf(obj.roomid);
    if (conf === null){
        data.status_code = '404';
        data.msg = 'room not found';
        data.member = [];
    } else {        
        //update conf
        conf.curcount -= 1;
        for (let i = 0; i < conf.members.length; ++i){
            if (obj.from === conf.members[i]){
                conf.members.splice(i, 1);
                break;
            }
        }

        data.status_code = '200';
        data.msg = 'success';
        data.member = conf.members;
    }

    //res
    socket.emit('room', __FORMAT({
        event: __IO_EVENT.RESPONSE,
        from: obj.from,
        roomid: obj.roomid,
        data: data
    }));
}

//exports
module.exports = {
    Dispatch: Dispatch
}