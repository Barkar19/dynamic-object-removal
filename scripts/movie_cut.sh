ffmpeg -i $1 -ss $2 -t $3 -async 1 -c copy ${1/./_cuted.}
