
/*
 * Define interface to user space spin locks used in GeekOS
 *
 */

typedef struct {
    volatile int lock;
} User_Spin_Lock_t;

int Is_Locked(User_Spin_Lock_t * lock);
void Spin_Lock_Init(User_Spin_Lock_t * lock);
void Spin_Lock(User_Spin_Lock_t * lock);
int Spin_Unlock(User_Spin_Lock_t * lock);
