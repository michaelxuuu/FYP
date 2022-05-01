#ifndef UNISTD_H
#define UNISTD_H
    
extern int fork();

extern int exec(char *p);

extern int execv(char *p, char **args);

extern void wait();

extern void exit();

#endif