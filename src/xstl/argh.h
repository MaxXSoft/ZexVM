#ifndef XSTL_ARGH_H_
#define XSTL_ARGH_H_

#include <map>
#include <string>
#include <functional>

namespace xstl {

using StrRef = const std::string &;

class ArgumentHandler {
public:
    using HandFunc = std::function<int(StrRef)>;

    ArgumentHandler() {}
    ~ArgumentHandler() {}

    bool ParseArguments(int argc, const char *argv[]) {
        auto ErrorHandler = [this](const std::string &str) {
            if (error_handler_) error_handler_(str);
            return false;
        };
        for (int i = 1; i < argc; ++i) {
            if (argv[i][0] == '-') {
                std::string arg_name(argv[i] + 1);
                HandFunc handler;
                if (argv[i][1] == '-') {
                    auto sub = arg_name.substr(1);
                    auto it = alias_dict_.find(sub);
                    if (it == alias_dict_.end()) return ErrorHandler(sub);
                    handler = it->second;
                }
                else {
                    auto it = handlers_.find(arg_name);
                    if (it == handlers_.end()) return ErrorHandler(arg_name);
                    handler = it->second;
                }
                handler((i + 1 >= argc || argv[++i][0] == '-') ? "" : argv[i]);
            }
            else {
                auto it = handlers_.find("");
                if (it == handlers_.end()) return ErrorHandler(argv[i]);
                if (it->second(argv[i])) return false;
            }
        }
        return true;
    }

    void AddHandler(const std::string &arg_name, HandFunc handler) { handlers_[arg_name] = handler; }
    void SetErrorHandler(HandFunc error_handler) { error_handler_ = error_handler; }
    void AddAlias(const std::string &alias, const std::string &source) { if (handlers_[source]) alias_dict_[alias] = handlers_[source]; }

private:
    std::map<std::string, HandFunc> handlers_;
    HandFunc error_handler_;
    std::map<std::string, HandFunc> alias_dict_;
};

} // namespace xstl

#endif // XSTL_ARGH_H_
