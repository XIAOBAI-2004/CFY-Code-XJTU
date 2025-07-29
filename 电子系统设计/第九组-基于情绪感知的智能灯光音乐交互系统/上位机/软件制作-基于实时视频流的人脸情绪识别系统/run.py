import cv2
import numpy as np
from statistics import mode
import onnxruntime as ort
import matplotlib.pyplot as plt
import os
import sys
from datetime import datetime
import subprocess

# 改用paramiko库（纯Python实现，无需系统命令）
import paramiko

def upload_with_paramiko():
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect('192.168.137.2', username='HwHiAiUser', password='Mind@123')
    
    with ssh.open_sftp() as sftp:
        sftp.put('emotion_results.txt', '/home/HwHiAiUser/emotion_results.txt')
    
    print("上传成功")

result_file = 'emotion_results.txt'

if os.path.exists(result_file):
    os.remove(result_file)
    print(f"已删除旧文件: {result_file}")

# 创建新空文件
with open(result_file, 'w') as f:
    pass  # 不写入任何内容

def save_emotion_result(timestamp, emotion, probability):
    """将情绪结果追加到文件"""
    with open(result_file, 'a') as f:
        f.write(f"{timestamp},{emotion},{probability:.4f}\n")

def get_labels(dataset_name):
    if dataset_name == 'fer2013':
        return {0:'angry',1:'disgust',2:'fear',3:'happy',
                4:'sad',5:'surprise',6:'neutral'}
    elif dataset_name == 'imdb':
        return {0:'woman', 1:'man'}
    elif dataset_name == 'KDEF':
        return {0:'AN', 1:'DI', 2:'AF', 3:'HA', 4:'SA', 5:'SU', 6:'NE'}
    else:
        raise Exception('Invalid dataset name')

def preprocess_input(x, v2=True):
    """标准化输入图像数据"""
    x = x.astype('float32')
    x = x / 255.0  # 归一化到 [0,1]
    if v2:
        x = x - 0.5  # 移动到 [-0.5, 0.5]
        x = x * 2.0  # 缩放到 [-1, 1]
    return x

def load_onnx_model(model_path):
    """加载ONNX模型并创建推理会话"""
    session = ort.InferenceSession(model_path)
    # 获取输入输出信息
    input_name = session.get_inputs()[0].name
    output_name = session.get_outputs()[0].name
    input_shape = session.get_inputs()[0].shape
    return session, input_name, output_name, input_shape

def load_detection_model(model_path):
    detection_model = cv2.CascadeClassifier(model_path)
    return detection_model

def detect_faces(detection_model, gray_image_array):
    return detection_model.detectMultiScale(gray_image_array, 1.3, 5)

def draw_bounding_box(face_coordinates, image_array, color):
    x, y, w, h = face_coordinates
    cv2.rectangle(image_array, (x, y), (x + w, y + h), color, 2)

def apply_offsets(face_coordinates, offsets):
    x, y, width, height = face_coordinates
    x_off, y_off = offsets
    return (x - x_off, x + width + x_off, y - y_off, y + height + y_off)

def draw_text(coordinates, image_array, text, color, x_offset=0, y_offset=0,
                                            font_scale=2, thickness=2):
    x, y = coordinates[:2]
    cv2.putText(image_array, text, (x + x_offset, y + y_offset),
                cv2.FONT_HERSHEY_SIMPLEX,
                font_scale, color, thickness, cv2.LINE_AA)

def get_colors(num_classes):
    colors = plt.cm.hsv(np.linspace(0, 1, num_classes)).tolist()
    colors = np.asarray(colors) * 255
    return colors

#将代码中的相对路径改为动态获取路径
def resource_path(relative_path):
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(os.path.abspath("."), relative_path)

# 配置参数
USE_WEBCAM = True
emotion_model_path = resource_path('./model.onnx' ) # 替换为ONNX模型路径
emotion_labels = get_labels('fer2013')

# 超参数
frame_window = 10
emotion_offsets = (20, 40)

# 加载模型
face_cascade = cv2.CascadeClassifier(resource_path('./haarcascade_frontalface_default.xml'))
emotion_session, input_name, output_name, input_shape = load_onnx_model(emotion_model_path)
emotion_target_size = input_shape[1:3]  # 获取输入尺寸 (height, width)

# 初始化情绪窗口
emotion_window = []

# 启动视频流
cv2.namedWindow('window_frame')
cap = cv2.VideoCapture(0)


emo_lines=0


while cap.isOpened():
    ret, bgr_image = cap.read()
    if bgr_image is None:
        print("警告: 读取到空帧")
        continue
    if not ret:
        break

    gray_image = cv2.cvtColor(bgr_image, cv2.COLOR_BGR2GRAY)
    rgb_image = cv2.cvtColor(bgr_image, cv2.COLOR_BGR2RGB)

    faces = face_cascade.detectMultiScale(gray_image, scaleFactor=1.1, 
                                        minNeighbors=5, minSize=(30, 30))

    for face_coordinates in faces:
        x1, x2, y1, y2 = apply_offsets(face_coordinates, emotion_offsets)
        gray_face = gray_image[y1:y2, x1:x2]
        if gray_face.size == 0:
            # print("警告: 检测到空人脸区域")
            continue
        try:
            # 预处理
            gray_face = cv2.resize(gray_face, emotion_target_size)
            gray_face = preprocess_input(gray_face, True)
            gray_face = np.expand_dims(gray_face, 0)  # 添加batch维度
            gray_face = np.expand_dims(gray_face, -1)  # 添加channel维度
            
            # ONNX推理
            emotion_prediction = emotion_session.run(
                [output_name], {input_name: gray_face}
            )[0]
            
            emotion_probability = np.max(emotion_prediction)
            emotion_label_arg = np.argmax(emotion_prediction)
            emotion_text = emotion_labels[emotion_label_arg]
            emotion_window.append(emotion_text)

            if len(emotion_window) > frame_window:
                emotion_window.pop(0)
                
            try:
                emotion_mode = mode(emotion_window)
            except:
                continue

            # 颜色设置
            if emotion_text == 'angry':
                color = emotion_probability * np.asarray((255, 0, 0))
            elif emotion_text == 'sad':
                color = emotion_probability * np.asarray((0, 0, 255))
            elif emotion_text == 'happy':
                color = emotion_probability * np.asarray((255, 255, 0))
            elif emotion_text == 'surprise':
                color = emotion_probability * np.asarray((0, 255, 255))
            else:
                color = emotion_probability * np.asarray((0, 255, 0))

            color = color.astype(int).tolist()

            draw_bounding_box(face_coordinates, rgb_image, color)
            draw_text(face_coordinates, rgb_image, emotion_mode,
                      color, 0, -45, 1, 1)
            print("检测到人脸情绪:{}: {}".format(emotion_text, emotion_probability))
            emo_lines+=1
            timestamp = datetime.now().strftime("%Y-%m-%d-%H:%M:%S.%f")[:-3]
            with open(result_file, "a") as f:
                f.write(f"{timestamp},{emotion_mode},{emotion_probability:.4f}\n")

        except Exception as e:
            print(f"处理人脸时出错: {str(e)}")
            continue

    bgr_image = cv2.cvtColor(rgb_image, cv2.COLOR_RGB2BGR)
    cv2.imshow('window_frame', bgr_image)
    if emo_lines%30==0:
        upload_with_paramiko()
    if cv2.waitKey(1) & 0xFF == ord('q'):
                
        
        break

cap.release()
cv2.destroyAllWindows()



