#ifndef UNISTD_H
#define UNISTD_H
    
extern int fork();

extern int exec(char *p);

extern void wait();

#endif