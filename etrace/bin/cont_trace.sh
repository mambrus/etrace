#! /bin/bash

if [ -z $CONT_TRACE_SH ]; then
CONT_TRACE_SH="cont_trace.sh"
SCRIPTS_DIR=$(dirname $(readlink -f $0))

source "${SCRIPTS_DIR}/fix_pname.sh"

function maxof() {
    for P in $(ps -Al | \
        grep $1 | \
        grep -v sandbox | \
        grep -v defunct | \
        awk '{print $4}'); \
        do \
            echo -n "$P "; \
            pstree -p $P | \
                grep '{.*}' | \
                wc -l ; \
    done | sort -n -k2 | \
        tac | \
        head -n1 | \
        cut -f1 -d" "
}

function _mytrace() {
	sudo etrace -p$(maxof $1) -t \
		-e sched/sched_kthread_stop -f 'common_pid == %tid%' \
		-e sched/sched_kthread_stop_ret -f 'common_pid == %tid%' \
		-e sched/sched_migrate_task -f 'common_pid == %tid%' \
		-e sched/sched_pi_setprio -f 'common_pid == %tid%' \
		-e sched/sched_process_exec -f 'common_pid == %tid%' \
		-e sched/sched_process_exit -f 'common_pid == %tid%' \
		-e sched/sched_process_fork -f 'common_pid == %tid%' \
		-e sched/sched_process_free -f 'common_pid == %tid%' \
		-e sched/sched_process_wait -f 'common_pid == %tid%' \
		-e sched/sched_stat_blocked -f 'common_pid == %tid%' \
		-e sched/sched_stat_iowait -f 'common_pid == %tid%' \
		-e sched/sched_stat_runtime -f 'common_pid == %tid%' \
		-e sched/sched_stat_sleep -f 'common_pid == %tid%' \
		-e sched/sched_stat_wait -f 'common_pid == %tid%' \
		-e sched/sched_switch -f 'common_pid == %tid%' \
		-e sched/sched_wait_task -f 'common_pid == %tid%' \
		-e sched/sched_wakeup -f 'common_pid == %tid%' \
		-e sched/sched_wakeup_new -f 'common_pid == %tid%' \
		-verror \
		-T$2 
		
#		| fix_pname
}

function __mytrace() {
	sudo etrace -p$(maxof $1) -t \
		-e sched/sched_kthread_stop -f 'common_pid == %tid%' \
		-e sched/sched_kthread_stop_ret -f 'common_pid == %tid%' \
		-e sched/sched_migrate_task -f '(common_pid == %tid%) || (pid == %tid%)' \
		-e sched/sched_pi_setprio -f 'common_pid == %tid%' \
		-e sched/sched_process_exec -f '(common_pid == %tid%) || (old_pid == %tid%) || (pid == %tid%)' \
		-e sched/sched_process_exit -f '(common_pid == %tid%) || (pid == %tid%)' \
		-e sched/sched_process_fork -f '(common_pid == %tid%) || (parent_pid == %tid%) || (child_pid == %tid%)' \
		-e sched/sched_process_free -f '(common_pid == %tid%) || (pid == %tid%)' \
		-e sched/sched_process_wait -f '(common_pid == %tid%) || (pid == %tid%)' \
		-e sched/sched_switch -f '(common_pid == %tid%) || (prev_pid == %tid%) || (next_pid == %tid%)' \
		-e sched/sched_wait_task -f '(common_pid == %tid%) || (pid == %tid%)' \
		-e sched/sched_wakeup -f '(common_pid == %tid%) || (pid == %tid%)' \
		-e sched/sched_wakeup_new -f '(common_pid == %tid%) || (pid == %tid%)' \
		-verror \
		-T$2 
		
#		| fix_pname
}

function mytrace() {
	sudo etrace -p$(maxof $1) -t \
		-e sched/sched_switch -f '(prev_pid == %tid%) || (next_pid == %tid%)' \
		-verror \
		-T$2 
		
}

function forever_etrace() {
	for ((;1;)); do
		mytrace $1 $2
	done
}

if [ "$CHECK_SH" == $(basename $(readlink -f $0)) ]; then
#Not sourced, do something with this.
	export PATH=${SCRIPTS_DIR}:$PATH

	forever_etrace "${@}"
fi

#To unset if sourced:
# for D in $(cat ../bin/cont_trace.sh | grep function | cut -f2 -d" " | \
#   sed -e 's/()//');do echo $D; unset $D;
# done


fi
