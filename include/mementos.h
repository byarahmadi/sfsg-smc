#ifndef __MEMENTOS_H__
#define __MEMENTOS_H__
unsigned int __mementos_locate_next_bundle();
unsigned int __mementos_find_active_bundle();
unsigned int __mementos_bundle_in_range();
void __mementos_restore(unsigned int);
void __checkpoint();
void _checkpoint();
void _safepoint();
void _sp_checkpoint();
void hook_checkpoint();
unsigned int getXOR_trace(unsigned int);
#define xstr(s) str(s)
#define str(s) #s
#endif
