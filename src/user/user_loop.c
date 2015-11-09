/* This file eases the integration of your personal animations which are not
   meant to be included in the official Borgware-2D repository.

   It is inserted at the middle of the big switch/case block of the
   src/display_loop.c file. Please make sure that you use a mode number
   200<=n<=252 to avoid conflicts with newer upstream animations.

   Just add your header #include directives as shown in the commented
   samples. */

// #ifdef ANIMATION_MY_SIMPLE_ANIM
// 		case 200:
// 		  my_simple_animation();
// 		  break;
// #endif

// #ifdef ANIMATION_MY_COMPLEX_ANIM
// 		case 201:
// 		  my_complex_animation(MY_SAMPLE_INT, MY_SAMPLE_BOOL);
// 		  break;
// #endif

#ifdef ANIMATION_LABOR10THANNIVERSAIRY
 		case 200:
 		  logo_Labor10thAnniversairy();
 		  break;
#endif

