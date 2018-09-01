#include"BOPL.h"

int main(int argc, char *argv[])
{
	int i;
	char* five = (char*) malloc(sizeof(char)*4);
	char* three = (char*) malloc(sizeof(char)*4);
	five = "secatudo";
	three = "tudoseca";
	int grain = 1;
    bopl_init(512, &grain, FLUSH_ONLY);
    bopl_insert(sizeof(char) * 8, five, 20);
    bopl_insert(sizeof(char) * 8, three, 22);
    for( i = 0; i < 5; i++)
    	printf("This is the frase: %s\n", (char*) bopl_lookup(18 + i));
    bopl_close(123);
}
