import cv2
from ultralytics import YOLO
from djitellopy import Tello
import time
import keyboard  # 用于非阻塞的键盘输入

# 初始化 Tello 无人机
tello = Tello()
tello.connect()
print("Battery level:", tello.get_battery())

# 确保视频流开启，并检查连接状态
try:
    tello.streamon()
except Exception as e:
    print("Error starting video stream:", e)
time.sleep(2)  # 稍等以确保视频流稳定

# 设置视频输出文件路径
video_path = 'C:\\Users\\14620\\video_name_here.mp4'  # 请修改为自己的路径
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
video_out = cv2.VideoWriter(video_path, fourcc, 20, (960, 720), isColor=True)

# 加载预训练的 YOLOv8 模型
model = YOLO('C:/Users/14620/DJITelloDrone-YOLOv8/yolov8s.pt', task='detect')

# 获取 Tello 无人机视频帧
frame_read = tello.get_frame_read(with_queue=False, max_queue_len=1)  # 将队列长度设为 1 以减少延迟

# 起飞
try:
    tello.takeoff()
    print("Drone has taken off.")
    time.sleep(1)  # 短暂等待以稳定
except Exception as e:
    print("Takeoff failed:", e)

# 主循环
while True:
    try:
        frame = frame_read.frame

        if frame is not None:
            # 每隔一帧进行 YOLO 目标检测，减少处理量
            results = model(frame)
            annotated_frame = results[0].plot()
            
            # 写入视频输出文件
            video_out.write(annotated_frame)

            # 显示带注释的帧
            cv2.imshow("YOLOv8 Tello Drone Tracking", annotated_frame)

            # 非阻塞键盘控制
            if keyboard.is_pressed("w"):
                tello.move_forward(30)
            elif keyboard.is_pressed("s"):
                tello.move_back(30)
            elif keyboard.is_pressed("a"):
                tello.move_left(30)
            elif keyboard.is_pressed("d"):
                tello.move_right(30)
            elif keyboard.is_pressed("r"):  # 上升
                tello.move_up(30)
            elif keyboard.is_pressed("f"):  # 下降
                tello.move_down(30)
            elif keyboard.is_pressed("e"):
                tello.rotate_clockwise(15)
            elif keyboard.is_pressed("q"):
                tello.rotate_counter_clockwise(15)
            elif keyboard.is_pressed("x"):
                tello.land()
                print("Landing...")
                break

            # 增加短暂延迟
            if cv2.waitKey(1) & 0xFF == ord("x"):
                break
        else:
            print("No frame received")

    except Exception as e:
        print("Error during streaming or processing:", e)
        tello.streamoff()
        time.sleep(1)
        tello.streamon()

# 释放资源
video_out.release()
tello.end()
cv2.destroyAllWindows()
