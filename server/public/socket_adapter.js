/**
 * Created by john on 5/10/17.
 */

"use strict";

class SocketAdapter {
    constructor(type, socket) {
        this.type = type;
        this.socket = socket;
        this.uid = -1;
        this.state = __USER_STATE.UNREGISTERED;
    }

    Send(data, title) {
        switch (this.type){
            case 'SOCKET_IO':
                this.socket.emit(title, __FORMAT(data));
                break;
            case 'SOCKET_TCP':
                //add package length head
                let str = __FORMAT({title: title, data: data});
                let buf = new Buffer(str.length + 4);
                buf.writeInt32BE(str.length, 0);
                buf.write(str, 4);
                this.socket.write(buf);
                break;
            default:
        }
    }

    close() {
        switch (this.type){
            case 'SOCKET_IO':
                this.socket.close();
                break;
            case 'SOCKET_TCP':
                this.socket.end();
                break;
            default:
        }
    }

    setUid(id) {
        this.uid = id;
    }

    getUid() {
        return this.uid;
    }

    setState(st){
        this.state = st;
    }

    getState(){
        return this.state;
    }

    getSocket(){
        return this.socket;
    }

    equal(sk){
        return this.socket === sk.getSocket();
    }
}

module.exports = SocketAdapter;