//========= --------------------------------------------------- ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "colorscheme.h"
#include "platform.h"


Color black(0, 0, 0, 255);				
Color red(255, 0, 0, 255);				
Color green(0, 255, 0, 255);			
Color yellow(255, 255, 0, 255);			
Color blue(0, 0, 255, 255);				
Color magenta(255, 0, 255, 255);		
Color cyan(0, 255, 255, 255);			
Color white(255, 255, 255, 255);		


// Dark Colors (Low Intensity, non-saturated)
Color dark_yellow(255, 255, 0);
Color dark_grey(0, 0, 0, 128);


//Colors used by the strings. By default they will be on non-saturated mode. 
Color header_color				= cyan;				//Used by headers strings.
Color path_color				= dark_yellow;		//Used by paths strings.
Color sucesfullprocess_color	= green;			//Used to indicate .exe process ok
Color done_color				= dark_grey;		//Used by "done()"