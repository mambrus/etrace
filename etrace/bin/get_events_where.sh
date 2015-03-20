#! /bin/bash

# Will select events of certain type and from this match the last argument
# against the entire PID-group. The result is formatted the same as it
# originally was.

# NOTE: Currently this script can't work in piped mode!
# ====================================================

# Examples (TBD):
# get_events_where sched_switch 'next_pid=' < file.etrace
# get_events_where sched_switch 'prev_pid=' < file.etrace
# get_events_where sched_wakeup 'pid=' < file.etrace
#
# Examples :
# get_events_where file.etrace sched_switch 'next_pid='
# get_events_where file.etrace sched_switch 'prev_pid='
# get_events_where file.etrace sched_wakeup 'pid='

if [ -z $GET_EVENTS_WHERE_SH ]; then
GET_EVENTS_WHERE_SH="get_events_where.sh"
SCRIPTS_DIR=$(dirname $(readlink -f $0))

source "${SCRIPTS_DIR}/get_events_of.sh"
source "${SCRIPTS_DIR}/pids.sh"

function get_events_where(){
    local IN="${1}"
    local E="${2}"
    local M="${3}"

    #Note: If piped mode, this **consumes** in-pipe
    EXPR=$(
        for P in $(pids $IN); do 
            echo -n "$P|";
        done;);

    FILTER=$(echo "($(echo $EXPR | sed -E 's/.$//'))")

    get_events_of ${IN} ${E} | grep -E "${M}${FILTER}"
}

if [ "$GET_EVENTS_WHERE_SH" == $(basename $(readlink -f $0)) ]; then
#Not sourced, do something with this.
	export PATH=${SCRIPTS_DIR}:$PATH

    if [ -t 0 ]; then
        if [ $# != 3 ]; then
            echo "$(basename $(readlink -f $0)) failed:" 1>&2
            echo "  in terminal-mode 3 arguments are required" 1>&2
            echo "  #1: filename" 1>&2
            echo "  #2: event" 1>&2
            echo "  #3: Pattern that can belong to PID:s" 1>&2
            exit 1
        fi
        INFILE="${1}"
        EVENT="${2}"
        MATCH="${3}"
    else
        if [ $# != 2 ]; then
            echo "$(basename $(readlink -f $0)) failed:" 1>&2
            echo "  in piped-mode 2 arguments are required" 1>&2
            echo "  #1: event" 1>&2
            echo "  #2: Pattern that can belong to PID:s" 1>&2
            exit 1
        fi
        INFILE="--"
        EVENT="${1}"
        MATCH="${2}"
    fi

	get_events_where "${INFILE}" "${EVENT}" "${MATCH}"
fi

fi


