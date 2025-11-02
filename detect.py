from ultralytics import YOLO
import cv2

# โหลดโมเดล YOLO (เปลี่ยนเป็นโมเดลของคุณ)
model = YOLO("best.pt")

# กำหนดค่าความมั่นใจขั้นต่ำ
CONF_THRESHOLD = 0.6

# เปิดกล้อง
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("❌ ไม่สามารถเปิดกล้องได้")
    exit()

# ------------------------------------------------------------
# ⚙️ ตั้งค่ากล้อง (จำเป็นและเหมาะสม)
# ------------------------------------------------------------
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)       # ความกว้าง
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)       # ความสูง
cap.set(cv2.CAP_PROP_FPS, 60)                 # เฟรมต่อวินาที
# cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))  # Codec MJPEG (ลด delay)
# cap.set(cv2.CAP_PROP_BRIGHTNESS, 150)         # ความสว่าง
# cap.set(cv2.CAP_PROP_CONTRAST, 50)            # ความคมชัด
# cap.set(cv2.CAP_PROP_SATURATION, 50)          # ความอิ่มสี
# cap.set(cv2.CAP_PROP_GAIN, 0)                 # ลดการขยายสัญญาณ noise
# cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 0.25)     # ปิด Auto exposure
# cap.set(cv2.CAP_PROP_EXPOSURE, -4)            # ตั้งค่าแสง manual
# cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)           # ลด delay ของกล้อง

print("✅ เริ่มตรวจจับจากกล้อง (กด 'q' เพื่อออก)")

while True:
    ret, frame = cap.read()
    if not ret:
        print("⚠️ ไม่สามารถอ่านภาพจากกล้องได้")
        break

    # ตรวจจับวัตถุ
    results = model(frame, verbose=False)

    # กรองเฉพาะผลลัพธ์ที่มั่นใจมากกว่า CONF_THRESHOLD
    boxes = results[0].boxes
    if boxes is not None and len(boxes) > 0:
        confs = boxes.conf
        indices = [i for i, c in enumerate(confs) if c >= CONF_THRESHOLD]

        if len(indices) > 0:
            filtered_boxes = boxes[indices]
            results[0].boxes = filtered_boxes
            annotated_frame = results[0].plot()

            # แสดงชื่อคลาสและค่า conf ใน console
            names = model.names
            for i in indices:
                cls_id = int(boxes[i].cls[0])
                conf = float(boxes[i].conf[0])
                label = f"{names[cls_id]} {conf*100:.1f}%"
                print(f"🔹 {label}")
        else:
            annotated_frame = frame
    else:
        annotated_frame = frame

    # แสดงผลลัพธ์
    cv2.imshow("YOLO11 Real-time Detection", annotated_frame)

    # ออกจากโปรแกรมเมื่อกด q
    if cv2.waitKey(1) & 0xFF == ord('q'):
        print("🚪 ออกจากโปรแกรม")
        break

# ปิดกล้องและหน้าต่างทั้งหมด
cap.release()
cv2.destroyAllWindows()
