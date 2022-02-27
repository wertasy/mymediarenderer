#include "mymediarenderer.h"
#include "PltUPnP.h"
#include "NptResults.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

struct Options {
    const char *friendly_name;
} Options;

static void PrintUsageAndExit(char **args) {
    fprintf(stderr, "usage: %s [-f <friendly_name>]\n", args[0]);
    fprintf(stderr, "-f : optional upnp server friendly name\n");
    exit(1);
}

static void ParseCommandLine(char **args) {
    const char *arg;
    char **tmp = args + 1;

    Options.friendly_name = NULL;

    while ((arg = *tmp++)) {
        if (!strcmp(arg, "-f")) {
            Options.friendly_name = *tmp++;
        } else {
            fprintf(stderr, "ERROR: too many arguments\n");
            PrintUsageAndExit(args);
        }
    }
}


void sighandler(int sig) {
    printf("have a nice day!\n");
    exit(0);
}

int main(int /* argc */, char **argv) {
    signal(SIGSTOP, sighandler);
    signal(SIGINT, sighandler);

    // setup Neptune logging
    NPT_LogManager::GetDefault().Configure(
            "plist:.level=INFO;"
            ".handlers=ConsoleHandler;"
            ".ConsoleHandler.colors=on;"
            ".ConsoleHandler.filter=58");

    /* parse command line */
    ParseCommandLine(argv);

    MyMediaRenderer *render = new MyMediaRenderer(
            Options.friendly_name ?
            Options.friendly_name : "My Media Renderer",
            false, "e6572b54-f3c7-2d91-2fb5-b757f2537e21");

    PLT_UPnP upnp;
    PLT_DeviceHostReference device(render);
    upnp.AddDevice(device);
    upnp.Start();

    if (NPT_SUCCESS != render->SetupAVTransportService()) {
        upnp.Stop();
        return 1;
    }

    render->StartUpdateServices();
    upnp.Stop();

    return 0;
}
