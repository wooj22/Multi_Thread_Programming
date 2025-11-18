# Actor Model - 메시지 패싱 기반 동시성

## Actor Model의 개념과 필요성
전통적인 멀티스레드 프로그래밍에서는 여러 스레드가 공유 메모리에 직접 접근하여 데이터를 읽고 쓴다. 이러한 접근 방식은 동기화 문제를 야기하며, 뮤텍스(Mutex), 크리티컬 섹션(Critical Section) 등의 동기화 객체를 사용하여 데이터 무결성을 보장해야 한다. 하지만 락(Lock)을 사용하는 방식은 데드락(Deadlock), 우선순위 역전(Priority Inversion), 경합 상태(Race Condition) 등 다양한 문제를 발생시킬 수 있다.

Actor Model은 이러한 문제를 근본적으로 다른 방식으로 접근한다. 각 Actor는 독립적인 실행 단위로, 자신만의 상태(State)를 가지며, 다른 Actor와는 오직 메시지를 통해서만 통신한다. Actor는 공유 메모리를 사용하지 않기 때문에 전통적인 락 메커니즘이 필요하지 않다.

### Actor Model의 핵심 원칙
Actor Model은 다음과 같은 세 가지 핵심 원칙을 따른다.

첫째, Actor는 수신한 메시지에 대한 응답으로 새로운 Actor를 생성할 수 있다. 이는 시스템의 동적 확장성을 제공한다.

둘째, Actor는 다른 Actor에게 메시지를 전송할 수 있다. 메시지는 비동기적으로 전송되며, 발신자는 메시지를 보낸 후 즉시 다음 작업을 수행할 수 있다.

셋째, Actor는 다음에 수신할 메시지를 처리하는 방법을 결정할 수 있다. 즉, 메시지 처리 로직을 동적으로 변경할 수 있다.

### Actor Model의 장점
Actor Model은 여러 중요한 장점을 제공한다. 가장 중요한 것은 동시성 프로그래밍의 복잡도를 크게 줄인다는 점이다. 각 Actor는 한 번에 하나의 메시지만 처리하므로, Actor 내부에서는 동기화가 필요하지 않다. 이는 프로그래머가 복잡한 락 관리를 고민할 필요가 없음을 의미한다.

또한 Actor Model은 높은 수준의 캡슐화를 제공한다. 각 Actor의 내부 상태는 외부에서 직접 접근할 수 없으며, 오직 메시지를 통해서만 상호작용할 수 있다. 이는 시스템의 모듈성을 향상시키고 유지보수를 용이하게 만든다.

확장성 측면에서도 Actor Model은 우수하다. Actor는 독립적으로 실행되므로, 여러 프로세서 코어나 심지어 분산 시스템에서도 쉽게 확장할 수 있다.
  

## Win32 환경에서의 Actor Model 구현
Win32 API를 사용하여 Actor Model을 구현하기 위해서는 스레드와 메시지 큐를 적절히 활용해야 한다. 각 Actor는 자신만의 스레드와 메시지 큐를 가지며, 메시지 큐를 통해 다른 Actor와 통신한다.

### 메시지 구조 설계
Actor 간의 통신을 위한 메시지 구조는 확장 가능하고 유연해야 한다. 메시지는 메시지 타입, 발신자 정보, 그리고 실제 데이터를 포함해야 한다.

```cpp
// 메시지 타입 정의
enum class MessageType {
    PROCESS_DATA,
    REQUEST_STATE,
    SHUTDOWN,
    CUSTOM_MESSAGE
};

// 기본 메시지 구조
struct Message {
    MessageType type;
    void* sender;
    void* data;
    size_t dataSize;
    
    Message(MessageType t, void* s, void* d, size_t size)
        : type(t), sender(s), data(d), dataSize(size) {}
    
    ~Message() {
        if (data != nullptr) {
            delete[] static_cast<char*>(data);
        }
    }
};
```

