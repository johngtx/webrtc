/*{
	no: 			//auto increase
 	from: 			//src id
 	to: 			//dst id
 	beg: 			//beg time
    end: 			//end time
    media_type: 	//0 audio only 1 video only 3 both
 	errno: 			//0 success 1 reject 2 no answer
}*/

"use strict";
//record model
let _RECORDS = require('../model/recordmodel');

//record error no
let RD_ERRNO = {
	SUCCESS: 		0,
	REJECT: 		1,
	NOANSWER: 		2
};

function StartNewRecord(from, to){
	let newrecord = {
	};
}

function EndRecord(id, reason){
	//errno is default to be success
	let err = reason || RD_ERRNO.SUCCESS;

	//get record
	let record = _RECORDS.GetRecord(id || -1);
	if (record){
		record.err = reason;
		record.end = new Date().currenttime();
	}
}