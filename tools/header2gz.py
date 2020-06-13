#!/usr/bin/env python3
# coding: utf-8

# ヘッダファイルをZIPファイルに変換します（CameraWebServer用）
# Copyright (c) 2020 Wataru KUNINO

filename = 'camera_index.h'
camera	 = 'ov2640' # 'ov2640'or 'ov3660'
saveto	 = 'camera_index_' + camera + '.html.gz'

from time import sleep
from sys import argv					# 本プログラムの引数argvを取得する
print(argv[0])							# プログラム名を表示する
if len(argv) == 2:
	filename = argv[1]
	i = filename.find('.h')
	if i > 0 and i + 2 == len(filename):
		saveto = filename[0:i] + '.html.gz'

print('input filename  =',filename)
print('output filename =',saveto)
fp = open(filename)
data_array = fp.readlines()
fp.close()
data = ''
for d in data_array:
	data += d.rstrip()	# 右側の改行を削除
out = b''
i = data.find(camera)
if i < 0:
	print('ファイル内に選択したカメラ',camera,'がありませんでした')
	exit()
start = data[i:].find('{') + i
stop = data[i:].find('}') + i
print('start =',start,data[start])
print('stop  =',stop,data[stop])
sleep(1)

i = start
d_prev = -1
while True:
	d = data[i + 1:].find(',')
	if d < 0:
		print('最終データの処理(findでカンマが見つからなかったため)')
		i = i + 1 + d_prev
	else:
		i = i + 1 + d
	if i > stop:
		print('最終データの処理(別のカメラのデータまで進んだため)')
		i = i - d + d_prev
	s = data[i-2:i]
	h = int(s,16)
	print('i =',i, 'd =',d, data[i],s, ', h =', h)
	out += h.to_bytes(1,byteorder='big')
	if d_prev > 0 and d != d_prev:
		break
	d_prev = 5
fp = open(saveto, mode='bw')
fp.write(out)
fp.close()
