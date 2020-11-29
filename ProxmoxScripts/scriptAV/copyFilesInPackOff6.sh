list=(/var/lib/vz/samples/!VIRUS/dos/7z/*)     # an array containing all the current file and subdir names
nf=6
for ((d=1, i=0; i < ${#list[@]}; d++, i+=nf)); do
    mkdir "/var/lib/vz/samples/!VIRUS/dos/$d"
    mv "${list[@]:i:nf}" "/var/lib/vz/samples/!VIRUS/dos/$d" -v
done