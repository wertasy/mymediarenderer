#include "mymediarenderer.h"

#include "NptLogging.h"
#include "NptResults.h"
#include "NptThreads.h"
#include "PltUPnP.h"

#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <vlcpp/vlc.hpp>

NPT_SET_LOCAL_LOGGER("MyMediaRenderer")

NPT_Result MyMediaRenderer::OnSetAVTransportURI(PLT_ActionReference &action) {
    NPT_AutoLock lock(mutex);

    NPT_LOG_INFO("MyMediaRenderer::OnSetAVTransportURI");

    NPT_String currentURI;
    NPT_String currentURIMetadata;

    action->GetArgumentValue("CurrentURI", currentURI);
    action->GetArgumentValue("CurrentURIMetaData", currentURIMetadata);

    service->SetStateVariable("NumberOfTracks", "0");
    service->SetStateVariable("AVTransportURI", currentURI.GetChars());
    service->SetStateVariable("AVTransportURIMetadata", currentURIMetadata.GetChars());

    try {
        auto media = VLC::Media(instance, currentURI.GetChars(), VLC::Media::FromLocation);
        mediaPlayer.setMedia(media);
        NPT_LOG_INFO_1("set meida for media player: %s", currentURI.GetChars());
    } catch (std::exception& e) {
        NPT_LOG_INFO_1("%s", e.what());
    }

    mediaPlayer.setFullscreen(true);

    return NPT_SUCCESS;
}

NPT_Result MyMediaRenderer::OnPlay(PLT_ActionReference &action) {
    NPT_AutoLock lock(mutex);
    NPT_LOG_INFO("MyMediaRenderer::OnPlay");

    NPT_String speed;
    float rate;
    action->GetArgumentValue("Speed", speed);
    sscanf(speed.GetChars(), "%f", &rate);

    mediaPlayer.setRate(rate);
    NPT_LOG_INFO_1("MyMediaRenderer::OnPlay - speed %f", rate);

    if (mediaPlayer.isPlaying()) {
        return NPT_SUCCESS;
    }  

    mediaPlayer.play();
    service->SetStateVariable("TransportState", "PLAYING");
    NPT_LOG_INFO("TransportState PLAYING");

    return NPT_SUCCESS;
}

NPT_Result MyMediaRenderer::OnGetVolumeDBRange(PLT_ActionReference &action) {
    return NPT_SUCCESS;
}

NPT_Result MyMediaRenderer::OnPause(PLT_ActionReference &action) {
    NPT_AutoLock lock(mutex);

    mediaPlayer.pause();

    if (!mediaPlayer.isPlaying()) {
        service->SetStateVariable("TransportState", "PAUSED_PLAYBACK");
        NPT_LOG_INFO("TransportState PAUSED_PLAYBACK");

        return NPT_SUCCESS;
    }

    service->SetStateVariable("TransportState", "PLAYING");
    NPT_LOG_INFO("TransportState PLAYING");

    return NPT_SUCCESS;
}

NPT_Result MyMediaRenderer::OnNext(PLT_ActionReference &action) {
    return NPT_SUCCESS;
}

NPT_Result MyMediaRenderer::OnPrevious(PLT_ActionReference &action) {
    return NPT_SUCCESS;
}

float parse_time(const char *time_str) {
    float h, m, s;
    if (sscanf(time_str, "%2lld:%02lld:%02lld", &h, &m, &s) < 3) {
        return -1;
    }
    return (h * 3600LL + m * 60LL + s) * 1000LL;
}

NPT_Result MyMediaRenderer::OnSeek(PLT_ActionReference &action) {
    NPT_AutoLock lock(mutex);
    NPT_LOG_INFO("OnSeek");

    NPT_String unit;
    action->GetArgumentValue("Unit", unit);
    NPT_LOG_INFO_1("Unit: %s\n", unit.GetChars());

    NPT_String target;
    action->GetArgumentValue("Target", target);
    NPT_LOG_INFO_1("Target: %s\n", target.GetChars());

    float current = parse_time(target.GetChars());
    float total = mediaPlayer.length();

    float pos = current / total;
    NPT_LOG_INFO_1("Pos: %f\n", pos);

    mediaPlayer.setPosition(pos);

    return NPT_SUCCESS;
}

NPT_Result MyMediaRenderer::OnStop(PLT_ActionReference &action) {
    NPT_AutoLock lock(mutex);
    NPT_LOG_INFO("MyMediaRenderer::OnStop");

    return stop();
}

NPT_Result MyMediaRenderer::SetupAVTransportService() {
    if (NPT_ERROR_NO_SUCH_ITEM == FindServiceByName("AVTransport", service)) {
        return NPT_FAILURE;
    }
    return NPT_SUCCESS;
}

NPT_Result MyMediaRenderer::stop() {
    service->SetStateVariable("TransportState", "STOPPED");
    mediaPlayer.stop();
    return NPT_SUCCESS;
}

NPT_Result MyMediaRenderer::OnSetPlayMode(PLT_ActionReference &action) {
    NPT_AutoLock lock(mutex);
    NPT_LOG_INFO("OnSetPlayMode");

    NPT_String instanceId;
    NPT_String newPlayMode;

    action->GetArgumentValue("InstanceID", instanceId);
    action->GetArgumentValue("NewPlayMode", newPlayMode);

    NPT_LOG_INFO_1("InstanceID: %s", instanceId.GetChars());
    NPT_LOG_INFO_1("NewPlayMode: %s", newPlayMode.GetChars());

    return NPT_SUCCESS;
}

NPT_Result MyMediaRenderer::OnSetVolume(PLT_ActionReference &action) {
    NPT_AutoLock lock(mutex);
    NPT_LOG_INFO("OnSetVolume");

    NPT_String instanceId;
    action->GetArgumentValue("InstanceID", instanceId);

    NPT_String channel;
    action->GetArgumentValue("Channel", channel);

    NPT_Int32 desiredVolume;
    action->GetArgumentValue("DesiredVolume", desiredVolume);
    mediaPlayer.setVolume(desiredVolume);

    return NPT_SUCCESS;
}

int fmttime(float time, char *outbuf) {
    uint64_t s = time / 1000LL;
    return sprintf(outbuf, "%02lld:%02lld:%02lld", s / 3600LL,
            s / 60LL % 60LL, s % 60LL);
}

NPT_Result MyMediaRenderer::Update() {
    libvlc_state_t stat = mediaPlayer.state();
    switch (stat) {
        case libvlc_Ended:
            return stop();
        case libvlc_Paused:
            return NPT_SUCCESS;
        case libvlc_Playing:
            break;
        default:
            return NPT_SUCCESS;
    }

    libvlc_time_t duration = mediaPlayer.length();
    if (duration == -1) {
        NPT_LOG_INFO_1("time: %lld", duration);
        return NPT_FAILURE;
    }

    char buf[256];
    fmttime(duration, buf);
    service->SetStateVariable("CurrentMediaDuration", buf);
    service->SetStateVariable("CurrentTrackDuration", buf);
    // NPT_LOG_INFO_2("CurrentMediaDuration: %s(%lld)", buf, duration);

    float pos = mediaPlayer.position();
    float current = duration * pos;
    fmttime(current, buf);
    service->SetStateVariable("RelativeTimePosition", buf);
    // NPT_LOG_INFO_2("RelativeTimePosition: %s(%lld)", buf, current);

    return NPT_SUCCESS;
}

NPT_Result MyMediaRenderer::StartUpdateServices() {
    for (;;) {
        Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return NPT_SUCCESS;
}
