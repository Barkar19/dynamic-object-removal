mkdir ${1/.*/_frames}
ffmpeg -i $1 -f image2 ${1/.*/_frames}/${1/.*/_%d.jpg}
