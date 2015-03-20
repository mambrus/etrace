#! /bin/bash
# This script shows how easy it's to filter out what you want using awk
# and to optionally format it to your liking
#

if [ -z $GET_EVENTS_OF_SH ]; then
GET_EVENTS_OF_SH="get_events_of.sh"
SCRIPTS_DIR=$(dirname $(readlink -f $0))

source "${SCRIPTS_DIR}/fix_pname.sh"

# Extract the get_events_of in a etrace-file and print a list of it
#
function get_events_of(){
    local IN=$1
    local E=$2

    cat $IN | fix_pname | \
    awk '
        function rtrim(s) { sub(/[:]+$/, "", s); return s }

        rtrim($5)=="'$E'"{
            print
        }'
}

if [ "$GET_EVENTS_OF_SH" == $(basename $(readlink -f $0)) ]; then
#Not sourced, do something with this.
	export PATH=${SCRIPTS_DIR}:$PATH

    if [ -t 0 ]; then
        if [ $# != 2 ]; then
            echo "$(basename $(readlink -f $0)) failed:" 1>&2
            echo "  in terminal-mode 2 arguments are required" 1>&2
            echo "  #1: filename" 1>&2
            echo "  #2: event" 1>&2
            exit 1
        fi
        INFILE="${1}"
        EVENT=$2
    else
        if [ $# != 1 ]; then
            echo "$(basename $(readlink -f $0)) failed:" 1>&2
            echo "  in piped-mode 1 argument is required" 1>&2
            echo "  #1: event" 1>&2
            exit 1
        fi
        INFILE="--"
        EVENT=$1
    fi

	get_events_of "${INFILE}" $EVENT
fi

fi


