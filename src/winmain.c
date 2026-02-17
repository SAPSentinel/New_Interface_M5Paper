#include <windows.h>

// Forward declarations
void setup(void);
void loop(void);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    
    setup();
    while (1) {
        loop();
    }
    return 0;
}
