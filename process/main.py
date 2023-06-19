from pipeline import serial_upload, serial_download
import model


if __name__ == '__main__':
    origin_image = serial_upload()
    output_image = model.img2img('cyberpunk')
    serial_download()
    output_image.show()