### 스레드 안전 메시지 큐 구현
Actor 간의 메시지 전달을 위해서는 스레드 안전한 메시지 큐가 필요하다. 이 큐는 여러 스레드에서 동시에 접근할 수 있으므로, 크리티컬 섹션을 사용하여 동기화해야 한다.

```cpp
#include <windows.h>
#include <queue>
#include <memory>

class MessageQueue {
private:
    std::queue<std::unique_ptr<Message>> queue;
    CRITICAL_SECTION cs;
    HANDLE semaphore;
    bool isShutdown;

public:
    MessageQueue() : isShutdown(false) {
        InitializeCriticalSection(&cs);
        semaphore = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
    }

    ~MessageQueue() {
        Shutdown();
        CloseHandle(semaphore);
        DeleteCriticalSection(&cs);
    }

    void Enqueue(std::unique_ptr<Message> msg) {
        EnterCriticalSection(&cs);
        
        if (!isShutdown) {
            queue.push(std::move(msg));
            ReleaseSemaphore(semaphore, 1, NULL);
        }
        
        LeaveCriticalSection(&cs);
    }

    std::unique_ptr<Message> Dequeue(DWORD timeout = INFINITE) {
        DWORD result = WaitForSingleObject(semaphore, timeout);
        
        if (result != WAIT_OBJECT_0) {
            return nullptr;
        }

        EnterCriticalSection(&cs);
        
        std::unique_ptr<Message> msg = nullptr;
        if (!queue.empty()) {
            msg = std::move(queue.front());
            queue.pop();
        }
        
        LeaveCriticalSection(&cs);
        
        return msg;
    }

    void Shutdown() {
        EnterCriticalSection(&cs);
        isShutdown = true;
        LeaveCriticalSection(&cs);
        
        // 대기 중인 스레드를 깨우기 위해 세마포어 신호
        ReleaseSemaphore(semaphore, 1, NULL);
    }

    bool IsShutdown() const {
        return isShutdown;
    }
};
```

### Actor 기본 클래스 구현
모든 Actor가 상속받을 기본 클래스를 구현한다. 이 클래스는 메시지 큐, 스레드 관리, 메시지 처리 루프 등의 공통 기능을 제공한다.

```cpp
class Actor {
protected:
    MessageQueue messageQueue;
    HANDLE thread;
    DWORD threadId;
    bool running;

    // 순수 가상 함수: 파생 클래스에서 구현
    virtual void HandleMessage(const Message& msg) = 0;

    // 메시지 처리 루프
    static DWORD WINAPI ThreadProc(LPVOID param) {
        Actor* actor = static_cast<Actor*>(param);
        actor->MessageLoop();
        return 0;
    }

    void MessageLoop() {
        while (running) {
            auto msg = messageQueue.Dequeue(100);
            
            if (msg != nullptr) {
                if (msg->type == MessageType::SHUTDOWN) {
                    running = false;
                    break;
                }
                
                HandleMessage(*msg);
            }
        }
    }

public:
    Actor() : thread(NULL), threadId(0), running(false) {}

    virtual ~Actor() {
        Stop();
    }

    void Start() {
        if (!running) {
            running = true;
            thread = CreateThread(NULL, 0, ThreadProc, this, 0, &threadId);
        }
    }

    void Stop() {
        if (running) {
            // 종료 메시지 전송
            SendMessage(MessageType::SHUTDOWN, nullptr, nullptr, 0);
            
            // 스레드 종료 대기
            if (thread != NULL) {
                WaitForSingleObject(thread, INFINITE);
                CloseHandle(thread);
                thread = NULL;
            }
        }
    }

    void SendMessage(MessageType type, void* sender, void* data, size_t dataSize) {
        // 데이터 복사 (소유권 이전)
        void* dataCopy = nullptr;
        if (data != nullptr && dataSize > 0) {
            dataCopy = new char[dataSize];
            memcpy(dataCopy, data, dataSize);
        }
        
        auto msg = std::make_unique<Message>(type, sender, dataCopy, dataSize);
        messageQueue.Enqueue(std::move(msg));
    }

    DWORD GetThreadId() const {
        return threadId;
    }
};
```
  

