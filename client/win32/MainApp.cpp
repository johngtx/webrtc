#include "stdafx.h"
#include "mainapp.h"
#include <windows.h>
#include <winsock.h>
#include <iostream>
#include <sstream>
#include "webrtc/third_party/jsoncpp/json/json.h"
#include "webrtc/api/test/fakeconstraints.h"
#include "webrtc/media/engine/webrtcvideocapturerfactory.h"
#include "webrtc/modules/video_capture/video_capture_factory.h"
#include "webrtc/base/arraysize.h"
#include "webrtc/base/win32socketinit.h"
#include "webrtc/base/win32socketserver.h"

#pragma comment(lib,"ws2_32.lib")

#define BUFF_LEN			8 * 1024
#define HEAD_LEN			sizeof(UINT32)
SOCKET __CLIENT;

#define MAX_CONNECTION		4

#define PROTO_TLV


class DummySetSessionDescriptionObserver
	: public webrtc::SetSessionDescriptionObserver 
{
public:
	static DummySetSessionDescriptionObserver* Create() 
	{
		return
			new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
	}
	virtual void OnSuccess() 
	{
		std::cout << "SetSession Description success" << std::endl;
	}
	virtual void OnFailure(const std::string& error) 
	{
		std::cerr << "set session description error " << error << std::endl;
	}

protected:
	DummySetSessionDescriptionObserver() {}
	~DummySetSessionDescriptionObserver() {}
};

CMainApp::CMainApp()
	: connect_status_(false)
	, bcalling_(false)
	, id_("")
	, target_id_("")
	, target_callid_("")
	, capturer_(nullptr)
{

}

CMainApp::~CMainApp()
{
	DisConnect();
	peer_connection_factory_ = nullptr;
	peer_connection_ = nullptr;
}

void CMainApp::Init()
{
	InitPeerConnection();
}

void CMainApp::InitClientSocket()
{
	WSADATA Data;
	int status;

	/* initialize the Windows Socket DLL */
	status = WSAStartup(MAKEWORD(1, 1), &Data);
	if (status != 0)
		std::cout << "ERROR: WSAStartup unsuccessful" << std::endl;

	/* create a socket */
	__CLIENT = socket(AF_INET, SOCK_STREAM, 0);
	int error = WSAGetLastError();
	if (__CLIENT == INVALID_SOCKET)
	{
		std::cout << "ERROR: socket unsuccessful" << std::endl;
		status = WSACleanup();
		if (status == SOCKET_ERROR)
			std::cout << "ERROR: WSACleanup unsuccessful" << std::endl;
	}
}

void CMainApp::InitPeerConnection()
{
	//init
	peer_connection_factory_ = webrtc::CreatePeerConnectionFactory();

	if (!peer_connection_factory_.get())
	{
		std::cout << "create peerconnection factory failed!" << std::endl;
		return;
	}
}

void CMainApp::StartLocalRender(webrtc::VideoTrackInterface* video_track)
{
	local_renderer_.reset(
		new CVideoRenderer(local_view_, 1, 1, video_track));
}

void CMainApp::StopLocalRender()
{
	local_renderer_.reset();
	::InvalidateRect(local_view_, NULL, TRUE);
}

void CMainApp::StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video)
{
	remote_renderer_.reset(
		new CVideoRenderer(remote_view_, 1, 1, remote_video) );
}

void CMainApp::StopRemoteRenderer()
{
	remote_renderer_.reset();
	::InvalidateRect(remote_view_, NULL, TRUE);
}

rtc::scoped_refptr<webrtc::PeerConnectionInterface>
CMainApp::CreatePeerConnection()
{
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	webrtc::PeerConnectionInterface::IceServer server;
	server.uri = "turn:192.168.8.254:3100";
	server.username = "yiyun";
	server.password = "123456";
	config.servers.push_back(server);

	webrtc::FakeConstraints constraints;
	constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, true);
	
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer =
		peer_connection_factory_->CreatePeerConnection(
			config, &constraints, NULL, NULL, this
		);
	return peer;
}

void CMainApp::AddStream()
{
	rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
		peer_connection_factory_->CreateAudioTrack(
			"audio", 
			peer_connection_factory_->CreateAudioSource(NULL)));

	rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
		peer_connection_factory_->CreateVideoTrack(
			"video",
			peer_connection_factory_->CreateVideoSource(
				GetVideoCaptureDevice(), NULL)));
	StartLocalRender(video_track);

	rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
		peer_connection_factory_->CreateLocalMediaStream("local_stream");

	stream->AddTrack(audio_track);
	stream->AddTrack(video_track);
	if (!peer_connection_->AddStream(stream)) {
		LOG(LS_ERROR) << "Adding stream to PeerConnection failed";
	}
}

