#! /bin/bash

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

# Some vendors use white-space in their process-names which is extremely
# inconvenient for post-processing.
# Fix by replace ' ' by '#' but only in process name
function fix_pname(){
    cat -- | \
        grep -vE '^#' | \
        awk -F"-" '{
            PNAME=gensub("([[:graph:]])( )","\\1#","g",$1);
            print PNAME"-"$2$3$4
        }';
}

function mytrace() {
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
		-T$2 | fix_pname
}

function forever_etrace() {
	for ((;1;)); do
		mytrace $1 $2
	done
}

#forever_etrace "${@}"

#To unset if sourced: 
# for D in $(cat ../bin/cont_trace.sh | grep function | cut -f2 -d" " | \
#   sed -e 's/()//');do echo $D; unset $D;
# done