## 실용적인 Actor Model 예제: 이미지 처리 파이프라인
Actor Model의 실제 활용을 보여주기 위해 이미지 처리 파이프라인을 구현해보겠다. 이 시스템은 여러 단계의 이미지 처리를 각각의 Actor로 구현하여 병렬 처리를 수행한다.

### 이미지 데이터 구조

```cpp
struct ImageData {
    int width;
    int height;
    unsigned char* pixels;  // RGB 데이터 (width * height * 3)
    int imageId;

    ImageData(int w, int h, int id)
        : width(w), height(h), imageId(id) {
        pixels = new unsigned char[w * h * 3];
    }

    ~ImageData() {
        delete[] pixels;
    }

    // 복사 생성자
    ImageData(const ImageData& other)
        : width(other.width), height(other.height), imageId(other.imageId) {
        size_t size = width * height * 3;
        pixels = new unsigned char[size];
        memcpy(pixels, other.pixels, size);
    }
};
```

### 이미지 로더 Actor
파일에서 이미지를 읽어오는 Actor를 구현한다. (실제 구현에서는 간단한 더미 데이터를 생성한다.)

```cpp
class ImageLoaderActor : public Actor {
private:
    Actor* nextActor;  // 다음 처리 단계 Actor

protected:
    void HandleMessage(const Message& msg) override {
        if (msg.type == MessageType::PROCESS_DATA) {
            int imageId = *static_cast<int*>(msg.data);
            
            printf("[Loader] Loading image %d (Thread: %lu)\n", 
                   imageId, GetCurrentThreadId());
            
            // 이미지 로딩 시뮬레이션
            ImageData* imgData = new ImageData(800, 600, imageId);
            
            // 더미 데이터 생성 (그레이스케일)
            for (int i = 0; i < imgData->width * imgData->height * 3; i += 3) {
                unsigned char value = (unsigned char)(rand() % 256);
                imgData->pixels[i] = value;
                imgData->pixels[i + 1] = value;
                imgData->pixels[i + 2] = value;
            }
            
            Sleep(100);  // 로딩 시간 시뮬레이션
            
            // 다음 Actor로 전달
            if (nextActor != nullptr) {
                nextActor->SendMessage(MessageType::PROCESS_DATA, 
                                      this, imgData, sizeof(ImageData*));
            }
        }
    }

public:
    ImageLoaderActor(Actor* next) : nextActor(next) {}
};
```

### 이미지 필터 Actor
이미지에 필터를 적용하는 Actor를 구현한다.

```cpp
class ImageFilterActor : public Actor {
private:
    Actor* nextActor;

    void ApplyGrayscaleFilter(ImageData* img) {
        // 이미 그레이스케일이지만, 처리 시간을 시뮬레이션
        for (int i = 0; i < img->width * img->height * 3; i += 3) {
            unsigned char avg = (img->pixels[i] + 
                               img->pixels[i + 1] + 
                               img->pixels[i + 2]) / 3;
            img->pixels[i] = avg;
            img->pixels[i + 1] = avg;
            img->pixels[i + 2] = avg;
        }
    }

protected:
    void HandleMessage(const Message& msg) override {
        if (msg.type == MessageType::PROCESS_DATA) {
            ImageData* imgData = *static_cast<ImageData**>(msg.data);
            
            printf("[Filter] Processing image %d (Thread: %lu)\n", 
                   imgData->imageId, GetCurrentThreadId());
            
            ApplyGrayscaleFilter(imgData);
            Sleep(150);  // 필터링 시간 시뮬레이션
            
            // 다음 Actor로 전달
            if (nextActor != nullptr) {
                nextActor->SendMessage(MessageType::PROCESS_DATA, 
                                      this, imgData, sizeof(ImageData*));
            }
        }
    }

public:
    ImageFilterActor(Actor* next) : nextActor(next) {}
};
```

### 이미지 저장 Actor
처리된 이미지를 저장하는 Actor를 구현한다.

