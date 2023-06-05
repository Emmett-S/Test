#coding=utf-8
from PIL import Image
#import sys
import numpy as np
import matplotlib.pyplot as plt

#from time import sleep

#import os

#gcc buffer_show.c -L/usr/lib/aarch64-linux-gnu -lpython3.6m -o buffer_show


def from_NV12(yuv_data, frames,IMG_WIDTH,IMG_HEIGHT):
    
    IMG_SIZE = int(IMG_WIDTH * IMG_HEIGHT * 3 / 2)
    #Y通道参数
    Y_WIDTH = IMG_WIDTH
    Y_HEIGHT = IMG_HEIGHT
    Y_SIZE = int(Y_WIDTH * Y_HEIGHT)
    #U和V通道参数
    U_V_WIDTH = int(IMG_WIDTH / 2)
    U_V_HEIGHT = int(IMG_HEIGHT / 2)
    U_V_SIZE = int(U_V_WIDTH * U_V_HEIGHT)
    
    Y = np.zeros((frames, IMG_HEIGHT, IMG_WIDTH), dtype=np.uint8)
    U = np.zeros((frames, U_V_HEIGHT, U_V_WIDTH), dtype=np.uint8)
    V = np.zeros((frames, U_V_HEIGHT, U_V_WIDTH), dtype=np.uint8)

    for frame_idx in range(0, frames):
        y_start = frame_idx * IMG_SIZE
        u_v_start = y_start + Y_SIZE
        u_v_end = u_v_start + (U_V_SIZE * 2)

        Y[frame_idx, :, :] = yuv_data[y_start : u_v_start].reshape((Y_HEIGHT, Y_WIDTH))
        U_V = yuv_data[u_v_start : u_v_end].reshape((U_V_SIZE, 2))
        U[frame_idx, :, :] = U_V[:, 0].reshape((U_V_HEIGHT, U_V_WIDTH))
        V[frame_idx, :, :] = U_V[:, 1].reshape((U_V_HEIGHT, U_V_WIDTH))
        
    return Y, U, V
    
    
def array2dispaly(yuv_bytes):
    

    #图像大小参数
    IMG_WIDTH = 504
    IMG_HEIGHT = 378
    
    '''
    IMG_SIZE = int(IMG_WIDTH * IMG_HEIGHT * 3 / 2)
    #Y通道参数
    Y_WIDTH = IMG_WIDTH
    Y_HEIGHT = IMG_HEIGHT
    Y_SIZE = int(Y_WIDTH * Y_HEIGHT)
    #U和V通道参数
    U_V_WIDTH = int(IMG_WIDTH / 2)
    U_V_HEIGHT = int(IMG_HEIGHT / 2)
    U_V_SIZE = int(U_V_WIDTH * U_V_HEIGHT)
    '''
    
    print(yuv_bytes)
    
    #frames = int(os.path.getsize(yuv) / IMG_SIZE)
    frames = 1
    
    yuv_data = np.frombuffer(yuv_bytes, np.uint8)#得到ndarray对象

    #Y, U, V = from_I420(yuv_data, frames)
    # Y, U, V = from_YV12(yuv_data, frames)
    Y, U, V = from_NV12(yuv_data, frames,IMG_WIDTH,IMG_HEIGHT) #获取各通道数据
    #Y, U, V = from_NV21(yuv_data, frames)
    #Y, U, V = from_YUYV(yuv_data, frames)
    
    V = np.repeat(V, 2, 2)
    U = np.repeat(U, 2, 2)
    V = np.repeat(V, 2, 1)
    U = np.repeat(U, 2, 1)
    
    yuv_data = np.zeros((IMG_HEIGHT, IMG_WIDTH, 3), dtype=np.uint8) 
    yuv_data[:, :, 0] = Y
    yuv_data[:, :, 1] = U
    yuv_data[:, :, 2] = V
        
    im = Image.fromarray(yuv_data, mode="YCbCr")
    #im.show()
        
    #fig = plt.figure()
    #ax_img = fig.add_subplot(1,2,1,facecolor='c')
    #ax_proc = fig.add_subplot(1,2,2,facecolor='c')
        
        
    #plt.grid(True)
    plt.ion() #打开交互
    plt.axis('off') #关闭坐标轴
        
    #img_show = fig.imshow(im)
    img_show = plt.imshow(im) #显示图像
        
        
    plt.pause(5) #显示5秒
    #sleep(0.5) #暂停
    plt.ioff() #关闭交互
        
    plt.clf() #清空图片
    plt.close() #清空窗口
    
    
    print("显示成功！！！")
    return frames
    
'''   
yuv = "./pico_dump_yuv420sp.yuv"
with open(yuv, "rb") as yuv_f:
        
        yuv_bytes = yuv_f.read() #获取yuv文件字节数据
        array2dispaly(yuv_bytes)
        '''
def array3dispaly(yuv_bytes):
    print("传输YUV数据成功：", yuv_bytes)
    return len(yuv_bytes)
