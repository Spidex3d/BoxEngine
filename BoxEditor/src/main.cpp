#include <BoxEngine.h>
#include <BoxWindow.h>
#include <App.h>
#include <miniBoxLog.h>


int main() {

    BOX_LOG_INFO("Hello from the BoxEditor!");

    BoxEngine boxEngine;
    App app;

    if (!app.Init())
    {
        BOX_LOG_DEBUG("Failed to initialize BoxEditor");
        return -1;
    }

    boxEngine.testFunction();

    return app.Run();
}