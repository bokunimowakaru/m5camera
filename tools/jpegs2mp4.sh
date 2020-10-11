#!/bin/bash

# 撮影したJPEGファイルを H.264形式の動画に変換します。

ls -1 photo/*.jpg > list_.txt
if [ $? -ne 0 ]; then
	echo "ERROR: no photo files."
	exit
fi
mkdir -p photo/tmp_
i=0
cat list_.txt | while read line
do
	n=`printf {"%04d",$i}`
	echo $n: $line
	cp $line photo/tmp_/img${n}.jpg
	i=$((i + 1))
done
ffmpeg -r 15 -vcodec mjpeg -i photo/tmp_/img%04d.jpg -vcodec libx264 out.mp4
rm -Rf photo/tmp_
rm list_.txt
echo "Done"
