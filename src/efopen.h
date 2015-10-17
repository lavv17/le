/* efopen is like fopen, but using an embedded set of files */
#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>

extern FILE *efopen(const char *path, const char *mode);


#ifdef __cplusplus
}
#endif