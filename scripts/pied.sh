#!/bin/sh
set -u
set -e

LD_LIBRARY_PATH=/usr/local/lib
export LD_LIBRARY_PATH
sockets="/tmp/pie_new_media.sock /tmp/pie_update_meta.sock /tmp/pie_export_media.sock"
mediad_pid=".mediad.pid"
exportd_pid=".exportd.pid"
collectiond_pid=".collectiond.pid"
editd_pid=".editd.pid"
daemons="mediad exportd collectiond editd"
pid_files="${mediad_pid} ${exportd_pid} ${collectiond_pid} ${editd_pid}"

alive()
{
    for p in ${pid_files}; do
        name=`echo $p | tr -d . | sed 's/\(.*\)pid$/\1/'`

        if [ -n "$1" ] && [ "$1" != $name ]; then
            continue
        fi

        printf "${name}..."
        if [ -e $p ]; then
            pid=`cat $p`
            if kill -0 ${pid} >/dev/null 2>&1; then
                printf "running"
            else
                printf "stopped"
            fi
        else
            printf "stopped"
        fi
        printf "\n"
    done
}

startd()
{
    data=`alive $1`
    if echo $data | grep -q running; then
        echo pie is still running.
        exit 1
    fi

    # for s in ${sockets}; do
    #     if [ -e $s ]; then
    #         rm $s
    #     fi
    # done

    for d in ${daemons}; do
        if [ -n "$1" ] && [ $1 != $d ]; then
            continue
        fi

        printf "starting $d..."
        ./bin/$d -> logs/$d.log 2>&1 &
        echo $! > .$d.pid
        echo "done"
        sleep 1
    done
}

stopd()
{
    for p in ${pid_files}; do
        name=`echo $p | tr -d . | sed 's/\(.*\)pid$/\1/'`

        if [ -n "$1" ] && [ $1 != $name ]; then
            continue
        fi

        printf "${name}..."
        if [ -e $p ]; then
            pid=`cat $p`
            if /bin/kill -INT ${pid} >/dev/null 2>&1; then
                printf "running..."
                wait ${pid} || true
                printf "stopped"
            else
                printf "stopped"
            fi
            rm $p
        else
            printf "stopped"
        fi
        printf "\n"
    done
}

cmd=$1
sub_cmd=""
if [ $# -gt 1 ]; then
    sub_cmd=$2
fi

case ${cmd} in
    start)
        startd $sub_cmd
        ;;
    stop)
        stopd $sub_cmd
        ;;
    status)
        alive $sub_cmd
        ;;
    *)
        echo Invalid action $1
        exit 1
        ;;
esac

exit 0
