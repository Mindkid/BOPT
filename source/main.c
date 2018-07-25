#include"BOPL.h"

int main(int argc, char *argv[])
{
    bopl_init(512, 1);
    bopl_insert(5, 40);
    bopl_insert(3, 1);
    printf("%d\n", bopl_lookup(41));
    bopl_close(512);
}