cricket::VideoCapturer* CMainApp::GetVideoCaptureDevice()
{
	std::vector<std::string> device_names;
	{
		std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
			webrtc::VideoCaptureFactory::CreateDeviceInfo(0));
		if (!info) 
		{
			return nullptr;
		}
		int num_devices = info->NumberOfDevices();
		for (int i = 0; i < num_devices; ++i) 
		{
			const uint32_t kSize = 256;
			char name[kSize] = { 0 };
			char id[kSize] = { 0 };
			if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
				device_names.push_back(name);
			}
		}
	}

	cricket::WebRtcVideoDeviceCapturerFactory factory;
	cricket::VideoCapturer* capturer = nullptr;
	for (int i = device_names.size() - 1; i >= 0; --i)
	{
		capturer = factory.Create(cricket::Device(device_names[i], i));
		if (capturer)
		{
			break;
		}
	}
	return capturer;
}

int32_t CMainApp::ConnectToServer(char* sig, int32_t sport, char* turn, int32_t tport)
{
	InitClientSocket(); 
	std::cout << "SignalingServer: " << sig << ":" << sport << std::endl
		<< "TurnServer: "<< turn << ":" << tport << std::endl;
	SOCKADDR_IN destSockAddr;
	unsigned long destAddr;
	/* convert IP address into in_addr form */
	destAddr = inet_addr(sig);
	/* copy destAddr into sockaddr_in structure */
	memcpy(&destSockAddr.sin_addr, &destAddr, sizeof(destAddr));
	/* specify the port portion of the address */
	destSockAddr.sin_port = htons(sport);
	/* specify the address family as Internet */
	destSockAddr.sin_family = AF_INET;
	/* connect to the server */
	int status = connect(__CLIENT, (LPSOCKADDR)&destSockAddr, sizeof(destSockAddr));
	if (status == SOCKET_ERROR)
	{
		std::cout << "ERROR: connect unsuccessful" << std::endl;
		status = closesocket(__CLIENT);
		if (status == SOCKET_ERROR)
			std::cout << "ERROR: closesocket unsuccessful" << std::endl;
		status = WSACleanup();
		if (status == SOCKET_ERROR)
			std::cout << "ERROR: WSACleanup unsuccessful" << std::endl;
	} 
	else
	{
		std::cout << "connect to server" << std::endl;
		//start workthread
		connect_status_ = true;
		work_thread_ = std::thread(&CMainApp::WorkThreadProcess, this);
	}
	return 1;
}

int32_t CMainApp::DisConnect()
{
	connect_status_ = false;
	work_thread_.join();
	if (__CLIENT != INVALID_SOCKET)
	{
		closesocket(__CLIENT);
		__CLIENT = INVALID_SOCKET;
	}
	WSACleanup();
	return 0;
}

int32_t CMainApp::GetState()
{
	return 1;
}

int32_t CMainApp::Register(char* id)
{
	SIG_MSG_DATA msg;
	msg.event = IO_EVENT_REGISTER;
	msg.from = id;
	msg.to = id;
	
	id_ = id;
	std::string str = ConvertMsgDataToBuff(msg);
	int ret = SendBuff(str);
	return ret;
}

int32_t CMainApp::UnRegister()
{
	SIG_MSG_DATA msg;
	msg.event = IO_EVENT_UNREGISTER;
	msg.from = id_;
	msg.to = id_;

	std::string str = ConvertMsgDataToBuff(msg);
	int ret = SendBuff(str);
	return ret;
}

int32_t CMainApp::Answer()
{
	if (!bcalling_ || peer_connection_.get() == nullptr)
	{
		std::cout << "havent got an offer" << std::endl;
		return 0;
	}

	//add local stream
	AddStream();

	//create answer
	peer_connection_->CreateAnswer(this, NULL);

	//send answer sig
	SIG_MSG_DATA msg;
	msg.event = IO_EVENT_ANSWER;
	msg.from = id_;
	msg.to = target_id_;
	msg.callid = target_callid_;

	std::string str = ConvertMsgDataToBuff(msg);
	int ret = SendBuff(str);
	bcalling_ = true;
	return ret;
}

