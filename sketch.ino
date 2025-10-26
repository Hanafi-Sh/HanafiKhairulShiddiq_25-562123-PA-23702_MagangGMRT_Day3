// Nama: Hanafi Khairul Shiddiq
// NIM: 25/562123/PA/23702

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <ESP32Servo.h>

Adafruit_MPU6050 mpu; // Inisialisasi objek pada komponen MPU6050 ke variabel mpu
Servo servo1; // Inisialisasi objek untuk semua komponen servo ke masing-masing servo
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;

float yawBefore = 0; // variabel ini digunakan untuk menyimpan yaw sebelumnya untuk mengambil selisih dari yaw saat ini. 
int sudutServo5Before = 0; // variabel ini digunakan untuk menyimpan sudut terakhir servo5 untuk mengambil selisih dari sudut saat ini
unsigned long lastStableTime = 0; // variabel ini menyimpan waktu terakhir servo5 stabil
bool servo5Back = false; // variabel ini bertujuan untuk memberi tahu apakah servo5 berada dalam masa 'delay lalu kembali', atau tidak
int counter = 0; // fungsi counter adalah agar mikrokontroller tidak terlalu sensitif untuk menentukan bahwa servo5 sudah stabil (tidak bergerak)
// jika tidak menggunakan counter, maka mikon akan sangat sensitif dan mudah memutuskan bahwa sensor sudah tidak bergerak lagi, sehingga sebentar saja sensor berhenti akan dideteksi sebagai stabil padahal masih bergerak.

const float threshold = 0.0; // threshold untuk menentukan seberapa sensitif pergerakan yang dihitung pergerakan, bukan noise atau kesalahan

// Mendefinisikan pin-pin sesuai dengan GPIO yang dikaitkan pada hardware sehingga tidak perlu menghafal setiap nomornya
#define PIN_PIR 13
#define PIN_SERVO1 23
#define PIN_SERVO2 5
#define PIN_SERVO3 19
#define PIN_SERVO4 18
#define PIN_SERVO5 17
#define PIN_SCL 22
#define PIN_SDA 21

void setup() {
  Serial.begin(115200); //Inisialisasi Serial
  Serial.println("Hello, ESP32!"); // Memberikan output bahwa program berhasil dimulai

  mpu.begin(); // Inisialisasi MPU6050
  pinMode(PIN_PIR, INPUT); // Mengatur pin dari PIR sebagai Input 
  servo1.attach(PIN_SERVO1); // Menghubungkan setiap servo ke GPIO nya masing-masing
  servo2.attach(PIN_SERVO2);
  servo3.attach(PIN_SERVO3);
  servo4.attach(PIN_SERVO4);
  servo5.attach(PIN_SERVO5);

  // Inisialisasi semua servo ke posisi awal
  servo1.write(0);
  servo2.write(0);
  servo3.write(0);
  servo4.write(0);
  servo5.write(0);
}

