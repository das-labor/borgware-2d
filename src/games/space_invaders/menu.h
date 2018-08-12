#ifdef GAME_SPACE_INVADERS
    #if defined(MENU_TAB_VARS)
        extern game_descriptor_t invaders_game_descriptor;
    #elif defined(MENU_TAB_ENTRY)
        &invaders_game_descriptor,
    #endif
#endif
