#include "u8g2.h"
#include "rotary_encoder.h"
#include "pico_userinterface.h"

#include "mdaEPiano.h"

/*
  Draw a string at x,y
  Center string within w (left adjust if w < pixel len of s)
  
  Side effects:
    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFontPosBaseline(u8g2);

*/
void u8g2_SelDrawUTF8Line(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, const char *s, uint8_t border_size, uint8_t is_invert)
{
  u8g2_uint_t d, str_width;
  u8g2_uint_t fx, fy, fw, fh;

  /* only horizontal strings are supported, so force this here */
  u8g2_SetFontDirection(u8g2, 0);

  /* revert y position back to baseline ref */
  y += u8g2->font_calc_vref(u8g2);   

  /* calculate the width of the string in pixel */
  str_width = u8g2_GetUTF8Width(u8g2, s);

  /* calculate delta d within the box */
  d = 0;
  if ( str_width < w )
  {
    d = w;
    d -=str_width;
    d /= 2;
  }
  else
  {
    w = str_width;
  }

  /* caluclate text box */
  fx = x;
  fy = y - u8g2_GetAscent(u8g2) ;
  fw = w;
  fh = u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2) ;

  /* draw the box, if inverted */
  u8g2_SetDrawColor(u8g2, 1);
  if ( is_invert )
  {
    u8g2_DrawBox(u8g2, fx, fy, fw, fh);
  }

  /* draw the frame */
  while( border_size > 0 )
  {
    fx--;
    fy--;
    fw +=2;
    fh +=2;
    u8g2_DrawFrame(u8g2, fx, fy, fw, fh );
    border_size--;
  }

  if ( is_invert )
  {
    u8g2_SetDrawColor(u8g2, 0);
  }
  else
  {
    u8g2_SetDrawColor(u8g2, 1);
  }

  /* draw the text */
  u8g2_DrawUTF8(u8g2, x+d, y, s);

  /* revert draw color */
  u8g2_SetDrawColor(u8g2, 1);

}

#ifdef __cplusplus
extern "C"
{
#endif

uint8_t pico_UserInterfaceProgramSelect(u8g2_t *u8g2, RotaryEncoder *enc, mdaEPiano *ep)
{
  u8g2_uint_t  y, yy;
  u8g2_uint_t  x, xx;
  
  uint8_t local_value = ep->getProgram();
  //uint8_t r; /* not used ??? */
  uint8_t event;
  int32_t delta;
  char buf[24];

  /* only horizontal strings are supported, so force this here */
  u8g2_SetFontDirection(u8g2, 0);

  /* force baseline position */
  u8g2_SetFontPosBaseline(u8g2);
  
  y = u8g2_GetAscent(u8g2);
  x = 0;
  
 // ep->setProgram(local_value);
  /* event loop */
  for(;;)
  {
    u8g2_FirstPage(u8g2);
    do
    {
      /* render */
	  u8g2_SetFont(u8g2, u8g2_font_8x13B_tf);		
      yy = y;
	  strcpy(buf, "PROGRAM");
	  u8g2_SelDrawUTF8Line(u8g2, 0, 10, u8g2_GetDisplayWidth(u8g2)-2, buf, 0, 0);
	  ep->getProgramName(buf);
	  u8g2_SelDrawUTF8Line(u8g2, 0, 60, u8g2_GetDisplayWidth(u8g2)-2, buf, 0, 0);
	  
	  u8g2_DrawHLine(u8g2, 0, 12, u8g2_GetDisplayWidth(u8g2));
	  
	  /* Program Name */
	  xx = 0;
	  u8g2_SetFont(u8g2, u8g2_font_fub25_tf);	
	  xx += u8g2_DrawUTF8(u8g2, xx, 44, "P");	  
	  u8g2_DrawUTF8(u8g2, xx, 44, u8x8_u8toa(local_value, 3));

    } while( u8g2_NextPage(u8g2) );
        
    for(;;)
    {
	  delta = enc->get_delta();
      if  (enc->get_button() == 0)
      {
		return local_value;
      }
      else if (delta > 0)
      {
		if ( local_value >= ep->getProgramCount() - 1 )
			local_value = 0;
		else
			local_value++;
			ep->setProgram(local_value);
		break;
      }
      else if (delta < 0)
      {
		if ( local_value <= 0 )
		  local_value = ep->getProgramCount() - 1;
		else
		  local_value--;
		  ep->setProgram(local_value);
		break;
      }        
    }
  }
  
  /* never reached */
  //return r;  
}

#ifdef __cplusplus
}
#endif