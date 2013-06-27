
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _la {
	int			maxitems;
	size_t		listsize;
	int			items;
	struct _la	*laNext;
} _LISTARRAY, *LISTARRAY;

LISTARRAY listarray_new(size_t listsize, int maxitems);
void listarray_clr(LISTARRAY laHandle);
void listarray_destroy(LISTARRAY laHandle);

int listarray_getitems(LISTARRAY laHandle);
void *listarray_append(LISTARRAY laHandle, const void *vpItem);
void *listarray_enum(LISTARRAY laHandle,
				BOOL (*cbProc)(void *vpItem, void *vpArg), void *vpArg);

#ifdef __cplusplus
}
#endif

