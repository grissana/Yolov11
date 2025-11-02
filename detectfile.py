from ultralytics import YOLO
import cv2
import os
import glob
import torch

# โหลดโมเดล YOLO
model = YOLO("best.pt")

# 🔹 กำหนดโฟลเดอร์ที่เก็บภาพ
image_folder = "test"

# 🔹 หาไฟล์ภาพทั้งหมดในโฟลเดอร์ (รองรับ .jpg .png .jpeg)
image_paths = sorted(
    glob.glob(os.path.join(image_folder, "*.jpg")) +
    glob.glob(os.path.join(image_folder, "*.png")) +
    glob.glob(os.path.join(image_folder, "*.jpeg"))
)

# ตรวจสอบว่ามีไฟล์ไหม
if not image_paths:
    print(f"❌ ไม่พบไฟล์ภาพในโฟลเดอร์: {image_folder}")
    exit()

index = 0  # เริ่มจากภาพแรก
CONF_THRESHOLD = 0.6  # ✅ กำหนดค่าความมั่นใจขั้นต่ำ

while True:
    # ตรวจสอบขอบเขต index
    index = max(0, min(index, len(image_paths) - 1))

    path = image_paths[index]
    print(f"\n🔍 [{index+1}/{len(image_paths)}] กำลังประมวลผล: {path}")

    img = cv2.imread(path)
    if img is None:
        print(f"⚠️ อ่านภาพไม่ได้: {path}")
        index += 1
        continue

    # ตรวจจับวัตถุ
    results = model(img, verbose=False)
    boxes = results[0].boxes
    names = model.names  # ชื่อคลาสทั้งหมด

    # 🔹 กรองเฉพาะ box ที่มี conf >= 0.6
    filtered_indices = [i for i, b in enumerate(boxes.conf) if b >= CONF_THRESHOLD]

    if len(filtered_indices) > 0:
        print(f"🟩 ตรวจพบ {len(filtered_indices)} วัตถุ (ค่า conf >= {CONF_THRESHOLD*100:.0f}%)")

        # เก็บค่าที่มั่นใจสูงสุดของแต่ละคลาส
        best_results = {}
        for i in filtered_indices:
            box = boxes[i]
            cls_id = int(box.cls[0])
            conf = float(box.conf[0])
            label = names[cls_id]
            if label not in best_results or conf > best_results[label]:
                best_results[label] = conf

        print(f"✨ ผลลัพธ์ที่มั่นใจสูงสุดในแต่ละคลาส:")
        for label, conf in best_results.items():
            print(f"   - {label} ({conf*100:.2f}%)")

        # ✅ วาดเฉพาะกรอบที่ผ่านเกณฑ์
        filtered_boxes = boxes[filtered_indices]
        results[0].boxes = filtered_boxes
        annotated_img = results[0].plot()

    else:
        print(f"❌ ไม่มีวัตถุที่ conf >= {CONF_THRESHOLD*100:.0f}%")
        annotated_img = img

    # แสดงภาพ
    cv2.imshow("YOLO11 Batch Detection", annotated_img)

    # รับการกดปุ่ม
    key = cv2.waitKey(0) & 0xFF
    if key == ord('q'):
        print("🚪 ออกจากโปรแกรม")
        break
    elif key == ord('n'):
        index += 1  # ถัดไป
    elif key == ord('b'):
        index -= 1  # ย้อนกลับ

cv2.destroyAllWindows()
print("✅ ตรวจจับภาพทั้งหมดเสร็จสิ้น")
