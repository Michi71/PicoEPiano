#include "u8g2.h"
#include "rotary_encoder.h"
#include "pico_userinterface.h"

#include "mdaEPiano.h"


#ifdef __cplusplus
extern "C"
{
#endif

uint8_t pico_UserInterfaceParamSelect(u8g2_t *u8g2, RotaryEncoder *enc, mdaEPiano *ep)
{
	int32_t count = ep->getParameterCount();
	
	char buf[1024];
	char label[24];
	
	u8g2_SetFont(u8g2, u8g2_font_8x13B_tf);	
	
	memset(buf, 0, 1024);
	for (int i=0; i<count; i++) {
		ep->getParameterName(i, label);		
		strcat(buf, label);
		strcat(buf, "\n");
	}
	strcat(buf, "<< BACK");
	
	int res = pico_UserInterfaceSelectionList(u8g2, enc, "Parameter", 0, buf);
	
	if (res > count - 1) res = -1;
	return (res);	
}


uint8_t pico_UserInterfaceParamInput(u8g2_t *u8g2, RotaryEncoder *enc, mdaEPiano *ep, uint32_t paramIndex)
{
	uint8_t line_height;
	uint8_t height;
	u8g2_uint_t pixel_height;
	u8g2_uint_t  y, yy;
	u8g2_uint_t  pixel_width;
	u8g2_uint_t  x, xx;

	float local_value = ep->getParameter(paramIndex);
	float old_value = local_value;
	int32_t delta;
	
	char title[24];
	char label[24];
	char displ[24];
	
	float lo;
	float hi;
	float step;
	
	ep->getParameterName(paramIndex, title);
	ep->getParameterLabel(paramIndex, label);	
	ep->getParameterDisplay(paramIndex, displ);	

	lo = 0;
	hi = 1;
	step = 0.01;
	
	u8g2_SetFont(u8g2, u8g2_font_8x13B_tf);	
	u8g2_SetFontDirection(u8g2, 0);
	u8g2_SetFontPosBaseline(u8g2);
	line_height = u8g2_GetAscent(u8g2);
	line_height -= u8g2_GetDescent(u8g2);
	height = 1;	/* value input line */
	height += u8x8_GetStringLineCnt(title);
	pixel_height = height;
	pixel_height *= line_height;

	y = 0;
	if ( pixel_height < u8g2_GetDisplayHeight(u8g2)  )
	{
		y = u8g2_GetDisplayHeight(u8g2);
		y -= pixel_height;
		y /= 2;
	}
	
	/* calculate offset from left for the label */
	x = 0;
	pixel_width = u8g2_GetUTF8Width(u8g2, displ);
	pixel_width += u8g2_GetUTF8Width(u8g2, " ");
	pixel_width += u8g2_GetUTF8Width(u8g2, label);
	if ( pixel_width < u8g2_GetDisplayWidth(u8g2) )
	{
		x = u8g2_GetDisplayWidth(u8g2);
		x -= pixel_width;
		x /= 2;
	}
	
	 /* event loop */
	for(;;)
	{
		u8g2_FirstPage(u8g2);
		do
		{
			if (old_value != local_value) {
				ep->setParameter(paramIndex, local_value);
				old_value = local_value;
			}
			ep->getParameterDisplay(paramIndex, displ);
			
			/* render */
			yy = y;
			yy += u8g2_DrawUTF8Lines(u8g2, 0, yy, u8g2_GetDisplayWidth(u8g2), line_height, title);
			yy += 4;
			xx = x;
			xx += u8g2_DrawUTF8(u8g2, xx, yy, displ);
			xx += u8g2_DrawUTF8(u8g2, xx, yy, " ");
			u8g2_DrawUTF8(u8g2, xx, yy, label);			
		} while( u8g2_NextPage(u8g2) );
		
		for(;;)
		{
			delta = enc->get_delta();
			if  (enc->get_button() == 0)
			{
				return 1;
			}
			else if (delta > 0)
			{
				local_value += step;								
				if ( local_value >= hi )
					local_value = hi;
				break;
			}
			else if (delta < 0)
			{
				local_value -= step;
				if ( local_value <= lo )
					local_value = lo;				  
				break;
			}       			
		}
	}
  
	/* never reached */
	return 0;  
}




#ifdef __cplusplus
}
#endif