#ifndef COLORSCHEME_H
#define COLORSCHEME_H

#include "Color.h"


// Colors (saturated)
extern Color black;				// Black
extern Color red;				// Red
extern Color green;				// Green
extern Color yellow;			// Yellow
extern Color blue;				// Blue
extern Color magenta;			// Magenta
extern Color cyan;				// Cyan
extern Color white;				// White

#if 0 //these do not work in all the windows console. 
extern Color bright_black;      // Bright Black (Gray)
extern Color bright_red;        // Bright Red
extern Color bright_green;      // Bright Green
extern Color bright_yellow;     // Bright Yellow
extern Color bright_blue;       // Bright Blue
extern Color bright_magenta;    // Bright Magenta
extern Color bright_cyan;       // Bright Cyan
extern Color bright_white;      // Bright White
#endif //0

extern Color white_pure;

//Colors used by the strings. 
extern Color header_color;
extern Color path_color;
extern Color sucesfullprocess_color;
extern Color done_color;


void InitColorCmd();


#endif //COLORSCHEME_H