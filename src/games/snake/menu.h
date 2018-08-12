#ifdef GAME_KART
    #if defined(MENU_TAB_VARS)
        extern game_descriptor_t snake_game_descriptor;
    #elif defined(MENU_TAB_ENTRY)
        &snake_game_descriptor,
    #endif
#endif