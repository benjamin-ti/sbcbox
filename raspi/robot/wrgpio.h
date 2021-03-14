#ifndef WRGPIO_H
#define WRGPIO_H

int wrgpio_init(char* appname);
void wrgpio_close(void);
int wrgpio_set_pinmode2outp(unsigned int gpioNum);
int wrgpio_set(unsigned int gpioNum);
int wrgpio_del(unsigned int gpioNum);

#endif // WRGPIO_H
