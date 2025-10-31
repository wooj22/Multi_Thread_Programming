
/*
  [�����ٸ� �纸]
    std::this_thread::yield
    void yield() noexcept

    �����ٷ��� ���ٸ� �غ�� �����带 ���� �����ص� �ȴ١��� ��Ʈ�� �ش�. 
    ���� �������� ������ ��� �纸�ϰ� �ٽ� �����ٵ� �� �ִ�.
    ������ ���ؽ�Ʈ ����ġ�� �Ͼ��, ��� ���� ��������� ������ �ý��� ���¿� ���� �ٸ���. 
    �غ�� �ٸ� �����尡 ������ �ƹ� ��ȭ�� ���� �� �ִ�. 
    ������ ����� ������ ��ĥ �� �����Ƿ� ���� ��⿡���� ����� �ܰ� � ���������� ���� ���� ����. 
    ����ȭ���� `condition_variable`, ��������, `atomic::wait/notify` ���� ������Ƽ�갡 �� �����ϴ�.
*/


#include <atomic>
#include <thread>

std::atomic<bool> ready = false;        // automic������ ������ ���� ����

void wait_ready() {
    int spins = 0;

    // load() : ���� ������ ���� �д� �Լ�
    // memory_order_acquire : load�� �б� �庮�� ���� ������ ��� �޸� ������ �� load���� �ڷ� �и�
    while (!ready.load(std::memory_order_acquire)) {
        if (spins < 100) ++spins;           // ���� ª�� �ٻ� ��� ��
        else std::this_thread::yield();     // �ٸ� �����忡�� CPU �ڿ� �纸
    }
}