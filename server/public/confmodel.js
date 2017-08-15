/**
 * Created by yiyun on 4/10/17.
 */

"use strict";
let _confs = [];
/* conf{
 *      confid:
 *      name:
 *      type:
 *      maxcount:
 *      curcount:
 *      member:     [string, string ...]
 * }
 */

//get conf
function _GetConf(confid){
    for (let i = 0; i < _confs.length; ++i){
        let obj = _confs[i];
        if (obj.confid === confid){
            return obj;
        }
    }

    return null;
}

function _CreateConf(confid){
    let newconf = {
        confid:         confid,
        maxcound:       16,
        curcount:       0,
        name:           '',
        type:           '',
        members:        []
    };

    _confs.push(newconf);
    return newconf;
}

function _DelConf(confid){
    for (let i = 0; i < _confs.length; ++i){
        if (conf.confid === _confs[i].confid){
            _confs.splice(i, 1);
            break;
        }
    }
}

function _GetAllConfs(){
    return _confs;
}

module.exports = {
    GetConf:        _GetConf,
    CreateConf:     _CreateConf,
    DelConf:        _DelConf
}