```cpp
class ImageSaverActor : public Actor {
private:
    int savedCount;
    CRITICAL_SECTION countCs;

protected:
    void HandleMessage(const Message& msg) override {
        if (msg.type == MessageType::PROCESS_DATA) {
            ImageData* imgData = *static_cast<ImageData**>(msg.data);
            
            printf("[Saver] Saving image %d (Thread: %lu)\n", 
                   imgData->imageId, GetCurrentThreadId());
            
            Sleep(100);  // 저장 시간 시뮬레이션
            
            EnterCriticalSection(&countCs);
            savedCount++;
            printf("[Saver] Total saved: %d\n", savedCount);
            LeaveCriticalSection(&countCs);
            
            // 메모리 해제
            delete imgData;
        }
    }

public:
    ImageSaverActor() : savedCount(0) {
        InitializeCriticalSection(&countCs);
    }

    ~ImageSaverActor() {
        DeleteCriticalSection(&countCs);
    }

    int GetSavedCount() {
        EnterCriticalSection(&countCs);
        int count = savedCount;
        LeaveCriticalSection(&countCs);
        return count;
    }
};
```

### 파이프라인 통합 예제

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand((unsigned int)time(NULL));
    
    printf("=== Actor Model Image Processing Pipeline ===\n\n");
    
    // Actor 파이프라인 생성 (역순으로 생성)
    ImageSaverActor saver;
    ImageFilterActor filter(&saver);
    ImageLoaderActor loader(&filter);
    
    // 모든 Actor 시작
    saver.Start();
    filter.Start();
    loader.Start();
    
    printf("Pipeline started. Processing 10 images...\n\n");
    
    // 10개의 이미지 처리 요청
    DWORD startTime = GetTickCount();
    
    for (int i = 1; i <= 10; i++) {
        loader.SendMessage(MessageType::PROCESS_DATA, nullptr, &i, sizeof(int));
        Sleep(50);  // 요청 간격
    }
    
    // 모든 이미지가 처리될 때까지 대기
    while (saver.GetSavedCount() < 10) {
        Sleep(100);
    }
    
    DWORD endTime = GetTickCount();
    
    printf("\n=== Processing Complete ===\n");
    printf("Total time: %lu ms\n", endTime - startTime);
    printf("Images processed: %d\n", saver.GetSavedCount());
    
    // Actor 정리
    loader.Stop();
    filter.Stop();
    saver.Stop();
    
    return 0;
}
```
  

## Actor Model의 고급 패턴

### Request-Reply 패턴
Actor 간의 양방향 통신을 구현하는 패턴이다. 요청을 보낸 Actor가 응답을 기다려야 할 때 사용한다.

```cpp
// 응답 대기를 위한 Future 클래스
template<typename T>
class Future {
private:
    T* value;
    HANDLE event;
    CRITICAL_SECTION cs;
    bool ready;

public:
    Future() : value(nullptr), ready(false) {
        InitializeCriticalSection(&cs);
        event = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    ~Future() {
        CloseHandle(event);
        DeleteCriticalSection(&cs);
        if (value != nullptr) {
            delete value;
        }
    }

    void Set(const T& val) {
        EnterCriticalSection(&cs);
        if (!ready) {
            value = new T(val);
            ready = true;
            SetEvent(event);
        }
        LeaveCriticalSection(&cs);
    }

    T Get(DWORD timeout = INFINITE) {
        WaitForSingleObject(event, timeout);
        
        EnterCriticalSection(&cs);
        T result = *value;
        LeaveCriticalSection(&cs);
        
        return result;
    }

    bool IsReady() const {
        return ready;
    }
};

// Request-Reply를 지원하는 Actor
class RequestReplyActor : public Actor {
private:
    struct RequestContext {
        Future<int>* future;
        int requestData;
    };

protected:
    void HandleMessage(const Message& msg) override {
        if (msg.type == MessageType::REQUEST_STATE) {
            RequestContext* ctx = static_cast<RequestContext*>(msg.data);
            
            printf("[RequestReply] Processing request: %d (Thread: %lu)\n", 
                   ctx->requestData, GetCurrentThreadId());
            
            // 요청 처리
            Sleep(200);
            int result = ctx->requestData * 2;
            
            // 응답 설정
            ctx->future->Set(result);
            
            delete ctx;
        }
    }

public:
    Future<int>* SendRequest(int requestData) {
        Future<int>* future = new Future<int>();
        
        RequestContext* ctx = new RequestContext();
        ctx->future = future;
        ctx->requestData = requestData;
        
        SendMessage(MessageType::REQUEST_STATE, this, ctx, sizeof(RequestContext*));
        
        return future;
    }
};
```

### Supervisor 패턴
Actor의 오류를 감지하고 복구하는 Supervisor 패턴을 구현한다.

```cpp
class SupervisorActor : public Actor {
private:
    struct ChildInfo {
        Actor* actor;
        bool isAlive;
        DWORD lastHeartbeat;
    };
    
