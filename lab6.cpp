#include <iostream>
#include <coroutine>
#include <exception>
#include <memory>
#include <random>


using namespace std;


class Dialog
{
public:
    struct promise_type;
    using handle = std::coroutine_handle<promise_type>;

    int ask(int question)
    {
        *coro.promise().question = question;
        coro.resume();
        return coro.promise().answer;
    }

    struct promise_type
    {
        int answer = 0;
        std::shared_ptr<int> question;

        
        promise_type(std::shared_ptr<int> question) : question(question) {}

        
        promise_type(std::shared_ptr<int> question, int) : question(question) {}

        void unhandled_exception() { std::terminate(); }

        auto initial_suspend() { return std::suspend_always{}; }

        auto final_suspend() noexcept { return std::suspend_always{}; }

        auto yield_value(int value)
        {
            answer = value;
            return std::suspend_always{};
        }

        auto get_return_object()
        {
            return Dialog{ handle::from_promise(*this) };
        }
        void return_void() {}
    };

    Dialog(Dialog const&) = delete;
    Dialog(Dialog&& other) noexcept : coro(other.coro) { other.coro = nullptr; }
    ~Dialog()
    {
        if (coro)
            coro.destroy();
    }

private:
    Dialog(handle h) : coro(h) {}
    handle coro;
};

Dialog create_thinker(shared_ptr<int> guess_input, int secret)
{
    int response = 0;

   
    while (true)
    {
        int guess = *guess_input; 

        if (guess < secret)
            response = -1; 
        else if (guess > secret)
            response = 1;  
        else
            response = 0;  

        co_yield response;
    }
}

Dialog create_guesser(shared_ptr<int> feedback_input)
{
    int min = 1;
    int max = 100;
    int current_guess = 50; 

    co_yield current_guess; 

    while (true)
    {
        int feedback = *feedback_input; 

       
        if (feedback == -1)
            min = current_guess + 1;
        else if (feedback == 1)
            max = current_guess - 1; 

        
        current_guess = min + (max - min) / 2;
        co_yield current_guess;
    }
}


int main()
{
    system("chcp 65001");

    cout << "Варіант 10" << endl;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1, 100);

   
    int secret = dist(gen);

    cout << "Загадане число: " << secret << endl;
    

    
    auto guess_channel = make_shared<int>(0);
    auto feedback_channel = make_shared<int>(0);

    
    Dialog thinker = create_thinker(guess_channel, secret);
    Dialog guesser = create_guesser(feedback_channel);

    int step = 1;
    int current_guess = 0;
    int response = -99;

    
    current_guess = guesser.ask(0);

    while (true)
    {
        cout << "Крок " << step << ": Вгадувач пропонує " << current_guess << "-";

        
        response = thinker.ask(current_guess);

        
        if (response == 0) {
            cout << "Правильно (0)" << endl;
            break;
        }
        else if (response == -1) {
            cout << "Число замале (-1)" << endl;
        }
        else {
            cout << "Число завелике (1)" << endl;
        }

       
        current_guess = guesser.ask(response);

        step++;
    }

    cout << endl;
    cout << "Гру завершено." << endl;

    
    if (step % 2 == 0)
        cout << "Переможець: загадувач (парний крок)." << endl;
    else
        cout << "Переможець: вгадувач (непарний крок)." << endl;

    return 0;
}