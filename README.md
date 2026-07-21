### ESP32-TLE5012-PCNT: High-Resolution Hardware Quadrature Interface

Repositori ini mengimplementasikan antarmuka *zero-CPU-overhead* untuk *magnetic encoder* Infineon TLE5012 pada resolusi penuh 12-bit (16384 CPR) menggunakan perangkat keras Pulse Counter (PCNT) bawaan ESP32.

## 🧠 Konteks Rekayasa & Justifikasi Desain

Sebagian besar implementasi *encoder* inkremental di ekosistem Arduino mengandalkan *Software Interrupts* (ISR). Pendekatan ini gagal pada sistem beresolusi tinggi (seperti TLE5012 yang memuntahkan 16384 pulsa per putaran) ketika motor berputar cepat, menyebabkan *pulse loss*, latensi CPU tinggi, dan kegagalan pada *loop* kontrol PID (misalnya pada robotika atau mesin CNC).

**Pendekatan Kami:**
Kami mem-Bypass ISR perangkat lunak dan mengalihkan seluruh tugas penghitungan pulsa ke **Hardware PCNT Unit** di ESP32.

* **CPU Bebas:** Penghitungan kuadratur dilakukan murni oleh silikon *peripheral*, menyisakan 100% siklus CPU untuk komputasi utama (kontroler, filter, atau RTOS *tasks*).
* **Presisi Ekstrem:** Mempertahankan keakuratan 16384 CPR tanpa meleset satu pulsa pun pada kecepatan tinggi.

## ⚙️ Arsitektur Data

* **Mode Dekoding:** *Full Quadrature* (x4 Evaluation).
* **Target CPR:** 4096 PPR $\times$ 4 = **16384 Counts per Revolution**.
* **Timer Akuisisi:** Asinkron / Non-blocking 20Hz (50ms) menggunakan deteksi selisih *Timestamp*.

## ⚠️ Batasan Sistem (Known Trade-offs)

Kami percaya pada transparansi teknis. Sebelum Anda menggunakan kode ini dalam lingkungan industri, perhatikan hal berikut:

1. **Velocity Jitter pada Kecepatan Rendah:** Kalkulasi RPM dilakukan dengan metode turunan diskrit ($\Delta Count / \Delta Time$) pada jendela 50ms. Pada kecepatan sangat rendah, efek kuantisasi dari diskritisasi waktu dapat memicu osilasi sinyal kecepatan. Direkomendasikan untuk menambahkan filter *Low-Pass* (EMA atau Kalman 1D) pada variabel `rpm` sebelum diumpankan ke kontroler PID.
2. **Hardware Overflow:** PCNT beroperasi pada register 16-bit (-32768 hingga 32767). Pada resolusi 16384 CPR, register mencapai batas maksimal dalam $\sim$ 2 putaran. Pustaka menangani penumpukan (*overflow handling*) di latar belakang, namun rotasi pada puluhan ribu RPM secara teoretis dapat membanjiri *interrupt* penanganan *overflow* tersebut.
3. **Integritas Sinyal:** Kode mengaktifkan *weak internal pull-up* ESP32 sebagai pengaman. Untuk kabel *encoder* di atas 20 cm yang berada di lingkungan penuh EMI (dekat motor *brushless/stepper*), pasang resistor *pull-up* eksternal $4.7 k\Omega$ perangkat keras pada jalur A dan B.

## 🚀 Penggunaan Lintas Platform

Integrasikan ke dalam loop utama tanpa memblokir sistem:

```cpp
// Contoh keluaran (Baud: 115200)
// CNT: 4096      Angle: 90.00°    RPM: 120.5
// CNT: -16384    Angle: 0.00°     RPM: -240.2

```
