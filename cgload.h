
#ifdef __cplusplus
extern "C" {
#endif

BOOL cgload_data(VRAMHDL *vram, UINT type, const char *name);
BOOL cgload_mask(VRAMHDL *vram, UINT type, const char *name);

BOOL cgload_gan(VRAMHDL vram, int ref, const BYTE *ptr, int size);

#ifdef __cplusplus
}
#endif

