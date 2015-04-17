#! /bin/bash
# This is a specific script that takes a etrace with pre-determined
# event-types (sched_switch) and makes a sampler compatible output of it.
#
# Usage:
#   to_sampler.sh <tracefile> <delimitered-list-of-PIDs>
#   Valid delimiters are ,; and space (one of). If space is used, you have
#   make sure it's passed as *one* argument.
#
# Usage is a little awkward ATM. The trace-file is usually an etrace-file,
# i.e. it contains only PID:s of interest. In the header there's a list
# of the PID:s however. Here's an example of how to use this script with an
# etrace file.

# ETRACE_FILE=somefile.etrace
#
# to_sampler.sh $ETRACE_FILE $(
#  pids.sh $ETRACE_FILE | \
#  awk '{printf("%s,",$1);}END{printf("\n");}' | \
#  sed -e 's/.$//') > somefile.spraw

if [ -z $TO_SAMPLER_SH ]; then
TO_SAMPLER_SH="to_sampler.sh"
SCRIPTS_DIR=$(dirname $(readlink -f $0))

source "${SCRIPTS_DIR}/fix_pname.sh"

# Convert to sampler output-format
function to_sampler(){
    local IN=$1
    local THREADS="${2}"
    local MAX_CPUS=16

    cat $IN | fix_pname | \
    awk -vTHREADS="${THREADS}" -vMAX_CPUS="${MAX_CPUS}" '
		# Trims ":"
        function rtrim_1(s) { sub(/[:]+$/, "", s); return s }
        function ltrim_1(s) { sub(/^[:]+/, "", s); return s }

		# Trims "[" or "]"
        function rtrim_2(s) { sub(/[\[\]]+$/, "", s); return s }
        function ltrim_2(s) { sub(/^[\[\]]+/, "", s); return s }

		# In a line (arg 1) with assignments (i.e. "par=val"), return the nth
		# assignment (arg 2)
		function get_nth_value(s, m) {
			split(s, a1, /=/);
			split(a1[m+1], a2, / /);
			return a2[1]
		}

		# Return 1 if "p" is among the ones in the array "a", else 0
		# Note. AWK arrays start with 1, not 0 as in C
		function is_in(p, a) {
			n = length(a);
			R = 0;
			for (i=1; i<=n; i++){
				#print "testing: " a[i] " against: " p
				if (p == a[i]) {
					#print "hit"
					R = 1;
				}
			}
			return R;
		}

		BEGIN{
			NPIDS=split(THREADS, THREADA, /[,; ]/);
            # Initialize last seen CPU_freq
            for (i=0; i<MAX_CPUS; i++) {
                FCPU[i+1]=-1;
            }
		}

        rtrim_1($5)=="cpu_frequency"{
			cpu=get_nth_value($0,2);
			cpu_freq=get_nth_value($0,1);

            FCPU[cpu+1]=cpu_freq
            # print cpu" "cpu_freq
            # print FCPU[cpu]
        }

        rtrim_1($5)=="sched_switch"{
			TIME=rtrim_1($4);
			DBG="0";
			CPU=rtrim_2(ltrim_2($2));
			FLGS=$3;
			PPID=get_nth_value($0,2);
			NPID=get_nth_value($0,8);
			LSTAT=get_nth_value($0,4);
			FROM_OURS=is_in(PPID, THREADA);
			TO_OURS=is_in(NPID, THREADA);

            printf("%s;%s;%d;%s;%s;%d;%d;%s;%s;%d\n",
				TIME,
				DBG,
				CPU,
				FLGS,
				LSTAT,
				FROM_OURS,
				TO_OURS,
				PPID,
				NPID,
                FCPU[CPU+1]);
        }'
}

if [ "$TO_SAMPLER_SH" == $(basename $(readlink -f $0)) ]; then
#Not sourced, do something with this.
	export PATH=${SCRIPTS_DIR}:$PATH

    if [ -t 0 ]; then
        if [ $# != 2 ]; then
            echo "$(basename $(readlink -f $0)) failed:" 1>&2
            echo "  in terminal-mode 2 arguments are required" 1>&2
            echo "  #1: Filename" 1>&2
            echo "  #2: Delimitered list of PID:s" 1>&2
            exit 1
        fi
        INFILE="${1}"
        THREADS="${2}"
    else
        if [ $# != 1 ]; then
            echo "$(basename $(readlink -f $0)) failed:" 1>&2
            echo "  #1: Delimitered list of PID:s" 1>&2
            exit 1
        fi
        INFILE="--"
        THREADS="${1}"
    fi

	to_sampler "${INFILE}" "${THREADS}"
fi

fi

