/* This file is a placeholder for storing per-cpu variables in a segment that is not 
   saved and restored per thread, but rather left alone although different on a per-cpu
   basis. */

void Init_PerCPU(int cpu);
int PerCPU_Get_CPU(void);
struct Kernel_Thread *PerCPU_Get_Current(void);