int32_t CMainApp::Reject()
{
	SIG_MSG_DATA msg;
	msg.event = IO_EVENT_REJECT; 
	msg.from = id_;
	msg.to = target_id_;
	msg.callid = target_callid_;

	std::string str = ConvertMsgDataToBuff(msg);
	int ret = SendBuff(str);

	peer_connection_ = nullptr;
	bcalling_ = false;
	return ret;
}

int32_t CMainApp::Offer(char* dst, char* callid)
{
	target_id_ = dst;
	target_callid_ = callid;
	//TODO
	peer_connection_ = nullptr;
	peer_connection_ = CreatePeerConnection();
	if (peer_connection_.get() == nullptr)
	{
		std::cout << "create peerconnection failed" << std::endl;
		peer_connection_ = nullptr;
		return -1;
	}
	AddStream();
	peer_connection_->CreateOffer(this, NULL);
	bcalling_ = true;

	return 1;
}

int32_t CMainApp::Bye()
{
	SIG_MSG_DATA msg;
	msg.event = IO_EVENT_BYE;
	msg.from = id_;
	msg.to = target_id_;
	msg.callid = target_callid_;

	std::string str = ConvertMsgDataToBuff(msg);
	int ret = SendBuff(str);

	StopRemoteRenderer();
	StopLocalRender();
	peer_connection_ = nullptr;
	bcalling_ = false;
	return 1;
}

//event
void CMainApp::onEventAnswer(SIG_MSG_DATA data)
{
	std::cout << "Event answer: from ... " << data.from;
	std::cout << " >>> to ... " << data.to << std::endl;

	if (!bcalling_)
	{
		std::cout << "not calling state, error answer" 
			<< std::endl;
		return;
	}
	//TODO
	//set remote sdp
	webrtc::SdpParseError error;
	webrtc::SessionDescriptionInterface* session_description(
		webrtc::CreateSessionDescription(
			data.data.sdp.type, data.data.sdp.sdp, &error
		)
	);
	if (!session_description)
	{
		std::cout << __FUNCTION__ << " canot parse remote sdp:"
			<< error.description;
	}

	peer_connection_->SetRemoteDescription(
		DummySetSessionDescriptionObserver::Create(),
		session_description
	);
}

void CMainApp::onEventOffer(SIG_MSG_DATA data)
{
	std::cout << "Event call: from ... " << data.from;
	std::cout << " >>> to ... " << data.to << std::endl;
	if (bcalling_)
	{
		std::cout << "is already calling ... " << std::endl;
		return;
	}

	target_id_ = data.from;
	target_callid_ = data.callid;

	//set remote sdp
	webrtc::SdpParseError error;
	webrtc::SessionDescriptionInterface* session_description(
		webrtc::CreateSessionDescription(
			data.data.sdp.type, data.data.sdp.sdp, &error
		)
	);
	if (!session_description)
	{
		std::cout << __FUNCTION__ << " canot parse remote sdp:"
			<< error.description;
	}

	peer_connection_ = nullptr;
	peer_connection_ = CreatePeerConnection();
	peer_connection_->SetRemoteDescription(
		DummySetSessionDescriptionObserver::Create(),
		session_description
	);

	bcalling_ = true;
}

void CMainApp::onEventReject(SIG_MSG_DATA data)
{
	std::cout << "Event reject: from ... " << data.from;
	std::cout << " >>> to ... " << data.to << std::endl;
	peer_connection_ = nullptr;
	bcalling_ = false;
}

void CMainApp::onEventBye(SIG_MSG_DATA data)
{
	std::cout << "Event bye: from ... " << data.from;
	std::cout << " >>> to ... " << data.to << std::endl;
	StopRemoteRenderer();
	StopLocalRender();
	peer_connection_ = nullptr;
	bcalling_ = false;
}

void CMainApp::onEventCandidate(SIG_MSG_DATA data)
{
	std::cout << "Event candidate: from ... " << data.from;
	std::cout << " >>> to ... " << data.to << std::endl;
	if (peer_connection_.get() == nullptr)
	{
		std::cout << "peer connection error, nullptr" << std::endl;
		return;
	}

	//TODO
	webrtc::SdpParseError err;
	std::unique_ptr<webrtc::IceCandidateInterface> candidate(
		webrtc::CreateIceCandidate(
			data.data.candidate.sdpMid,
			data.data.candidate.sdpMLineIndex,
			data.data.candidate.candidate,
			&err
		)
	);

	peer_connection_->AddIceCandidate(candidate.get());
}

