#ifdef __x86_64__
extern "C" char ps2_get_keyboard_character(void);
#endif

namespace arch_abstractors {
    char getchar(void) {
        #ifdef __x86_64__
        return ps2_get_keyboard_character();
        #endif
    }
}