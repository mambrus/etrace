#include <stdio.h>
#include "doc.h"
#include "cmd_help.h"

static char etrace_doc[] = CLI_CMD_HELPTEXT_calibrate;

void doc_print()
{
    printf("%s\n", etrace_doc);
}
