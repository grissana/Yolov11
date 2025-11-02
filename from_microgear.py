from ultralytics import YOLO
import cv2
import paho.mqtt.client as mqtt
import json
import time

# ------------------------------
# ⚙️ ตั้งค่า NETPIE2020
# ------------------------------
CLIENT_ID = "d62b294d-0bb5-46a9-82df-fbf2279e33f6"
TOKEN = "s6W82vfBBYRoXgJBdBeB8RRz29AUsNQ1"
SECRET = "3vQvfyTUsaxMszH2rwzHMSjkDruNmkx7"

HOST = "mqtt.netpie.io"
PORT = 1883

client = mqtt.Client(client_id=CLIENT_ID)
client.username_pw_set(TOKEN, SECRET)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("✅ Connected to NETPIE2020")
    else:
        print("❌ Failed to connect:", rc)

def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())
        print(f"📩 {msg.topic}: {data}")
    except:
        print(f"📩 {msg.topic}: {msg.payload.decode()}")

client.on_connect = on_connect
client.on_message = on_message
client.connect(HOST, PORT, 60)
client.loop_start()

# ------------------------------
# ⚙️ ตั้งค่า YOLO
# ------------------------------
model = YOLO("best.pt")  # โหลดโมเดล YOLO ของคุณ
CONF_THRESHOLD = 0.6

cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("❌ ไม่สามารถเปิดกล้องได้")
    exit()

# ตั้งค่ากล้อง (เหมาะสม)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
cap.set(cv2.CAP_PROP_FPS, 30)
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

print("✅ เริ่มตรวจจับ (กด 'q' เพื่อออก)")

last_send_time = 0  # ส่งข้อมูลทุก 5 วินาที
sum_coconut = 1

while True:
    ret, frame = cap.read()
    if not ret:
        print("⚠️ ไม่สามารถอ่านภาพจากกล้องได้")
        break

    results = model(frame, verbose=False)
    boxes = results[0].boxes
    annotated_frame = frame

    status = "none"  # ตัวแปรเก็บสถานะ (default)

    if boxes is not None and len(boxes) > 0:
        confs = boxes.conf
        indices = [i for i, c in enumerate(confs) if c >= CONF_THRESHOLD]

        if len(indices) > 0:
            annotated_frame = results[0].plot()
            names = model.names

            for i in indices:
                cls_id = int(boxes[i].cls[0])
                conf = float(boxes[i].conf[0])
                label = names[cls_id]

                # ✅ กำหนด status ตาม label
                if label == "coco_1.5":
                    status = "on"
                elif label == "coco_2.0":
                    status = "off"

            data_dict = {
                "coconut": label,
                "persent": round(conf * 100, 1),
                "sum": sum_coconut,
                "status": status
            }

            # ส่งข้อมูลขึ้น NETPIE ทุก 5 วินาที
            if time.time() - last_send_time > 5:
                sum_coconut += 1
                motor_number1 = 1
                shadow_data = {"data": data_dict}
                client.publish("@shadow/data/update", json.dumps(shadow_data))
                client.publish("@msg/update", f"{motor_number1}")
                print(f"🌤️ ส่งค่าแบบ Shadow: {json.dumps(shadow_data)}")
                last_send_time = time.time()
            else:
                motor_number1 = 0
                client.publish("@msg/update", f"{motor_number1}" , f"{status}")

    cv2.imshow("YOLO + NETPIE (@msg)", annotated_frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        print("🚪 ออกจากโปรแกรม")
        break

cap.release()
cv2.destroyAllWindows()
client.loop_stop()
client.disconnect()
