#! /bin/bash

if [ -z $FIX_PNAME_SH ]; then
FIX_PNAME_SH="fix_pname.sh"
SCRIPTS_DIR=$(dirname $(readlink -f $0))

# Some vendors use white-space in their process-names which is extremely
# inconvenient for post-processing.
# Fix by replace ' ' by '#' but only in process name.
#
function fix_pname(){
    cat -- | \
        grep -vE '^#' | \
        awk -F"[" '
        function ltrim(s) { sub(/^[ \t\r\n]+/, "", s); return s }
        function rtrim(s) { sub(/[ \t\r\n]+$/, "", s); return s }
        function trim(s) { return rtrim(ltrim(s)); }
        function rplace(s, r) { gsub(/[[:space:]]+/, r, s); return s }
        function _getname(s) { split(s, a, /\(.*\)\(-*\)\([0-9]+\)/); return a[1] }
        function getpid(s) { n=split(s, a, /-/); return a[n] }
        function getname(s) {
            R="";
            n=split(s, a, /-/);
            # Concatenate together a string, all but the last split
            for (i=0; i<n; i++) {
                R=sprintf("%s%s-",R,a[i]);
            }
            # Get rid of surplus leading "-"
            sub(/^-/, "", R)
            return R;
        }

        {
            rpname=rplace(trim($1), "#");
            pname=getname(rpname);
            pid=getpid(rpname);
            printf("%17s%-5s ",pname,pid);
            # Print the rest, including the "fake" separator
            $1=""
            print "[" substr($0,2)
        }'
}

if [ "$FIX_PNAME_SH" == $(basename $(readlink -f $0)) ]; then
#Not sourced, do something with this.
	export PATH=${SCRIPTS_DIR}:$PATH

	fix_pname "${@}"
fi

fi


