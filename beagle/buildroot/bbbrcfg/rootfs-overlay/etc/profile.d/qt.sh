if test -z "${XDG_RUNTIME_DIR}"; then
    export XDG_RUNTIME_DIR=/tmp/${UID}-runtime-dir
    if ! test -d "${XDG_RUNTIME_DIR}"; then
        mkdir "${XDG_RUNTIME_DIR}"
        chmod 0700 "${XDG_RUNTIME_DIR}"
    fi
fi

export QT_QPA_EGLFS_INTEGRATION=none
#export QT_QPA_PLATFORM=linuxfb

# export QWS_MOUSE_PROTO=linuxinput:/dev/input/event0