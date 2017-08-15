# signaling-server

protocol
```javascript
{
  event: string,
  from: string,
  to: string,
  callid: string,
  sessionid: string,
  data {
    type: string,
    status_code: string ,
    msg: string,
  },
  sdp {
    type: string, //("offer"/"answer")
    sdp: string,
  },
  candidate {
    candidate: string,
    sdpMid: string,
    sdpMLineIndex: int,
  }
}
