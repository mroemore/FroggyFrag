#include "callback.h"

void applyCallback(Callback *c){
    c->f(c->data);
}
