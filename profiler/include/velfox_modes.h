#ifndef VELFOX_MODES_H
#define VELFOX_MODES_H

#include "velfox_common.h"
#include "velfox_soc.h"

void perfcommon();
void apex_mode();
void adaptive_mode();
void efficiency_mode();
void set_dnd(int mode);
void read_configs();

#endif //VELFOX_MODES_H