#!/bin/bash

# 撮影したJPEGファイルを H.264形式の動画に変換します。

PHOTO_DIR="photo"                           # JPEGファイル
SAVETO=`pwd`                                # 保存先（ローカル）
PHOTO_DIR_Alt="/mnt/ssd1/minidlna/pictures" # JPEGファイル
SAVETO_Alt="/mnt/ssd1/minidlna/videos"      # 保存先（SSD）
DATE=`date "+%Y%m%d-%H%M%S"`                # 変換日時

cd ${PHOTO_DIR}
if [ $? -ne 0 ]; then
    cd ${PHOTO_DIR_Alt}
    if [ $? -ne 0 ]; then
        echo "ERROR: no photo dirs."
        exit
    fi
    PHOTO_DIR=${PHOTO_DIR_Alt}
    SAVETO=${SAVETO_Alt}
fi
mkdir -p tmp_
ls -1 *.jpg > tmp_/list.txt

i=0
cat tmp_/list.txt | while read line
do
    n=`printf {"%04d",$i}`
    echo $n: $line
    ln -s ../${line} tmp_/img${n}.jpg
    i=$((i + 1))
done
ffmpeg -r 15 -vcodec mjpeg -i tmp_/img%04d.jpg -vcodec libx264 ${SAVETO}/cam_${DATE}.mp4
rm -Rf tmp_
echo "Done"
