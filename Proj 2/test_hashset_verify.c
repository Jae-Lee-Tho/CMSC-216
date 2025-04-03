// tests sizes of structs
#include "hashset.h"
#include <stddef.h>
int main(){
  if((sizeof(hashset_t) != 32) ||
     (sizeof(hsnode_t) != 144) ||
     (offsetof(hsnode_t,data) != 16))
  {
    printf("Anti-theft countermeasures engaged\n");
    return 1;
  }
  return 0;
}
