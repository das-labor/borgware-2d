#ifndef TETRIS_HIGHSCORE_TYPES_H_
#define TETRIS_HIGHSCORE_TYPES_H_

#include <stdint.h>
#include <stddef.h>


/**
 * indexes for different tetris variants
 */
enum tetris_highscore_index_e
{
	TETRIS_HISCORE_TETRIS, /**< high score index for the standard variant */
	TETRIS_HISCORE_BASTET, /**< high score index for the bastet variant */
	TETRIS_HISCORE_FP,     /**< high score index for the first person variant */
	TETRIS_HISCORE_PAD,    /**< don't use (padding for an even array boundary)*/
	TETRIS_HISCORE_END     /**< boundary for the high score array */
};
#ifdef NDEBUG
	typedef uint8_t tetris_highscore_index_t;
#else
	typedef enum tetris_highscore_index_e tetris_highscore_index_t;
#endif


/**
 * type for global high score table
 */
typedef struct tetris_highscore_table_s
{
	uint16_t nHighScore[TETRIS_HISCORE_END];     /**< actual high scores */
	uint16_t nHighScoreName[TETRIS_HISCORE_END]; /**< champions' initials */
}
tetris_highscore_table_t;


#endif // TETRIS_HIGHSCORE_TYPES_H_
