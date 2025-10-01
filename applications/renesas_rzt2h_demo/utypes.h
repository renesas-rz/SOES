#ifndef __UTYPES_H__
#define __UTYPES_H__

#include "cc.h"

#define U32_MAX	((uint32_t)~0U)
#define S32_MAX	((int32_t)(U32_MAX>>1))

/* Object dictionary storage */
struct _Objects {
	/* Inputs */
	uint32_t BUTTON;

	/* Outputs */
	uint32_t LED;

	/* Parameters */

	/* Manufacturer specific data */

	/* Dynamic TX PDO:s */

	/* Dynamic RX PDO:s */

	/* Sync Managers */
};

extern struct _Objects Obj;

#endif /* __UTYPES_H__ */
