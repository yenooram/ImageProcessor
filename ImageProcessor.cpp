// ImageProcessor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>

struct Color {
    int r;
    int g;
    int b;
};

struct Message {
    enum class Id {
        SET_STATE = 0,
        SET_INTENSITY,
        SET_COLOR,
        TERMINATE
    };
    union Params {
        Color rgb;
        uint8_t intensity;
        bool state;
    };

    Id id;
    Params params;
};

void SetState(const bool state) {
    std::cout << "SetState " << state << "\n";
}

void SetIntensity(const uint8_t intensity) {
    std::cout << "SetIntensity " << (unsigned int)intensity << "\n";
}

void SetColor(const Color color) {
    std::cout << "SetColor " << color.r << ", " << color.g << ", " << color.b << "\n";
}
std::condition_variable cv;
std::mutex m;
std::queue<Message> msg_queue;

Message GetMessage() {
    Message message;
    std::unique_lock<std::mutex> lock(m);

    cv.wait(lock, []() {return not msg_queue.empty(); });
    message = msg_queue.front();
    msg_queue.pop();

    return message;
}

void PutMessage(const Message& message) {
    std::lock_guard<std::mutex> lk(m);
    msg_queue.push(message);
    cv.notify_one();
}

void controller()
{
    Color white = { 255, 255, 255 };
    uint8_t intensity = 255;

    PutMessage(Message{ Message::Id::SET_STATE, true });      // switch on
    PutMessage(Message{ Message::Id::SET_INTENSITY, intensity });   // set the highest intensity
    PutMessage(Message{ Message::Id::SET_COLOR, white });     // set color to white
    PutMessage(Message{ Message::Id::SET_STATE, false });     // switch off
    for (int i = 0; i < 10; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        PutMessage(Message{ Message::Id::SET_STATE, false });
        std::cout << "Keep resetting false #" << i << std::endl;
    }
    PutMessage(Message{ Message::Id::TERMINATE });
}

void handler()
{
    while (true) {
        const Message msg = GetMessage();
        std::cout << "got msg " << static_cast<int>(msg.id) << std::endl;
        switch (msg.id) {
        case Message::Id::SET_STATE:
            SetState(msg.params.state);
            break;
        case Message::Id::SET_INTENSITY:
            SetIntensity(msg.params.intensity);
            break;
        case Message::Id::SET_COLOR:
            SetColor(msg.params.rgb);
            break;
        case Message::Id::TERMINATE:
            std::exit(0);
        default:
            break;
        }
    }
}

int main() {
    std::thread c(handler);
    std::thread p(controller);

    c.join();
    p.join();

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
