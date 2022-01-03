// author: jack mcshane
// CSCI-P556: Advanced Opterating Systems

/* shared global var for producer, consumer */
extern int n;
// check resource for consumer proc
sid32 can_read;
// check resource for producer proc
sid32 can_write;

/* function prototypes */
void consumer (int count);
void producer (int count);
