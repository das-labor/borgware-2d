#if defined(MENU_TAB_VARS)
    #ifdef GAME_TETRIS
        extern game_descriptor_t tetris_game_descriptor;
    #endif
    #ifdef GAME_TETRIS_FP
        extern game_descriptor_t tetrisfp_game_descriptor;
    #endif 
    #ifdef GAME_BASTET
        extern game_descriptor_t bastet_game_descriptor;
    #endif
#elif defined(MENU_TAB_ENTRY)
    #ifdef GAME_TETRIS
        &tetris_game_descriptor,
    #endif 
    #ifdef GAME_TETRIS_FP
        &tetrisfp_game_descriptor,
    #endif 
        #ifdef GAME_TETRIS_FP
        &bastet_game_descriptor,
    #endif 
#endif
