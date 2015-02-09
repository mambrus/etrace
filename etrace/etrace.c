#include <stdio.h>
#include <config.h>
#include <log.h>

int main(int argc, char ** argv) {
	printf("Hello world: v%s \n", VERSION );
	log_set_verbosity(LOG_LEVEL_WARNING);
	log_write(LOG_LEVEL_WARNING, "Hello helllo %d\n", 1);
	return 0;
}
