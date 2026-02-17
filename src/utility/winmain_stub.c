// WinMain shim to satisfy MinGW's requirement when linking GUI subsystem.
#ifdef _WIN32
#include <windows.h>

// Forward declaration of the user's main
int main(int argc, char **argv);

// MinGW provides __argc and __argv
extern int __argc;
extern char **__argv;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;
    return main(__argc, __argv);
}
#endif
