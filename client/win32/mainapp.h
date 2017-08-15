#ifndef SIGNALING_H
#define SIGNALING_H

#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <windows.h>

#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"

#include "VideoRender.h"

//struct
typedef struct
{
	std::string		type;
	std::string		status_code;
	std::string		msg;
	struct 
	{
		std::string		type;
		std::string		sdp;
	} sdp;
	struct  
	{
		std::string		candidate;
		std::string		sdpMid;
		int				sdpMLineIndex;
	} candidate;
}SIG_DATA;


typedef struct
{
	std::string		event;
	std::string		from;
	std::string		to;
	std::string		callid;
	std::string		sessionid;
	SIG_DATA		data;
}SIG_MSG_DATA;

//event define
#define IO_EVENT_REGISTER		"_register"
#define IO_EVENT_UNREGISTER		"_unregister"
#define IO_EVENT_OFFER			"_offer"
#define IO_EVENT_ANSWER			"_answer"
#define IO_EVENT_REJECT			"_reject"
#define IO_EVENT_CANCLE			"_cancel"
#define IO_EVENT_RESPONSE		"_response"
#define IO_EVENT_CANDIDATE		"_ice_candidate"
#define IO_EVENT_BYE			"_bye"
#define IO_EVENT_JOINROOM		"_join"
#define IO_EVENT_LEAVEROOM		"_leave"

namespace webrtc {
	class VideoCaptureModule;
}  // namespace webrtc

namespace cricket {
	class VideoRenderer;
}  // namespace cricket

class CMainApp :
	public webrtc::PeerConnectionObserver,
	public webrtc::CreateSessionDescriptionObserver
{
public:
	CMainApp();
	~CMainApp();

	//init
	void Init();
	void InitClientSocket();
	void InitPeerConnection();

	//render
	inline void setLocalView(HWND view) { local_view_ = view; }
	inline void setRemoteView(HWND view) { remote_view_ = view; }
	void StartLocalRender(webrtc::VideoTrackInterface* video_track);
	void StopLocalRender();
	void StartRemoteRenderer(webrtc::VideoTrackInterface* video_track);
	void StopRemoteRenderer();

	//signaling
	int32_t ConnectToServer(char* sig, int32_t sport, char* turn, int32_t tport);
	int32_t DisConnect();
	int32_t GetState();
	int32_t Register(char* id);
	int32_t UnRegister();
	int32_t Answer();
	int32_t Reject();
	int32_t Offer(char* dst, char* callid);
	int32_t Bye();

	//event
	void onEventResponse(SIG_MSG_DATA);
	void onEventOffer(SIG_MSG_DATA);
	void onEventReject(SIG_MSG_DATA);
	void onEventAnswer(SIG_MSG_DATA);
	void onEventBye(SIG_MSG_DATA);
	void onEventCandidate(SIG_MSG_DATA);

	// PeerConnectionObserver implementation.
	void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {};
	void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
	void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
	void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
	void OnRenegotiationNeeded() override {}
	void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {};
	void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {};
	void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
	void OnIceConnectionReceivingChange(bool receiving) override {}

	// CreateSessionDescriptionObserver implementation.
	void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
	void OnFailure(const std::string& error) override;
	int AddRef() const override  { return 1; };
	int Release() const override { return 1; };

protected:
	//convert
	SIG_MSG_DATA ConvertBuffToMsgData(const char* buff);
	std::string ConvertMsgDataToBuff(const SIG_MSG_DATA&);
	//thread process
	static void WorkThreadProcess(CMainApp* obs);
	//send buff
	int SendBuff(const std::string& msg);
	void MsgDispatch(const SIG_MSG_DATA& msg);
	//get capturedevice
	cricket::VideoCapturer* GetVideoCaptureDevice();
	rtc::scoped_refptr<webrtc::PeerConnectionInterface>
	CreatePeerConnection();
	void AddStream();

private:
	//signaling
	bool			connect_status_;
	bool			bcalling_;
	std::thread		work_thread_;
	std::mutex		package_mtx_;
	std::condition_variable package_cv_;
	std::string		id_;
	std::string		target_id_;
	std::string		target_callid_;
	std::string     target_sdp_;
	std::string		target_sdp_type_;

	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
		peer_connection_factory_;
	rtc::scoped_refptr<webrtc::PeerConnectionInterface>
		peer_connection_;
	cricket::VideoCapturer*	capturer_;

	//render
	std::unique_ptr<CVideoRenderer> local_renderer_;
	std::unique_ptr<CVideoRenderer> remote_renderer_;

	HWND	local_view_;
	HWND	remote_view_;
};

#endif // SIGNALING_H