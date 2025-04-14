#include "colorscheme.h"
#include "platform.h"

// We will use the next color format in the tools.
//	- Green:	process done.
//	- Red:		process failed.
//	- Cyan: Only for header strings.

// +--------------------+------------------+
// |       Color        | ANSI Code        |
// +--------------------+------------------+
// | Black              | \033[30m         |
// | Red                | \033[31m         |
// | Green              | \033[32m         |
// | Yellow             | \033[33m         |
// | Blue               | \033[34m         |
// | Magenta            | \033[35m         |
// | Cyan               | \033[36m         |
// | White              | \033[37m         |
// +--------------------+------------------+
// | Bright Black       | \033[90m         |
// | Bright Red         | \033[91m         |
// | Bright Green       | \033[92m         |
// | Bright Yellow      | \033[93m         |
// | Bright Blue        | \033[94m         |
// | Bright Magenta     | \033[95m         |
// | Bright Cyan        | \033[96m         |
// | Bright White       | \033[97m         |
// +--------------------+------------------+
// | Reset              | \033[0m          |
// +--------------------+------------------+

// Ansi Color Codes in valve format.
// Format:	Color <name>(R, G, B, 255);
// Funtion: ColorSpewMessage( SPEW_MESSAGE, &<name>, "");

// Basic Colors (saturated)
Color black(0, 0, 0, 255);				
Color red(255, 0, 0, 255);				
Color green(0, 255, 0, 255);			
Color yellow(255, 255, 0, 255);			
Color blue(0, 0, 255, 255);				
Color magenta(255, 0, 255, 255);		
Color cyan(0, 255, 255, 255);			
Color white(255, 255, 255, 255);		

#if 0
// Bright Colors (High Intensity)
Color bright_black(85, 85, 85, 255);    // Bright Black (Gray)
Color bright_red(255, 85, 85, 255);     // Bright Red
Color bright_green(85, 255, 85, 255);   // Bright Green
Color bright_yellow(255, 255, 85, 255); // Bright Yellow
Color bright_blue(85, 85, 255, 255);    // Bright Blue
Color bright_magenta(255, 85, 255, 255);// Bright Magenta
Color bright_cyan(85, 255, 255, 255);   // Bright Cyan
Color bright_white(255, 255, 255, 255); // Bright White
#endif //0

// Dark Colors (Low Intensity|non-saturated)
Color dark_yellow(155, 132, 0, 255);
Color dark_grey(0, 0, 0, 128);

//Colors used by the strings. By default they will be on non-saturated mode. 
Color header_color = cyan;					//Used by headers strings.
Color path_color =	dark_yellow;					//Used by paths strings.
Color sucesfullprocess_color = green;		//Used to indicate .exe process ok
Color done_color = dark_grey;				//Used by "done()"


//REMOVE THIS!
#if 0
//This enables more colors in the cmd, for legacy consoles. In theory now, Bright Colors should work.
void InitColorCmd() 
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#endif //0
