# EMBEDDED SYSTEM PROJECT
## RTC 시계 및 복합 계산기 시스템 개발
### Data Sheet 해석 및 I2C 통신 기반 시스템 통합

---

## 1. 개요

### 💡 개발 목적
* **데이터시트 해석:** 제조사 제공 문서를 분석하여 레지스터 레벨의 제어 로직을 직접 개발
* **I2C 통신 체득:** DS1307과의 핸드쉐이크 과정을 통해 2-Wire 인터페이스의 물리 계층 동작 원리 이해
* **기능 통합:** 실시간 시간 표출 및 키패드 입력 기반 복합 계산 알고리즘의 시스템 통합

### 🛠️ 개발 환경
* **MCU:** ATmega128 (8-bit AVR)
* **IDE/Tool:** Microchip Studio (Atmel Studio)
* **Main Comm:** I2C, 4-bit Parallel
* **Analysis:** Oscilloscope (Waveform Debug)

---

## 2. H/W: 외관 사진 및 시스템 회로도

### 📸 외관 사진
<img width="620" height="827" alt="image" src="https://github.com/user-attachments/assets/3385ccdb-8cfe-4729-ad70-a75a9929cd7d" />


### 🔌 시스템 회로도
<img width="1165" height="807" alt="image" src="https://github.com/user-attachments/assets/d7de1420-1735-476f-b86d-7ab18be46973" />


### 📋 2.1 PART LIST
| 부품명 | 상세 사양 | 수량 | 주요 역할 |
| :--- | :--- | :---: | :--- |
| **LCD1602** | 16x2 Character, 4-bit Parallel | 1 | 데이터 시각화 출력 장치 |
| **DS1307** | I2C Real-Time Clock | 1 | 실시간 시간 데이터 공급 |
| **Push Switch** | Tact Switch (4-Pin) | 5 | 설정 모드 및 계산기 모드 입력 |
| **Resistor** | 1/4W Axial, 10kΩ | 7 | I2C 풀업 및 스위치 풀다운/풀업 저항 |
| **4x4 Keypad** | Matrix Membrane Type | 1 | 계산기 숫자 및 연산 기호 입력 |
| **MCU** | ATmega128A | 1 | 전체 시스템 로직 및 통신 제어 |

---

## 3. S/W 구현 및 분석

### 🔄 3.1 전체 FSM (Finite State Machine)
<img width="1294" height="808" alt="image" src="https://github.com/user-attachments/assets/d6080207-07e7-459b-a6dc-c0259f448b49" />


---

### 📺 3.2.1 LCD Driver

#### 📌 기본 사양 및 중요 명령어
* **Display:** 16 characters x 2 lines
* **Input High:** 0.7 Vcc min. / **Input Low:** 0.55V max.
* **E-Cycle Time:** 1000ns min. / **E-Pulse Width:** 450ns min.
* **주요 명령어 (Instruction):**
  * 0x01 : Clear Display (1.64ms 대기)
  * 0x06 : Entry Mode (Cursor Auto Inc)
  * 0x28 : Function Set (4-bit, 2-line)
  * 0x0C : Display ON (Cursor OFF)
  * 0x02 : Cursor Return (1.64ms 대기)

#### ⚙️ 제어 및 구조적 사양
* **레지스터 선택 (RS 핀)**
  * RS = 0 : Instruction Register (화면 초기화, 입력 설정 등 명령용)
  * RS = 1 : Data Register (화면에 문자를 직접 표출하기 위한 아스크 코드 저장)
* **내부 메모리 아키텍처**
  * **DDRAM (Display RAM):** 문자 코드 저장 영역. 1행은 0x00, 2행은 0x40 주소 할당
  * **CGROM & CGRAM:** 기본 폰트용 CGROM 및 사용자 커스텀 폰트 구현용 CGRAM
* **동기 제어 타이밍 (E 핀)**
  * **Enable (E) 트리거:** 데이터를 버스에 로드한 후 E 핀을 Pulse 형태로 제어
  * **Falling Edge Latch:** E 핀이 High에서 Low로 천이되는 시점에 내부 래치 회로가 캡처

#### 🔄 LCD 초기화 시퀀스 상세 (4-bit)
1. **STEP 1 (Power On):** Wait 40ms (VCC 안정화)
2. **STEP 2 (8-bit Reset):** 0x30 전송 (3회) (각각 4.1ms / 100us 대기)
3. **STEP 3 (4-bit Conv):** 0x20 전송 (4비트 모드 진입)
4. **STEP 4 (Function Set):** 4-bit 0x28 전송 (4-bit, 2-Line, 16-Chars)
5. **STEP 5 (Final Set):** Disp ON / Clear / Entry Mode 설정

> 💡 **상세 분석:** 부팅 초기 LCD는 8비트 모드인지 4비트 모드인지 알 수 없는 상태일 수 있습니다. 따라서 0x30을 세 번 전송하는 "Software Reset" 과정을 통해 어떤 명령 상태에 있더라도 강제로 8비트 모드로 동기화시킨 후 4비트로 전환하는 것이 신뢰성 확보의 핵심입니다.

---

### ⏰ 3.2.2 DS1307 (I2C) RTC

#### 📌 Spec Summary
* **Interface:** 2-Wire Serial (I2C)
* **Slave Addr:** 0x68 (1101000)
* **BCD Format:** 모든 시간 데이터는 BCD 코드로 관리
* **Battery Backup:** 전원 차단 시 배터리 자동 전환
* *⚠️ 주의: 00h 레지스터의 CH(Clock Halt) 비트가 1이면 시간이 흐르지 않으므로 초기화 시 반드시 0으로 클리어해야 함.*

