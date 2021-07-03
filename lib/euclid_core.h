#ifndef EUCLID_CORE_H // include guard
#define EUCLID_CORE_H

#include "tuple.h"
#include "clock_core.h"

/*DATA TYPES*/

struct EuclidRhythmParameters {
  unsigned short barLengthInBeats; // rename barlen in beats??
  unsigned short beats; //rename to beats?
  unsigned short rotation;
  unsigned short counter;
};

/*CORE (pure functions)*/

inline const bool euclid(
  const unsigned short& count,
  const unsigned short& beats,
  const unsigned short& barLengthInBeats,
  const unsigned short& rotation
){
  return (((count + rotation) * beats) % barLengthInBeats) < beats;
}

#endif /* EUCLID_CORE_H */
