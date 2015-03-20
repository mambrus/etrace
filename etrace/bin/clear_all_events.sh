#! /bin/bash

if [ -z $CLEAR_ALL_EVENTS_SH ]; then
CLEAR_ALL_EVENTS_SH="clear_all_events.sh"
SCRIPTS_DIR=$(dirname $(readlink -f $0))


# Stops all event triggers and all event filters filters.
# This so that on next ftrace start that might have different filters and
# event specification, the old ones are not interfering.
function clear_all_events() {

#eventfs level
    for E in $(echo /sys/kernel/debug/tracing/events/enable); do
        echo 0 > $E
    done

#Category level
    for E in $(echo /sys/kernel/debug/tracing/events/*/enable); do
        echo "Disable $E"
        echo 0 > $E
    done
    for F in $(echo /sys/kernel/debug/tracing/events/*/filter); do
        echo "Clear $F"
        echo 0 > $F
    done

#Leaf level
    for E in $(echo /sys/kernel/debug/tracing/events/*/*/enable); do
        echo "Disable $E"
        echo 0 > $E
    done
    for F in $(echo /sys/kernel/debug/tracing/events/*/*/filter); do
        echo "Clear $F"
        echo 0 > $F
    done

}

if [ "$CLEAR_ALL_EVENTS_SH" == $(basename $(readlink -f $0)) ]; then
#Not sourced, do something with this.
	export PATH=${SCRIPTS_DIR}:$PATH

	clear_all_events "${@}"
fi


fi
