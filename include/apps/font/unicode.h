// unicode.h 
//

#ifndef AFX_FONT_UNICODE_H__6E6D67A4_78AE_410B_B728_2464EF2E40C7__INCLUDED_
#define AFX_FONT_UNICODE_H__6E6D67A4_78AE_410B_B728_2464EF2E40C7__INCLUDED_


#include "typedef.h"

int font_read_sections();
void font_release_sections();
DWORD read_char_info_unicode(WORD wCode);

#endif // !defined(AFX_FONT_UNICODE_H__6E6D67A4_78AE_410B_B728_2464EF2E40C7__INCLUDED_)
