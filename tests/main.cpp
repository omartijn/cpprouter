#include <router.h>
#include <iostream>

struct greeter
{
    int object;

    using dto = router::dto<greeter>
        ::bind<&greeter::object>;
};

void greet_callback(greeter&& output)
{
    std::cout << "Hello, " << output.object << " world!" << std::endl;
}

class blaat
{
    public:
        blaat(std::string&& message) :
            _message{ std::move(message) }
        {}

        void callback1()
        {
            std::cout << "Callback 1: " << _message << std::endl;
        }
    private:
        std::string _message;
};

int main()
{
    router::table<void()>   table;
    blaat                   bla { "dum dum dum" };

    table.add<&greet_callback>("/hello/{\\w+}/world");
    table.add<&blaat::callback1>("/apfelstrudeln", &bla);

    table.route("/hello/10/world");
    table.route("/apfelstrudeln");

    return 0;
}
