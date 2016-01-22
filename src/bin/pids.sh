#! /bin/bash

if [ -z $PIDS_SH ]; then
PIDS_SH="pids.sh"
SCRIPTS_DIR=$(dirname $(readlink -f $0))

# Extract the pids in a etrace-file and print a list of it
#
function pids(){
    local IN=$1

    cat $IN | \
        grep  -E "^#" | \
        grep -E "#   " | \
        sed -e 's/#//'  | \
        awk '{
            for (i=1; i<=NF; i++) {
                print $i"\n"
            }
        }' | \
        grep -E '^[0-9]' | \
        sort -n
}

if [ "$PIDS_SH" == $(basename $(readlink -f $0)) ]; then
#Not sourced, do something with this.
	export PATH=${SCRIPTS_DIR}:$PATH

    if [ -t 0 ]; then
        INFILE=$1
    else
        INFILE="--"
    fi

    if [ "X${INFILE}" == "X" ]; then
        echo "$(basename $(readlink -f $0)) failed:" 1>&2
        echo "  in terminal-mode a filename is expected as #1 argument" 1>&2
        exit 1
    fi

	pids $INFILE
fi

fi


