#include"BOPL.h"

int main(int argc, char *argv[])
{
	int i;
	int grain = 1;
    bopl_init(512, &grain);
    bopl_insert(5, 20);
    bopl_insert(3, 22);
    for( i = 0; i < 5; i++)
    	printf("%d\n", bopl_lookup(18 + i));
    bopl_close(123);
}
