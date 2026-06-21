# دليل الرفع باستخدام Arduino IDE / Arduino IDE Upload Guide

يحتوي هذا الدليل على خطوات تفصيلية لكيفية فتح وبرمجة المشروع على بوردة ESP32 باستخدام واجهة **Arduino IDE**.

---

## 🇪🇬 باللغة العربية: خطوات الرفع بالتفصيل

### 1. المتطلبات الأساسية
* تحميل وتثبيت برنامج **[Arduino IDE](https://www.arduino.cc/en/software)** (يفضل إصدار 2.0 أو أحدث).
* كابل USB مناسب لتوصيل البوردة بالكمبيوتر.

### 2. تثبيت حزمة تعريفات ESP32 في Arduino IDE
إذا لم تكن قد قمت بتثبيت دعم بوردات ESP32 من قبل، اتبع التالي:
1. افتح برنامج Arduino IDE.
2. اذهب إلى **File** -> **Preferences**.
3. في خانة **Additional boards manager URLs**، ضع الرابط التالي:
   ```text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. اضغط **OK**.
5. اذهب إلى **Tools** -> **Board** -> **Boards Manager**.
6. ابحث عن **esp32** بواسطة Espressif Systems واضغط **Install**.

### 3. فتح المشروع
1. قم بتحميل أو عمل Clone للمستودع على جهازك.
2. افتح مجلد `EspScopeGen` وافتح ملف `EspScopeGen.ino` باستخدام برنامج Arduino IDE.

### 4. إعداد خيارات البوردة (Board Settings)
اذهب إلى قائمة **Tools** وقم بضبط الإعدادات التالية:
* **Board**: اختر `ESP32 Wrover Module` (أو البوردة المتوافقة مع نوع رقاقتك).
* **Upload Speed**: اختر `921600` (لتسريع الرفع) أو `115200` (إذا واجهت مشاكل اتصال).
* **Flash Frequency**: اختر `80MHz`.
* **Partition Scheme**: اختر `Huge APP (3MB No OTA/1MB SPIFFS)` أو `Default 4MB with spiffs` (حجم الكود حوالي 1 ميجابايت لذا سيعمل على الإعدادات الافتراضية).
* **Port**: اختر منفذ الـ COM الخاص بالبوردة بعد توصيلها (مثال: `COM5`).

### 5. ترجمة ورفع الكود (Compile & Upload)
1. اضغط على زر **Verify** (علامة الصح) في الأعلى للتأكد من عدم وجود أخطاء.
2. اضغط على زر **Upload** (السهم المتجه لليمين).
3. **💡 ملاحظة هامة جداً أثناء الرفع:**
   * بمجرد أن ترى جملة `Connecting........___` تظهر في الجزء السفلي من الشاشة، **اضغط فوراً وبشكل مستمر على زر الـ BOOT** الموجود في البوردة.
   * لا تترك الزر حتى ترى النسبة المئوية للرفع بدأت بالعد (مثل `Writing at 0x00010000... (10 %)`). حينها يمكنك ترك الزر.
   * عند اكتمال الرفع سترى رسالة `Hard resetting via RTS pin...`.

### 6. الاتصال بالواجهة
1. افتح شبكة الـ WiFi في جوالك أو الكمبيوتر.
2. اتصل بالشبكة التي تحمل اسم: `ESP32_WROVER_DASHBOARD`.
3. افتح المتصفح واذهب إلى العنوان: `http://192.168.4.1` لرؤية لوحة التحكم وراسم الإشارة!

---

## 🇬🇧 English: Step-by-Step Upload Instructions

### 1. Prerequisites
* Download and install the **[Arduino IDE](https://www.arduino.cc/en/software)** (v2.0 or newer recommended).
* A USB cable to connect the ESP32 board to your computer.

### 2. Installing ESP32 Board Support
If you haven't installed ESP32 support in your Arduino IDE yet:
1. Open Arduino IDE.
2. Go to **File** -> **Preferences**.
3. In **Additional boards manager URLs**, add this URL:
   ```text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click **OK**.
5. Go to **Tools** -> **Board** -> **Boards Manager**.
6. Search for **esp32** by Espressif Systems and click **Install**.

### 3. Opening the Project
1. Clone or download this repository.
2. Open the `EspScopeGen` folder and double-click `EspScopeGen.ino` to open it in Arduino IDE.

### 4. Board Configuration
Under the **Tools** menu, configure the following settings:
* **Board**: Select `ESP32 Wrover Module` (or your specific ESP32 variant).
* **Upload Speed**: Select `921600` (or `115200` if you encounter stability issues).
* **Flash Frequency**: `80MHz`.
* **Partition Scheme**: `Huge APP (3MB No OTA/1MB SPIFFS)` or `Default 4MB with spiffs`.
* **Port**: Select the COM port corresponding to your connected board (e.g. `COM5`).

### 5. Compile & Upload
1. Click the **Verify** button (check icon) to compile and check for errors.
2. Click the **Upload** button (arrow icon).
3. **💡 CRITICAL STEP DURING UPLOAD:**
   * As soon as you see the message `Connecting........___` in the console, **press and hold the BOOT button** on the ESP32 board.
   * Keep holding it until you see the flashing progress start (e.g., `Writing at 0x00010000... (10 %)`). You can then release the button.
   * Upon successful upload, you will see `Hard resetting via RTS pin...`.

### 6. Connect to the Web Dashboard
1. Connect your PC/Phone's Wi-Fi to the network: `ESP32_WROVER_DASHBOARD`.
2. Open a web browser and go to: `http://192.168.4.1`.
