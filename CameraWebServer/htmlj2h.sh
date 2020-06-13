#!/bin/bash

################################################################################
# スマートフォンへ送信するHTMLファイルcamera_index_ov2640_J.htmlを
# ヘッダ・ファイルに変換するためのツールです
# 実行には、gzipとPython 3の実行環境が必要です．
#
# camera_index_ov2640_J.html → camera_index_ov2640.h
#
################################################################################
# 使用、改変、再配布は自由に行えますが、無保証です。権利情報の改変は不可です。
# Copyright (c) 2020 Wataru KUNINO
################################################################################

gzip -k camera_index_ov2640_J.html
mv camera_index_ov2640_J.html.gz camera_index_ov2640.html.gz
../tools/gz2header.py ../CameraWebServer/camera_index_ov2640.html.gz
