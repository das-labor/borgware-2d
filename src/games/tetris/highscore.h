/**
 * \addtogroup tetris
 * @{
 */

/**
 * @file highscore.h
 * @brief Public interface definitions of the high score table input module.
 * @author Michael Holzt, Christian Kroll
 */


#ifndef TETRIS_HIGHSCORE_H_
#define TETRIS_HIGHSCORE_H_

#include "highscore_types.h"
#include "../../compat/eeprom.h"

/**
 * the actual high score table
 */
//extern tetris_highscore_table_t g_highScoreTable EEMEM;


/**
 * lets the user enter his initials (three characters)
 * @return name packed into a uint16_t
 */
uint16_t tetris_highscore_inputName(void);


/**
 * retrieves the high score from storage (EEPROM)
 * @param nIndex the variant dependent index of the high score
 * @return the high score
 */
uint16_t tetris_highscore_retrieveHighScore(tetris_highscore_index_t nIndex);


/**
 * saves the high score into the storage (EEPROM)
 * @param nIdx the variant dependent index of the high score
 * @param nHighscoreName the high score
 */
inline static
void tetris_highscore_saveHighScore(tetris_highscore_index_t nIndex,
                                    uint16_t nHighScore)
{
	if (nHighScore > tetris_highscore_retrieveHighScore(nIndex))
	{
		eeprom_busy_wait();
		eeprom_write_word(EEP_TETRIS + offsetof(tetris_highscore_table_t, nHighScore[nIndex]), nHighScore);
	}
}


/**
 * retrieves the champion's initials from storage (EEPROM)
 * @param nIdx the variant dependent index of the high score
 * @return the initials of the champion packed into a uint16_t
 */
inline static
uint16_t tetris_highscore_retrieveHighScoreName(tetris_highscore_index_t nIdx)
{
	eeprom_busy_wait();
	uint16_t nHighScoreName =
			eeprom_read_word(EEP_TETRIS + offsetof(tetris_highscore_table_t, nHighScoreName[nIdx]));

	return nHighScoreName;
}


/**
 * saves the champion's initials into the storage (EEPROM)
 * @param nIndex the variant dependent index of the high score
 * @param nHighscoreName the initials of the champion packed into a uint16_t
 */
inline static
void tetris_highscore_saveHighScoreName(tetris_highscore_index_t nIndex,
                                        uint16_t nHighscoreName)
{
	eeprom_busy_wait();
	eeprom_write_word(EEP_TETRIS + offsetof(tetris_highscore_table_t, nHighScoreName[nIndex]), nHighscoreName);
}


#endif /*TETRIS_HIGHSCORE_H_*/

/*@}*/
