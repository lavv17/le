#include <config.h>
#include <wchar.h>

#if REPLACE_WCWIDTH
/* substitute system function (mostly for ncurses/xterm combination) */
#undef wcwidth
extern int mk_wcwidth(wchar_t ucs);
int wcwidth(wchar_t ch)
{
	return mk_wcwidth(ch);
}
#endif
