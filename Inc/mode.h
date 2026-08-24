
#ifndef MODE_H
#define MODE_H

typedef enum
{
    MODE_MANUAL = 0,
    MODE_AUTO
} CarMode_t;

void Mode_Init(void);
void Mode_Set(CarMode_t newMode);
CarMode_t Mode_Get(void);

#endif
