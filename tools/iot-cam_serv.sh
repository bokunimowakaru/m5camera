#!/bin/bash
################################################################################
# Server Example: 防犯システム IFTTT 送信
#                                          Copyright (c) 2016-2020 Wataru KUNINO
################################################################################

# IFTTTのKeyを(https://ifttt.com/maker_webhooks)から取得し、変数KEYへ代入する
KEY="xxxxxxx_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"   # IFTTTのKey(鍵)

URL="https://maker.ifttt.com/trigger/"              # IFTTTのURL(変更不要)
MES="Wi-Fiカメラで写真を撮影しました。"             # IFTTTへ送信するメッセージ
PORT=1024                                           # 受信UDPポート番号を1024に
REED=1                                              # ドアスイッチON検出=0 OFF=1
DEVICE="cam_a_5"                                    # カメラ名・保存時ファイル名
IP_CAM="192.168.4.1"                                # カメラのIPアドレス(仮)

echo "Server Example 04 Cam + 09 IFTTT (usage: $0 port)"    # タイトル表示
if [ $# -ge 1 ]; then                               # 入力パラメータ数の確認
    if [ $1 -ge 1024 ] && [ $1 -le 65535 ]; then    # ポート番号の範囲確認
        PORT=$1                                     # ポート番号を設定
    fi                                              # ifの終了
fi                                                  # ifの終了
if [ ${KEY:0:2} = "xx" ]; then
    IFTTT=0; echo "IFTTT = OFF"
else
    IFTTT=1; echo "IFTTT = ON"
fi
echo "Listening UDP port "$PORT"..."                # ポート番号表示
mkdir photo >& /dev/null                            # 写真保存用フォルダ作成
while true; do                                      # 永遠に繰り返し
    UDP=`nc -luw0 $PORT|tr -d [:cntrl:]|\
    tr -d "\!\"\$\%\&\'\(\)\*\+\;\<\=\>\?\[\\\]\^\{\|\}\~"`
                                                    # UDPパケットを取得
    DATE=`date "+%Y/%m/%d %R"`                      # 日時を取得
    DEV=${UDP#,*}                                   # デバイス名を取得(前方)
    DEV=${DEV%%,*}                                  # デバイス名を取得(後方)
    echo -E $DATE, $UDP                             # 取得日時とデータを表示
#   echo -E $DATE, $UDP >> log_$DEV.csv             # 取得日時とデータを保存
    CAM=0                                           # 変数CAMの初期化
    case "$DEV" in                                  # DEVの内容に応じて
        "rd_sw_"? ) DET=`echo -E $UDP|tr -d ' '|cut -d, -f2`
                    if [ $DET -eq $REED ]; then     # 応答値とREED値が同じとき
                        CAM=1
                    fi ;;
        "pir_s_"? ) DET=`echo -E $UDP|tr -d ' '|cut -d, -f2`
                    if [ $DET != 0 ]; then          # 応答値が0以外の時
                        CAM=1
                    fi ;;
        "Pong" )    CAM=1 ;;
        "${DEVICE}" )
                    if [ $SECONDS -lt 300 ]; then
                        IP_CAM=`echo -E $UDP|tr -d ' '|cut -d'/' -f3`
                        echo "IP_CAM="${IP_CAM}
                    else
                        echo "起動後5分を経過したので送信先は更新しません"
                    fi
                    CAM=1
                    ;;
    esac
    if [ ${CAM} -ne 0 ]; then                       # CAMが0で無いとき
        echo -n "wget "${IP_CAM}"/cam.jpg"
        wget -qT3 -t1 $IP_CAM/cam.jpg               # 写真撮影と写真取得
        if [ $? -eq 0 ]; then
            echo " OK"
        else
            echo " ERROR"
        fi
        SFX=`date "+%Y%m%d-%H%M%S"`                 # 撮影日時を取得し変数SFXへ
        mv cam.jpg photo/${DEVICE}"_"${SFX}.jpg >& /dev/null
        if [ ${IFTTT} -ne 0 ]; then
            RES=`curl -s -m3 -X POST -H "Content-Type: application/json" \
                -d '{"value1":"'${MES}'","value2":"'${DEVICE}'_'${SFX}.jpg'"}' \
                ${URL}notify/with/key/${KEY}`
            if [ -n "$RES" ]; then                  # 応答があった場合
                echo $RES                           # 応答内容を表示
            else                                    # 応答が無かった場合
                echo "ERROR, curl"                  # ERRORを表示
            fi                                      # ifの終了
        fi
    fi
done                                                # 繰り返しここまで
