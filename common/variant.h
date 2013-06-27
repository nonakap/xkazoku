enum {
	VARIANT_BIT		= 1,				// BYTE
	VARIANT_SINT32	= 32				// SINT32
};

typedef struct {
	int		max;
	int		type;
	int		size;
} _VARIANT;

void *variant_create(int max, int type);
void variant_destory(void *val);

BOOL variant_set(void *val, int num, SINT32 value);
BOOL variant_get(void *val, int num, void *value);

