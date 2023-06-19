#从像素生成图片
def pixel_to_image():
    # 导入所需的库
    from PIL import Image, ImageDraw
    import os

    # 定义图像的尺寸
    w = 320  # 宽度
    h = 240  # 高度

    # 源文件路径
    FileAdd = os.getcwd() + "\\output.txt"

    # 输出文件名称
    OutFile = os.getcwd() + "\\out.jpeg"

    # 打开源文件
    f = open(FileAdd, encoding="utf-8")
    print("正在合成来自" + FileAdd + "的图像数据，合成为" + str(w) + "*" + str(h) + "的图像")

    # 16进制到10进制的映射
    Hex_To_Dec = {'0': 0, '1': 1, '2': 2, '3': 3, '4': 4, '5': 5, '6': 6, '7': 7, '8': 8, '9': 9, 'a': 10, 'b': 11,
                  'c': 12,
                  'd': 13, 'e': 14, 'f': 15, 'A': 10, 'B': 11, 'C': 12, 'D': 13, 'E': 14, 'F': 15}

    z = 0
    RGB = []

    # 将文本型十六进制转为数值型十六进制
    for x in range(w * h * 60):
        f.seek(z + 0, 0)
        f1 = f.read(1)
        f.seek(z + 1, 0)
        f2 = f.read(1)
        f.seek(z + 3, 0)
        f3 = f.read(1)
        f.seek(z + 4, 0)
        f4 = f.read(1)
        z = z + 6

        # 将4个16进制字符转换为一个16位的整数
        s = (Hex_To_Dec[f1] << 12) + (Hex_To_Dec[f2] << 8) + (Hex_To_Dec[f3] << 4) + Hex_To_Dec[f4]

        # 将RGB565转为RGB888
        r = (s & 0xF800) >> 8
        g = (s & 0x7E0) >> 3
        b = (s & 0x1F) << 3
        RGB.append((r, g, b))

        if z >= (w * h * 6):
            break

    # 关闭源文件
    f.close()

    # 创建新图片对象
    image = Image.new('RGB', (w, h), (255, 255, 255))

    # 创建Draw对象用于绘制新图
    draw = ImageDraw.Draw(image)

    i = 0

    # 填充每个像素并对对应像素填上RGB值
    for y in range(h):
        for x in range(w):
            draw.point((x, y), fill=RGB[i])
            i = i + 1

    # 保存文件
    image.save(OutFile, 'jpeg')
    print("图片保存成功")
    # # 查看图像  注意：当该代码没有被注释时，必须关掉显示的图片才能按键，否则缓冲区会移出。
    image.show()

# 从图片生成像素
def image_to_pixel():
    # 导入所需的库
    from PIL import Image
    import os

    # 源文件路径
    FileAdd = os.getcwd() + "\\gen_from_photo.png"

    # 输出文件名称
    OutFile = os.getcwd() + "\\output.txt"

    # 打开源文件
    image = Image.open(FileAdd)

    # 获取图像的宽度和高度
    w = image.width
    h = image.height

    # 创建新文件
    f = open(OutFile, 'w')

    # 16进制到10进制的映射
    Dec_To_Hex = {0: '0', 1: '1', 2: '2', 3: '3', 4: '4',
                    5: '5', 6: '6', 7: '7', 8: '8', 9: '9',
                    10: 'a', 11: 'b', 12: 'c', 13: 'd', 14: 'e',
                    15: 'f'}
    
    # 将RGB888转为RGB565
    for y in range(h):
        for x in range(w):
            # 获取像素点的RGB值
            r, g, b = image.getpixel((x, y))

            # 将RGB888转为RGB565
            r = (r >> 3) << 11
            g = (g >> 2) << 5
            b = b >> 3

            # 将RGB565转为16进制
            s = r + g + b
            s1 = Dec_To_Hex[(s & 0xF000) >> 12]
            s2 = Dec_To_Hex[(s & 0x0F00) >> 8]
            s3 = Dec_To_Hex[(s & 0x00F0) >> 4]
            s4 = Dec_To_Hex[s & 0x000F]

            # 写入文件
            f.write(s1)
            f.write(s2)
            f.write(s3)
            f.write(s4)
            f.write("\n")

    # 关闭文件
    f.close()

    print("图像转换成功")



