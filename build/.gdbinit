target remote localhost:10851
break Dump_Interrupt_State
break Hardware_Shutdown
commands 1
thread apply all bt
# auto generated in Makefile.common
