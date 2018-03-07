
#include <geekos/io.h>
#include <geekos/int.h>
#include <geekos/irq.h>
#include <geekos/dma.h>
#include <geekos/malloc.h>
#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/kthread.h>
#include <geekos/list.h>
#include <geekos/timer.h>
#include <geekos/alarm.h>
#include <geekos/mem.h>

#include <geekos/vfs.h>

#include <geekos/projects.h>

#include <geekos/string.h>
#include <geekos/sound.h>



void SB16_Play_File(const char *filename) {
    TODO_P(PROJECT_SOUND, "Play a named file");
}

void Init_Sound_Devices(void) {
    TODO_P(PROJECT_SOUND, "Initialize sound card");
}
