#include <phpcpp.h>
#include <iostream>
#include <stdlib.h>

Php::Value myFunction()
{
    if (rand() % 2 == 0)
    {
        return "string";
    }
    else
    {
        return 123;
    }
}

Php::Value sum_everything(Php::Parameters &params)
{
    int result = 0;
    for (auto &param : params)
    {
        result += param;
    }
    return result;
}

void example(Php::Parameters &params)
{
    Php::out << "I am a example" << std::endl;
}

void swap(Php::Parameters &params)
{
    Php::Value temp = params[0];
    params[0] = params[1];
    params[1] = temp;
}

void test_const(Php::Parameters &params)
{
    const char * name = params[0];
    Php::Value v;
    if (Php::defined(name))
    {
        v = Php::constant(name);
        Php::out << name << " is " << v << std::endl;
    }
    else {
        if (params.size() == 2)
        {
            v = params[1];
        }
        Php::define(name, v);
    }
}

void sample_ini()
{
    const char *name = "sample.name";
    const char *ver  = "sample.ver";
    Php::out << name << ": " << Php::ini_get(name) << std::endl;
    Php::out << ver << ": " << Php::ini_get(ver) << std::endl;
}

class Counter : public Php::Base
{
private:
    /**
     * The initial value
     * @var     int
     */
    int _value = 0;

public:
    Counter() = default;
    virtual ~Counter() = default;
    void __construct(Php::Parameters &params)
    {
        if (!params.empty())
        {
            _value = params[0];
        }
    }
    Php::Value increment(Php::Parameters &params)
    {
        return _value += params.empty() ? 1 : (int)params[0];
    }
    Php::Value decrement(Php::Parameters &params)
    {
        return _value -= params.empty() ? 1 : (int)params[0];
    }
    Php::Value value() const
    {
        return _value;
    }
};

class User : public Php::Base
{
private:
    std::string _name;
    std::string _email;

public:
    User() = default;
    ~User() = default;
    Php::Value __get(const Php::Value &name)
    {
        Php::out << "__get: " << name << std::endl;
        if (name == "name") return _name;
        if (name == "email") return _email;
        return Php::Base::__get(name);
    }
    void __set(const Php::Value &name, const Php::Value value)
    {
        Php::out << "__set: " << name << ", " << value << std::endl;
        if (name == "name")
        {
            _name = value.stringValue();
        }
        else if (name == "email")
        {
            std::string email = value.stringValue();
            if (email.find('@') == std::string::npos)
            {
                throw Php::Exception("Invalid Email address");
            }
            _email = email;
        }
        else
        {
            Php::Base::__set(name, value);
        }
    }
    bool __isset(const Php::Value &name)
    {
        Php::out << "__isset: " << name << std::endl;
        if (name == "name" || name == "email")
        {
            return true;
        }
        return Php::Base::__isset(name);
    }
    void __unset(const Php::Value &name)
    {
        Php::out << "__unset: " << name << std::endl;
        if (name == "name" || name == "email")
        {
            throw Php::Exception("Name and email address can not be removed");
        }
        Php::Base::__unset(name);
    }
    /*
    Php::Value __call(const Php::Value &name, Php::Parameters &params)
    {
        std::string retval = std::string("__call: ") + name.stringValue();
        for (auto &param : params)
        {
            retval += " " + param.stringValue();
        }
        return retval;
    }
    static Php::Value __callStatic(const char *name, Php::Parameters &params)
    {
        std::string retval = std::string("__callStatic: ") + name;
        for (auto &param : params)
        {
            retval += " " + param.stringValue();
        }
        return retval;
    }
    Php::Value __invoke(Php::Parameters &params)
    {
        std::string retval = std::string("__invoke: ");
        for (auto &param : params)
        {
            retval += " " + param.stringValue();
        }
        return retval;
    }
    Php::Value __toString()
    {
        return "User: " + _name;
    }
    /*/
};

/**
 *  tell the compiler that the get_module is a pure C function
 */
extern "C" {

    /**
     *  Function that is called by PHP right after the PHP process
     *  has started, and that returns an address of an internal PHP
     *  strucure with all the details and features of your extension
     *
     *  @return void*   a pointer to an address that is understood by PHP
     */
    PHPCPP_EXPORT void *get_module()
    {
        // static(!) Php::Extension object that should stay in memory
        // for the entire duration of the process (that's why it's static)
        static Php::Extension extension("sample", "1.0");

        // @todo    add your own functions, classes, namespaces to the extension
        extension.add<myFunction>("myFunction");
        extension.add<sum_everything>("sum_everything");
        extension.add<example>("example", {
                Php::ByVal("a", Php::Type::Numeric),
                Php::ByVal("b", Php::Type::Array),
                Php::ByVal("c", "ExampleClass")
                });
        extension.add<swap>("swap", {
                Php::ByRef("a", Php::Type::Numeric),
                Php::ByRef("a", Php::Type::Numeric)
                });

        Php::Interface valueInterface("ValueInterface");
        valueInterface.method("value");
        extension.add(std::move(valueInterface));

        Php::Class<Counter> counter("Counter");
        counter.implements(valueInterface);
        counter.method<&Counter::__construct>("__construct", {
            Php::ByVal("count", Php::Type::Numeric, false)
        });
        counter.method("__clone", Php::Private);
        counter.method<&Counter::increment>("increment", {
            Php::ByVal("delta", Php::Type::Numeric, false)
        });
        counter.method<&Counter::decrement>("decrement", {
            Php::ByVal("delta", Php::Type::Numeric, false)
        });
        counter.method<&Counter::value>("value");
        extension.add(std::move(counter));

        Php::Class<User> user("User");
        user.property("GUEST", "GUEST", Php::Const);
        user.constant("BRONZE", "BRONZE");
        user.add(Php::Constant("ADMIN", "ADMIN"));
        extension.add(std::move(user));

        extension.add(Php::Constant("SAMPLE_CONSTANT_BOOL", true));
        extension.add(Php::Constant("SAMPLE_CONSTANT_INT", 1));
        extension.add(Php::Constant("SAMPLE_CONSTANT_STR", "sample constant"));
        extension.add(Php::Constant("SAMPLE_CONSTANT_STR1", "sample constant"));
        extension.add(Php::Constant("SAMPLE_CONSTANT_STR2", "sample constant"));
        extension.add<test_const>("test_const");

        extension.add(Php::Ini("sample.name", "sample"));
        extension.add(Php::Ini("sample.ver", 1.0));
        extension.add<sample_ini>("sample_ini");

        // return the extension
        return extension;
    }
}
