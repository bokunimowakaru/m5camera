#!/bin/bash

# 撮影したJPEGファイルを H.264形式の動画に変換します。

PHOTO_DIR="photo"                           # JPEGファイル
SAVETO=""                                   # 保存先（ローカル）
PHOTO_DIR_Alt="/mnt/ssd1/minidlna/pictures" # JPEGファイル
SAVETO_Alt="/mnt/ssd1/minidlna/videos/"     # 保存先（SSD）
DATE=`date "+%Y%m%d-%H%M%S"`                # 変換日時

ls -1 ${PHOTO_DIR}/*.jpg > list_.txt
if [ $? -ne 0 ]; then
    ls -1 ${PHOTO_DIR2}/*.jpg > list_.txt
    if [ $? -ne 0 ]; then
        echo "ERROR: no photo files."
        exit
    fi
    PHOTO_DIR=${PHOTO_DIR_Alt}
    SAVETO=${SAVETO_Alt}
fi
mkdir -p ${PHOTO_DIR}/tmp_
i=0
cat list_.txt | while read line
do
    n=`printf {"%04d",$i}`
    echo $n: $line
    cp $line ${PHOTO_DIR}/tmp_/img${n}.jpg
    i=$((i + 1))
done
ffmpeg -r 15 -vcodec mjpeg -i ${PHOTO_DIR}/tmp_/img%04d.jpg -vcodec libx264 ${SAVETO}cam_${DATE}.mp4
rm -Rf ${PHOTO_DIR}/tmp_
rm list_.txt
echo "Done"
