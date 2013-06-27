#include	"compiler.h"


void *variant_create(int max, int type) {

	int		size = 0;
	void	*ret = NULL;

	switch(type) {
		case VARIANT_BIT:
		case VARIANT_SINT32:
			size = max * type;
			size /= 32;					// 頭悪い…
			size++;
			size *= 4;
			break;
	}
	if (size) {
		ret = (void *)_MALLOC(sizeof(_VARIANT) + size, "variant");
		if (ret) {
			ZeroMemory(ret, sizeof(_VARIANT) + size);
			((_VARIANT *)ret)->max = max;
			((_VARIANT *)ret)->type = type;
			((_VARIANT *)ret)->size = size;
		}
	}
	return(ret);
}

void variant_destory(void *val) {

	if (val) {
		_MFREE(val);
	}
}

BOOL variant_set(void *val, int num, SINT32 value) {

	if ((val == NULL) || (num < 0) || (((_VARIANT *)val)->max <= num)) {
		return(FAILURE);
	}
	switch(((_VARIANT *)val)->type) {
		case VARIANT_BIT:
			{
				BYTE *p = (BYTE *)&((_VARIANT *)val)[1];
				BYTE bit = 1 << (num & 7);
				if (!value) {
					p[num / 8] &= ~bit;
				}
				else {
					p[num / 8] |= bit;
				}
			}
			break;

		case VARIANT_SINT32:
			{
				BYTE *p;
				p = (BYTE *)&((_VARIANT *)val)[1];
				p += num * 4;
				STOREINTELDWORD(p, value);
			}
			break;
	}
	return(SUCCESS);
}

BOOL variant_get(void *val, int num, void *value) {

	if ((val == NULL) || (num < 0) || (((_VARIANT *)val)->max <= num)) {
		return(FAILURE);
	}
	switch(((_VARIANT *)val)->type) {
		case VARIANT_BIT:
			{
				BYTE *p = (BYTE *)&((_VARIANT *)val)[1];
				*(BYTE *)value = (p[num / 8] >> (num & 7)) & 1;
			}
			break;

		case VARIANT_SINT32:
			{
				BYTE *p;
				p = (BYTE *)&((_VARIANT *)val)[1];
				p += num * 4;
				*(SINT32 *)value = LOADINTELDWORD(p);
			}
			break;
	}
	return(SUCCESS);
}

