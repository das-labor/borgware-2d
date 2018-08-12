#ifdef GAME_BREAKOUT
    #if defined(MENU_TAB_VARS)
        extern game_descriptor_t breakout_game_descriptor;
    #elif defined(MENU_TAB_ENTRY)
        &breakout_game_descriptor,
    #endif
#endif
