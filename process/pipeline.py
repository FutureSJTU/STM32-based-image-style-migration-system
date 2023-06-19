import serial.tools.list_ports
import time
from image import pixel_to_image
from image import image_to_pixel



def serial_upload():
    ports_list = list(serial.tools.list_ports.comports())
    if len(ports_list) <= 0:
        print("无串口设备。")
    else:
        print("可用的串口设备如下：")
        for comport in ports_list:
            print(list(comport)[0], list(comport)[1])

    # 打开COM3，将波特率配置为115200，其余参数使用默认值
    ser = serial.Serial("COM7", baudrate=115200, timeout=1)

    # 判断串口是否成功打开
    if ser.isOpen():
        print("打开串口成功。")
        print(ser.name)  # 输出串口号
    else:
        print("打开串口失败。")
    ser.reset_input_buffer()

    i = 0
    # 读取像素数据
    with open('output.txt', 'w') as f:
        while True:
            if ser.in_waiting:
                # 读取数据
                time.sleep(2.8)

                i += ser.in_waiting

                data = ser.read(ser.in_waiting)  # 读取串口中所有的数据

                # 转换为16进制并写入文件
                f.write(' '.join(format(x, '02x') for x in data))
                f.write(' ')  # 在每个字节之间添加一个空格
                f.flush()  # 刷新文件缓冲区，确保数据写入文件
                print(ser.in_waiting)
                # 开始处理
                if i >= 153600:
                    # 合成图片
                    return pixel_to_image()

def serial_download():
    ports_list = list(serial.tools.list_ports.comports())
    if len(ports_list) <= 0:
        print("无串口设备。")
    else:
        print("可用的串口设备如下：")
        for comport in ports_list:
            print(list(comport)[0], list(comport)[1])

    # 打开COM7，将波特率配置为115200，其余参数使用默认值
    ser = serial.Serial("COM7", baudrate=115200, timeout=1)

    # 判断串口是否成功打开
    if ser.isOpen():
        print("打开串口成功。")
        print(ser.name)  # 输出串口号
    else:
        print("打开串口失败。")
    ser.reset_input_buffer()

    # 将图片转换为像素
    image_to_pixel()

    # 将output.txt中的数据读取出来并发送到串口
    with open('output.txt', 'r') as f:
        data = f.read()
        data = data.split(' ')
        data = [int(x, 16) for x in data]
        data = bytes(data)
        ser.write(data)
        print(data)