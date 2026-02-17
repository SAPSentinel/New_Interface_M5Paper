#include "lvgl.h"
#include "gui/gui.h"
#include "gui/app.h"

#include "hardware/hardware.h"
#include "hardware/powermgm.h"

#if defined( NATIVE_64BIT )
    #include <cstdio>
    #include <cstdint>
    /**
     * for non arduino
     */                 
    void setup( void );
    void loop( void );

    int main_impl( void ) {
        setup();
        while( 1 ) { loop(); };
        return( 0 );
    }
    
    // Windows entry point for MinGW
    #ifdef _WIN32
        #include <windows.h>
        int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
        {
            (void)hInstance;
            (void)hPrevInstance;
            (void)lpCmdLine;
            (void)nCmdShow;
            return main_impl();
        }
    #else
        int main( void ) {
            return main_impl();
        }
    #endif
#endif // NATIVE_64BIT

void setup() {
    /**
     * hardware setup
     */
    hardware_setup();
    /**
     * gui setup
     */
    gui_setup();
    /**
     * apps autocall setup
     */
    app_autocall_all_setup_functions();
    /**
     * post hardware setup
     */
    hardware_post_setup();
}

void loop(){
    static uint32_t last_print = 0;
    uint32_t now = lv_tick_get();
    
    powermgm_loop();
    
    // Print status every 3 seconds
    if (now - last_print > 3000) {
        printf("[M5Paper Emulator] Running... tick=%u\n", now);
        fflush(stdout);
        last_print = now;
    }
}
