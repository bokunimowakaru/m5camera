#!/bin/bash

################################################################################
# カメラからの配信画像を取得する
#
################################################################################
# 使用、改変、再配布は自由に行えますが、無保証です。権利情報の改変は不可です。
#                                          Copyright (c) 2016-2020 Wataru KUNINO
################################################################################

DEVICE1="cam_a_5"                                   # 配信デバイス名(カメラ)
DEVICE2="pir_s_5"                                   # 配信デバイス名(人感)
IP_CAM="192.168.4.1"                                # カメラのIPアドレス(仮)
PORT=1024                                           # UDPポート番号を1024に

echo "UDP Logger (usage: ${0} port)"                # タイトル表示
if [ ${#} = 1 ]; then                               # 入力パラメータ数が1つ
    if [ ${1} -ge 1 ] && [ ${1} -le 65535 ]; then   # ポート番号の範囲確認
        PORT=${1}                                   # ポート番号を設定
    fi                                              # ifの終了
fi                                                  # ifの終了
echo "Listening UDP port "${PORT}"..."              # ポート番号表示
mkdir photo >& /dev/null                            # 写真保存用フォルダ作成
while true                                          # 永遠に
do                                                  # 繰り返し
    UDP=`nc -luw0 ${PORT}|tr -d [:cntrl:]|\
    tr -d "\!\"\$\%\&\'\(\)\*\+\-\;\<\=\>\?\[\\\]\^\{\|\}\~"`
                                                    # UDPパケットを取得
    DEV=${UDP#,*}                                   # デバイス名を取得(前方)
    DEV=${DEV%%,*}                                  # デバイス名を取得(後方)
    if [ ${DEVICE1} = ${DEV} ]; then                # 対象機器(カメラ)の時
        if [ $SECONDS -lt 300 ]; then
            IP_CAM=`echo -E $UDP|tr -d ' '|cut -d'/' -f3`   # IPアドレスを保持
            echo "IP_CAM="${IP_CAM}
        else
            echo "起動後5分を経過したので送信先は更新しません"
        fi
    fi
    if [ ${DEVICE1} = ${DEV} ] || [ ${DEVICE2} = ${DEV} ]; then # 対象機器の時
        DATE=`date "+%Y/%m/%d %R"`                  # 日時を取得
        echo -E $DATE, $UDP|tee -a log_${DEV}.csv   # 取得日時とデータを保存
        DATE=`date "+%Y%m%d-%H%M%S"`                # 日時を取得
        URL=${IP_CAM}/cam.jpg                       # URLを作成
        echo -n "Get "${URL}                        # 画像取得t実行表示
        wget -qT10 ${URL} -Ophoto/${DEVICE1}"_"${DATE}.jpg  # wget実行
        echo " Done"                                # 終了表示
    fi
done                                                # 繰り返し範囲:ここまで
