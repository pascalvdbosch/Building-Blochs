#ifndef RECEIVER_H
#define RECEIVER_H

#include "bloch.h"

void receiver_setup();
void receiver_loop();
bool receiver_apply_permute(const BlochSphere::permute_t permute);
void receiver_reset_bloch();

#endif // RECEIVER_H