void CMainApp::onEventResponse(SIG_MSG_DATA data)
{
	std::cout << "Event response: from ... " << data.from;
	std::cout << " >>> to ... " << data.to << std::endl;
	std::cout << "status code:" << data.data.status_code
		<< "\tmsg:" << data.data.msg
		<< "\ttype:" << data.data.type
		<< std::endl;
}

//recv thread
void CMainApp::WorkThreadProcess(CMainApp* observer)
{
	std::cout << "workthread start ... " << std::endl;
	if (!observer || __CLIENT == INVALID_SOCKET) 
	{
		return;
	}

	//select
	fd_set		fd_write;
	struct timeval tv = { 1, 0 };

	//buff process
	char buff[BUFF_LEN];
	char package[BUFF_LEN];
	int32_t require_len = HEAD_LEN;
	int32_t offset = 0;
	int32_t offset2 = 0;
	int32_t head = 0;
	int32_t bufflen = 0;
	bool newhead = true;

	memset(buff, 0, BUFF_LEN);
	memset(package, 0, BUFF_LEN);
	while (observer->connect_status_ &&
		__CLIENT != INVALID_SOCKET)
	{
		FD_ZERO(&fd_write);
		FD_SET(__CLIENT, &fd_write);
		int ret = ::select(0, &fd_write, NULL, NULL, &tv);
		if (ret == 0)
		{
			//time expired
			continue;
		}

		if (FD_ISSET(__CLIENT, &fd_write))
		{
			memset(buff, 0, BUFF_LEN);
			offset2 = 0;
			bufflen = ::recv(__CLIENT, buff, BUFF_LEN, 0);
			if (bufflen == 0 || bufflen == SOCKET_ERROR &&
				::WSAGetLastError() == WSAECONNRESET)
			{
				std::cout << "client socket closed" << std::endl;
				return;
			}
			else
			{
				//dispatch package
				while (bufflen > 0)
				{
					if (newhead)
					{
						if (require_len > bufflen)
						{
							memcpy(&head + offset, buff, bufflen);
							offset += bufflen;
							require_len -= bufflen;
							break;
						}
						else
						{
							memcpy(&head + offset, buff, require_len);
							head = ntohl(head);
							if (head < 0 || head > BUFF_LEN)
							{
								std::cout << "error head len\t" << head << std::endl;
								return;
							}
							bufflen -= require_len;
							offset = 0;
							newhead = false;
							offset2 = require_len;
							require_len = head;
						}
					}
					else
					{
						if (require_len > bufflen)
						{
							memcpy(package + offset, buff + offset2, bufflen);
							offset += bufflen;
							require_len -= bufflen;
							break;
						}
						else
						{
							memcpy(package + offset, buff + offset2, require_len);
							offset2 += require_len;
							bufflen -= require_len;
							offset = 0;
							require_len = HEAD_LEN;
							newhead = true;

							//dispatch package
							SIG_MSG_DATA data = observer->ConvertBuffToMsgData(package);
							observer->MsgDispatch(data);
						}
					}
				}
			}
		}
	}
}

SIG_MSG_DATA CMainApp::ConvertBuffToMsgData(const char* buff)
{
	Json::Reader reader;
	Json::Value title;
	Json::Value root;
	Json::Value data;
	Json::Value sdp;
	Json::Value condidate;
	SIG_MSG_DATA msg;
	reader.parse(buff, title, false);
	root = title["data"];
	data = root["data"];
	sdp = data["sdp"];
	condidate = data["condidate"];

	msg.event = root["event"].asString();
	msg.from = root["from"].asString();
	msg.to = root["to"].asString();
	msg.callid = root["callid"].asString();
	msg.sessionid = root["sessionid"].asString();
	msg.data.type = data["type"].asString();
	msg.data.status_code = data["status_code"].asString();
	msg.data.msg = data["msg"].asString();
	msg.data.sdp.type = sdp["type"].asString();
	msg.data.sdp.sdp = sdp["sdp"].asString();
	msg.data.candidate.candidate = condidate["candidate"].asString();
	msg.data.candidate.sdpMid = condidate["sdpMid"].asString();
	msg.data.candidate.sdpMLineIndex = condidate["sdpMLineIndex"].asInt();

	return msg;
}

