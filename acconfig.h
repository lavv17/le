#ifndef _CONFIG_H
#define _CONFIG_H
@TOP@

/* the name of package */
#undef PACKAGE

/* the version of package */
#undef VERSION

@BOTTOM@

#ifndef HAVE_STRERROR
#define strerror(e)  (sys_errlist[e])
#endif

#endif /* _CONFIG_H */
