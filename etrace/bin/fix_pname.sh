#! /bin/bash

if [ -z $FIX_PNAME_SH ]; then
FIX_PNAME_SH="fix_pname.sh"
SCRIPTS_DIR=$(dirname $(readlink -f $0))

# Some vendors use white-space in their process-names which is extremely
# inconvenient for post-processing.
# Fix by replace ' ' by '#' but only in process name.
#
# Note the "note" below. If needed, use another character here, like @, do
# identify kernel-threads. Alternatively use the lack prev_pid
# (sched_switch)
function fix_pname(){
    cat -- | \
        grep -vE '^#' | \
        awk -F"-" '{
            PNAME=gensub("([[:graph:]])( )","\\1#","g",$1);
            print PNAME"-"$2$3$4
        }' | \
        sed -E 's/([[:space:]])*([[:alpha:]]-)*([[:alpha:]])([0-9]{3,4})/\1\2\3@\4/'
                                                                     #Note ----^
}


if [ "$FIX_PNAME_SH" == $(basename $(readlink -f $0)) ]; then
#Not sourced, do something with this.
	export PATH=${SCRIPTS_DIR}:$PATH

	fix_pname "${@}"
fi

fi


