/**
 * Created by john on 5/17/17.
 */

const net = require('net');
const client = net.connect({port:5000}, ()=>{
    console.log('connected to server');
});

client.on('data', (data)=>{
    console.log('data recv');
});

let pkgs = [
    JSON.stringify({name: '111', id: 0, pwd:'fasdfasdresfdavfasdfasdresfdavfasdfasdresfdavfasdfasdresfdavfasdfasdresfdavfasdfasdresfdav', str: '123'}),
    JSON.stringify({name: '111', id: 1, str1: '234234234234234234'}),
    JSON.stringify({name: '111', id: 2, str2:'sdfsadvawersafsdfsadvawersafsdfsadvawersafsdfsadvawersafsdfsadvawersafsdfsadvawersafsdfsadvawersafsdfsadvawersafsdfsadvawersafsdfsadvawersafsdfsadvawersaf'}),
    JSON.stringify({name: '111', id: 3}),
    JSON.stringify({name: '111', id: 4, str34:'sadvasddrfweafvsadvasddrfweafvsadvasddrfweafvsadvasddrfweafv'}),
    JSON.stringify({name: '111', id: 5, str1: '111111111111111111111111111111111111111111111111111111111111111111111'})
];

let heads = [];
Init();

function Init(){
    for (let i = 0; i < pkgs.length; ++i){
        let headlen = pkgs[i].length;
        let buf = new Buffer(4);
        buf.fill(0);
        buf.writeInt32BE(headlen);
        heads.push(buf);
    }

    //send random pkg
    setInterval(() => {
        let index = new Date().getMilliseconds() % 6;
        let ms = new Date().getMilliseconds() + new Date().getSeconds();
        let index2 = ms % 6;

        let h1 = heads[index];
        let h2 = heads[index2];
        let pkg1 = pkgs[index];
        let pkg2 = pkgs[index2];

        let len = 4 * 2 + pkg1.length + pkg2.length;
        let buf = Buffer.alloc(len);

        h1.copy(buf, 0);
        buf.write(pkg1, 4);
        let offset = 4 + pkg1.length;
        h2.copy(buf, offset);
        buf.write(pkg2, offset + 4);

        // let offset = 0;
        // while (offset < buf.length){
        //     let rl = new Date().getMilliseconds();
        //     if (offset + rl > buf.length){
        //         offset = buf.length;
        //     } else {
        //         offset += rl;
        //     }
        //
        //     client.write()
        // }
        console.log('send:\t' + index + '\t' + pkg1.length);
        console.log('send:\t' + index2 + '\t' + pkg2.length);
        client.write(buf);
    }, 20);
}

