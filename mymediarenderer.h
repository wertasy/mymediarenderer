#include "NptThreads.h"
#include "PltMediaRenderer.h"

#include <vlcpp/vlc.hpp>

class PLT_MediaRenderer;

class MyMediaRenderer : public PLT_MediaRenderer {
    public:
        MyMediaRenderer(
                const char     *friendly_name,
                bool            show_ip = false,
                const char     *uuid    = nullptr,
                unsigned int    port    = 0,
                bool            port_rebind = false)
            : PLT_MediaRenderer(
                    friendly_name,
                    show_ip,
                    uuid,
                    port,
                    port_rebind),
            instance(0, nullptr),
            mediaPlayer(instance) {}

        ~MyMediaRenderer() {}

        // ConnectionManager
        // NPT_Result OnGetCurrentConnectionInfo(PLT_ActionReference &action);
        // AVTransport
        NPT_Result OnNext(PLT_ActionReference &action);
        NPT_Result OnPause(PLT_ActionReference &action);
        NPT_Result OnPlay(PLT_ActionReference &action);
        NPT_Result OnPrevious(PLT_ActionReference &action);
        NPT_Result OnSeek(PLT_ActionReference &action);
        NPT_Result OnStop(PLT_ActionReference &action);
        NPT_Result OnSetAVTransportURI(PLT_ActionReference &action);
        NPT_Result OnSetPlayMode(PLT_ActionReference &action);
        NPT_Result OnSetVolume(PLT_ActionReference &action);
        // NPT_Result OnSetVolumeDB(PLT_ActionReference &action);
        NPT_Result OnGetVolumeDBRange(PLT_ActionReference &action);
        // NPT_Result OnSetMute(PLT_ActionReference &action);

        NPT_Result StartUpdateServices();
        NPT_Result Update();

        NPT_Result SetupAVTransportService();

    private:
        NPT_Result            stop();

        NPT_Mutex             mutex;
        PLT_Service          *service;
        VLC::Instance         instance;
        VLC::MediaPlayer      mediaPlayer;
};
