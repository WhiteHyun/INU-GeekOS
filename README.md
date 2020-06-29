# GeekOS codes for OS classes of CS dept. in INU

## 소개

- `GeekOS`는 2001년 Maryland 대학에서 개발된 교육용 운영체제입니다.
- 운영체제가 제공하는 최소한의 기능만 포함되어있습니다(부팅 및 기본 자원 초기화 등).

## 설명

- 위 레포지토리는 인천대학교 `박문주 교수`님의 **운영체제** 수업에 사용됩니다.
- 이론수업과 겸하여 운영체제의 핵심기능을 몇 가지 구현하는 것으로 **Lab0**부터 **Lab4**까지의 프로젝트가 주어집니다. 해당 레포지토리는 **Lab**에 해당되는 자료입니다.

### 설치 과정

- 만약 사용자 운영체제가 `Linux`가 아닌 경우 별도의 가상 환경을 구성해야 합니다.
- `Ubuntu 32-bit` 환경을 구성 후 `QEMU Emulator`, `NASM`, `ruby`, `build-essential` 설치하여야 `GeekOS`를 이용할 수 있습니다.
  > - QEMU Emulator: `GeekOS`를 사용하기 위해 필수적으로 설치해야 하는 에뮬레이터입니다. `Bochs`와 같은 에뮬레이터를 사용해도 되나 사용법이 복잡하여 강의에서는 `QEMU`를 사용합니다.
  >   - 사이트: [QEMU](https://www.qemu.org/)
  > - NASM: 인텔 x86 아키텍처 기반 어셈블러입니다. 다양한 운영체제에서 사용가능하며 개발 용도로도 많이 쓰입니다.
  > - build-essential: 소스코드 빌드 시 필요한 기본적인 패키지들로 구성되어 있습니다. 설치 후에는 gcc, g++, make, perl 등과 각종 라이브러리들이 설치됩니다.
  >   - 출처: [build-essential](http://linux-command.org/ko/build-essential.html)

### Lab

- Lab0: 소스코드, 에뮬레이터에 익숙해지기위해 구성된 프로젝트
- Lab1: `elf parsing` 구성
- Lab2: `Segmentation` 기법을 이용하여 가상메모리 구현 및 유저 프로세스인 shell을 수행
- Lab3: `System-Call` 구현하여 프로세스 상태 제공
- Lab4: `Semaphore` 구현

### 프로젝트 사용 언어

- C