    std::vector<ChildInfo> children;
    CRITICAL_SECTION childrenCs;
    HANDLE monitorThread;
    bool monitoring;

    static DWORD WINAPI MonitorProc(LPVOID param) {
        SupervisorActor* supervisor = static_cast<SupervisorActor*>(param);
        
        while (supervisor->monitoring) {
            supervisor->CheckChildren();
            Sleep(1000);
        }
        
        return 0;
    }

    void CheckChildren() {
        EnterCriticalSection(&childrenCs);
        
        DWORD currentTime = GetTickCount();
        
        for (auto& child : children) {
            if (child.isAlive && 
                (currentTime - child.lastHeartbeat) > 5000) {
                printf("[Supervisor] Child Actor unresponsive, restarting...\n");
                
                // 실제로는 Actor를 재시작하는 로직이 필요
                child.isAlive = false;
            }
        }
        
        LeaveCriticalSection(&childrenCs);
    }

protected:
    void HandleMessage(const Message& msg) override {
        // Supervisor의 메시지 처리 로직
    }

public:
    SupervisorActor() : monitoring(false), monitorThread(NULL) {
        InitializeCriticalSection(&childrenCs);
    }

    ~SupervisorActor() {
        StopMonitoring();
        DeleteCriticalSection(&childrenCs);
    }

    void AddChild(Actor* child) {
        EnterCriticalSection(&childrenCs);
        
        ChildInfo info;
        info.actor = child;
        info.isAlive = true;
        info.lastHeartbeat = GetTickCount();
        
        children.push_back(info);
        
        LeaveCriticalSection(&childrenCs);
    }

    void StartMonitoring() {
        if (!monitoring) {
            monitoring = true;
            monitorThread = CreateThread(NULL, 0, MonitorProc, this, 0, NULL);
        }
    }

    void StopMonitoring() {
        if (monitoring) {
            monitoring = false;
            if (monitorThread != NULL) {
                WaitForSingleObject(monitorThread, INFINITE);
                CloseHandle(monitorThread);
                monitorThread = NULL;
            }
        }
    }
};
```
    
  
## Actor Model의 성능 고려사항
Actor Model을 사용할 때는 몇 가지 성능 관련 고려사항이 있다.

첫째, 메시지 전달 오버헤드다. 메시지를 복사하고 큐에 넣는 과정은 비용이 든다. 큰 데이터를 전달할 때는 데이터 자체를 복사하는 대신 포인터를 전달하는 것이 효율적이다. 다만 이 경우 메모리 소유권 관리에 주의해야 한다.

둘째, Actor의 개수와 스레드 개수의 균형이다. 모든 Actor가 각자의 스레드를 가지면 컨텍스트 스위칭 오버헤드가 증가할 수 있다. 여러 Actor가 하나의 스레드 풀을 공유하는 것도 좋은 방법이다.

셋째, 메시지 큐의 크기 관리다. 메시지가 너무 빨리 쌓이면 메모리 문제가 발생할 수 있다. 백프레셔(Backpressure) 메커니즘을 구현하여 메시지 생성 속도를 조절할 수 있다.
 