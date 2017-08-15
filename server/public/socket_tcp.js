/**
 * Created by john on 5/10/17.
 */
"use strict";

//#interface
module.exports = {
    Init: Init
}

let SocketAdapter = require('./socket_adapter');
let net = require('net');
let server = net.createServer();

function Init(config) {
    let port = config.tcpport || 3100;
    //add event listener
    //buff
    let tlv = {};
    tlv.package_ = new Buffer(8 * 1024);
    tlv.head = new Buffer(4);
    tlv.require_len = 4;
    tlv.newhead = true;
    tlv.offset = 0;

    tlv.package_.fill(0);
    tlv.head.fill(0);

    server.on('connection', (cli) => {
        __LOGGER.log('new tcp connection ... ');
        let socket = new SocketAdapter('SOCKET_TCP', cli);
        cli.on('data', (data) => {
            _BuffProcess(data, socket, tlv);
        });
        cli.on('end', () => {
            __UserModel.Disconnect(socket);
            cli.destroy();
        });
        cli.on('error', (error) => {
            __LOGGER.error('socket tcp client error:' + error.name);
            __UserModel.Disconnect(socket);
            cli.destroy();
        });
    });

    server.on('listening', () => {
        __LOGGER.log('tcp server listening @port:' + port);
    })
    server.on('end', () => {
        __LOGGER.log('tcp server end');
    });
    server.on('error', (e) => {
        __LOGGER.error('tcp server error:' + e);
        __LOGGER.log('retrying ... ');
        setTimeout(() => {
            server.close();
            server.listen(port);
        }, 1000);
    });
    server.listen(port);
}



function _PreprocessData(obj, socket){
    switch (obj.title){
        case __PROTO_TITLE.MESSAGE:
            require('./dispatch_message').Dispatch(obj.data, socket);
            break;
        case __PROTO_TITLE.ROOM:
            require('./dispatch_room').Dispatch(obj.data, socket);
            break;
        default:
            __LOGGER.error('unknow proto title:' + obj.title);
    }
}

//TLV package
function _BuffProcess(buff, socket, tlv){
    "use strict";
    let bufflen = buff.length;
    let src_offset = 0;
    while (bufflen > 0){
        if (tlv.newhead) {
            if (tlv.require_len > bufflen) {
                buff.copy(tlv.head, tlv.offset, src_offset);
                tlv.offset += bufflen;
                tlv.require_len -= bufflen;
                break;
            } else {
                buff.copy(tlv.head, tlv.offset, src_offset, src_offset + tlv.require_len);
                bufflen -= tlv.require_len;
                tlv.offset = 0;
                tlv.newhead = false;
                src_offset += tlv.require_len;
                tlv.require_len = tlv.head.readInt32BE();
                //check package length
                if (tlv.require_len <= 0 || tlv.require_len > (8 * 1024)){
                    __LOGGER.error('package length invalid:' + tlv.require_len);
                    __LOGGER.info('close client socket');
                    __LOGGER.log(buff);
                    socket.close();
                    return;
                }
            }
        } else {
            if (tlv.require_len > bufflen){
                buff.copy(tlv.package_, tlv.offset, src_offset);
                tlv.offset += bufflen;
                tlv.require_len -= bufflen;
                break;
            } else {
                buff.copy(tlv.package_, tlv.offset, src_offset, src_offset + tlv.require_len + 4);
                let str = tlv.package_.toString('utf8', 0, tlv.head.readInt32BE());
                try {
                    _PreprocessData(JSON.parse(str), socket);
                } catch (e){
                    __LOGGER.error(e.message);
                    __LOGGER.error(e.name);                 
                    __LOGGER.info('close client socket');
                    socket.close();
                    return;
                }
                src_offset += tlv.require_len;
                bufflen -= tlv.require_len;
                tlv.offset = 0;
                tlv.require_len = 4;
                tlv.newhead = true;
                tlv.head.fill(0);
                tlv.package_.fill(0);
            }
        }
    }
}