#### 🗺️ Register Map
| ADDR | BIT 7 | Function |
| :---: | :---: | :--- |
| **00h** | CH | Seconds (CH: Halt) |
| **01h** | X | Minutes (00-59) |
| **02h** | 12/24 | Hours (12/24 Mode) |
| **07h** | OUT | Control (SQW/OUT) |

#### 📡 RTC I2C 통신 동작 시나리오
* **[ Write 시나리오 ]**
  START ➡️ ADDRESS (Slave Addr 0x68 + Write Bit 0) ➡️ ACK (Slave Low) ➡️ Data (Master: Decimal -> BCD) ➡️ ACK (Slave Low) ➡️ STOP
* **[ Read 시나리오 ]**
  START ➡️ ADDRESS (Slave Addr 0x68 + Read Bit 1) ➡️ ACK (Slave Low) ➡️ Data (Slave: BCD -> Decimal) ➡️ NACK (Master High) ➡️ STOP

#### 🔬 오실로스코프 파형 분석
* **시작 주소 지정:** <img width="852" height="710" alt="image" src="https://github.com/user-attachments/assets/84c64f02-13b6-49af-a834-cd83f544cef5" />
START ➡️ 0x68 (addr) + WRITE ➡️ ACK ➡️ 0x00 ➡️ ACK ➡️ STOP
* **READ 시작:** <img width="853" height="710" alt="image" src="https://github.com/user-attachments/assets/fcb0b827-f567-4e7c-99cc-23a3d3e8f69d" />
START ➡️ 0x68 (addr) + READ ➡️ ACK ➡️ 0x54 (second)
<img width="790" height="732" alt="image" src="https://github.com/user-attachments/assets/db21b128-e6fa-4758-afea-284d93d28c23" />
➡️ ACK ➡️ 0x49 (minutes) ➡️ ACK ➡️ 0x12 (hours)
<img width="869" height="716" alt="image" src="https://github.com/user-attachments/assets/5f9a04e7-ce73-44ca-b814-3abb78ac7aff" />
➡️ ACK ➡️ 0x02 (day) ➡️ ACK ➡️ 0x02 (date) ➡️ ACK
<img width="851" height="708" alt="image" src="https://github.com/user-attachments/assets/b2a93ae4-0e4a-4017-a2f7-a7027895a676" />
➡️ 0x07 (month) ➡️ ACK ➡️ 0x26 (year) ➡️ NACK ➡️ STOP
<img width="844" height="688" alt="image" src="https://github.com/user-attachments/assets/a323a3b6-c81a-49d6-b270-8bcddaf38fdf" />

---

### 🧮 3.2.3 계산기 알고리즘: Stack & Queue

<img width="1556" height="797" alt="image" src="https://github.com/user-attachments/assets/9f3b59ef-79f5-489d-9b9f-b7b748bc3fbf" />

#### 📌 후위 표기법 (Postfix) 변환 및 연산
* 복합 수식의 연산자 우선순위 해결을 위해 중위 표기법(Infix)을 후위 표기법으로 변환하여 계산합니다.
* **예시:** 3 + 4 * 2 (Infix) ➡️ 3 4 2 * + (Postfix)

#### ⚙️ 구현 메커니즘
* **STACK:** 연산자( +, -, *, / ) 및 괄호의 우선순위 관리
* **QUEUE:** 변환된 수식의 토큰을 순서대로 저장
* **Algorithm:** 피연산자는 큐(Queue)로 직행하고, 연산자는 스택(Stack) 최상단 원소와 우선순위를 비교하여 Pop/Push를 수행합니다.

---

## 4. 전체 구조

<img width="1224" height="566" alt="image" src="https://github.com/user-attachments/assets/434a332c-8b74-4fe5-9591-185c2d83798b" />

---

## 5. 동작 영상

[[Reset 동작]](https://github.com/user-attachments/assets/9201ae2a-b272-4af6-81cc-fa5492c17b1b)


[[계산기 대기 동작]](https://github.com/user-attachments/assets/16aa15ed-f6f5-4f53-aef4-6619253aef0c)


[[계산기 Shift 동작]](https://github.com/user-attachments/assets/3c961f86-75e2-441f-8e36-236fd8bb8af0)


[[전체 동작]](https://github.com/user-attachments/assets/b943fc2d-21e4-4a6d-8d8f-e8051909b757)


---

## 📝 6. 구현 후기

* **[LCD 하드웨어 시프트]** 명령어의 물리적 실행 시간(1.64ms) 제약을 고려하여 4비트 인터페이스의 타이밍 동기화를 정밀하게 제어함으로써, 긴 수식 입력 시 화면이 부드럽게 밀리는 하드웨어 스크롤 기능을 성공적으로 구현했습니다.
* **[오버플로우 방지]** 정수 범위 한계를 극복하기 위해 문자열 변환 버퍼 함수로 제어하여 큰 자릿수도 깨짐없이 정상 계산되도록 완성했습니다.
* **[데이터시트 및 I2C 후기]** 데이터시트의 타이밍 스펙을 엄격히 준수하는 것이 시스템 신뢰성의 핵심임을 깨달았고, 오실로스코프로 I2C 버스의 실시간 BCD 파형을 직접 계측 및 분석해 보며 임베디드 통신 제어의 깊이를 더한 값진 경험이었습니다.
