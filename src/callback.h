#ifndef CALLBACK_H
#define CALLBACK_H

typedef void (*CallbackFunction)(void *self);

typedef struct {
    CallbackFunction f;
    void* data;
} Callback;

void applyCallback(Callback *c);

#endif
