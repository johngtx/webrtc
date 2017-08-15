/**
 * Created by john on 5/10/17.
 */
"use strict";


let net = require('net');
let server = net.createServer();

server.on('connection', (socket) => {
    console.log('new client connection');
    socket.on('data', (data) => {
        //TODO
        console.log('client recv data:' + data.length);
        _BuffProcess(data);
    });
    socket.on('end', () => {
        //TODO
        console.log('client socket end');
    });
    socket.on('error', (error) => {
        //TODO
        console.log('client socket error');
    });
});

server.on('listening', () => {
    console.log('listening 5000');
})
server.on('end', () => {
    console.log('tcp server end');
});
server.on('error', (e) => {
    console.log('tcp server error retring...');
    setTimeout(() => {
        server.close();
        server.listen(3103);
    }, 1000);
});
server.listen(5000);


//TLV package
let package_ = new Buffer(8 * 1024);
let head = new Buffer(4);
let length = 0;
let require_len = 4;
let newhead = true;
let offset = 0;

package_.fill(0);
head.fill(0);

function _BuffProcess(buff, socket){
    "use strict";
    let bufflen = buff.length;
    let src_offset = 0;
    while (bufflen > 0){
        if (newhead) {
            if (require_len > bufflen) {
                buff.copy(head, offset, src_offset);
                offset += bufflen;
                require_len -= bufflen;
                break;
            } else {
                buff.copy(head, offset, src_offset, src_offset + require_len);
                bufflen -= require_len;
                offset = 0;
                newhead = false;
                src_offset += require_len;
                require_len = head.readInt32BE();
                length = require_len;
                //check package length
                if (require_len <= 0 || require_len > (8 * 1024)){
                    // __LOGGER.error('package length invalid:' + require_len);
                    // __LOGGER.info('close client socket');
                    console.log('invilad length');
                    socket.close();
                    return;
                }
            }
        } else {
            if (require_len > bufflen){
                buff.copy(package_, offset, src_offset);
                offset += bufflen;
                require_len -= bufflen;
                break;
            } else {
                buff.copy(package_, offset, src_offset, src_offset + require_len + 4);
                let str = package_.toString('utf8', 0, length);
                let obj = {}
                try {
                    obj = JSON.parse(str);
                } catch(e) {
                    console.log(e.name);
                    console.log(e.message);
                    console.log(str);
                    console.log('length\treqlen\tsoffset\tbuflen\tstrlen');
                    console.log(length + '\t' + require_len + '\t' + src_offset + '\t' + buff.length + '\t' + str.length);
                    socket.close();
                    return;
                }
                console.log('pkg:\t' + obj.id + '\t' + str.length + '\t' + length);
                //fnSleep(new Date().getMilliseconds() / 2 + 100);

                src_offset += length;
                bufflen -= require_len;
                offset = 0;
                require_len = 4;
                length = 0;
                newhead = true;
                head.fill(0);
                package_.fill(0);
            }
        }
    }
}

function fnSleep(ms) {
    let cnt;
    for (let i = 0; i < 5000000; ++i){
        let a = i % 3 + i / 45 - i %6 + i / 7;
        cnt = i;
    }
    console.log('cnt:' + cnt);
}