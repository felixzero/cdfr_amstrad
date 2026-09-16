#include <string.h>

main()
{
  memcpy(0xC000, 0x4000, 0x4000);
  //printf("Hello world");
  while(1) {};
}
