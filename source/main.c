#include"BOPL.h"

int main(int argc, char *argv[])
{
	int grain = 1;
	int* result;
	int* a = (int*) malloc(sizeof(int));
	*a = 8;

	
    bopl_init(512, &grain, FLUSH_ONLY_MODE);
    
	*a = 400;
	bopl_insert(80, sizeof(a), a);
	result = (int*) bopl_lookup(80);

	printf("This is the frase: %d\n", *result);
    	
    *a = 60;
    bopl_insert(235, sizeof(a), a);
    result = (int*) bopl_lookup(235);
	printf("This is the frase: %d\n", *result);
    
    bopl_remove(80);
    result = (int*) bopl_lookup(123);
    if(result == NULL)
        puts("It Worked");
        
    result = (int*) bopl_lookup(80);
    if(result == NULL)
        puts("It Worked");
    
    *a = 8000;
    bopl_update(235, sizeof(a), a);
    result = (int*) bopl_lookup(235);
	printf("This is the frase: %d\n", *result);
    
    
    bopl_close(512);
    
}