std::string CMainApp::ConvertMsgDataToBuff(const SIG_MSG_DATA& msg)
{
	Json::Value title;
	Json::Value root;
	Json::Value data;
	Json::Value sdp;
	Json::Value candidate;

	candidate["candidate"] = msg.data.candidate.candidate;
	candidate["sdpMid"] = msg.data.candidate.sdpMid;
	candidate["sdpMLineIndex"] = msg.data.candidate.sdpMLineIndex;
	sdp["type"] = msg.data.sdp.type;
	sdp["sdp"] = msg.data.sdp.sdp;
	data["type"] = msg.data.type;
	data["status_code"] = msg.data.status_code;
	data["msg"] = msg.data.msg;
	data["sdp"] = sdp;
	data["candidate"] = candidate;
	root["event"] = msg.event;
	root["from"] = msg.from;
	root["to"] = msg.to;
	root["callid"] = msg.callid;
	root["sessionid"] = msg.sessionid;
	root["data"] = data;
	title["title"] = "message";
	title["data"] = root;

	std::string str = title.toStyledString();
	return str;
}

int CMainApp::SendBuff(const std::string& msg)
{
	if (__CLIENT == INVALID_SOCKET)
	{
		return -1;
	}

	char buf[BUFF_LEN];
	size_t len = msg.length();
	size_t nlen = htonl(len);
	memset(buf, 0, BUFF_LEN);
	memcpy(buf, &nlen, 4);
	memcpy(buf + 4, msg.c_str(), len);
	//check head
	int a = 0;
	memcpy(&a, buf, 4);

	int ret = ::send(__CLIENT, buf, len + HEAD_LEN, 0);
	int err = ::WSAGetLastError();
	if (ret == 0)
	{
		return ret;
	}

	std::cout << "send buff:" << ret 
		<< "\tstrlen:" << msg.length()
		<< std::endl;
	return 1;
}

void CMainApp::MsgDispatch(const SIG_MSG_DATA& msg)
{
	if (msg.event == IO_EVENT_RESPONSE)
	{
		onEventResponse(msg);
	}
	else if (msg.event == IO_EVENT_OFFER)
	{
		onEventOffer(msg);
	}
	else if (msg.event == IO_EVENT_ANSWER)
	{
		onEventAnswer(msg);
	}
	else if (msg.event == IO_EVENT_REJECT)
	{
		onEventReject(msg);
	}
	else if (msg.event == IO_EVENT_BYE)
	{
		onEventBye(msg);
	}
	else if (msg.event == IO_EVENT_CANDIDATE)
	{
		onEventCandidate(msg);
	}
	else
	{
		std::cout << msg.event << std::endl;
		std::cout << "unknow io event" << std::endl;
	}
}

// PeerConnectionObserver implementation.
void CMainApp::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
	std::cout << __FUNCTION__ << " " << stream->label() << std::endl;
	std::cout << "on remote add stream" << std::endl;
	//remote stream on
	//TODO
	StartRemoteRenderer(stream->GetVideoTracks()[0]);
}

void CMainApp::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
	std::cout << __FUNCTION__ << " " << stream->label();
	//remote stream off
	//TODO
	StopRemoteRenderer();
	stream->Release();
}

void CMainApp::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
	std::cout << __FUNCTION__ << " " << candidate->sdp_mline_index() << std::endl;

	//send ice candidate message to signaling server
	//TODO
	std::string sdp;
	candidate->ToString(&sdp);

	SIG_MSG_DATA msg;
	msg.event = IO_EVENT_CANDIDATE;
	msg.from = id_;
	msg.to = target_id_;
	msg.callid = target_callid_;
	msg.data.candidate.candidate = sdp;
	msg.data.candidate.sdpMid = candidate->sdp_mid();
	msg.data.candidate.sdpMLineIndex = candidate->sdp_mline_index();

	std::string buff = ConvertMsgDataToBuff(msg);
	SendBuff(buff);
}

// CreateSessionDescriptionObserver implementation.
void CMainApp::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
	std::string sdp;
	desc->ToString(&sdp);

	std::cout << "local sdp create success:" << std::endl;
	//set local sdp
	peer_connection_->SetLocalDescription(
		DummySetSessionDescriptionObserver::Create(), desc
	);

	if (bcalling_)
	{
		return;
	}

	//send offer to signaling server
	SIG_MSG_DATA msg;
	msg.event = IO_EVENT_OFFER;
	msg.from = id_;
	msg.to = target_id_;
	msg.callid = target_callid_;
	msg.data.sdp.type = "offer";
	msg.data.sdp.sdp = sdp;

	std::string str = ConvertMsgDataToBuff(msg);
	SendBuff(str);
}

void CMainApp::OnFailure(const std::string& error)
{
	std::cout << "create local sdp fail: " << error << std::endl;
}