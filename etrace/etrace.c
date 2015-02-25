#include <stdio.h>
#include <config.h>
#include <log.h>

int main(int argc, char ** argv) {
	printf("Hello world: v%s \n", VERSION );
	//log_set_verbosity(LOG_LEVEL_VERBOSE);
	LOGV("Hello [VERBOSE] %d\n",	LOG_LEVEL_VERBOSE);
	LOGD("Hello [DEBUG] %d\n",		LOG_LEVEL_DEBUG);
	LOGI("Hello [INFO] %d\n",		LOG_LEVEL_INFO);
	LOGW("Hello [WARNING] %d\n",	LOG_LEVEL_WARNING);
	LOGE("Hello [ERROR] %d\n",		LOG_LEVEL_ERROR);
	//LOGC("Hello [CRITICAL] %d\n",	LOG_LEVEL_CRITICAL);

	return 0;
}