void loop() {
  sensors_event_t a, g, temp; // Deklarasi variabel dari MPU
  mpu.getEvent(&a, &g, &temp); // Meminta data terbaru dari mpu

  float roll = g.gyro.x; // menyimpan perubahan x ke dalam variabel roll
  float pitch = g.gyro.y; // menyimpan perubahan y ke dalam variabel roll
  float yaw = g.gyro.z;// menyimpan perubahan z ke dalam variabel roll
  int statusPIR = digitalRead(PIN_PIR); //baca sensor dari PIR

  int sudut_ditentukan = 50; // variabel untuk servo bergerak ke sudut tertentu ketika PIR mendeteksi sesuatu
  if (statusPIR == HIGH) { // Mengecek apakah PIR menerima sinyal (terjadi gerakan) atau tidak
    Serial.println("Ada gerakan pada PIR!");
    servo1.write(sudut_ditentukan); // Menggerakkan semua servo ke sudut yang sudah ditentukan
    servo2.write(sudut_ditentukan);
    servo3.write(sudut_ditentukan);
    servo4.write(sudut_ditentukan);
    servo5.write(sudut_ditentukan);
    delay(1000); // Delay selama 1 detika sebelum servo kembali ke posisi semula
    servo1.write(0); // Mengembalikan semua servo ke posisi awal, yaitu 0
    servo2.write(0);
    servo3.write(0);
    servo4.write(0);
    servo5.write(0);
  } else {
    // Serial.println("Tidak ada gerakan yang terdeteksi...");
    int sudutServo1_2 = map(roll, -4.36, 4.36, 180, 0); // rentang input MPU adalah dari -4.36 sampai 4.36 (kita ubah rentangnya menjadi rentang pada servo), dan proses ini dibalik sesuai permintaan soal.
    
    servo1.write(sudutServo1_2); // Menggerakkan servo 1 berlawanan arah berdasarkan input yang masuk dari MPU
    servo2.write(sudutServo1_2); // Menggerakkan servo 2 berlawanan arah berdasarkan input yang masuk dari MPU

    int sudutServo3_4 = map(pitch, -4.36, 4.36, 0, 180); // rentang MPU adalah dari -4.36 sampai 4.36 (kita ubah rentangnya menjadi rentang pada servo), proses ini tidak dibalik karena permintaan soal searah.
    servo3.write(sudutServo3_4); // Menggerakkan servo 3 searah berdasarkan input yang masuk dari MPU
    servo4.write(sudutServo3_4); // Menggerakkan servo 4 searah berdasarkan input yang masuk dari MPU
    
    int sudutServo5 = map(yaw, -4.36, 4.36, 0, 180); // rentang MPU adalah dari -4.36 sampai 4.36 (kita ubah rentangnya menjadi rentang pada servo).
    if (!servo5Back) { // apakah variabel servo5Back bernilai false?
        float selisihYaw = abs(yaw - yawBefore); // Buat variabel selisih Yaw untuk mendeteksi pergerakan
        if (selisihYaw > threshold) { // apakah selisihYaw (pergerakan) melebih threshold, semakin rendah threshold maka makin sensitif dengan gerakan. 
            servo5.write(sudutServo5); // Menggerakkan servo5 ke sudutServo5
            counter = 0; // Mengubah counter menjadi 0 agar sensor tidak sensitif mendeteksi bahwa benda berhenti bergerak
        } else if (sudutServo5 != sudutServo5Before && counter > 9) { // apakah terjadi pergerakan, dan apakah counter sudah melebihi 9
            lastStableTime = millis(); // memasukkan waktu sekarang untuk memberikan nilai kapan terakhir sensor stabil (tidak mendeteksi pergerakan)
            servo5Back = true; // mengubah servo5Back menjadi true, yang artinya setelah ini servo akan masuk ke mode delay dan kembali ke posisi awal
            counter = 0; // mereset counter menjadi 0 setelah menginisialisai servo5Back menjadi true. kalau hal ini tidak dilakukan, maka counter tidak akan terreset setelah penggunaan dan counter menjadi tidak berguna.
        } else counter++; // increment counter
    } else {
        Serial.println(millis() - lastStableTime); // ini untuk debug
        if (millis() - lastStableTime > 1000) { // apakah delay servo5 sudah 1 detik? jika langsung menggunakan fungsi delay() akan memberhentikan seluruh program.
            servo5.write(0); // jika delay sudah 1 detik, kembalikan posisi servo5 ke posisi semula (sesuai permintaan soal)
            servo5Back = false; // mengubah variabel servo5Back menjadi false lagi sebagai tanda bahwa servo5 tidak lagi dalam masa ''delay lalu kembali''
            sudutServo5Before = sudutServo5; // mengubah sudutBefore menjadi sudut saat ini. ini merupakan inisialisasi dasar untuk semua variabel before
        }
    }
    yawBefore = g.gyro.z; // ini juga merupakan inisialisasi dasar untuk variabel before
  }

  delay(10); // delay utama agar ESP32 tidak bekerja terlalu berat
}
