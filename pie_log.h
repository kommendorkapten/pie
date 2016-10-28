#ifndef __PIE_DEBUG_H__
#define __PIE_DEBUG_H__

#include <stdio.h>

#if DEBUG > 0
# define PIE_DEBUG(...) do {printf("DEBUG: %s:", __func__);printf(__VA_ARGS__);printf("\n");} while(0)
#else
# define PIE_DEBUG(...)
#endif

#if DEBUG > 1
# define PIE_TRACE(...) do {printf("TRACE: %s:", __func__);printf(__VA_ARGS__);printf("\n");} while(0)
#else
# define PIE_TRACE(...)
#endif

#define PIE_LOG(...) do {printf("INFO: %s:", __func__);printf(__VA_ARGS__);printf("\n");} while(0)

#define PIE_WARN(...) do {printf("WARNING: %s:", __func__);printf(__VA_ARGS__);printf("\n");} while(0)

#define PIE_ERR(...) do {printf("ERROR: %s:", __func__);printf(__VA_ARGS__);printf("\n");} while(0)

#endif /* __PIE_DEBUG_H__ */
