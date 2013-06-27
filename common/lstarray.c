#include	"compiler.h"


LISTARRAY listarray_new(size_t listsize, int maxitems) {

	LISTARRAY	laRet = NULL;
	int			dwSize;

	dwSize = sizeof(_LISTARRAY);
	dwSize += listsize * maxitems;

#ifdef TRACE
	{
		char work[256];
		sprintf(work, "listarray %dx%d", listsize, maxitems);
		laRet = (LISTARRAY)_MALLOC(dwSize, work);
	}
#else
	laRet = (LISTARRAY)_MALLOC(dwSize, "listarray");
#endif
	if (laRet) {
		ZeroMemory(laRet, dwSize);
		laRet->maxitems = maxitems;
		laRet->listsize = listsize;
	}
	return(laRet);
}


void listarray_clr(LISTARRAY laHandle) {

	if (laHandle) {
		laHandle->items = 0;
		listarray_destroy(laHandle->laNext);
		laHandle->laNext = NULL;
	}
}


void listarray_destroy(LISTARRAY laHandle) {

	LISTARRAY	laNext;

	while(laHandle) {
		laNext = laHandle->laNext;
		_MFREE(laHandle);
		laHandle = laNext;
	}
}


int listarray_getitems(LISTARRAY laHandle) {

	int		dwRet;

	dwRet = 0;
	while(laHandle) {
		dwRet += laHandle->items;
		laHandle = laHandle->laNext;
	}
	return(dwRet);
}


void *listarray_append(LISTARRAY laHandle, const void *vpItem) {

	LISTARRAY	laNew;
	char		*p;

	if ((laHandle == NULL) || (vpItem == NULL)) {
		goto laapp_err;
	}

	while(laHandle->laNext) {
		laHandle = laHandle->laNext;
	}
	if (laHandle->items >= laHandle->maxitems) {
		laNew = listarray_new(laHandle->listsize, laHandle->maxitems);
		if (laNew == NULL) {
			goto laapp_err;
		}
		laHandle->laNext = laNew;
		laHandle = laNew;
	}
	p = (char *)(laHandle + 1);
	p += laHandle->items * laHandle->listsize;
	CopyMemory(p, vpItem, laHandle->listsize);
	laHandle->items++;
	return(p);

laapp_err:
	return(NULL);
}


void *listarray_enum(LISTARRAY laHandle,
				BOOL (*cbProc)(void *vpItem, void *vpArg), void *vpArg) {

	int		i;

	if (cbProc == NULL) {
		goto laenum_end;
	}

	while(laHandle) {
		char *p = (char *)(laHandle + 1);
		for (i=0; i<laHandle->items; i++) {
			if (cbProc((void *)p, vpArg)) {
				return((void *)p);
			}
			p += laHandle->listsize;
		}
		laHandle = laHandle->laNext;
	}

laenum_end:
	return(NULL);
}

