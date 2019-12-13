/* systemcalls.h - 
 */

#ifndef _SYSTEMCALLS_H
#define _SYSTEMCALLS_H
#include "types.h"
#include "lib.h"
#include "file_system.h"
#include "rtc.h"
#include "terminal.h"
#include "paging.h"
#include "keyboard.h"

int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t none();
/*functions need to be implemented in cp 4*/
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);


#endif /* _SYSTEMCALLS_H */

